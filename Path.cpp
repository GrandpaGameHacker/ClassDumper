#include "Path.h"

std::wstring GetDesktopPath()
{
	wchar_t* p;
	if (S_OK != SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &p)) return std::wstring();
	std::wstring result = p;
	CoTaskMemFree(p);
	return result;
}
