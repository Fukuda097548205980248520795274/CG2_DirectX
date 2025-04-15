#pragma once
#include <cassert>
#include <string>
#include <format>
#include <filesystem>
#include <fstream>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include "../../Func/Window/Window.h"
#include "../../Func/String/String.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxcompiler.lib")

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
/// debugContorllerを取得する
/// </summary>
/// <returns></returns>
ID3D12Debug1* GetDebugController();