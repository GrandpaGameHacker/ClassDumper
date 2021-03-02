#include "StringConversions.h"

std::string Utf8Encode(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), nullptr, 0, nullptr,
	                                      nullptr);
	std::string stringTo(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &stringTo[0], sizeNeeded, nullptr, nullptr);
	return stringTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring Utf8Decode(const std::string& str)
{
	if (str.empty()) return std::wstring();
	const int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), nullptr, 0);
	std::wstring wstringTo(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstringTo[0], sizeNeeded);
	return wstringTo;
}
