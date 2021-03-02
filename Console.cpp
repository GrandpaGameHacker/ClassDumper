#include "Console.h"


Console::Console() :
	m_fpIn(nullptr),
	m_fpOut(nullptr),
	m_fpErr(nullptr),
	m_hStdin(nullptr),
	m_hStdout(nullptr),
	m_csbi()
{
	Init();
}

Console::Console(const std::string& title) :
	m_fpIn(nullptr),
	m_fpOut(nullptr),
	m_fpErr(nullptr),
	m_hStdin(nullptr),
	m_hStdout(nullptr),
	m_csbi()
{
	Init();
	SetTitle(title);
}

Console::~Console()
{
	Free();
}

void Console::Init()
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen_s(&m_fpIn, "CONIN$", "r", stdin);
	freopen_s(&m_fpOut, "CONOUT$", "w", stdout);
	freopen_s(&m_fpErr, "CONOUT$", "w", stderr);

	m_hStdin = GetStdHandle(STD_INPUT_HANDLE);
	m_hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(m_hStdout, &m_csbi);
	SetAttribute(CONSTYLE_DEFAULT);
}

void Console::Free()
{
	fclose(m_fpIn);
	fclose(m_fpOut);
	fclose(m_fpErr);
	CloseHandle(m_hStdin);
	CloseHandle(m_hStdout);
	FreeConsole();
}

void Console::SetTitle(const std::string& str)
{
	SetConsoleTitle(str.c_str());
}

void Console::SetFont(const std::wstring& fontName, short size)
{
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof cfi;
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;
	cfi.dwFontSize.Y = size;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, fontName.c_str());
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
}

void Console::SetCtrlHandler(PHANDLER_ROUTINE HandlerRoutine, BOOL Add)
{
	SetConsoleCtrlHandler(HandlerRoutine, Add);
}

void Console::SetAttribute(WORD attribute)
{
	SetConsoleTextAttribute(m_hStdout, attribute);
}

void Console::ClearScreen()
{
	SMALL_RECT scrollRect;
	COORD scrollTarget;
	CHAR_INFO fill;
	if (!GetConsoleScreenBufferInfo(m_hStdout, &m_csbi))
	{
		return;
	}

	scrollRect.Left = 0;
	scrollRect.Top = 0;
	scrollRect.Right = m_csbi.dwSize.X;
	scrollRect.Bottom = m_csbi.dwSize.Y;

	scrollTarget.X = 0;
	scrollTarget.Y = static_cast<SHORT>(0 - m_csbi.dwSize.Y);

	fill.Char.UnicodeChar = TEXT(' ');
	fill.Attributes = m_csbi.wAttributes;

	ScrollConsoleScreenBuffer(m_hStdout, &scrollRect, nullptr, scrollTarget, &fill);

	m_csbi.dwCursorPosition.X = 0;
	m_csbi.dwCursorPosition.Y = 0;

	SetConsoleCursorPosition(m_hStdout, m_csbi.dwCursorPosition);
}


void Console::Write(const std::string& str)
{
	std::cout << str << "\n";
}

void Console::FWrite(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void Console::WriteBold(const std::string& str)
{
	SetAttribute(CONSTYLE_BOLD);
	Write(str);
	SetAttribute(CONSTYLE_DEFAULT);
}

void Console::FWriteBold(const char* format, ...)
{
	SetAttribute(CONSTYLE_BOLD);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	SetAttribute(CONSTYLE_DEFAULT);
}

void Console::WaitInput()
{
	std::cout << "Press anything to continue...\n";
	getchar();
}
