#include <Windows.h>
#define _USE_MATH_DEFINES
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include "Struct.h"
#include "./Func/ErrorStop/ErrorStop.h"
#include "./Func/Window/Window.h"
#include "./Func/Get/Get.h"
#include "./Func/Barrier/Barrier.h"
#include "./Func/Shader/Shader.h"
#include "./Func/Matrix/Matrix.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxcompiler.lib")

// Windowアプリでのエントリーポイント（main関数）
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	SetUnhandledExceptionFilter(ExportDump);

	/*---------------------------
	    ログをファイルに書き込む
	---------------------------*/

	// ログのディレクトリを用意
	std::filesystem::create_directory("Logs");

	// 現在時刻を取得
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	// ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> nowSeconds =
		std::chrono::time_point_cast<std::chrono::seconds>(now);

	// 日本時間（PCの設定時間）に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };

	// formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m&d_%H%M%S}",localTime);

	// 時刻を使ってファイル名を決定
	std::string logFilePath = std::string("Logs/") + dateString + ".log";

	// ファイルを作って書き込み準備
	std::ofstream logStream(logFilePath);


	/*----------------------------
	    ウィンドウクラスを登録する
	----------------------------*/

	WNDCLASS wc{};

	// ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;

	// ウィンドウクラス名
	wc.lpszClassName = L"CG2WindowClass";

	// インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);

	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// 登録する
	RegisterClass(&wc);


	/*---------------------------
	    ウィンドウサイズを決める
	---------------------------*/

	// クライアント領域のサイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0 , 0 , kClientWidth , kClientHeight };

	// クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);


	/*------------------------------
	    ウィンドウを生成して表示する
	------------------------------*/

	HWND hwnd = CreateWindow(

		// 利用するクラス名
		wc.lpszClassName,

		// タイトルバーの文字
		L"DirectXGame",

		// よく見るウィンドウスタイル
		WS_OVERLAPPEDWINDOW,

		// 表示X座標
		CW_USEDEFAULT,

		// 表示Y座標
		CW_USEDEFAULT,

		// ウィンドウの横幅
		wrc.right - wrc.left,

		// ウィンドウの縦幅
		wrc.bottom - wrc.top,

		// 親ウィンドウハンドル
		nullptr,

		// メニューハンドル
		nullptr,

		// インスタンスハンドル
		wc.hInstance,

		nullptr
	);

	// ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);

	
	// debugLayerを有効化する
	ID3D12Debug1* debugController = GetDebugController();


	/*-----------------------
	    DirectXを初期化する
	-----------------------*/

	// DXGIFactoryを生成する
	IDXGIFactory7* dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	// 使用するアダプタ（GPU）を決定する
	IDXGIAdapter4* useAdapter = GetUseAdapter(dxgiFactory , logStream);

	// ID3D12Deviceを生成する
	ID3D12Device* device = GetDevice(useAdapter, logStream);

	// 初期化完了!!
	Log(logStream, "Complete create D3D12Device!!! \n");

	// エラー、警告が出たら停止する
	ErrorWarningStop(device);


	/*------------------------------------
	    CPUからCPUに命令群を送る機構を作る
	------------------------------------*/

	// commandQueueを生成する
	ID3D12CommandQueue* commandQueue = GetCommandQueue(device);

	// commandAllocatorを生成する
	ID3D12CommandAllocator* commandAllocator = GetCommandAllocator(device);

	// commandListを生成する
	ID3D12GraphicsCommandList* commandList = GetCommandList(device, commandAllocator);


	/*---------------
	    画面を作る
	---------------*/

	// swapChainを生成する
	IDXGISwapChain4* swapChain = GetSwapChain(kClientWidth, kClientHeight, hwnd, dxgiFactory, commandQueue);


	/*-----------------------------
	    DescriptorHeapを生成する
	-----------------------------*/

	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDeac{};

	// レンダーターゲットビュー用
	rtvDescriptorHeapDeac.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	// ダブルバッファように2つ
	rtvDescriptorHeapDeac.NumDescriptors = 2;

	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDeac, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// SwapChainからResourceを引っ張ってくる
	ID3D12Resource* swapChainResources[2] = { nullptr };

	hr = swapChain->GetBuffer(0 , IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));

	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));


	/*--------------
	    RTVを作る
	--------------*/

	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	// 出力結果はSRGBに変換して書き込む
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 2dテクスチャとして読み込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	
	// RTVを2つ作るのでDescriptorを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);

	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);


	/*---------------------------
	    FenceとEventを生成する
	---------------------------*/

	// 初期値0でFenceを作る
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	// FenceのSignalを待つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);


	/*--------------------
	    DXCを初期化する
	--------------------*/

	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	// 現時点でincludeはしないが、includeに対応するための設定を行っておく
	IDxcIncludeHandler* includeHandler = nullptr;
	
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));



	/*----------------------------
	    rootSignatureを生成する
	----------------------------*/

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	D3D12_ROOT_PARAMETER rootParameter[2] = {};

	// CBV PixelShader b0 
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[0].Descriptor.ShaderRegister = 0;

	// CBV VertexShader b0
	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[1].Descriptor.ShaderRegister = 0;

	descriptionRootSignature.pParameters = rootParameter;
	descriptionRootSignature.NumParameters = _countof(rootParameter);


	// シリアライズにしてバイナリにする
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Log(logStream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成する
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));


	/*----------------------------
		inputLayoutの設定を行う
	----------------------------*/

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};

	// float4 POSITION 0
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);


	/*---------------------------
		BlendStateの設定を行う
	---------------------------*/

	D3D12_BLEND_DESC blendDesc{};

	// 全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


	/*--------------------------------
		RasterizerStateの設定を行う
	--------------------------------*/

	D3D12_RASTERIZER_DESC rasterizerDesc{};

	// 裏面（時計回りを表示しない）
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;

	// 三角形の内部を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;


	/*---------------------------
	    Shaderをコンパイルする
	---------------------------*/

	// VertexShader
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler, logStream);
	assert(vertexShaderBlob != nullptr);

	// PixelShader
	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler, logStream);
	assert(pixelShaderBlob != nullptr);



	/*------------------
		PSOを生成する
	------------------*/

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer() , vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer() , pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 利用するトポロジ（形状）のタイプ
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 実際に生成
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));


	/*------------------------
	    ViewportとScissor
	------------------------*/

	// ビューポート
	D3D12_VIEWPORT viewport{};
	viewport.Width = kClientWidth;
	viewport.Height = kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// シザー矩形
	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;



	/*----------------
	    変数を作る
	----------------*/

	// Transform
	Transform3D transform;
	transform.scale = { 1.0f , 1.0f , 1.0f };
	transform.rotate = { 0.0f , 0.0f , 0.0f };
	transform.translate = { 0.0f , 0.0f , 0.0f };

	// カメラ用のTransform
	Transform3D cameraTransform;
	cameraTransform.scale = { 1.0f , 1.0f , 1.0f };
	cameraTransform.rotate = { 0.0f , 0.0f , 0.0f };
	cameraTransform.translate = { 0.0f , 0.0f , -5.0f };



	/*-----------------
	    メインループ
	-----------------*/

	MSG msg{};

	// ウィンドウのxボタンを押されるまでループ
	while (msg.message != WM_QUIT)
	{
		// Windowにメッセージが来たら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// ゲーム処理


			//  BackBufferを選ぶ（インデックスで見つける）
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

			// BackBufferを 表示 -> 描画 に変更する
			TransitionBarrier(swapChainResources[backBufferIndex], commandList, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

			// 描画先（BackBuffer）のRTVを設定する
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);

			// 指定した色で画面全体をクリアする
			float clearColor[] = { 0.1f , 0.25f , 0.5f , 1.0f };
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);


			///
			/// ↓ 更新処理ここから
			/// 

			// Y軸に回転させる
			transform.rotate.y += 0.03f;

			// 一周したら角度を初期化する
			if (transform.rotate.y >= float(M_PI) * 2)
			{
				transform.rotate.y = 0.0f;
			}

			// 座標変換を行う
			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 viewMatrix = MakeInverseMatrix(MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate));
			Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, static_cast<float>(kClientWidth) / static_cast<float>(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Multiply(Multiply(worldMatrix, viewMatrix), projectionMatrix);



			///
			/// ↑ 更新処理ここまで
			/// 
			
			///
			/// ↓ 描画処理ここから
			/// 


			// viewportを設定
			commandList->RSSetViewports(1,&viewport);

			// scissorを設定
			commandList->RSSetScissorRects(1, &scissorRect);
			
			// rootSignature
			commandList->SetGraphicsRootSignature(rootSignature);

			// PSO
			commandList->SetPipelineState(graphicsPipelineState);


			// 頂点
			ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(Vector4) * 3);

			D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
			vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
			vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
			vertexBufferView.StrideInBytes = sizeof(Vector4);

			Vector4* vertexData = nullptr;
			vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
			vertexData[0] = { -0.5f , -0.5f , 0.0f , 1.0f };
			vertexData[1] = { 0.0f , 0.5f , 0.0f , 1.0f };
			vertexData[2] = { 0.5f , -0.5f , 0.0f , 1.0f };


			// マテリアル
			ID3D12Resource* materialResource = CreateBufferResource(device, sizeof(Vector4));

			Vector4* materialData = nullptr;
			materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
			*materialData = { 1.0f , 0.0f ,0.0 , 1.0f };


			// 座標変換
			ID3D12Resource* wvpResource = CreateBufferResource(device, sizeof(Matrix4x4));

			Matrix4x4* wvpData = nullptr;
			wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
			*wvpData = worldViewProjectionMatrix;


			// VBVを設定
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

			// 形状
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// CBVのマテリアルを設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

			// CBVの座標変換を設定
			commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

			// 描画
			commandList->DrawInstanced(3, 1, 0, 0);


			///
			/// ↑ 描画処理ここまで
			/// 


			// BackBufferを 描画 -> 表示 に変更する
			TransitionBarrier(swapChainResources[backBufferIndex], commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			// commandListの内容を確定させる　全てのコマンドを積んでからCloseする
			hr = commandList->Close();
			assert(SUCCEEDED(hr));


			// GPUにcommandListの実行を行わせる
			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);

			// GPUとOSに画面の交換を行うよう通知する
			swapChain->Present(1, 0);

			// Fenceの値を更新
			fenceValue++;

			// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
			commandQueue->Signal(fence, fenceValue);

			// Fenceの値が指定したSignal値にたどり着いているかを確認する
			if (fence->GetCompletedValue() < fenceValue)
			{
				// 指定したSignal値にたどり着いていないので、たどり着くまで待つようにEventを設定する
				fence->SetEventOnCompletion(fenceValue, fenceEvent);

				// イベントを待つ
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			// 次のフレーム用のcommandListを準備
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator, nullptr);
			assert(SUCCEEDED(hr));


			
			// 解放処理
			wvpResource->Release();
			materialResource->Release();
			vertexResource->Release();
		}
	}


	/*-------------
	    解放処理
	--------------*/

	graphicsPipelineState->Release();
	signatureBlob->Release();
	if (errorBlob)
	{
		errorBlob->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();
	CloseHandle(fenceEvent);
	fence->Release();
	rtvDescriptorHeap->Release();
	swapChainResources[0]->Release();
	swapChainResources[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();
	debugController->Release();
	CloseWindow(hwnd);


	// リソースリークチェッカー
	ResourceLeakChecker();

	return 0;
}
