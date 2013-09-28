#define INCL_PM

#include <string.h>
#include <os2.h>
#include "exedit.h"

HWND CreateChild(HWND hwnd,PSZ Title)
{
 ULONG ulCreate = FCF_STANDARD & ~FCF_MENU & ~FCF_ACCELTABLE;
 TChild *Child;
 TMain *Main=WinQueryWindowPtr(hwnd,0);
 HWND hwndClient,hwndFrame;
 SWP swp;

 if (!ChildCount(hwnd)) CItemsEnable(hwnd,TRUE);

 hwndFrame=WinCreateStdWindow(hwnd,0,&ulCreate,WC_CHILD,Title,0,NULLHANDLE,
                              Id_ChildWindow,&hwndClient);
 Child=WinQueryWindowPtr(hwndClient,0);

 if (Title)
  strcpy(Child->FileName,Title);
 else
 {
  Child->FileName[0]=0;
  Main->Untitled[8]++;
  if (Main->Untitled[8]>'9') Main->Untitled[8]='1';
  WinSetWindowText(hwndFrame,Main->Untitled);
 }

 UpdateSettings(hwndClient,&Main->DefSettings,FALSE);
 WinGetMaxPosition(hwndFrame,&swp);
 WinSetWindowPos(hwndFrame,HWND_TOP,swp.x+4,swp.y+4,swp.cx-7,swp.cy-7,
                 SWP_ACTIVATE|SWP_MOVE|SWP_SIZE|SWP_NOADJUST|SWP_SHOW);
 WinSetActiveWindow(HWND_DESKTOP,hwndFrame);
 RefreshWinList(hwnd);
 return hwndClient;
}

MPARAM ChildSendMsg(HWND hwndMain,ULONG msg,MPARAM mp1,MPARAM mp2)
{
 HWND hwnd;
 if ((hwnd=WinQueryActiveWindow(hwndMain))!=NULLHANDLE)
 return WinSendMsg(hwnd,msg,mp1,mp2);

 return 0;
}

void FileOpen(TChild *Child)
{
 CHAR  *Buffer;
 HFILE hfFile ;
 ULONG Action, BytesRead = 0;
 IPT   ipt = -1;

 DosOpen (Child->FileName,&hfFile,&Action,0,FILE_NORMAL,
          FILE_OPEN,OPEN_FLAGS_FAIL_ON_ERROR |OPEN_SHARE_DENYWRITE,
          NULL);

 if (hfFile)
  {
   Buffer = (char *)malloc(0x8FFF);
   WinSendMsg(Child->hwndMLE,MLM_SETIMPORTEXPORT,
              (MPARAM) Buffer,(MPARAM) 0x8FFF);
   do {
       DosRead(hfFile, Buffer, 0x8000, &BytesRead);
       WinSendMsg(Child->hwndMLE,MLM_IMPORT,
                  (MPARAM) &ipt,MPFROMLONG(BytesRead));
      }
   while (BytesRead == 0x8000);

   DosClose(hfFile);
   free(Buffer);
  }
 WinSendMsg(Child->hwndMLE,MLM_SETCHANGED,0,0);
}

void FileSave (TChild *Child)
{
 PVOID  Buffer;
 HFILE  hfFile;
 ULONG  Action, Wrote, amount, amountexp;
 IPT    FirstChar = 0;
 LONG   FileLength;
 APIRET rc;

 FileLength = (LONG) WinSendMsg(Child->hwndMLE,MLM_QUERYTEXTLENGTH,
                                0,(MPARAM)0xffffffff);
 if (FileLength > 0)
  {
   DosOpen(Child->FileName,&hfFile,&Action,FileLength,FILE_NORMAL,
           OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
           OPEN_FLAGS_FAIL_ON_ERROR |OPEN_SHARE_DENYREADWRITE|
           OPEN_ACCESS_READWRITE,NULL);

   if (hfFile)
    {
     Buffer = (char *)malloc(64000);

     WinSendMsg(Child->hwndMLE,MLM_SETIMPORTEXPORT,Buffer,
                MPFROMLONG(64000));

     do {
         amount = 64000;
         amountexp = (ULONG) WinSendMsg(Child->hwndMLE, MLM_EXPORT, &FirstChar, &amount);
         DosWrite(hfFile, (PVOID)Buffer, amountexp, &Wrote);
        }
     while(amountexp != 0);

     free (Buffer);
     DosClose (hfFile) ;
     }
    }
 WinSendMsg(Child->hwndMLE,MLM_SETCHANGED,0,0);
 RefreshWinList(WinQueryWindow(WinQueryWindow(WinQueryWindow(Child->hwndMLE,QW_PARENT),QW_PARENT),QW_PARENT));
}

int ChildCount(HWND hwndMain)
{
 HENUM henum;
 int count=0;

 henum = WinBeginEnumWindows(hwndMain);
 while(WinGetNextWindow(henum)!=NULLHANDLE) count++;
 WinEndEnumWindows(henum);
 return count;
}

void CItemsEnable(HWND hwnd,BOOL State)
{
  int i;
  HWND hwndFrame=WinQueryWindow(hwnd,QW_PARENT);
  TFMain *FMain=WinQueryWindowPtr(hwndFrame,0);

  for (i=CITEM_FIRST;i<=CITEM_LAST;i++)
  {
   WinEnableMenuItem(WinWindowFromID(hwndFrame,FID_MENU),i,State);
   WinSendMsg(FMain->hwndToolbar,TM_HIDEITEM,(MPARAM)i,(MPARAM)!State);
  }

  WinInvalidateRect(FMain->hwndToolbar,NULL,FALSE);
}

void RefreshWinList (HWND hwndClient)
{
 MENUITEM item;
 char iText[255];
 HWND hwndSubMenu,hwnd;
 HENUM henum;
 int x=0;

 WinSendMsg(WinWindowFromID(WinQueryWindow(hwndClient,QW_PARENT),FID_MENU),MM_QUERYITEM,MPFROM2SHORT(M_Window,TRUE),MPFROMP(&item));

 //Delete old items
 hwndSubMenu=item.hwndSubMenu;
 while (SHORT1FROMMP(WinSendMsg(hwndSubMenu,MM_DELETEITEM,MPFROM2SHORT(WinList_First+x,TRUE),0))!=6)
 x++;

 //Add new items
 memset(&item,0,sizeof(MENUITEM));
 item.iPosition=MIT_END;
 item.id=WinList_First;

 henum = WinBeginEnumWindows(hwndClient);
 while((hwnd=WinGetNextWindow(henum))!=NULLHANDLE)
 {
  WinQueryWindowText(hwnd,253,iText);
  if (iText[0])
  {
   item.id++;
   item.hItem=hwnd;
   WinSendMsg(hwndSubMenu,MM_INSERTITEM,MPFROMP(&item),MPFROMP(iText));
  }
 }
 WinEndEnumWindows(henum);
}

void MaxChilds(HWND hwndMain)
{
 HENUM henum;
 HWND hwnd;

 henum = WinBeginEnumWindows(hwndMain);
 while((hwnd=WinGetNextWindow(henum))!=NULLHANDLE)
  WinSetWindowPos(hwnd,0,0,0,0,0,SWP_MAXIMIZE);
 WinEndEnumWindows(henum);
}

