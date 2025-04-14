#include "Get.h"

/// <summary>
/// 使用するアダプタ（GPU）を取得する
/// </summary>
/// <param name="dxgiFactory">DXGIFactory</param>
/// <param name="logStream"></param>
/// <returns></returns>
IDXGIAdapter4* GetUseAdapter(IDXGIFactory7* dxgiFactory , std::ostream& logStream)
{
	// 使用するアダプタ用の変数
	IDXGIAdapter4* useAdapter = nullptr;

	// 良い順にアダプタを頼む
	for (UINT i = 0;
		dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		HRESULT hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		// ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
		{
			// 採用したアダプタの情報をログに出力
			Log(logStream, ConvertString(std::format(L"useAdapter : {} \n", adapterDesc.Description)));
			break;
		}

		// ソフトウェアアダプタの場合は見なかったことにする
		useAdapter = nullptr;
	}

	// アダプタが見つからなかったら起動しないようにする
	assert(useAdapter != nullptr);

	return useAdapter;
}

/// <summary>
/// Deviceを生成し取得する
/// </summary>
/// <param name="useAdapter">使用するアダプタ（GPU）</param>
/// <param name="logStream"></param>
/// <returns></returns>
ID3D12Device* GetDevice(IDXGIAdapter4* useAdapter, std::ostream& logStream)
{
	ID3D12Device* device = nullptr;

	// 機能レベル
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2 , D3D_FEATURE_LEVEL_12_1 , D3D_FEATURE_LEVEL_12_0
	};

	// ログ出力用の文字列
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };

	// 機能レベルが高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		// 採用したアダプタでデバイスを生成
		HRESULT hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));

		// 指定した機能レベルで生成できたか確認する
		if (SUCCEEDED(hr))
		{
			// 機能レベルをログに出力する
			Log(logStream, std::format("FeatureLevel : {} \n", featureLevelStrings[i]));

			break;
		}
	}

	assert(device != nullptr);

	return device;
}

/// <summary>
/// commandQueueを生成し取得する
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
ID3D12CommandQueue* GetCommandQueue(ID3D12Device* device)
{
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

	HRESULT hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(hr));

	return commandQueue;
}

/// <summary>
/// commandAllocatorを生成し取得する
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
ID3D12CommandAllocator* GetCommandAllocator(ID3D12Device* device)
{
	ID3D12CommandAllocator* commandAllocator = nullptr;

	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

	assert(SUCCEEDED(hr));

	return commandAllocator;
}

/// <summary>
/// commandListを生成し取得する
/// </summary>
/// <param name="device"></param>
/// <param name="commandAllocator"></param>
/// <returns></returns>
ID3D12GraphicsCommandList* GetCommandList(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator)
{
	ID3D12GraphicsCommandList* commandList = nullptr;

	HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));

	assert(SUCCEEDED(hr));

	return commandList;
}

/// <summary>
/// swapChainを生成する
/// </summary>
/// <param name="width">幅</param>
/// <param name="height">高さ</param>
/// <param name="hwnd">ウィンドウハンドル</param>
/// <param name="dxgiFactory"></param>
/// <param name="commandQueue"></param>
/// <returns></returns>
IDXGISwapChain4* GetSwapChain(int32_t width, int32_t height,HWND hwnd, IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue)
{
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	// 画面の大きさ
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;

	// 色の形式
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// マルチサンプルしない
	swapChainDesc.SampleDesc.Count = 1;

	// 描画のターゲットとして利用する
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// ダブルバッファ
	swapChainDesc.BufferCount = 2;

	// モニタに移したら中身を破棄する
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;


	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));

	assert(SUCCEEDED(hr));

	return swapChain;
}

/// <summary>
/// 
/// </summary>
/// <param name="device"></param>
/// <param name="rootSignature"></param>
/// <param name="vertexShaderBlob"></param>
/// <param name="pixelShaderblob"></param>
/// <returns></returns>
ID3D12PipelineState* GetPipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature, IDxcBlob* vertexShaderBlob, IDxcBlob* pixelShaderblob)
{
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


	/*------------------
	    PSOを生成する
	------------------*/

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer() , vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderblob->GetBufferPointer() , pixelShaderblob->GetBufferSize() };
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
	HRESULT hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	return graphicsPipelineState;
}

/// <summary>
/// 
/// </summary>
/// <param name="device"></param>
/// <param name="sizeInBytes"></param>
/// <returns></returns>
ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes)
{
	// 頂点リソース用のヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};

	// uploadHeapを使う
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;


	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};

	// バッファリソース
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;

	// バッファの場合は、これらを位置にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	// バッファの場合は、これにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際に頂点リソースを作る
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));

	assert(SUCCEEDED(hr));

	return resource;
}