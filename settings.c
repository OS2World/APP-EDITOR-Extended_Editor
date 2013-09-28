#define INCL_PM

#include <math.h>
#include <string.h>
#include <os2.h>
#include "exedit.h"

//Settings Pages
SHORT CTable[]={CLR_NEUTRAL,CLR_WHITE,CLR_BLACK,CLR_BACKGROUND,CLR_BLUE,CLR_RED,CLR_PINK,
               CLR_GREEN,CLR_CYAN,CLR_YELLOW,CLR_DARKGRAY,CLR_DARKBLUE,CLR_DARKRED,
               CLR_DARKPINK,CLR_DARKGREEN,CLR_DARKCYAN,CLR_BROWN,CLR_PALEGRAY};

MRESULT EXPENTRY Dlg_SetPage1_Proc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2)
{
 TSet1 *Set1;
 Set1=WinQueryWindowPtr(hwnd,0);

 switch(msg)
 {
  case WM_INITDLG:
   Set1=PVOIDFROMMP(mp2);
   WinSetWindowPtr(hwnd,0,Set1);
   WinSendDlgItemMsg(hwnd,Sb_Tabs,SPBM_SETLIMITS,MPFROMLONG(255),MPFROMLONG(0));
   WinSendDlgItemMsg(hwnd,Sb_Tabs,SPBM_SETCURRENTVALUE,(MPARAM)Set1->SSb_Tabs,0);
   WinSendDlgItemMsg(hwnd,Cb_Wrap,BM_SETCHECK,(MPARAM)Set1->SCb_Wrap,0);
   WinSendDlgItemMsg(hwnd,Cb_ReadOnly,BM_SETCHECK,(MPARAM)Set1->SCb_ReadOnly,0);
  break;

  case WM_CONTROL:
  {
   switch (SHORT1FROMMP(mp1))
   {
    case Sb_Tabs:
    {
     ULONG Temp;
     WinSendDlgItemMsg(hwnd,Sb_Tabs,SPBM_QUERYVALUE,MPFROMP(&Temp),0);
     if (Temp) Set1->SSb_Tabs=Temp;
    }
    break;

    case Cb_Wrap:
    Set1->SCb_Wrap=(BOOL)WinSendDlgItemMsg(hwnd,Cb_Wrap,BM_QUERYCHECK,0,0);
    break;

    case Cb_ReadOnly:
    Set1->SCb_ReadOnly=(BOOL)WinSendDlgItemMsg(hwnd,Cb_ReadOnly,BM_QUERYCHECK,0,0);
    break;

    default: break;
   }
  }
  default: break;
 }
 return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

MRESULT EXPENTRY Dlg_SetPage2_Proc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2)
{
 TSet2 *Set2;
 Set2=WinQueryWindowPtr(hwnd,0);

 switch(msg)
 {
  case WM_INITDLG:
  {
   HPS hps;
   int k,i;
   static LONG CTable[]={CLR_WHITE,CLR_BLACK,CLR_BLUE,CLR_RED,CLR_PINK,
                          CLR_GREEN,CLR_CYAN,CLR_YELLOW,CLR_DARKGRAY,CLR_DARKBLUE,CLR_DARKRED,
                          CLR_DARKPINK,CLR_DARKGREEN,CLR_DARKCYAN,CLR_BROWN,CLR_PALEGRAY};

   Set2=PVOIDFROMMP(mp2);
   WinSetWindowPtr(hwnd,0,Set2);

   for (k=0;k<4;k++)
   for (i=0;i<4;i++)
   {
    WinSendDlgItemMsg(hwnd,Vs_Back,VM_SETITEM,MPFROM2SHORT(k+1,i+1),(MPARAM)(0,CTable[k*4+i]));
    WinSendDlgItemMsg(hwnd,Vs_Text,VM_SETITEM,MPFROM2SHORT(k+1,i+1),(MPARAM)(CTable[k*4+i]));
   }

   WinSendDlgItemMsg(hwnd,Vs_Back,VM_SELECTITEM,(MPARAM)Set2->Sel_Back,0);
   WinSendDlgItemMsg(hwnd,Vs_Text,VM_SELECTITEM,(MPARAM)Set2->Sel_Text,0);
  }
  break;

  case WM_PAINT:
  {
   HPS hps;
   SWP swp;
   RECTL rect1,rect2,rect3;

   WinDefDlgProc(hwnd,msg,mp1,mp2);

   WinQueryWindowPos(WinWindowFromID(hwnd,St_Prev),&swp);
   rect1.xLeft=rect2.xLeft=rect3.xLeft=swp.x+3;
   rect1.xRight=rect2.xRight=rect3.xRight=swp.x+swp.cx-3;
   rect1.yBottom=rect2.yBottom=swp.y+3;
   rect1.yTop=rect3.yTop=swp.y+swp.cy-20;
   rect2.yTop=rect3.yBottom=swp.y+(swp.cy-20)/2;

   hps=WinGetPS(hwnd);
   WinFillRect(hps,&rect1,Set2->SVs_Back);
   WinDrawText(hps,-1,"Normal",&rect3,Set2->SVs_Text,0,DT_CENTER|DT_VCENTER);
   GpiSetBackMix(hps,FM_XOR);
   GpiSetMix(hps,FM_NOTCOPYSRC);
   WinDrawText(hps,-1,"Selected",&rect2,Set2->SVs_Text,0,DT_CENTER|DT_VCENTER);
   WinReleasePS(hps);
  }
  return 0;

  case WM_CONTROL:
  {
   switch(SHORT1FROMMP(mp1))
   {
    case Vs_Back:
     Set2->Sel_Back=(ULONG)WinSendDlgItemMsg(hwnd,Vs_Back,VM_QUERYSELECTEDITEM,0,0);
     Set2->SVs_Back=(ULONG)WinSendDlgItemMsg(hwnd,Vs_Back,VM_QUERYITEM,(MPARAM)Set2->Sel_Back,0);
     WinSendMsg(hwnd,WM_PAINT,0,0);
    break;

    case Vs_Text:
     Set2->Sel_Text=(ULONG)WinSendDlgItemMsg(hwnd,Vs_Text,VM_QUERYSELECTEDITEM,0,0);
     Set2->SVs_Text=(ULONG)WinSendDlgItemMsg(hwnd,Vs_Text,VM_QUERYITEM,(MPARAM)Set2->Sel_Text,0);
     WinSendMsg(hwnd,WM_PAINT,0,0);
    break;


    default: break;
   }
  }

  case WM_COMMAND:
  switch(SHORT1FROMMP(mp1))
  {
   case Btn_Font:
   {
    static FONTDLG   FontDlg;
    CHAR      szCurrentFont[FACESIZE];

    FontDlg.cbSize         = sizeof(FONTDLG);
    FontDlg.fl             = FNTS_INITFROMFATTRS|FNTS_CENTER;
    FontDlg.pszFamilyname  = szCurrentFont;
    FontDlg.usFamilyBufLen = FACESIZE;
    FontDlg.clrFore        = CLR_NEUTRAL;
    FontDlg.clrBack        = CLR_DEFAULT;

    memcpy(&FontDlg.fAttrs,&Set2->Font,sizeof(FATTRS));
    FontDlg.pszTitle  = "Select Text Font";
    WinFontDlg(HWND_DESKTOP, hwnd, &FontDlg);
    if (FontDlg.lReturn == DID_OK)
     memcpy(&Set2->Font,&FontDlg.fAttrs,sizeof(FATTRS));
   }
   return 0;
  }

  default: break;
 }
 return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

void AddPages(TSettingsNB *SettingsNB)
{
 int PageId,i;
 TNBPage *Page;

 for (i=0;i<N_Pages;i++)
 {
  Page=SettingsNB->Pages[i];
  PageId=(int)WinSendMsg(SettingsNB->hwndNB,BKM_INSERTPAGE,0,MPFROM2SHORT(BKA_MAJOR|BKA_AUTOPAGESIZE|BKA_STATUSTEXTON,BKA_LAST));
  WinSendMsg(SettingsNB->hwndNB,BKM_SETTABTEXT,(MPARAM)PageId,Page->Tab);
  WinSendMsg(SettingsNB->hwndNB,BKM_SETSTATUSLINETEXT,(MPARAM)PageId,Page->Status);
  WinSendMsg(SettingsNB->hwndNB,BKM_SETPAGEWINDOWHWND,(MPARAM)PageId,(MPARAM)Page->hwndDlg);
 }
}

void SetDataPage(TNBPage *Page,int DialogId,PFNWP DialogProc,PSZ Status,PSZ Tab,PVOID Data)
{
 Page->hwndDlg=WinLoadDlg(HWND_DESKTOP,HWND_DESKTOP,DialogProc,0,DialogId,Data);
 Page->Tab=strdup(Tab);
 Page->Status=strdup(Status);
}

void FreeDataPages(TSettingsNB *SettingsNB)
{
 TNBPage *Page;
 int i;

 for (i=0;i<N_Pages;i++)
 {
  Page=SettingsNB->Pages[i];
  WinDestroyWindow(Page->hwndDlg);
  DosFreeMem(Page->Tab);
  DosFreeMem(Page->Status);
  DosFreeMem(Page);
 }
}

MRESULT EXPENTRY Dlg_Settings_Proc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2)
{
 TSettingsNB *SettingsNB;
 SettingsNB=WinQueryWindowPtr(hwnd,0);

 switch(msg)
 {
  case WM_INITDLG:
  {
   int i;

   SettingsNB=PVOIDFROMMP(mp2);
   WinSetWindowPtr(hwnd,0,SettingsNB);
   SettingsNB->usSzStruct=sizeof(TSettingsNB);
   SettingsNB->hwndNB=WinWindowFromID(hwnd,Stg_NoteBook);

   //Set Data and Add Page
   for (i=0;i<N_Pages;i++)
   SettingsNB->Pages[i]=(TNBPage*)malloc(sizeof(TNBPage));

   SetDataPage(SettingsNB->Pages[1],Id_Set2,Dlg_SetPage2_Proc,"Colors and font","Appareance",(PVOID)&SettingsNB->Settings.Set2);
   SetDataPage(SettingsNB->Pages[0],Id_Set1,Dlg_SetPage1_Proc,"Editing Options","Options",(PVOID)&SettingsNB->Settings.Set1);
   AddPages(SettingsNB);
   WinSetWindowPos(hwnd,HWND_BOTTOM,0,0,0,0,SWP_HIDE);
  }
  break;

  case WM_PAINT:
  {
   SWP swpClient;
   WinQueryWindowPos(hwnd,&swpClient);
   WinSetWindowPos(SettingsNB->hwndNB,HWND_TOP,3,36,swpClient.cx-6,swpClient.cy-59,
                    SWP_SIZE | SWP_MOVE);
   WinSetWindowPos(WinWindowFromID(hwnd,DID_OK),HWND_TOP,swpClient.cx/2-85,3,80,30,
                   SWP_SIZE | SWP_MOVE);
   WinSetWindowPos(WinWindowFromID(hwnd,DID_CANCEL),HWND_TOP,swpClient.cx/2+5,3,80,30,
                   SWP_SIZE | SWP_MOVE);
  }  break;

  case DID_OK:
   WinDismissDlg(hwnd,TRUE);
  return 0;

  case DID_CANCEL:
   WinDismissDlg(hwnd,FALSE);
  return 0;

  default:
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
 }
 return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


void UpdateSettings(HWND hwnd,TSettings *Set,BOOL all)
{
 TSet1 *Set1=&Set->Set1;
 TSet2 *Set2=&Set->Set2;

 if (all)
 {
  WinBroadcastMsg(hwnd,MLM_SETREADONLY,(MPARAM)Set1->SCb_ReadOnly,0,BMSG_SEND);
  WinBroadcastMsg(hwnd,MLM_SETWRAP,(MPARAM)Set1->SCb_Wrap,0,BMSG_SEND);
  WinBroadcastMsg(hwnd,MLM_SETTABSTOP,(MPARAM)Set1->SSb_Tabs,0,BMSG_SEND);
  WinBroadcastMsg(hwnd,MLM_SETBACKCOLOR,(MPARAM)Set2->SVs_Back,0,BMSG_SEND);
  WinBroadcastMsg(hwnd,MLM_SETTEXTCOLOR,(MPARAM)Set2->SVs_Text,0,BMSG_SEND);
  WinBroadcastMsg(hwnd,MLM_SETFONT,(MPARAM)&Set2->Font,0,BMSG_SEND);
 }
 else
 {
  WinSendMsg(hwnd,MLM_SETREADONLY,(MPARAM)Set1->SCb_ReadOnly,0);
  WinSendMsg(hwnd,MLM_SETWRAP,(MPARAM)Set1->SCb_Wrap,0);
  WinSendMsg(hwnd,MLM_SETTABSTOP,(MPARAM)Set1->SSb_Tabs,0);
  WinSendMsg(hwnd,MLM_SETBACKCOLOR,(MPARAM)Set2->SVs_Back,0);
  WinSendMsg(hwnd,MLM_SETTEXTCOLOR,(MPARAM)Set2->SVs_Text,0);
  WinSendMsg(hwnd,MLM_SETFONT,(MPARAM)&Set2->Font,0);
 }
}
