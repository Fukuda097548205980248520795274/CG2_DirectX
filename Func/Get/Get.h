#pragma once
#include <cassert>
#include <string>
#include <format>
#include <filesystem>
#include <fstream>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "../../Func/Window/Window.h"
#include "../../Func/String/String.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

/// <summary>
/// 使用するアダプタ（GPU）を取得する
/// </summary>
/// <param name="dxgiFactory">DXGIFactory</param>
/// <param name="logStream"></param>
/// <returns></returns>
IDXGIAdapter4* GetUseAdapter(IDXGIFactory7* dxgiFactory, std::ostream& logStream);

/// <summary>
/// Deviceを生成し取得する
/// </summary>
/// <param name="useAdapter">使用するアダプタ（GPU）</param>
/// <param name="logStream"></param>
/// <returns></returns>
ID3D12Device* GetDevice(IDXGIAdapter4* useAdapter, std::ostream& logStream);

/// <summary>
/// commandQueueを生成し取得する
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
ID3D12CommandQueue* GetCommandQueue(ID3D12Device* device);

/// <summary>
/// commandAllocatorを生成し取得する
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
ID3D12CommandAllocator* GetCommandAllocator(ID3D12Device* device);

/// <summary>
/// commandListを生成し取得する
/// </summary>
/// <param name="device"></param>
/// <param name="commandAllocator"></param>
/// <returns></returns>
ID3D12GraphicsCommandList* GetCommandList(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator);

/// <summary>
/// swapChainを生成する
/// </summary>
/// <param name="width">幅</param>
/// <param name="height">高さ</param>
/// <param name="hwnd">ウィンドウハンドル</param>
/// <param name="dxgiFactory"></param>
/// <param name="commandQueue"></param>
/// <returns></returns>
IDXGISwapChain4* GetSwapChain(int32_t width, int32_t height, HWND hwnd, IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue);