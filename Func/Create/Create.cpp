#include "Create.h"

/// <summary>
/// commandQueueを生成し取得する
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
ID3D12CommandQueue* CreateCommandQueue(ID3D12Device* device)
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
ID3D12CommandAllocator* CreateCommandAllocator(ID3D12Device* device)
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
ID3D12GraphicsCommandList* CreateCommandList(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator)
{
	ID3D12GraphicsCommandList* commandList = nullptr;

	HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));

	assert(SUCCEEDED(hr));

	return commandList;
}

/// <summary>
/// DescriptorHeapを作成する
/// </summary>
/// <param name="device"></param>
/// <param name="heapType"></param>
/// <param name="numDescriptors"></param>
/// <param name="shaderVisible"></param>
/// <returns></returns>
ID3D12DescriptorHeap* CreateDescriptorHeap
(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDeac{};

	// Heapの種類
	descriptorHeapDeac.Type = heapType;

	// ダブルバッファように2つ
	descriptorHeapDeac.NumDescriptors = numDescriptors;

	descriptorHeapDeac.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDeac, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}

/// <summary>
/// Resourceを作成する
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
