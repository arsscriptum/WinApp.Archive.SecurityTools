#include "console.h"

void WriteConsoleString(const char* str, StrTypeT t)
{
	DWORD BytesWritten = 0;
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	switch (t)
	{
	case CON_STRING:
  SetConsoleTextAttribute(hStdout, CONSOLE_ATTR);
		break;
	case STDOUT_STRING:
		SetConsoleTextAttribute(hStdout, STDOUT_ATTR);
		break;
	case STDERR_STRING:
		SetConsoleTextAttribute(hStdout, STDERR_ATTR);
		break;
	}

	WriteFile(hStdout, str, (DWORD)strlen(str), &BytesWritten, NULL);
}