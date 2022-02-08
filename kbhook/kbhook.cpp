//http://qsoft.cjb.net
//For any questions/comments/thing send an email to arkon@ragestorm.com
//Feel free to do with this anything you want!
//21/4/2001

#include <windows.h>

#pragma data_seg("SharedBlock")
HHOOK hhook = NULL;
unsigned long keystrokes = 0;
int pcount = 0;
#pragma data_seg()

HINSTANCE g_hInstance = NULL;

_declspec(dllexport) LRESULT CALLBACK KBHookProc(int Code, WPARAM wParam, LPARAM lParam)
{
 if (Code < 0) return(CallNextHookEx(hhook, Code, wParam, lParam));

 if (lParam & (1 << 31)) keystrokes++;

 return(CallNextHookEx(hhook, Code, wParam, lParam));
}

_declspec(dllexport) void SetKBHook()
{
 if (!hhook) hhook = SetWindowsHookEx(WH_KEYBOARD, KBHookProc, g_hInstance, 0);
}

_declspec(dllexport) unsigned long GetKeyStrokes()
{
 return(keystrokes);
}

_declspec(dllexport) void KillKBHook()
{
 if(hhook) UnhookWindowsHookEx(hhook);
}

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
 if (dwReason == DLL_PROCESS_ATTACH)
 {
  if(pcount == 0)
  {
   // Init
   g_hInstance = hInstance;
  }
  pcount++;
 }
 else if (dwReason == DLL_PROCESS_DETACH)
 {
  if(pcount == 1)
  {
   // Free
  }
  pcount--;
 }
 return(1);
}