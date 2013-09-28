#define INCL_PM
#include <memory.h>
#include <string.h>
#include <os2.h>
#include "exedit.h"

void RegisterToolBar(HAB anHab)
{
 WinRegisterClass(anHab,WC_TOOL,&WinToolProc,CS_SIZEREDRAW,4);
}

MRESULT EXPENTRY WinToolProc(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2)
{
TTool *Tool;
Tool=WinQueryWindowPtr(hwnd,QWL_USER);

switch(msg)
{

 case WM_CREATE:
 {
  long fl = FCF_BORDER;
  RGB rgb = { 78, 255, 255 };

  Tool = (TTool*)malloc(sizeof(TTool));

  Tool->usSzStruct=sizeof(TTool);
  Tool->Items=NULL;
  Tool->MiddleShadow=(LONG)WinQuerySysColor(HWND_DESKTOP,SYSCLR_SHADOWHILITEBGND, 0L);
  Tool->DarkShadow=0;
  Tool->LightShadow=0x00ffffff;
  Tool->OnTool = FALSE;
  Tool->TipOn = FALSE;

  Tool->hTipFr = WinCreateStdWindow(HWND_DESKTOP,
                                    0L,
                                    &fl,
                                    WC_STATIC,
                                    "",
                                    SS_TEXT | SS_AUTOSIZE | DT_LEFT | DT_VCENTER,
                                    NULLHANDLE,
                                    0L,
                                    &Tool->hTipCli);

  WinSetPresParam(Tool->hTipCli,PP_FONTNAMESIZE,7,"8.Helv");
  WinSetPresParam(Tool->hTipCli,PP_BACKGROUNDCOLOR,3,&rgb);
  WinSetWindowPos(Tool->hTipFr,HWND_TOP,0,0,500,16,SWP_SIZE);

  WinQueryWindowRect(hwnd,&Tool->rcl);
  WinSetWindowPtr(hwnd,QWL_USER,Tool);
 }
 break;

 case WM_BUTTON1DOWN:
  if (Tool->OnItem)
  {
   Tool->PressedItem=Tool->OnItem;
   Tool->OnItem->Status=TIT_PRESSED;
   WinSendMsg(hwnd,TM_DRAWITEM,(MPARAM)Tool->OnItem,0);
  }
 break;

 case WM_BUTTON1UP:
  if ((Tool->PressedItem == Tool->OnItem) && Tool->OnItem)
  {
   Tool->PressedItem->Status = TIT_NORMAL;
   WinSendMsg(hwnd,TM_DRAWITEM,(MPARAM)Tool->PressedItem,0);
   WinPostMsg(WinQueryWindow(hwnd,QW_OWNER),WM_COMMAND,MPFROM2SHORT(Tool->PressedItem->Id,0),0);
   Tool->PressedItem=NULL;
  }
  else Tool->PressedItem = NULL;
 break;

 case WM_TIMER:
 {
   switch(SHORT1FROMMP(mp1)) {
    case 1 :
      {
      POINTL ptl;
      SWP    swp;

      WinQueryWindowPos(hwnd, &swp);
      WinQueryPointerPos(HWND_DESKTOP, &ptl);
      WinMapWindowPoints(HWND_DESKTOP, WinQueryWindow(hwnd,QW_PARENT), &ptl, 1);

      if (swp.x > ptl.x || swp.x + swp.cx < ptl.x ||
          swp.y > ptl.y || swp.y + swp.cy < ptl.y)       {
        if (Tool->PressedItem) {
                                 Tool->PressedItem->Status = TIT_NORMAL;
                                 WinSendMsg(hwnd,TM_DRAWITEM,(MPARAM)Tool->PressedItem,0);
                                 Tool->PressedItem=NULL;
                               }

        if (Tool->TipOn) WinShowWindow(Tool->hTipFr,(Tool->TipOn = FALSE));

        Tool->OnTool = FALSE;
        Tool->OnItem = NULL;

       WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, 1);
      }
      break;
      }
    case 2 :
     if (Tool->OnTool && !Tool->TipOn) {
     if (Tool->OnItem)
        if (Tool->OnItem->text)
          { POINTL pos;
            RECTL r;
            HPS hps;

            r.xLeft = 0;
            r.xRight = 400;
            r.yBottom = 0;
            r.yTop = 12;
            hps = WinGetPS(Tool->hTipCli);
            if (!hps) DosBeep(1000,20);

            WinDrawText(hps,strlen(Tool->OnItem->text),Tool->OnItem->text,&r,0L,0L,DT_QUERYEXTENT);
            WinReleasePS(hps);

            WinSetWindowText(Tool->hTipCli,Tool->OnItem->text);
            pos.x = Tool->OnItem->pos.x; pos.y = Tool->OnItem->pos.y;
            WinMapWindowPoints(hwnd,HWND_DESKTOP,&pos,1);
            WinSetWindowPos(Tool->hTipFr,HWND_TOP,pos.x,pos.y-16,r.xRight + 3,16,SWP_MOVE | SWP_SIZE | SWP_ZORDER | SWP_SHOW);
            Tool->TipOn = TRUE; }
     }
     WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, 2);
     break;
    }
 }
 break;

 case WM_MOUSEMOVE:
 {
  SHORT x=SHORT1FROMMP(mp1),y=SHORT2FROMMP(mp1);
  TToolItem *Item;

  WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 1, 50);
  WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 2, 750);

  Tool->OnTool = TRUE;


  Item = Tool->Items;
  while (Item)
  {
   if ((Item->BType == TIT_BUTTON) && (Item->Status != TIT_HIDE))
    if (x > Item->pos.x && x<Item->pos.x+TIT_CXCY && y > Item->pos.y && y<Item->pos.y+TIT_CXCY)
    {

     if ((Item != Tool->OnItem) && Tool->TipOn) WinShowWindow(Tool->hTipFr,(Tool->TipOn = FALSE));

     Tool->OnItem=Item;

     if (Tool->PressedItem)
       if (Tool->OnItem != Tool->PressedItem)
         { Tool->PressedItem->Status = TIT_NORMAL;
           WinSendMsg(hwnd,TM_DRAWITEM,(MPARAM)Tool->PressedItem,0); }
       else
         { Tool->PressedItem->Status = TIT_PRESSED;
           WinSendMsg(hwnd,TM_DRAWITEM,(MPARAM)Tool->PressedItem,0); }

     return((MRESULT) TRUE);
    }
   Item=Item->Next;
  }


  Tool->OnItem = NULL;

  if (Tool->TipOn) WinShowWindow(Tool->hTipFr,(Tool->TipOn = FALSE));

  if (Tool->PressedItem)
    { Tool->PressedItem->Status = TIT_NORMAL;
      WinSendMsg(hwnd,TM_DRAWITEM,(MPARAM)Tool->PressedItem,0); }
 }
 break;

