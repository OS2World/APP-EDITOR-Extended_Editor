#define INCL_PM

#include <string.h>
#include <os2.h>
#include "exedit.h"


int main (int argc, char *argv[])
{
 HAB hab;
 HINI Profile;
 ULONG SetSize=sizeof(TSettings);
 TMain *Main;
 TFMain *FMain;
 HMQ hmq;
 QMSG msg;
 ULONG ulCreate;
 HWND hwndFrame,hwndClient;
 HWND hwndToolbar,hwndStatusbar;

 hab=WinInitialize(0);
 hmq=WinCreateMsgQueue(hab,0);


 //Register and Create Main Window
 WinRegisterClass(hab,WC_MAIN,MainWindow_Proc,CS_SIZEREDRAW,sizeof(PVOID)*2);
 ulCreate=FCF_STANDARD & ~FCF_ACCELTABLE;
 hwndFrame=WinCreateStdWindow(HWND_DESKTOP,0,&ulCreate,WC_MAIN,
                              "ExEdit",0,NULLHANDLE,Id_MainWindow,&hwndClient);
 Main=WinQueryWindowPtr(hwndClient,0);

 // Load Settings from INI
 Profile=PrfOpenProfile(hab,"exedit.ini");
 PrfQueryProfileData(Profile,"exedit","Settings",&Main->DefSettings,&SetSize);
 // Register child Window
 RegisterChild(hab);

 //Create Toolbar and add items

 RegisterToolBar(hab);
 hwndToolbar=WinCreateWindow(hwndFrame,WC_TOOL, "ToolBar", WS_VISIBLE,
                0,0,0,0,hwndFrame,HWND_TOP,Id_Toolbar,NULL,NULL);

 WinSendMsg(hwndToolbar,TM_INSERTITEM,MPFROM2SHORT(M_File_New,M_File_New),(MPARAM)"Open a new file");
 WinSendMsg(hwndToolbar,TM_INSERTITEM,MPFROM2SHORT(M_File_Open,M_File_Open),(MPARAM)"Open a file");
 WinSendMsg(hwndToolbar,TM_INSERTITEM,MPFROM2SHORT(M_File_Save,M_File_Save),(MPARAM)"Save current file");
 WinSendMsg(hwndToolbar,TM_INSERTITEM,MPFROM2SHORT(M_File_Print,M_File_Print),(MPARAM)"Print current file");
 WinSendMsg(hwndToolbar,TM_INSERTITEM,(MPARAM)TIT_SEPARATOR,0);
 WinSendMsg(hwndToolbar,TM_INSERTITEM,MPFROM2SHORT(M_Edit_Copy,M_Edit_Copy),(MPARAM)"Copy selected text");
 WinSendMsg(hwndToolbar,TM_INSERTITEM,MPFROM2SHORT(M_Edit_Cut,M_Edit_Cut),(MPARAM)"Cut selected text");
 WinSendMsg(hwndToolbar,TM_INSERTITEM,MPFROM2SHORT(M_Edit_Paste,M_Edit_Paste),(MPARAM)"Paste clipboard text");
 WinSendMsg(hwndToolbar,TM_INSERTITEM,MPFROM2SHORT(M_Edit_Clear,M_Edit_Clear),(MPARAM)"Clear selected text");
 WinSendMsg(hwndToolbar,TM_INSERTITEM,MPFROM2SHORT(M_Edit_Search,M_Edit_Search),(MPARAM)"Search and replace");
 WinSendMsg(hwndToolbar,TM_INSERTITEM,(MPARAM)TIT_SEPARATOR,0);
 WinSendMsg(hwndToolbar,TM_INSERTITEM,MPFROM2SHORT(M_File_Settings,M_File_Settings),(MPARAM)"Change default settings");
 WinSendMsg(hwndToolbar,TM_INSERTITEM,MPFROM2SHORT(M_Help_About,M_Help_About),(MPARAM)"Help");

 //Create StatusBar
 RegisterStatusBar(hab);
 hwndStatusbar=WinCreateWindow(hwndFrame,WC_STATUS, "StatusBar", WS_VISIBLE,
                0,0,0,0,hwndFrame,HWND_TOP,Id_Statusbar,NULL,NULL);
 WinSendMsg(hwndStatusbar,SBM_ADDPANEL,MPFROM2SHORT(30,SB_PERCENT),MPFROM2SHORT(TRUE,39));
 WinSendMsg(hwndStatusbar,SBM_ADDPANEL,MPFROM2SHORT(80,SB_FIX),MPFROM2SHORT(TRUE,40));
 Main->hwndStatusbar=hwndStatusbar;

 //Subclass Frame window
 FMain=(TFMain*)malloc(sizeof(TFMain));
 FMain->usSzStruct=sizeof(TFMain);
 FMain->hwndToolbar=hwndToolbar;
 FMain->hwndStatusbar=hwndStatusbar;
 FMain->OldProc=WinSubclassWindow(hwndFrame,FrameWindow_Proc);
 WinSetWindowULong(hwndFrame,QWL_USER,(ULONG)FMain);

 //Size and Active Frame Window
 if (WinRestoreWindowPos(APP_NAME,SAVE_WINPOS,hwndFrame))
  WinSetWindowPos(hwndFrame,HWND_TOP,0,0,0,0,SWP_ACTIVATE | SWP_SHOW);
 else
  WinSetWindowPos(hwndFrame,NULLHANDLE,10,10,600,400,SWP_ACTIVATE|SWP_MOVE|SWP_SIZE|SWP_SHOW);

 //Disable Child Menu Items
 CItemsEnable(hwndClient,FALSE);

 //Command line
 if (argc>1)
  FileOpen(WinQueryWindowPtr(CreateChild(hwndClient,argv[1]),0));

 while (WinGetMsg(hab,&msg,NULLHANDLE,0,0))
 WinDispatchMsg(hab,&msg);

 PrfWriteProfileData(Profile,"exedit","Settings",&Main->DefSettings,SetSize);
 PrfCloseProfile(Profile);

 WinDestroyWindow(hwndFrame);
 DosFreeMem(FMain);
 WinDestroyWindow(hwndStatusbar);
 WinDestroyWindow(hwndToolbar);
 WinDestroyMsgQueue(hmq);
 WinTerminate(hab);
 return 0;
}

