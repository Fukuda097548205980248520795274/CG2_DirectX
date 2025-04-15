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