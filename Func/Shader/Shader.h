#pragma once
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include "../../Func/Window/Window.h"
#include "../../Func/Get/Get.h"
#include "../../Func/Barrier/Barrier.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxcompiler.lib")

/// <summary>
/// HLSLをコンパイルし、バイナリデータで返却する
/// </summary>
/// <param name="filePath"></param>
/// <param name="profile"></param>
/// <param name="dxcUtils"></param>
/// <param name="dxcCompiler"></param>
/// <param name="includeHandler"></param>
/// <param name="logStream"></param>
/// <returns></returns>
IDxcBlob* CompileShader
(
	// CompilerするShaderファイルへのパス
	const std::wstring& filePath,

	// Compilerに使用するProfile
	const wchar_t* profile,

	// 初期化で生成したもの3つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler,

	std::ostream& logStream
);