MRESULT EXPENTRY MainWindow_Proc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
 TMain *Main;
 Main=WinQueryWindowPtr(hwnd,0);

 switch (msg)
 {

  case WM_CREATE:
   Main = (TMain*)malloc(sizeof(TMain));
   WinSetWindowPtr(hwnd,0,Main);
   Main->usSzStruct= sizeof(TMain);
   strcpy(Main->Untitled,"Untitled0");
   memset(&Main->FileDlg, 0, sizeof(FILEDLG));
   Main->FileDlg.cbSize = sizeof(FILEDLG);
   Main->hwndMleMenu=WinLoadMenu(HWND_DESKTOP,NULLHANDLE,M_Mle);
  break;

  case DM_DRAGOVER:
  {
   PDRAGINFO pDragInfo=(PDRAGINFO)mp1;
   PDRAGITEM pDragItem;
   int x;

   DrgAccessDraginfo(pDragInfo);
   for(x=0;x<pDragInfo->cditem;x++)
   {
    pDragItem=DrgQueryDragitemPtr(pDragInfo,x);
    if (pDragItem->fsControl & DC_CONTAINER || pDragItem->fsControl & DC_REF)
    {
     DrgFreeDraginfo(pDragInfo);
     return MPFROM2SHORT(DOR_NEVERDROP,DO_UNKNOWN);
    }
   }
   DrgFreeDraginfo(pDragInfo);

  }
  return MPFROM2SHORT(DOR_DROP,DO_UNKNOWN);

  case DM_DROP:
  {
   PDRAGINFO pDragInfo;
   PDRAGITEM pDragItem;

   char dragname[CCHMAXPATH];
   int x;

   pDragInfo=(PDRAGINFO)mp1;
   DrgAccessDraginfo(pDragInfo);

   for (x=0;x<pDragInfo->cditem;x++)
   {
    pDragItem=DrgQueryDragitemPtr(pDragInfo,x);
    DrgQueryStrName(pDragItem->hstrContainerName,CCHMAXPATH,dragname);
    DrgQueryStrName(pDragItem->hstrSourceName,CCHMAXPATH,dragname+strlen(dragname));

    FileOpen(WinQueryWindowPtr(CreateChild(hwnd,dragname),0));
   }
   DrgFreeDraginfo(pDragInfo);

  }
  return 0;


  case WM_COMMAND:
  {
   switch (SHORT1FROMMP(mp1))
   {
    //Menu Commands

    case M_File_New:
    CreateChild(hwnd,NULL);
    break;

    case M_File_Open:
     Main->FileDlg.fl=FDS_OPEN_DIALOG|FDS_CENTER;
     WinFileDlg(HWND_DESKTOP,hwnd, &Main->FileDlg);
     if (Main->FileDlg.lReturn == DID_OK)
     WinSendMsg(CreateChild(hwnd,Main->FileDlg.szFullFile),WM_COMMAND,mp1,0);
    break;

    case M_File_Settings:
    {
     TSettingsNB *SettingsNB;
     SettingsNB = (TSettingsNB*)malloc(sizeof(TSettingsNB));
     memcpy(&SettingsNB->Settings,&Main->DefSettings,sizeof(TSettings));
     if(WinDlgBox(HWND_DESKTOP,hwnd,Dlg_Settings_Proc,NULLHANDLE,Id_Settings,(PVOID)SettingsNB)==TRUE)
     {
      memcpy(&Main->DefSettings,&SettingsNB->Settings,sizeof(TSettings));
      UpdateSettings(hwnd,&Main->DefSettings,TRUE);
     }
     DosFreeMem(SettingsNB);
    }
    break;

    case M_File_Exit:
     WinSendMsg(hwnd,WM_CLOSE,0,0);
    break;

    case M_Window_Cascade:
    {
     SWP Swp;
     HWND hwndChild;
     HENUM henum;
     int x=0,y=0;

     MaxChilds(hwnd);
     henum = WinBeginEnumWindows(hwnd);
     hwndChild=WinGetNextWindow(henum);
     WinGetMaxPosition(hwndChild,&Swp);
     do
     {
      WinSetWindowPos(WinQueryWindow(hwndChild,QW_PARENT),HWND_TOP,0,0,0,0,
                      SWP_RESTORE);
      WinSetWindowPos(hwndChild,HWND_TOP,Swp.x+x+4,Swp.y+4,Swp.cx-x-7,Swp.cy-y-7,
                      SWP_SIZE |SWP_MOVE| SWP_NOADJUST |SWP_ZORDER);
      WinSetActiveWindow(HWND_DESKTOP,hwndChild);
      x+=15;
      y+=15;
     }
     while((hwndChild=WinGetNextWindow(henum))!=NULLHANDLE);
     WinEndEnumWindows(henum);
    }
    return 0;

    case M_Window_VTile:
    {
     SWP Swp;
     HWND hwndChild;
     HENUM henum;
     int iy,y=0,count=0;

     MaxChilds(hwnd);
     count=ChildCount(hwnd);
     if (count)
     {

      henum=WinBeginEnumWindows(hwnd);
      hwndChild=WinGetNextWindow(henum);
      WinGetMaxPosition(hwndChild,&Swp);
      Swp.cx-=7;
      Swp.x+=4;
      Swp.cy-=7;
      Swp.y+=4;
      iy=Swp.cy/count;
      do
      {
       WinSetWindowPos(WinQueryWindow(hwndChild,QW_PARENT),HWND_TOP,0,0,0,0,
                      SWP_RESTORE);
       WinSetWindowPos(hwndChild,HWND_TOP,Swp.x,Swp.y+y,Swp.cx,iy,
                       SWP_SIZE |SWP_MOVE| SWP_NOADJUST |SWP_ZORDER);
       y+=iy;
      }
      while((hwndChild=WinGetNextWindow(henum))!=NULLHANDLE);
     }
    }
    return 0;

    case M_Window_HTile:
    {
     SWP Swp;
     HWND hwndChild;
     HENUM henum;
     int ix,x=0,count=0;

     MaxChilds(hwnd);
     count=ChildCount(hwnd);
     if (count)
     {

      henum = WinBeginEnumWindows(hwnd);
      hwndChild=WinGetNextWindow(henum);
      WinGetMaxPosition(hwndChild,&Swp);
      Swp.cx-=7;
      Swp.x+=4;
      Swp.cy-=7;
      Swp.y+=4;
      ix=Swp.cx/count;
      do
      {
       WinSetWindowPos(hwndChild,HWND_TOP,Swp.x+x,Swp.y,ix,Swp.cy,
                       SWP_SIZE |SWP_MOVE| SWP_NOADJUST |SWP_ZORDER);
       x+=ix;
      }
      while((hwndChild=WinGetNextWindow(henum))!=NULLHANDLE);
      WinEndEnumWindows(henum);
     }
    }
    return 0;

    case M_Window_Next:
    {
     HWND hwndActive;
     hwndActive=WinQueryActiveWindow(hwnd);
     WinSetActiveWindow(HWND_DESKTOP,WinQueryWindow(hwndActive,QW_NEXT));
     WinSetWindowPos(hwndActive,HWND_BOTTOM,0,0,0,0,SWP_ZORDER|SWP_RESTORE);
    }
    return 0;

    case M_Window_CloseAll:
     WinBroadcastMsg(hwnd,WM_CLOSEALL,0,0,BMSG_SEND);
    return 0;

    case M_Help_About:
     WinDlgBox(HWND_DESKTOP,hwnd,WinDefDlgProc,NULLHANDLE,DLG_ABOUT,0);
    return 0;

    default:
    if (SHORT1FROMMP(mp1)>=WinList_First)
    {
     MENUITEM item;
     WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),FID_MENU),MM_QUERYITEM,MPFROM2SHORT(SHORT1FROMMP(mp1),TRUE),MPFROMP(&item));
     WinSetActiveWindow(HWND_DESKTOP,item.hItem);
     break;
    }
    ChildSendMsg(hwnd,WM_COMMAND,mp1,0);
    return WinDefWindowProc(hwnd,msg,mp1,mp2);
    }
  }

  case WM_PAINT:
  {
   HPS hpsPaint;
   RECTL rclPaint;
   hpsPaint=WinBeginPaint(hwnd,NULLHANDLE,&rclPaint);
   WinFillRect(hpsPaint,&rclPaint,CLR_DARKGRAY);
   WinEndPaint(hpsPaint);
  }
  break;

  case WM_SAVEAPPLICATION:
   WinStoreWindowPos(APP_NAME,SAVE_WINPOS,WinQueryWindow(hwnd, QW_PARENT));
  break;

  case WM_CLOSE:
   WinSendMsg(hwnd,WM_COMMAND,MPFROM2SHORT(M_Window_CloseAll,0),0);
  break;

  case WM_DESTROY:
  WinDestroyWindow(Main->hwndMleMenu);
  break;

  default:
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
 }

