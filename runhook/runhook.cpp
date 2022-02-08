//http://qsoft.cjb.net
//For any questions/comments/thing send an email to arkon@ragestorm.com
//Feel free to do with this anything you want!
//21/4/2001

#include <windows.h>
#include <stdio.h>
#include "resource.h"

HMODULE hKBHookDLL;

typedef void (*LPSetKBHook)(void);
LPSetKBHook SetKBHook = NULL;

typedef unsigned long (*LPGetKeyStrokes)(void);
LPGetKeyStrokes GetKeyStrokes = NULL;

typedef void (*LPKillKBHook)(void);
LPKillKBHook KillKBHook = NULL;

BOOL LoadHookDLL()
{
 hKBHookDLL = LoadLibrary("kbhook.dll");
 if (!hKBHookDLL) return(0);

 SetKBHook = (LPSetKBHook)GetProcAddress(hKBHookDLL, "SetKBHook");
 if (!SetKBHook) return(0);

 GetKeyStrokes = (LPGetKeyStrokes)GetProcAddress(hKBHookDLL, "GetKeyStrokes");
 if (!GetKeyStrokes) return(0);

 KillKBHook = (LPKillKBHook)GetProcAddress(hKBHookDLL, "KillKBHook");
 if (!KillKBHook) return(0);

 SetKBHook();

 return(1);
}

void FreeHookDLL()
{
 KillKBHook();
 FreeLibrary(hKBHookDLL);
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
 switch(msg)
 {
  case WM_INITDIALOG:
   if (!LoadHookDLL()) EndDialog(hDlg, 0);
   SetTimer(hDlg, 0, 10, NULL);
  break;
  case WM_COMMAND:
   switch(LOWORD(wParam))
   {
    case IDC_OK: EndDialog(hDlg, 0); break;
   }
  break;
  case WM_TIMER:
   switch(wParam)
   {
    case 0:
     HWND hTmp = GetDlgItem(hDlg, IDC_EDIT1);
     char str[50];
     sprintf(str, "%d", GetKeyStrokes());
     SetWindowText(hTmp, str);
    break;
   }
  break;
  case WM_DESTROY:
   FreeHookDLL();
  break;
 }
 return(0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
 DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc);
 return(1);
}
