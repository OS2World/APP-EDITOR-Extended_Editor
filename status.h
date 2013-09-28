//Class Name
#define WC_STATUS "StatusBar"

//Id
#define Id_TextWindow 3000

//Dimensions
#define H_Status 25
#define Panels_Sep 3

//Messages
#define SBM_ADDPANEL     100
#define SBM_SETPANELTEXT 101
#define SBM_HIDEPANEL    102

//Types
#define SB_PERCENT  1
#define SB_FIX      2

//Structures

typedef struct _TPanel TPanel;

typedef
struct _TStatus
{
 USHORT usSzStruct;
 RECTL rcl;
 ULONG Dark_Shadow,Light_Shadow,Background;
 TPanel *Panels;
 SHORT PanelsCount,FixPanelsL,Perc;
}TStatus;

struct _TPanel
{
 SHORT cx;
 SHORT Type;
 SHORT id;
 RECTL rcl;
 BOOL  Visible;
 HWND hwndText;
 TPanel *Next;
};

//Procedures
void RegisterStatusBar(HAB anHab);
MRESULT EXPENTRY WinStatusProc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2);

