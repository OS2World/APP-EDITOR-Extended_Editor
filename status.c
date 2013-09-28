#define INCL_PM
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <os2.h>
#include "exedit.h"

void RegisterStatusBar(HAB anHab)
{
 WinRegisterClass(anHab,WC_STATUS,&WinStatusProc,CS_SIZEREDRAW,4);
}

void DrawShadowedRect(TStatus* Status,HPS hps,PRECTL rect,BOOL Raised)
{
 POINTL pts[2];

 if (!Raised) GpiSetColor(hps,Status->Dark_Shadow);
 else GpiSetColor(hps,Status->Light_Shadow);
 GpiMove(hps,(PPOINTL)rect);
 pts[0].x=rect->xLeft;
 pts[0].y=rect->yTop;
 pts[1].x=rect->xRight;
 pts[1].y=rect->yTop;
 GpiPolyLine(hps,2,pts);

 if (Raised) GpiSetColor(hps,Status->Dark_Shadow);
 else GpiSetColor(hps,Status->Light_Shadow);
 GpiMove(hps,(PPOINTL)rect);
 pts[0].x=rect->xRight;
 pts[0].y=rect->yBottom;
 pts[1].x=rect->xRight;
 pts[1].y=rect->yTop;
 GpiPolyLine(hps,2,pts);
}

void RclCalc(TStatus* Status)
{

 PRECTL rcl=&Status->rcl;
 TPanel *Panel=Status->Panels;
 ULONG Left=rcl->xLeft+Panels_Sep,
       Right,
       FreeL=(rcl->xRight-rcl->xLeft)-(Status->FixPanelsL+(Status->PanelsCount+1)*Panels_Sep);

 while (Panel)
 {
  if (Panel->Visible)
  {
   Panel->rcl.xLeft=Left;
   if (Panel->Type==SB_FIX)
   Panel->rcl.xRight=Left+Panel->cx;
   else
   if (Panel->cx)
    Panel->rcl.xRight=Left+(Panel->cx*FreeL/Status->Perc);

   Panel->rcl.yTop=rcl->yTop-Panels_Sep;
   Panel->rcl.yBottom=rcl->yBottom+Panels_Sep;

   Left=Panel->rcl.xRight+Panels_Sep;
   if (!Panel->Next) Panel->rcl.xRight=rcl->xRight-Panels_Sep;
   WinSetWindowPos(Panel->hwndText,HWND_TOP,Panel->rcl.xLeft+1,Panel->rcl.yBottom+1,
                   Panel->rcl.xRight-Panel->rcl.xLeft-Panels_Sep,
                   Panel->rcl.yTop-Panel->rcl.yBottom-Panels_Sep,SWP_SIZE|SWP_MOVE|SWP_SHOW);
  }

  Panel=Panel->Next;
 }
}

TPanel* GetPanel(TPanel* First,SHORT Index)
{
 int i;
 for (i=0;i<Index;i++) First=First->Next;
 return First;
}

