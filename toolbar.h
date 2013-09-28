#define WC_TOOL   "ToolBar"

#define TM_INSERTITEM  WM_USER+13
#define TM_HIDEITEM    WM_USER+14
#define TM_DRAWITEM    WM_USER+15

#define TIT_CXCY      27
#define TIT_FIRSTX    4
#define TIT_FIRSTY    3
#define TIT_DISTANCE  1
#define TIT_CXSEPARATOR  5

#define TIT_NORMAL    0
#define TIT_HIDE      1
#define TIT_PRESSED   2

#define TIT_SEPARATOR 1
#define TIT_BUTTON    2

typedef struct _TToolItem  TToolItem;
struct _TToolItem
{
 USHORT Id;
 HBITMAP hbm;
 POINTL pos;
 char Status;
 char BType;
 char *text;
 TToolItem *Next;
};


typedef
struct _TTool
{
 USHORT usSzStruct;
 RECTL rcl;
 TToolItem* Items;
 TToolItem *PressedItem,*OnItem;
 ULONG DarkShadow,MiddleShadow,LightShadow;
 HWND hTipFr, hTipCli;
 BOOL TipOn, OnTool;
} TTool;

void RegisterToolBar(HAB anHab);

MRESULT EXPENTRY WinToolProc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2);
void DrawItem(HPS hps,TToolItem *Item,TTool *Tool);

