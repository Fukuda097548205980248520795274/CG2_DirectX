#pragma once
#include <Windows.h>
#include <strsafe.h>
#include <string>
#include <DbgHelp.h>
#include "../../externals/imgui/imgui.h"
#include "../../externals/imgui/imgui_impl_dx12.h"
#include "../../externals/imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#pragma comment(lib , "Dbghelp.lib")

/// <summary>
/// ウィンドウプロシージャ
/// </summary>
/// <param name="hwnd"></param>
/// <param name="msg"></param>
/// <param name="wparam"></param>
/// <param name="lparam"></param>
/// <returns></returns>
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

/// <summary>
/// 
/// </summary>
/// <param name="exception"></param>
/// <returns></returns>
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception)
{
	// 時刻を取得して、時刻を名前に入れたファイルを作成　Dumpsディレクトリ以下に出力
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	// processId（このexeのId）とクラッシュ（例外）の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();

	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;

	// Dumpを出力　MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);

	// 他に関連づけられているSEH例外ハンドラがあれば実行　通常はプロセスを終了する
	return EXCEPTION_EXECUTE_HANDLER;
}
