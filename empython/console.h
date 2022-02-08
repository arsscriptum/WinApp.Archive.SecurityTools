#pragma once

#include <windows.h>

enum StrTypeT {CON_STRING, STDOUT_STRING, STDERR_STRING};

// White for console:
#define CONSOLE_ATTR (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
// Blue for standard output:
#define STDOUT_ATTR (FOREGROUND_BLUE | FOREGROUND_INTENSITY)
// Red for standard error output:
#define STDERR_ATTR (FOREGROUND_RED | FOREGROUND_INTENSITY)

void WriteConsoleString(const char* str, StrTypeT t);