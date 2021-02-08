#pragma once
#include <filesystem>
#include <string>

#include <windows.h>
#include <shlobj.h>
#include <objbase.h>

#pragma comment(lib,"Shell32")
#pragma comment(lib,"Ole32")

std::wstring GetDesktopPath()
{
	wchar_t* p;
	if (S_OK != SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &p)) return std::wstring();
	std::wstring result = p;
	CoTaskMemFree(p);
	return result;
}