MRESULT EXPENTRY WinStatusProc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2)
{
 TStatus *Status;
 Status=WinQueryWindowPtr(hwnd,QWL_USER);

 switch (msg)
 {
  case WM_CREATE:
   Status=(TStatus*)malloc(sizeof(TStatus));
   WinSetWindowPtr(hwnd,QWL_USER,Status);

   Status->usSzStruct=sizeof(TStatus);
   Status->Dark_Shadow=(LONG)WinQuerySysColor(HWND_DESKTOP,SYSCLR_SHADOWHILITEBGND, 0L);
   Status->Light_Shadow=0x00FFFFFF;
   Status->Background=SYSCLR_DIALOGBACKGROUND;
   Status->PanelsCount=0;
   Status->FixPanelsL=0;
   Status->Perc=0;
  break;

  case WM_SIZE:
   WinQueryWindowRect(hwnd,&Status->rcl);
   Status->rcl.xRight--;
   Status->rcl.yTop--;
   RclCalc(Status);
  break;

  case WM_ERASEBACKGROUND:
  return (MPARAM)FALSE;

  case WM_PAINT :
  {
   HPS hps=WinGetPS(hwnd);
   TPanel* Panel=Status->Panels;

   GpiCreateLogColorTable(hps,0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
   WinFillRect(hps,&Status->rcl,Status->Background);
   DrawShadowedRect(Status,hps,&Status->rcl,TRUE);
   while (Panel)
   {
    if (Panel->Visible)
    {
     DrawShadowedRect(Status,hps,&Panel->rcl,FALSE);
     WinSetWindowPos(Panel->hwndText,HWND_TOP,0,0,0,0,SWP_SHOW);
     WinInvalidateRect(Panel->hwndText,NULL,FALSE);
    }
    else
     WinSetWindowPos(Panel->hwndText,HWND_TOP,0,0,0,0,SWP_HIDE);

    Panel=Panel->Next;
   }

   WinReleasePS(hps);
  }
  break;

 // mp1  1 cx  2 type mp2 1 visible 2 Id
  case SBM_ADDPANEL:
  {
   TPanel *Panel,*NewPanel;
   ULONG Back=WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0L),
         Fore=0;

   NewPanel=(TPanel*)malloc(sizeof(TPanel));
   if (!Status->Panels) Status->Panels=NewPanel;
   else
   {
    Panel=Status->Panels;
    while (Panel->Next) Panel=Panel->Next;
    Panel->Next=NewPanel;
   }

   NewPanel->cx=SHORT1FROMMP(mp1);
   NewPanel->Type=SHORT2FROMMP(mp1);
   NewPanel->Visible=(BOOL)SHORT1FROMMP(mp2);
   NewPanel->hwndText=WinCreateWindow(hwnd, WC_STATIC,"",
                                      WS_VISIBLE | SS_TEXT | DT_LEFT | DT_VCENTER,
                                      0,0,200,200,hwnd, HWND_TOP, SHORT2FROMMP(mp2),
                                      NULL, NULL);

   WinSetPresParam(NewPanel->hwndText, PP_FOREGROUNDCOLOR, 4L, (PVOID)&Fore);
   WinSetPresParam(NewPanel->hwndText, PP_BACKGROUNDCOLOR, 4L, (PVOID)&Back);
   WinSetPresParam(NewPanel->hwndText, PP_FONTNAMESIZE,sizeof("8.Helv.Bold"), (PVOID)"8.Helv.Bold");

   if (NewPanel->Type==SB_FIX) Status->FixPanelsL+=NewPanel->cx;
   else Status->Perc+=NewPanel->cx;
   Status->PanelsCount++;
  }
  break;

  // mp1  1 index 2 state
  case SBM_HIDEPANEL:
  {
   TPanel *Panel=GetPanel(Status->Panels,SHORT1FROMMP(mp1));
   BOOL Visible=!SHORT2FROMMP(mp1);

   if (Visible!=Panel->Visible)
   {
    Panel->Visible=Visible;
    if (Visible)
    {
     if (Panel->Type==SB_FIX) Status->FixPanelsL+=Panel->cx;
     else Status->Perc+=Panel->cx;
     Status->PanelsCount++;
    }
    else
    {
     if (Panel->Type==SB_FIX) Status->FixPanelsL-=Panel->cx;
     else Status->Perc-=Panel->cx;
     Status->PanelsCount--;
    }
   }
  }
  break;

  case SBM_SETPANELTEXT:
  {
   TPanel *Panel=GetPanel(Status->Panels,SHORT1FROMMP(mp1));
   WinSetWindowText(Panel->hwndText,(PSZ)mp2);
  }
  break;

  case WM_DESTROY:
  {
   TPanel *Panel=Status->Panels;
   TPanel *Temp;

   while (Panel)
   {
    WinDestroyWindow(Panel->hwndText);
    Temp=Panel;
    Panel=Panel->Next;
    DosFreeMem(Temp);
   }
   DosFreeMem(Status);
  }

  default:
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
 }
 return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


