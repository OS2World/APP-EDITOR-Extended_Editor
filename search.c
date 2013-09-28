#define INCL_PM
#include <stdio.h>
#include <string.h>
#include <os2.h>
#include "exedit.h"

LONG Search(char *in,char *str,char cs,char w)
{
 LONG k=0;
 int (*cmp)(const char *s1,const char *s2,size_t maxlen);
 int len;

 if (cs) cmp=strncmp;
 else cmp=strnicmp;
 if (*in==0x0d) in++;
 len=strlen(str);

 if (w)
 {
  while (*in)
  {
   if (cmp(in,str,len)==0)
   {
    if (*(in-1)==0x0A||*(in-1)==0x20)
    if (*(in+len)==0x0D||*(in+len)==0x20)
    return k;
   }
   in++;
   k++;
   if (*in==0x0d) in++;
  }
 }
 else
 {
  while (*in)
  {
   if (cmp(in,str,len)==0)  return k;
   in++;
   k++;
   if (*in==0x0d) in++;
  }
 }
 return -1;
}

MRESULT EXPENTRY D_Search_Proc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2)
{
 TSearchDlg* SearchDlg;
 SearchDlg=WinQueryWindowPtr(hwnd,0);

 switch(msg)
 {
  case WM_INITDLG:
   WinSetWindowPtr(hwnd,0,PVOIDFROMMP(mp2));
   SearchDlg=PVOIDFROMMP(mp2);
   WinSendDlgItemMsg(hwnd,Case,BM_SETCHECK,(MPARAM)SearchDlg->CaseSens,0);
   WinSendDlgItemMsg(hwnd,Selected,BM_SETCHECK,(MPARAM)SearchDlg->Select,0);
   WinSendDlgItemMsg(hwnd,Whole,BM_SETCHECK,(MPARAM)SearchDlg->WholeWord,0);
   WinSetDlgItemText(hwnd,Text,SearchDlg->FText);
   WinSetDlgItemText(hwnd,RText,SearchDlg->Rtext);
   SearchDlg->End=0;
   SearchDlg->SEnd=0;
   WinEnableControl(hwnd,Replace,FALSE);
  return 0;

  case SM_REFRESHDATA:
   SearchDlg->CaseSens=SHORT1FROMMP(WinSendDlgItemMsg(hwnd,Case,BM_QUERYCHECK,0,0));
   SearchDlg->Select=SHORT1FROMMP(WinSendDlgItemMsg(hwnd,Selected,BM_QUERYCHECK,0,0));
   SearchDlg->WholeWord=SHORT1FROMMP(WinSendDlgItemMsg(hwnd,Whole,BM_QUERYCHECK,0,0));
   WinQueryDlgItemText(hwnd,Text,50,SearchDlg->FText);
   WinQueryDlgItemText(hwnd,RText,50,SearchDlg->Rtext);
  return 0;

  case WM_CONTROL:
   switch(SHORT1FROMMP(mp1))
   {
   case RText:
   case Text:
   WinEnableControl(hwnd,Replace,FALSE);
   SearchDlg->End=0;
   return WinDefDlgProc(hwnd,msg,mp1,mp2);
   case Whole:
   case Case:
   case Selected:
   if (SHORT2FROMMP(mp1)==BN_CLICKED)
   {
    WinEnableControl(hwnd,Replace,FALSE);
    SearchDlg->End=0;
   }
   return WinDefDlgProc(hwnd,msg,mp1,mp2);

   }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);

  case WM_COMMAND:
  {
   switch(SHORT1FROMMP(mp1))
   {

    case ReplaceAll:
     SearchDlg->End=0;
     while (WinSendMsg(hwnd,WM_COMMAND,MPFROM2SHORT(Find,0),(MPARAM)100))
     WinSendMsg(hwnd,WM_COMMAND,MPFROM2SHORT(Replace,0),0);
    return 0;

    case Replace:
    {
     WinSendMsg(hwnd,SM_REFRESHDATA,0,0);
     WinSendMsg(SearchDlg->hwnd,SM_REPLACE,(MPARAM)SearchDlg,0);
     WinEnableControl(hwnd,Replace,FALSE);
     SearchDlg->Start-=strlen(SearchDlg->FText)-strlen(SearchDlg->Rtext);
    }
    return 0;

    case Find:
    {
     IPT Start;

     WinSendMsg(hwnd,SM_REFRESHDATA,0,0);
     if (!SearchDlg->FText[0])
     {
      WinMessageBox(HWND_DESKTOP,hwnd,"Insert text to find.",
                "Search result",0,MB_NOICON|MB_OK|MB_INFORMATION|MB_MOVEABLE);
      return 0;
     }

     Start=(IPT)WinSendMsg(SearchDlg->hwnd,SM_FIND,(MPARAM)SearchDlg,0);
     if (Start!=-1)
     {
      SearchDlg->Start=Start;
      WinEnableControl(hwnd,Replace,TRUE);
      return (MPARAM)TRUE;
     }
     else
     {
      if ((ULONG)mp2!=100)
      WinMessageBox(HWND_DESKTOP,hwnd,"String not found.",
                "Search result",0,MB_NOICON|MB_OK|MB_INFORMATION|MB_MOVEABLE);
     }
    }
    return 0;

    case DID_OK:
     WinSendMsg(hwnd,SM_REFRESHDATA,0,0);
     WinDismissDlg(hwnd,TRUE);
    return 0;

    default:
    return WinDefDlgProc(hwnd,msg,mp1,mp2);;
   }
  }
  default:
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
 }
}


