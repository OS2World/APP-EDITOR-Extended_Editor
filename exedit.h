#include "res.h"
#include "settings.h"
#include "toolbar.h"
#include "status.h"


// New Classes
#define WC_MAIN  "MainWindow"
#define WC_CHILD "ChildWindow"

//Identifiers
#define Id_MainWindow  1
#define Id_ChildWindow 2
#define Id_MLEWindow   3
#define Id_Toolbar     4
#define Id_WinList     5
#define Id_Statusbar   6

//Ini
#define APP_NAME "exedit"
#define SAVE_WINPOS "winsize"

//WinList
#define WinList_First    1500
#define WinList_MaxItems 15

//Window Procedures
MRESULT EXPENTRY MainWindow_Proc (HWND thisHWnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY FrameWindow_Proc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY ChildWindow_Proc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2);

MRESULT EXPENTRY Dlg_Settings_Proc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY D_Search_Proc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2);

// Search Dlg Struct and Messages

#define SM_FIND        WM_USER+1
#define SM_REPLACE     WM_USER+2
#define SM_REFRESHDATA WM_USER+3

typedef struct _TSearchDlg
{
 USHORT usSzStruct;
 char FText[50];
 BOOL CaseSens,Select;
 BOOL WholeWord;
 HWND hwnd;
 IPT Start,End;
 IPT SStart,SEnd;
 char Rtext[50];
 } TSearchDlg;

// Mle subclass data struct

typedef
struct _TMle
{
 USHORT usSzStruct;
 PFNWP OldProc;
 HWND hwndMenu,hwndMenuOwner;
 HWND hwndStatusbar;
 char StText[255];
} TMle;

// Main Frame subclass data struct

typedef
struct _TFMain
{
 USHORT usSzStruct;
 PFNWP OldProc;
 HWND hwndToolbar,hwndStatusbar;
}TFMain;


//Main Data Struct
typedef
struct _TMain
{
 USHORT usSzStruct;
 char Untitled[10];
 FILEDLG FileDlg;
 TSettings DefSettings;
 HWND hwndMleMenu,hwndStatusbar;
} TMain;

//Child Data Struct
typedef
struct _TChild
{
 USHORT usSzStruct;
 HWND hwndMLE;
 TMle *Mle;
 TMain *Main;
 char FileName[255];
 HWND hwndStatusbar;
 TSearchDlg SearchDlg;
} TChild;

//Misc Procedures
void RegisterChild(HAB hab);
HWND CreateChild(HWND hwnd,PSZ Title);
void FileOpen(TChild *Child);
MPARAM ChildSendMsg(HWND hwndMain,ULONG msg,MPARAM mp1,MPARAM mp2);
void FileSave (TChild *Child);
int ChildCount(HWND hwndMain);
void AllChildSendMsg(HWND hwndMain,ULONG msg,MPARAM mp1,MPARAM mp2);
void CItemsEnable(HWND hwnd,BOOL State);
void RefreshWinList (HWND hwndClient);
void UpdateSettings(HWND hwnd,TSettings *Set,BOOL all);
void MaxChilds(HWND hwndMain);
LONG Search(char *in,char *str,char cs,char w);

//Misc new messages

#define WM_CLOSEALL WM_USER+4
