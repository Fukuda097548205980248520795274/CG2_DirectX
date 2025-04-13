#include "Shader.h"

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
	const wchar_t*  profile,

	// 初期化で生成したもの3つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler,

	 std::ostream& logStream
)
{
	/*-----------------------
	    HLSLファイルを読む
	-----------------------*/

	// これからShaderをCompilerする旨をログに出す
	Log(logStream, ConvertString(std::format(L"Begin CompileShader , path : {} , profile : {} \n", filePath, profile)));

	// HLSLファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));

	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();

	// UTF8の文字コードであることを通知
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;


	/*-------------------
	    Compilerする
	-------------------*/

	LPCWSTR argments[] =
	{
		// コンパイル対象のHLSLファイル名
		filePath.c_str(),

		// エントリーポイントの指定
		L"-E" , L"main",

		// ShaderProfileの設定
		L"-T" , profile,

		// デバッグ用の情報を埋め込む
		L"-Zi" , L"-Qembed_debug",

		// 最適化を外しておく
		L"-Od" ,

		// メモリレイアウトは行優先
		L"-Zpr"
	};

	// 実際にShaderをCompilerする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile
	(
		// 読み込んだファイル
		&shaderSourceBuffer,

		// コンパイルオプション
		argments,

		// コンパイルオプションの数
		_countof(argments),

		// includeが含まれた諸々
		includeHandler,

		// コンパイル結果
		IID_PPV_ARGS(&shaderResult)
	);

	assert(SUCCEEDED(hr));


	/*-----------------------------------
	    警告・エラーが出ていないか確認する
	-----------------------------------*/

	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		Log(logStream, shaderError->GetStringPointer());
		assert(false);
	}


	/*-------------------------------
	    Compiler結果を受け取って返す
	-------------------------------*/

	// compile結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	// 成功したログを出す
	Log(logStream, ConvertString(std::format(L"Compile Succeeded, path : {} , profile : {} \n", filePath, profile)));

	// 使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();

	// 実行用バイナリを返却
	return shaderBlob;
}