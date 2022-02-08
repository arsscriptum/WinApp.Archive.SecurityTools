// Arkon 2005 Israel
// http://ragestorm.net

#include <windows.h>
#include <stdio.h>

void __declspec(dllexport) InitProc()
{
	MessageBox(NULL, "This is kewl! [InitProc]", "16 bits to 32 bits galaxy", MB_OK);
}

void __declspec(dllexport) MyRoutine()
{
	unsigned short ds = 0, si = 0, cx = 0;
	char* s = NULL;

	HMODULE h = LoadLibrary("NTVDM.EXE");
	if (h == INVALID_HANDLE_VALUE) {
		return ;
	}
	ds = ((unsigned short (__stdcall *)())GetProcAddress(h, "getDS"))(); // Segment:
	si = ((unsigned short (__stdcall *)())GetProcAddress(h, "getSI"))(); // Offset
	cx = ((unsigned short (__stdcall *)())GetProcAddress(h, "getCX"))(); // String Length

	s = ((char* (__stdcall *)(ULONG /*addr*/, ULONG /*size*/, char/*addressing type, seg:off - selector*/))GetProcAddress(h, "MGetVdmPointer"))((ULONG)((ds << 0x10) + si), cx, FALSE);
	MessageBox(NULL, s, "16 bits to 32 bits galaxy, hell yeah biatch", MB_OK);

	// Set result...
	((void (__stdcall *)(unsigned short))GetProcAddress(h, "setAX"))(0x2005);
	FreeLibrary(h);
}