#pragma once
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxcompiler.lib")

/// <summary>
/// commandQueueを生成し取得する
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
ID3D12CommandQueue* CreateCommandQueue(ID3D12Device* device);

/// <summary>
/// commandAllocatorを生成し取得する
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
ID3D12CommandAllocator* CreateCommandAllocator(ID3D12Device* device);

/// <summary>
/// commandListを生成し取得する
/// </summary>
/// <param name="device"></param>
/// <param name="commandAllocator"></param>
/// <returns></returns>
ID3D12GraphicsCommandList* CreateCommandList(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator);


/// <summary>
/// DescriptorHeapを作成する
/// </summary>
/// <param name="device"></param>
/// <param name="heapType"></param>
/// <param name="numDescriptors"></param>
/// <param name="shaderVisible"></param>
/// <returns></returns>
ID3D12DescriptorHeap* CreateDescriptorHeap
(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

/// <summary>
/// Resourceを作成する
/// </summary>
/// <param name="device"></param>
/// <param name="sizeInBytes"></param>
/// <returns></returns>
ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);