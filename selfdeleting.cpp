//
// Self Deleting Executable
// Arkon, http://ragestorm.net
// 1st July, 2004
//
// Supposed to work on all Windows versions!
// Was checked on: 98/2000/XP/2003.
// The following code does NOT use any function that Win9x won't support (not even CreateRemoteThread)!
//
// GreeTingZ: Assaf Segal, tH@nks y0
//
// Feel free to use or doing with it anything you wish...
// Please give credit where due.
//

//
// Proof of concept for self deleting executable:
// Spawning a new process in suspended mode, using CreateProcess.
// Allocating a memory block in that process, using VirtualAllocEx.
// Getting remote thread context.
// Write a thread procedure to that memory block.
// Write some data structure for late usage by the thread procedure.
// Update remote thread stack, for using our data structure.
// Setting remote thread new context.
// Resuming remote thread.
// Voila...
//
// Note that this idea, specifically, could be implemented in other ways, I chose the simples one IMHO.
//

#include <windows.h>
#include <stdio.h>

struct _RemoteData {
  CHAR moduleName[MAX_PATH];
  DWORD processID;
  HANDLE (__stdcall *OpenProcessPtr)(DWORD, BOOL, DWORD);
  DWORD (__stdcall *WaitForSingleObjectPtr)(HANDLE, DWORD);
  BOOL (__stdcall *CloseHandlePtr)(HANDLE);
  BOOL (__stdcall *DeleteFilePtr)(LPCTSTR);
  void (__stdcall *SleepPtr)(DWORD);
  void (__stdcall *ExitProcessPtr)(UINT);
};

// Allocation size for the InjectedThreadProc in the host process.
// Well, you can use some hacks to calc the function size in runtime, I leave it for you.
#define MAX_CODE_SIZE 1024

DWORD WINAPI InjectedThreadProc(_RemoteData* rd)
{
	HANDLE hProcess = NULL;
	// Open the process we want to delete its file, the one who injected this code...
	if (hProcess = rd->OpenProcessPtr(PROCESS_ALL_ACCESS, FALSE, rd->processID)) {
		// Wait 'till it finishes executing.
		rd->WaitForSingleObjectPtr(hProcess, INFINITE);
		// Close handle to it.
		rd->CloseHandlePtr(hProcess);
	}
	// Delete process's executable file, sleep a bit and retry if required...?
	while (!rd->DeleteFilePtr(rd->moduleName)) rd->SleepPtr(1000);
	// Kill host also.
	rd->ExitProcessPtr(0);
	return 0;
}

int InjectCode()
{
	int rc = -1;
	SIZE_T dummy = 0;
	LPVOID codeAddr = NULL;
	HANDLE hProcess = NULL, hThread = NULL;

	_RemoteData rd = {0};
	CONTEXT cntxt = {0};

	STARTUPINFO sui = {0};
	PROCESS_INFORMATION pi = {0};
	sui.cb = sizeof(STARTUPINFO);
	// Create a host process that will run our code.
	if (!CreateProcess(NULL, "notepad.exe", NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED, NULL, NULL, &sui, &pi)) {
		rc = 1;
		goto cleanup;
	}
	// Luckily, PROCESS_INFORMATION returns to us precious handles...
	hProcess = pi.hProcess;
	hThread = pi.hThread;

	// Allocate some memory for our thread function to be copied to the host process.
	codeAddr = VirtualAllocEx(hProcess, NULL, MAX_CODE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE); // XRW
	if (!codeAddr) {
		rc = 2;
		goto cleanup;
	}

	// Get pointers to required functions.
	rd.OpenProcessPtr = OpenProcess;
	rd.WaitForSingleObjectPtr = WaitForSingleObject;
	rd.CloseHandlePtr = CloseHandle;
	rd.DeleteFilePtr = DeleteFile;
	rd.SleepPtr = Sleep;
	rd.ExitProcessPtr = ExitProcess;

	// Pass our own process ID so the remote thread can wait for us to terminate.
	rd.processID = GetCurrentProcessId();
	// Get our own executable path name so the remote thread can later delete us.
	GetModuleFileName(NULL, rd.moduleName, MAX_PATH);

	// Write the CODE into the process!
	if (!WriteProcessMemory(hProcess, codeAddr, (LPVOID)&InjectedThreadProc, MAX_CODE_SIZE, &dummy)) {
		rc = 3;
		goto cleanup;
	}
	if (dummy != MAX_CODE_SIZE) {
		rc = 4;
		goto cleanup;
	}

	// Get remote thread's context, so we can use it for our needs.
	cntxt.ContextFlags = CONTEXT_FULL; // We actually need ESP and EIP.
	if (!GetThreadContext(hThread, &cntxt)) {
		rc = 5;
		goto cleanup;
	}
	// Change the EIP to point to the thread procedure we copied earlier.
	// So when we assume running, our code will immediately start running.
	cntxt.Eip = (DWORD)codeAddr;

	// Every thread function gets one argument, this will be a pointer to _RemoteData structure.
	// Put the structure on the stack so the remote thread will be able to use it, make sure it's 4 bytes aligned!
	cntxt.Esp -= sizeof(_RemoteData); // Make some room.
	// Write it into the host process' stack space.
	if (!WriteProcessMemory(hProcess, (LPVOID)cntxt.Esp, (LPVOID)&rd, sizeof(_RemoteData), &dummy)) {
		rc = 6;
		goto cleanup;
	}
	if (dummy != sizeof(_RemoteData)) {
		rc = 7;
		goto cleanup;
	}

	// Now PUSH the ESP to point to the structure we copied above.
	// Just as pushing a pointer to rd so the remote thread will POP(or whatever usage of ESP) this pointer as its argument and use it...
	// cntxt.Esp points by now to our copied structure.
	// Making more room for the pointer itself on the stack and writing it.
	if (!WriteProcessMemory(hProcess, (LPVOID)(cntxt.Esp - sizeof(DWORD)), (LPVOID)&cntxt.Esp, sizeof(DWORD), &dummy)) {
		rc = 8;
		goto cleanup;
	}
	if (dummy != sizeof(DWORD)) {
		rc = 9;
		goto cleanup;
	}

	// Remember that ESP is always advanced by one DWORD after a push, something like atomic: mov [esp], dwVal; sub esp, 4.
	// Advance it twice, because of the last WriteProcessMemory.
	cntxt.Esp -= sizeof(DWORD)*2;

	// After we've set everything correctly in place, set the remote thread's context to the changed one.
	if (!SetThreadContext(hThread, &cntxt)) {
		rc = 10;
		goto cleanup;
	}
	// Now the thread will RESUME running from our copied code using the pointer to rd we copied to stack.
	if (!ResumeThread(hThread)) {
		rc = 11;
		goto cleanup;
	}

	rc = 0;
cleanup:
	// Oh no? (@_@) I'm looking at the error code!
	if (pi.hProcess) CloseHandle(pi.hProcess);
	if (pi.hThread) CloseHandle(pi.hThread);
	return rc;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCommandLine, int nCmdshow)
{
	if (InjectCode() != 0) {
		MessageBox(NULL, "Cannot inject code to host process!", "Self Deleting Executable!", MB_OK);
		return -1;
	}
	MessageBox(NULL, "Do whatever you want here,\r\nthread is blocked on us.", "Self Deleting Executable!", MB_OK);
	return 0;
}