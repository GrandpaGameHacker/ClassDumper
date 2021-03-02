#pragma once
#include "stdheaders.h"
constexpr WORD CONSTYLE_BOLD = (BACKGROUND_RED | BACKGROUND_INTENSITY);
constexpr WORD CONSTYLE_DEFAULT = (FOREGROUND_GREEN | FOREGROUND_INTENSITY);

class Console
{
	FILE *m_fpIn, *m_fpOut, *m_fpErr;
	HANDLE m_hStdin, m_hStdout;
	CONSOLE_SCREEN_BUFFER_INFO m_csbi;
public:
	Console();
	explicit Console(const std::string& title);
	~Console();

	void Init();
	void Free();

	void SetTitle(const std::string& str);
	void SetFont(const std::wstring& fontName, short size);
	void SetCtrlHandler(PHANDLER_ROUTINE HandlerRoutine, BOOL Add);
	void SetAttribute(WORD attribute);

	void ClearScreen();
	void Write(const std::string& str);
	void FWrite(const char* format, ...);
	void WriteBold(const std::string& str);
	void FWriteBold(const char* format, ...);

	void WaitInput();
};
