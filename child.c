
#define INCL_PM
#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <fcntl.h>
#include <io.h>
#include <os2.h>
#include "exedit.h"

void RegisterChild(HAB hab)
{
 WinRegisterClass(hab,WC_CHILD,ChildWindow_Proc,CS_SIZEREDRAW,sizeof(PVOID)*2);
}

void UpdateStatusLC(HWND hwndMle,TMle *Mle)
{
 LONG Line,Col,Pos;

 Mle->StText[0]=0;
 Pos=LONGFROMMR(WinSendMsg(hwndMle, MLM_QUERYSEL, (MPARAM)MLFQS_CURSORSEL,0L));
 Line=LONGFROMMR(WinSendMsg(hwndMle, MLM_LINEFROMCHAR, (MPARAM)Pos, 0L));
 Col=LONGFROMMR(WinSendMsg(hwndMle, MLM_CHARFROMLINE, (MPARAM)Line, 0L));
 Col=Pos-Col;
 sprintf(Mle->StText," Line: %-5d Col: %-5d",Line,Col);
 WinSendMsg(Mle->hwndStatusbar,SBM_SETPANELTEXT,MPFROM2SHORT(0,0),(MPARAM)Mle->StText);
}

MRESULT EXPENTRY NewMLEProc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2)
{
 TMle *Mle;
 Mle = (TMle*) WinQueryWindowULong( hwnd, QWL_USER );

 switch (msg)
 {

  case WM_CONTEXTMENU:
  {
   POINTL ptlMouse;

   WinQueryPointerPos(HWND_DESKTOP,&ptlMouse);

   WinPopupMenu(HWND_DESKTOP,Mle->hwndMenuOwner,Mle->hwndMenu,
                ptlMouse.x,ptlMouse.y,0,
                PU_HCONSTRAIN |
                PU_VCONSTRAIN |
                PU_MOUSEBUTTON1 |
                PU_KEYBOARD);
  }
  return 0;

  case WM_BUTTON1UP:
   UpdateStatusLC(hwnd,Mle);
  break;

  case WM_CHAR:
   if (SHORT1FROMMP(mp1)&KC_KEYUP)
   {
     UpdateStatusLC(hwnd,Mle);
     if (SHORT2FROMMP(mp2)==VK_INSERT)
     {
      if (WinQuerySysValue(HWND_DESKTOP,SV_INSERTMODE))
       WinSendMsg(Mle->hwndStatusbar,SBM_SETPANELTEXT,MPFROM2SHORT(1,0)," Insert");
      else WinSendMsg(Mle->hwndStatusbar,SBM_SETPANELTEXT,MPFROM2SHORT(1,0)," Overwrite");
     }
   }
  break;

  case WM_FOCUSCHANGE:
   if (!SHORT1FROMMP(mp2))
   {
    WinSendMsg(Mle->hwndStatusbar,SBM_SETPANELTEXT,MPFROM2SHORT(0,0),"");
    WinSendMsg(Mle->hwndStatusbar,SBM_SETPANELTEXT,MPFROM2SHORT(1,0),"");
   }
   else
   {
    UpdateStatusLC(hwnd,Mle);
    if (WinQuerySysValue(HWND_DESKTOP,SV_INSERTMODE))
     WinSendMsg(Mle->hwndStatusbar,SBM_SETPANELTEXT,MPFROM2SHORT(1,0)," Insert");
    else WinSendMsg(Mle->hwndStatusbar,SBM_SETPANELTEXT,MPFROM2SHORT(1,0)," Overwrite");
   }
  break;

  default:
  break;
 }
 return Mle->OldProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ChildWindow_Proc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2)
{
 TChild *Child;
 Child=WinQueryWindowPtr(hwnd,0);

 switch (msg)
 {
  case WM_CREATE:
  {
   HWND hwndMClient=WinQueryWindow(WinQueryWindow(hwnd,QW_PARENT),QW_PARENT);


   Child = (TChild*)malloc(sizeof(TChild));
   WinSetWindowPtr(hwnd,0,Child);
   Child->usSzStruct= sizeof(TChild);

   //Create and subclass MLE Window
   Child->hwndMLE=WinCreateWindow(hwnd,WC_MLE,NULL,MLS_BORDER|MLS_HSCROLL|MLS_VSCROLL|
                                  WS_VISIBLE,0,0,0,0,hwnd,HWND_TOP,Id_MLEWindow,0,0);
   Child->Main=WinQueryWindowPtr(hwndMClient,0);
   Child->hwndStatusbar= Child->Main->hwndStatusbar;
   Child->Mle=(TMle*)malloc(sizeof(TMle));
   Child->Mle->OldProc = WinSubclassWindow(Child->hwndMLE,NewMLEProc);
   Child->Mle->hwndMenu = Child->Main->hwndMleMenu;
   Child->Mle->hwndMenuOwner = hwndMClient;
   Child->Mle->hwndStatusbar = Child->hwndStatusbar;
   WinSetWindowULong(Child->hwndMLE,QWL_USER,(ULONG)Child->Mle);
   Child->SearchDlg.usSzStruct=sizeof(TSearchDlg);
   Child->SearchDlg.FText[0]=0;
   Child->SearchDlg.Rtext[0]=0;
   Child->SearchDlg.CaseSens=FALSE;
   Child->SearchDlg.Select=FALSE;
   Child->SearchDlg.WholeWord=FALSE;
   Child->SearchDlg.hwnd=hwnd;
  }
  break;

  case WM_SIZE:
  {
   SWP swpClient;
   WinQueryWindowPos(hwnd,&swpClient);
   WinSetWindowPos(Child->hwndMLE,HWND_TOP,0,0,swpClient.cx,swpClient.cy,
                    SWP_SIZE | SWP_MOVE);
  }
  break;

  case WM_COMMAND:
  {
   switch(SHORT1FROMMP(mp1))
   {
    case M_File_Open:
     FileOpen(Child);
    return 0;

    case M_File_Save:
    if (!Child->FileName[0])
     WinSendMsg(hwnd,WM_COMMAND,MPFROM2SHORT(M_File_SaveAs,0),0);
    else FileSave(Child);
    return 0;

    case M_File_SaveAs:
    {
     HWND hwndFrame=WinQueryWindow(hwnd, QW_PARENT);
     TMain *Main=Child->Main;

     Main->FileDlg.fl     = FDS_SAVEAS_DIALOG|FDS_CENTER;
     WinFileDlg(HWND_DESKTOP, hwnd, &Main->FileDlg);
     if (Main->FileDlg.lReturn == DID_OK)
     {
      strcpy(Child->FileName,Main->FileDlg.szFullFile);
      WinSetWindowText(hwndFrame,Child->FileName);
      FileSave(Child);
     }
    }
    return 0;

    case M_Edit_Undo:
     WinSendMsg(Child->hwndMLE,MLM_UNDO,0,0);
    return 0;

    case M_Edit_Cut:
     WinSendMsg(Child->hwndMLE,MLM_CUT,0,0);
    return 0;

    case M_Edit_Clear:
     WinSendMsg(Child->hwndMLE,MLM_CLEAR,0,0);
    return 0;

    case M_Edit_Copy:
     WinSendMsg(Child->hwndMLE,MLM_COPY,0,0);
    return 0;

    case M_Edit_Paste:
     WinSendMsg(Child->hwndMLE,MLM_PASTE,0,0);
    return 0;

    case M_Edit_LocSet:
    {
     TSettingsNB *SettingsNB;
     SettingsNB = (TSettingsNB*)malloc(sizeof(TSettingsNB));
     memcpy(&SettingsNB->Settings,&Child->Main->DefSettings,sizeof(TSettings));
     if (WinDlgBox(HWND_DESKTOP,hwnd,Dlg_Settings_Proc,NULLHANDLE,Id_Settings,(PVOID)SettingsNB)==TRUE)
      UpdateSettings(hwnd,&SettingsNB->Settings,FALSE);
     DosFreeMem(SettingsNB);
    }
    return 0;

    case M_Edit_Search:
     WinSetFocus(HWND_DESKTOP,Child->hwndMLE);
     WinDlgBox(HWND_DESKTOP,hwnd,D_Search_Proc,NULLHANDLE,DLG_SEARCH,&Child->SearchDlg);
    return 0;

    case M_File_Print:
    {
     if (WinMessageBox(HWND_DESKTOP,hwnd,"Do you want to print current file?",Child->FileName,
                       0,MB_NOICON|MB_YESNO|MB_QUERY|MB_MOVEABLE)==MBID_YES)
      {
      PVOID  Buffer;
      ULONG  amount, amountexp;
      MPARAM sel=0;
      IPT FirstChar=0,Lenght;
      int HPrint=open("\\DEV\\PRN",O_RDWR | O_BINARY | O_CREAT | O_TRUNC, 0666);

      Buffer=(char*)malloc(64000);
      sel=WinSendMsg(Child->hwndMLE,MLM_QUERYSEL,(MPARAM)MLFQS_MINMAXSEL,0);
      WinSendMsg(Child->hwndMLE,MLM_SETIMPORTEXPORT,Buffer,MPFROMLONG(64000));

      if(SHORT1FROMMP(sel)!=SHORT2FROMMP(sel))
      {
       FirstChar=SHORT1FROMMP(sel);
       Lenght=SHORT2FROMMP(sel)-FirstChar;
       amountexp=(ULONG)WinSendMsg(Child->hwndMLE, MLM_EXPORT, &FirstChar, &Lenght);
       write(HPrint,Buffer,amountexp);
      }
      else
      do {
          amount = 64000;
          amountexp = (ULONG) WinSendMsg(Child->hwndMLE, MLM_EXPORT, &FirstChar, &amount);
          write(HPrint,Buffer,amountexp);
         }
      while(amountexp != 0);
      close(HPrint);
     }
    }
    return 0;



    default:
    return WinDefWindowProc(hwnd,msg,mp1,mp2);
   }
  }

  case SM_FIND:
  {
   PVOID Buffer;
   ULONG  amount, amountexp;
   PCHAR p;
   IPT anchor,search;
   MPARAM sel;
   IPT FirstChar=0,Lenght;
   TSearchDlg* SearchDlg=(TSearchDlg*)mp1;

   Buffer=(char*)malloc(64000);

   if (!SearchDlg->SEnd)
   {
    sel=WinSendMsg(Child->hwndMLE,MLM_QUERYSEL,(MPARAM)MLFQS_MINMAXSEL,0);
    SearchDlg->SStart=SHORT1FROMMP(sel);
    SearchDlg->SEnd=SHORT2FROMMP(sel);
   }

   if (!SearchDlg->End)
   {
    if (!SearchDlg->Select)
    {
     SearchDlg->Start=0;
     SearchDlg->End=(IPT)WinSendMsg(Child->hwndMLE,MLM_QUERYTEXTLENGTH,0,0);
    }
    else
    {
     SearchDlg->Start=SearchDlg->SStart;
     SearchDlg->End=SearchDlg->SEnd;
    }
   }

   WinSendMsg(Child->hwndMLE,MLM_SETIMPORTEXPORT,Buffer,MPFROMLONG(64000));
   FirstChar=SearchDlg->Start;
   Lenght=SearchDlg->End-SearchDlg->Start;

   do
   {
    amountexp=(ULONG)WinSendMsg(Child->hwndMLE, MLM_EXPORT, &FirstChar, &Lenght);
    p=(PCHAR)Buffer;
    p[amountexp+1]=0;
    if ((search=Search(p,SearchDlg->FText,SearchDlg->CaseSens,SearchDlg->WholeWord))!=-1)
    {
     anchor=SearchDlg->Start+search;
     WinSendMsg(Child->hwndMLE,MLM_SETSEL,(MPARAM)anchor,(MPARAM)(anchor+strlen(SearchDlg->FText)));
     DosFreeMem(Buffer);
     return (MPARAM)(anchor+strlen(SearchDlg->FText));
    }
   }
   while(amountexp!=0);
   DosFreeMem(Buffer);
  }
  return (MPARAM)-1;

  case SM_REPLACE:
  {
   TSearchDlg *SearchDlg=(MPARAM)mp1;
   WinSendMsg(Child->hwndMLE,MLM_INSERT,SearchDlg->Rtext,0);
  }
  return 0;

  case WM_CLOSEALL:
  case WM_CLOSE:
  {
   HWND hwndFrame=WinQueryWindow(hwnd, QW_PARENT);
   HWND hwndClient=WinQueryWindow(hwndFrame, QW_PARENT);

   if (WinSendMsg(Child->hwndMLE,MLM_QUERYCHANGED,0,0))
   if (WinMessageBox(HWND_DESKTOP,hwnd,"Do you want to save changes?",Child->FileName,
                     0,MB_NOICON|MB_YESNO|MB_QUERY|MB_MOVEABLE)==MBID_YES)
   WinSendMsg(hwnd,WM_COMMAND,(MPARAM)M_File_Save,0);

   WinDestroyWindow(Child->hwndMLE);
   WinDestroyWindow(hwndFrame);
   DosFreeMem(Child->Mle);
   DosFreeMem(Child);
   if (!ChildCount(hwndClient)) CItemsEnable(hwndClient,FALSE);
   RefreshWinList(hwndClient);
  }
  break;

  default:
   if ((ULONG)msg>=0x01b0 && (ULONG)msg<=0x01df)
   WinSendMsg(Child->hwndMLE,msg,mp1,mp2);
  break;
 }
 return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