return WinDefWindowProc(hwnd,msg,mp1,mp2);
}

MRESULT EXPENTRY FrameWindow_Proc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2)
{
 TFMain *FMain;
 FMain = (TFMain*) WinQueryWindowULong(hwnd,QWL_USER);

 switch (msg)
 {

  case WM_FORMATFRAME:
  {
   USHORT itemCount = SHORT1FROMMR( FMain->OldProc( hwnd, msg, mp1, mp2 ));
   USHORT Client=itemCount-1;
   USHORT Tool=itemCount;
   USHORT Status=itemCount+1;

   PSWP Pswp=PVOIDFROMMP(mp1);

   Pswp[Tool].fl = SWP_SIZE | SWP_MOVE;
   Pswp[Tool].cy = TIT_FIRSTY*2+TIT_CXCY;
   Pswp[Tool].cx = Pswp[Client].cx;
   Pswp[Tool].x = Pswp[Client].x;
   Pswp[Tool].y = Pswp[Client].y + Pswp[Client].cy - Pswp[Tool].cy;
   Pswp[Tool].hwndInsertBehind = HWND_TOP ;
   Pswp[Tool].hwnd = FMain->hwndToolbar;

   Pswp[Status].fl = SWP_SIZE | SWP_MOVE;
   Pswp[Status].cy = H_Status;
   Pswp[Status].cx = Pswp[Client].cx;
   Pswp[Status].x = Pswp[Client].x;
   Pswp[Status].y = Pswp[Client].y;
   Pswp[Status].hwndInsertBehind = HWND_TOP ;
   Pswp[Status].hwnd = FMain->hwndStatusbar;

   Pswp[Client].cy= Pswp[Client].cy - Pswp[Tool].cy-Pswp[Status].cy;
   Pswp[Client].y+= Pswp[Status].cy;
   return( MRFROMSHORT(itemCount+2));
  }

  default:
  return FMain->OldProc(hwnd,msg,mp1,mp2);
 }
}

