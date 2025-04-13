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
/// TransitionBarrierを張る
/// </summary>
/// <param name="resource"></param>
/// <param name="commandList"></param>
/// <param name="stateBefore"></param>
/// <param name="stateAfter"></param>
void TransitionBarrier(ID3D12Resource* resource, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);