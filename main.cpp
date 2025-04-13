#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "./Func/Window/Window.h"
#include "./Func/Get/Get.h"
#include "./Func/Debug/Debug.h"
#include "./Func/Barrier/Barrier.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

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

	// エラー・警告は止まるようにする
	StopErrorAndWarning(device);


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
		}
	}


	return 0;
}
