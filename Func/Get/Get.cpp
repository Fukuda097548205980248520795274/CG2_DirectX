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
