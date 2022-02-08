// Arkon, 09 July 2004 RageStorm
// http://ragestorm.net
// Feel free to do with this whatever you want!
// Thanks goes to Sagiv Malihi for this great idea and help.

#include <windows.h>
#include <python.h>

// When PyShell is opened write: import newmodule   newmodule.test("is it working?")
PyObject* newmodule_test(PyObject* pSelf, PyObject* pArgs)
{
	char* str = NULL;
	if (!PyArg_ParseTuple(pArgs, "s", &str)) return NULL;
	MessageBox(NULL, str, "exported function", MB_OK);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef newmoduleMethods[] = {
 {"test", newmodule_test, METH_VARARGS, "Exported function testing."},
 {NULL, NULL, 0, NULL}
};

DWORD WINAPI PyShellProc(LPVOID p)
{
	/*
	The following code is what we run below, it will open PyShell using TkInter.

		---
	from Tkinter import Tk
	from idlelib.PyShell import PyShell, PyShellFileList, fixwordbreaks
	import idlelib.PyShell
	idlelib.PyShell.use_subprocess = False # Patch some variable in PyShell, so it knows we are not a subprocess.
	root = Tk()
	fixwordbreaks(root)
	flist = PyShellFileList(root)
	flist.pyshell = PyShell(flist)
	root.withdraw()
	flist.pyshell.begin()
	root.mainloop()
	root.destroy()
	*/

	// This code is blocking.
	PyRun_SimpleString("from Tkinter import Tk\nfrom idlelib.PyShell import PyShell, PyShellFileList, fixwordbreaks\nimport idlelib.PyShell\nidlelib.PyShell.use_subprocess = False\nroot = Tk()\nfixwordbreaks(root)\nflist = PyShellFileList(root)\nflist.pyshell = PyShell(flist)\nroot.withdraw()\nflist.pyshell.begin()\nroot.mainloop()\nroot.destroy()\n");
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCommandLine, int nShowCmd)
{
	Py_Initialize();

	char *argv[] = {"", NULL};
	PySys_SetArgv(1, argv);
	PyObject* pyRet = PyString_FromString(">>> ");
	PySys_SetObject("ps1", pyRet);
	Py_XDECREF(pyRet);
	// ps2 is not used in PyShell, to be honset it's more comfortable, think of it
	// that you want to copy/paste the code and you have "..." in every line then you have to
	// remove it manually.

	// Init our newmodule.
	Py_InitModule3("newmodule", newmoduleMethods, "This is a new module");

	// Create a thread, so our main thread is still active and you could run your real application.
	DWORD tid = 0;
	HANDLE hPyShellThread = CreateThread(NULL, 0, PyShellProc, NULL, 0, &tid);

	// You could do whatever you want, usually your message loop will be here.
	MessageBox(NULL, "I am blocking, pyshell is working from the other thread", "Main Thread", MB_OK);
	// Note that, if this thread is over, you still have to shutdown PyShell, otherwise your process will terminate in a bad manner.
	WaitForSingleObject(hPyShellThread, INFINITE);

	Py_Finalize();
	return 0;
}