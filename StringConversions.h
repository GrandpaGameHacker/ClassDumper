#pragma once
#include <string>
#include <windows.h>
std::string Utf8Encode(const std::wstring& wstr);
std::wstring Utf8Decode(const std::string& str);