// ********************* Internal Toolbar Commands ************************************

 //mp1 SHORT1 Id SHORT2 BitmapId Item index
 //mp2 Description
 case TM_INSERTITEM:
 {
  TToolItem *Item,*NewItem;
  char *text=(char*) mp2;
  HPS hps;

  NewItem=(TToolItem*)malloc(sizeof(TToolItem));
  if (!Tool->Items) Tool->Items=NewItem;
  else
  {
   Item=Tool->Items;
   while (Item->Next) Item=Item->Next;
   Item->Next=NewItem;
  }

  if ((LONG)mp1!=TIT_SEPARATOR)
  {
   NewItem->Id=SHORT1FROMMP(mp1);

   hps=WinGetPS(hwnd);
   NewItem->hbm=GpiLoadBitmap(hps,NULLHANDLE,SHORT2FROMMP(mp1),TIT_CXCY-2,TIT_CXCY-2);
   WinReleasePS(hps);

   NewItem->BType = TIT_BUTTON;
   if (text) NewItem->text=strdup(text);
  }
  else NewItem->BType = TIT_SEPARATOR;

  NewItem->Status = TIT_NORMAL;
 }
 break;

 case TM_HIDEITEM:
 {
  TToolItem *Item=Tool->Items;
  USHORT Id=SHORT1FROMMP(mp1);

  while(Item)
  {
   if (Item->Id==Id)
   {
     Item->Status=(BOOL)mp2;
     break;
   }
   Item=Item->Next;
  }
 }
 break;

 case TM_DRAWITEM:
 {
  HPS hps=WinGetPS(hwnd);
  GpiCreateLogColorTable(hps,0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
  DrawItem(hps,(TToolItem*)mp1,Tool);
  WinReleasePS(hps);
 }
 break;

 case WM_SIZE:
  WinQueryWindowRect(hwnd,&Tool->rcl);
 break;

 case WM_PAINT :
 {
  TToolItem *Item;
  HPS hps=WinBeginPaint(hwnd,(HPS)NULL,NULL);
  POINTL point[3];

  GpiCreateLogColorTable(hps,0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
  WinFillRect(hps,&Tool->rcl,SYSCLR_DIALOGBACKGROUND);

  point[0].x=Tool->rcl.xLeft;
  point[0].y=Tool->rcl.yBottom;
  point[1].x=Tool->rcl.xRight;
  point[1].y=Tool->rcl.yBottom;

  GpiSetColor(hps,Tool->DarkShadow);
  GpiMove(hps,&point[0]);
  GpiLine(hps,&point[1]);
  GpiSetColor(hps,Tool->MiddleShadow);
  point[0].y++;
  point[1].y++;  GpiMove(hps,&point[0]);
  GpiLine(hps,&point[1]);

  Item=Tool->Items;
  if (Item)
  {
  Item->pos.x=TIT_FIRSTX;
  Item->pos.y=TIT_FIRSTY;

  while (Item)
  {

   if (Item->BType==TIT_SEPARATOR)
   {
    if (!Item->Next) break;
    Item->Next->pos.y=TIT_FIRSTY;
    Item->Next->pos.x=Item->pos.x+TIT_DISTANCE+TIT_CXSEPARATOR;
    Item=Item->Next;
   }
   else if (Item->Status==TIT_HIDE)
   {
    if (!Item->Next) break;
    Item->Next->pos.y=TIT_FIRSTY;
    Item->Next->pos.x=Item->pos.x;
    Item=Item->Next;
   }
   else
   {
    DrawItem(hps,Item,Tool);
    if (!Item->Next) break;
    Item->Next->pos.y=TIT_FIRSTY;
    Item->Next->pos.x=Item->pos.x+TIT_DISTANCE+TIT_CXCY;
    Item=Item->Next;
   }

   }
  }
  WinEndPaint(hps);
 }
 break;

 case WM_ERASEBACKGROUND:
 return (MPARAM)FALSE;

 case WM_DESTROY:
 {
  TToolItem *Item=Tool->Items;
  TToolItem *Temp;

  while (Item)
  {
   if (Item->text) DosFreeMem(Item->text);
   if (Item->hbm) GpiDeleteBitmap(Item->hbm);
   Temp=Item;
   Item=Item->Next;
   DosFreeMem(Temp);
  }
  WinDestroyWindow(Tool->hTipFr);
  DosFreeMem(Tool);
 }
 break;

 default :
 return(WinDefWindowProc(hwnd, msg, mp1, mp2));
 }
 return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

void DrawItem(HPS hps,TToolItem *Item,TTool *Tool)
{
  POINTL point[3];

  if (Item->Status==TIT_NORMAL)
   {
   GpiSetColor(hps,Tool->LightShadow);
   point[0].x=Item->pos.x;
   point[0].y=Item->pos.y+TIT_CXCY;
   point[1].x=Item->pos.x+TIT_CXCY;
   point[1].y=Item->pos.y+TIT_CXCY;
   GpiMove(hps,&Item->pos);
   GpiPolyLine(hps,2,point);

   GpiSetColor(hps,Tool->DarkShadow);
   point[0].x=Item->pos.x+TIT_CXCY;
   point[0].y=Item->pos.y;
   GpiMove(hps,&Item->pos);
   GpiPolyLine(hps,2,point);
   GpiSetColor(hps,Tool->MiddleShadow);
   point[0].y++;
   point[0].x--;
   point[1].x--;
   point[1].y--;
   point[2].x=Item->pos.x+1;
   point[2].y=Item->pos.y+1;
   GpiMove(hps,&point[2]);
   GpiPolyLine(hps,2,point);

   point[2].x=Item->pos.x+1;
   point[2].y=Item->pos.y+2;

   WinDrawBitmap(hps,Item->hbm,NULL,&point[2],0,0,DBM_NORMAL);
   }

  else if (Item->Status==TIT_PRESSED)
   {
   GpiSetColor(hps,Tool->MiddleShadow);
   point[0].x=Item->pos.x;
   point[0].y=Item->pos.y+TIT_CXCY;
   point[1].x=Item->pos.x+TIT_CXCY;
   point[1].y=Item->pos.y+TIT_CXCY;
   GpiMove(hps,&Item->pos);
   GpiPolyLine(hps,2,point);

   GpiSetColor(hps,Tool->DarkShadow);
   point[2].x=Item->pos.x+1;
   point[2].y=Item->pos.y+1;
   point[0].y--;
   point[0].x++;
   point[1].x--;
   point[1].y--;
   GpiMove(hps,&point[2]);
   GpiPolyLine(hps,2,point);


   GpiSetColor(hps,Tool->LightShadow);
   point[0].x=Item->pos.x+TIT_CXCY;
   point[0].y=Item->pos.y;
   point[1].x++;
   point[1].y++;
   GpiMove(hps,&Item->pos);
   GpiPolyLine(hps,2,point);


   point[2].x=Item->pos.x+2;
   point[2].y=Item->pos.y+1;

   WinDrawBitmap(hps,Item->hbm,NULL,&point[2],0,0,DBM_NORMAL);
   }
}


