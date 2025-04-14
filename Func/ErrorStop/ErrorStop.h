#pragma once
#include <Windows.h>
#define _USE_MATH_DEFINES
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include "../../Func/String/String.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

/// <summary>
/// DebugControllerを取得し、DebugLayerを有効化する
/// </summary>
/// <returns></returns>
ID3D12Debug1* GetDebugController();

/// <summary>
/// 警告・エラーが発生したら停止させる
/// </summary>
/// <param name="device"></param>
void ErrorWarningStop(ID3D12Device* device);

/// <summary>
/// リソースリークチェッカー
/// </summary>
void ResourceLeakChecker();