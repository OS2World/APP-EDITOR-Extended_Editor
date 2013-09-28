#define N_Pages 2


typedef
struct _TSet2
{
 ULONG SVs_Back,SVs_Text,Sel_Back,Sel_Text;
 FATTRS Font;
} TSet2;

typedef
struct _TSet1
{
 BOOL SCb_Wrap;
 BOOL SCb_ReadOnly;
 ULONG  SSb_Tabs;
} TSet1;

typedef
struct _TSettings
{
 TSet2 Set2;
 TSet1 Set1;
} TSettings;

typedef
struct _TNBPage
{
 HWND hwndDlg;
 PSZ Status;
 PSZ Tab;
} TNBPage;
typedef TNBPage *PNBPage;

typedef
struct _TSettingsNB
{
 USHORT usSzStruct;
 HWND hwndNB;
 PNBPage Pages[N_Pages];
 TSettings Settings;
} TSettingsNB;

