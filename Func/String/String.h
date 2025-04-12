#pragma once
#include <Windows.h>
#include <string>
#include <filesystem>
#include <fstream>

/// <summary>
/// ログを表示する
/// </summary>
/// <param name="message">文字列</param>
void Log(std::ostream& os, const std::string& message);

/// <summary>
/// string -> wstring
/// </summary>
/// <param name="str">string</param>
/// <returns></returns>
std::wstring ConvertString(const std::string& str);

/// <summary>
/// wstring -> string
/// </summary>
/// <param name="str">wstring</param>
/// <returns></returns>
std::string ConvertString(const std::wstring& str);
