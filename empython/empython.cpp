// Arkon --- RageStorm (C) 2003
// http://ragestorm.net
// Feel free to do with this whatever you want!
// Hope you'll learn something from this code...

// In order to see how to use the "newmodule" module, execute execfile("text.py") in this console!
#include <windows.h>

#include <python.h>

#include "logger.h"
#include "newmodule.h"

const char* EMPYCAPTION = "Embedded Python";
const char* QUITCOMMAND = "quit\r\n";

const char* NORMALPROMPT = ">>>";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCommandLine, int nShowCmd)
{
	if (AllocConsole() == 0) {
		MessageBox(NULL, "Can't allocate a console window!", EMPYCAPTION, MB_OK);
		return 0;
	}
	Py_Initialize();
	PyObject* globals = PyDict_New();
	if (globals == NULL) {
		Py_Finalize();
		MessageBox(NULL, "Can't create globals dictionary!", EMPYCAPTION, MB_OK);
		return 0;
	}
	PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
	// If you want to import modules, they will have to be added to the builtins list...

	// Init a logger, so all output will be printed to our special console window.
	InitLogger();

	// Init our new extended module.
	InitNewModule();

	BOOL Done = FALSE, ret = FALSE;
	DWORD BytesRead = 0;
	char UserInput[1024];
	while (!Done) {
		ZeroMemory(UserInput, 1024);
		// Read user input line.
		WriteConsoleString(NORMALPROMPT, CON_STRING);
		ret = ReadFile(GetStdHandle(STD_INPUT_HANDLE), &UserInput, 1024, &BytesRead, NULL);
		if (!ret) break;

		// "quit" ?
		if (stricmp(UserInput, QUITCOMMAND) == 0) break;

		// Execute it(We have to chop \r\n to be \n, ReadFile(from Console) suffixes with \r\n...)
		// We assume that the user input string was less than 1024!!!!!
		// If you want to extend it, making it limitless,
		// you'll have to concatenate the UserInput to another buffer 'till you encounter \r\n.
		UserInput[strlen(UserInput)-1] = '\0'; // Exclude \n.
		UserInput[strlen(UserInput)-1] = '\n'; // Change \r to \n.

		// Let Python run it!
		
		//PyRun_SimpleString(UserInput);

		// If you'll use PyRun_SimpleString you won't be able to see what functions returns.
		// So what we have to do, is to use another complicated function:
		PyObject* pyRet = PyRun_String(UserInput, Py_single_input, globals, globals);
		// Any exceptions raised? Print and free(by the printing function) them.
		if (PyErr_Occurred()) PyErr_Print();
		Py_XDECREF(pyRet);

		// Well I leave you the support of multi-line expressions...
	}

	// Clean ups:
	Py_DECREF(globals);
	DestroyNewModule();
	Py_Finalize();
	return 0;
}