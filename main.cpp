#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "./Func/Window/Window.h"
#include "./Func/Get/Get.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

// Windowアプリでのエントリーポイント（main関数）
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	SetUnhandledExceptionFilter(ExportDump);

	/*---------------------------
	    ログをファイルに書き込む
	---------------------------*/

	// ログのディレクトリを用意
	std::filesystem::create_directory("Logs");

	// 現在時刻を取得
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	// ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> nowSeconds =
		std::chrono::time_point_cast<std::chrono::seconds>(now);

	// 日本時間（PCの設定時間）に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };

	// formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m&d_%H%M%S}",localTime);

	// 時刻を使ってファイル名を決定
	std::string logFilePath = std::string("Logs/") + dateString + ".log";

	// ファイルを作って書き込み準備
	std::ofstream logStream(logFilePath);



	/*----------------------------
	    ウィンドウクラスを登録する
	----------------------------*/

	WNDCLASS wc{};

	// ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;

	// ウィンドウクラス名
	wc.lpszClassName = L"CG2WindowClass";

	// インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);

	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// 登録する
	RegisterClass(&wc);


	/*---------------------------
	    ウィンドウサイズを決める
	---------------------------*/

	// クライアント領域のサイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0 , 0 , kClientWidth , kClientHeight };

	// クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);


	/*------------------------------
	    ウィンドウを生成して表示する
	------------------------------*/

	HWND hwnd = CreateWindow(

		// 利用するクラス名
		wc.lpszClassName,

		// タイトルバーの文字
		L"DirectXGame",

		// よく見るウィンドウスタイル
		WS_OVERLAPPEDWINDOW,

		// 表示X座標
		CW_USEDEFAULT,

		// 表示Y座標
		CW_USEDEFAULT,

		// ウィンドウの横幅
		wrc.right - wrc.left,

		// ウィンドウの縦幅
		wrc.bottom - wrc.top,

		// 親ウィンドウハンドル
		nullptr,

		// メニューハンドル
		nullptr,

		// インスタンスハンドル
		wc.hInstance,

		nullptr
	);

	// ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);



	/*-----------------------
	    DirectXを初期化する
	-----------------------*/

	// DXGIFactoryを生成する
	IDXGIFactory7* dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	// 使用するアダプタ（GPU）を決定する
	IDXGIAdapter4* useAdapter = GetUseAdapter(dxgiFactory , logStream);

	// ID3D12Deviceを生成する
	ID3D12Device* device = GetDevice(useAdapter, logStream);

	// 初期化完了!!
	Log(logStream, "Complete create D3D12Device!!! \n");



	/*-----------------
	    メインループ
	-----------------*/

	MSG msg{};

	// ウィンドウのxボタンを押されるまでループ
	while (msg.message != WM_QUIT)
	{
		// Windowにメッセージが来たら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// ゲーム処理



		}
	}


	return 0;
}
