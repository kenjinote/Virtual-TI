//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "skintest.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSkinTestWnd *SkinTestWnd;

#define SIZE_NONE 0
#define SIZE_LEFT 1
#define SIZE_RIGHT 2
#define SIZE_TOP 3
#define SIZE_BOTTOM 4

int skinFloating=0;
int mouseClicked=0;
int mouseX,mouseY;
int sizeType=SIZE_NONE;

int IsCloseEnough(int color1,int color2)
{
    int r1=(color1>>16)&0xff;
    int g1=(color1>>8)&0xff;
    int b1=color1&0xff;
    int r2=(color2>>16)&0xff;
    int g2=(color2>>8)&0xff;
    int b2=color2&0xff;
    if ((abs(r1-r2)+abs(g1-g2)+abs(b1-b2))<48)
        return 1;
    return 0;
}

unsigned SkinColor(int x,int y)
{
    unsigned char *ptr=&((unsigned char *)SkinTestWnd->Image->Picture->Bitmap->ScanLine[y])[x*3];
    return ptr[0]|(ptr[1]<<8)|(ptr[2]<<16);
}

int CanSkinFloat()
{
    unsigned color=SkinColor(0,0);
    for (int x=0;x<SkinTestWnd->Image->Picture->Bitmap->Width;x++)
    {
        if (!IsCloseEnough(SkinColor(x,0),color))
            return false;
        if (!IsCloseEnough(SkinColor(x,SkinTestWnd->Image->Picture->Bitmap->Height-1),color))
            return false;
    }
    for (int y=0;y<SkinTestWnd->Image->Picture->Bitmap->Height;y++)
    {
        if (!IsCloseEnough(SkinColor(0,y),color))
            return false;
        if (!IsCloseEnough(SkinColor(SkinTestWnd->Image->Picture->Bitmap->Width-1,y),color))
            return false;
    }
    return true;
}

HRGN CreateFloatRgn()
{
    HRGN rgn=NULL;
    unsigned color=SkinColor(0,0);
    for (int _y=0;_y<SkinTestWnd->Height;_y++)
    {
        int y=(_y*SkinTestWnd->Image->Picture->Bitmap->Height)/SkinTestWnd->Height;
        int leftX,rightX;
        for (leftX=0;leftX<SkinTestWnd->Image->Picture->Bitmap->Width;leftX++)
        {
            if (!(IsCloseEnough(SkinColor(leftX,y),color)||IsCloseEnough(SkinColor(leftX,y+1),color)))
                break;
        }
        if (leftX>=SkinTestWnd->Image->Picture->Bitmap->Width)
            continue;
        for (rightX=SkinTestWnd->Image->Picture->Bitmap->Width-1;rightX>0;rightX--)
        {
            if (!(IsCloseEnough(SkinColor(rightX,y),color)||IsCloseEnough(SkinColor(rightX,y+1),color)))
                break;
        }
        if (rightX<=0)
            continue;
        leftX=(leftX*SkinTestWnd->Width+(SkinTestWnd->Image->Picture->Bitmap->Width-1))/SkinTestWnd->Image->Picture->Bitmap->Width;
        rightX=(rightX*SkinTestWnd->Width+(SkinTestWnd->Image->Picture->Bitmap->Width-1))/SkinTestWnd->Image->Picture->Bitmap->Width;
        HRGN newRgn=CreateRectRgn(leftX,_y,rightX,_y+1);
        if (rgn)
        {
            CombineRgn(rgn,rgn,newRgn,RGN_OR);
            DeleteObject(newRgn);
        }
        else
            rgn=newRgn;
    }
    return rgn;
}
//---------------------------------------------------------------------------
__fastcall TSkinTestWnd::TSkinTestWnd(TComponent* Owner)
    : TForm(Owner)
{
    enforceRatio=0;
    lcdBmp=new Graphics::TBitmap;
    lcdBmp->Width=480;
    lcdBmp->Height=256;
    lcdBmp->PixelFormat=pf24bit;
    sLcdBmp=new Graphics::TBitmap;
    sLcdBmp->Width=240;
    sLcdBmp->Height=128;
    sLcdBmp->PixelFormat=pf24bit;
}
//---------------------------------------------------------------------------
void __fastcall TSkinTestWnd::WMSizing(TMessage &Msg)
{
    RECT *r=(LPRECT)Msg.LParam;
    if (!enforceRatio)
    {
        Msg.Result=false;
        return;
    }
    Normal1xview1->Checked=false;
    Large2xview1->Checked=false;
    int cliW=r->right-r->left-(Width-ClientWidth);
    int cliH=r->bottom-r->top-(Height-ClientHeight);
    int newW=Image->Picture->Bitmap->Width*cliH/Image->Picture->Bitmap->Height;
    int newH=Image->Picture->Bitmap->Height*cliW/Image->Picture->Bitmap->Width;
    newW+=(Width-ClientWidth);
    newH+=(Height-ClientHeight);
    int dx=abs(newW-(r->right-r->left));
    int dy=abs(newH-(r->bottom-r->top));
    if ((dx>=dy)||(Msg.WParam==WMSZ_LEFT)||(Msg.WParam==WMSZ_RIGHT))
    {
        if (newH<GetSystemMetrics(SM_CYMIN))
            GetWindowRect(Handle,r);
        else
        {
            if ((Msg.WParam==WMSZ_TOP)||
                (Msg.WParam==WMSZ_TOPLEFT)||
                (Msg.WParam==WMSZ_TOPRIGHT))
                r->top=r->bottom-newH;
            else
                r->bottom=r->top+newH;
        }
    }
    else
    {
        if (newW<GetSystemMetrics(SM_CXMIN))
            GetWindowRect(Handle,r);
        else
        {
            if ((Msg.WParam==WMSZ_BOTTOMLEFT)||
                (Msg.WParam==WMSZ_LEFT)||
                (Msg.WParam==WMSZ_TOPLEFT))
                r->left=r->right-newW;
            else
                r->right=r->left+newW;
        }
    }
    Image->Width=r->right-r->left-(Width-ClientWidth);
    Image->Height=r->bottom-r->top-(Height-ClientHeight);
    PaintBox->Left=lcd.Left*Image->Width/Image->Picture->Bitmap->Width;
    PaintBox->Width=(lcd.Right-lcd.Left)*Image->Width/Image->Picture->Bitmap->Width;
    PaintBox->Top=lcd.Top*Image->Height/Image->Picture->Bitmap->Height;
    PaintBox->Height=(lcd.Bottom-lcd.Top)*Image->Height/Image->Picture->Bitmap->Height;
    Msg.Result=true;
}

void __fastcall TSkinTestWnd::PaintBoxPaint(TObject *Sender)
{
    TRect r;
    r.Left=0; r.Right=PaintBox->Width;
    r.Top=0; r.Bottom=PaintBox->Height;
    if (r.Right>sLcdBmp->Width)
        PaintBox->Canvas->StretchDraw(r,lcdBmp);
    else
        PaintBox->Canvas->StretchDraw(r,sLcdBmp);
}
//---------------------------------------------------------------------------

void __fastcall TSkinTestWnd::Normal1xview1Click(TObject *Sender)
{
    enforceRatio=0;
    ClientWidth=Image->Picture->Bitmap->Width>>1;
    ClientHeight=Image->Picture->Bitmap->Height>>1;
    Image->Width=Image->Picture->Bitmap->Width>>1;
    Image->Height=Image->Picture->Bitmap->Height>>1;
    PaintBox->Left=lcd.Left*Image->Width/Image->Picture->Bitmap->Width;
    PaintBox->Width=sLcdBmp->Width;
    PaintBox->Top=lcd.Top*Image->Height/Image->Picture->Bitmap->Height;
    PaintBox->Height=sLcdBmp->Height;
    Normal1xview1->Checked=true;
    Large2xview1->Checked=false;
    enforceRatio=1;
    UpdateFloat();
}
//---------------------------------------------------------------------------

void __fastcall TSkinTestWnd::Large2xview1Click(TObject *Sender)
{
    enforceRatio=0;
    ClientWidth=Image->Picture->Bitmap->Width;
    ClientHeight=Image->Picture->Bitmap->Height;
    Image->Width=Image->Picture->Bitmap->Width;
    Image->Height=Image->Picture->Bitmap->Height;
    PaintBox->Left=lcd.Left*Image->Width/Image->Picture->Bitmap->Width;
    PaintBox->Width=lcdBmp->Width;
    PaintBox->Top=lcd.Top*Image->Height/Image->Picture->Bitmap->Height;
    PaintBox->Height=lcdBmp->Height;
    Normal1xview1->Checked=false;
    Large2xview1->Checked=true;
    enforceRatio=1;
    UpdateFloat();
}
//---------------------------------------------------------------------------


void __fastcall TSkinTestWnd::ImageMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
    if (mouseClicked&&skinFloating)
    {
        if (sizeType==SIZE_NONE)
        {
            Left+=X-mouseX;
            Top+=Y-mouseY;
        }
        else if (sizeType==SIZE_LEFT)
        {
            TMessage msg;
            RECT r;
            r.left=Left+(X-mouseX);
            r.right=Left+Width;
            r.top=Top;
            r.bottom=Top+Height;
            msg.LParam=(DWORD)&r;
            msg.WParam=WMSZ_LEFT;
            WMSizing(msg);
            Left=r.left; Width=r.right-r.left;
            Top=r.top; Height=r.bottom-r.top;
            SetWindowRgn(Handle,CreateFloatRgn(),true);
            SkinTestWnd->Update();
        }
        else if (sizeType==SIZE_RIGHT)
        {
            TMessage msg;
            RECT r;
            r.left=Left;
            r.right=Left+Width+(X-mouseX);
            mouseX=X;
            r.top=Top;
            r.bottom=Top+Height;
            msg.LParam=(DWORD)&r;
            msg.WParam=WMSZ_RIGHT;
            WMSizing(msg);
            Left=r.left; Width=r.right-r.left;
            Top=r.top; Height=r.bottom-r.top;
            SetWindowRgn(Handle,CreateFloatRgn(),true);
            SkinTestWnd->Update();
        }
        else if (sizeType==SIZE_TOP)
        {
            TMessage msg;
            RECT r;
            r.left=Left;
            r.right=Left+Width;
            r.top=Top+(Y-mouseY);
            r.bottom=Top+Height;
            msg.LParam=(DWORD)&r;
            msg.WParam=WMSZ_TOP;
            WMSizing(msg);
            Left=r.left; Width=r.right-r.left;
            Top=r.top; Height=r.bottom-r.top;
            SetWindowRgn(Handle,CreateFloatRgn(),true);
            SkinTestWnd->Update();
        }
        else if (sizeType==SIZE_BOTTOM)
        {
            TMessage msg;
            RECT r;
            r.left=Left;
            r.right=Left+Width;
            r.top=Top;
            r.bottom=Top+Height+(Y-mouseY);
            mouseY=Y;
            msg.LParam=(DWORD)&r;
            msg.WParam=WMSZ_BOTTOM;
            WMSizing(msg);
            Left=r.left; Width=r.right-r.left;
            Top=r.top; Height=r.bottom-r.top;
            SetWindowRgn(Handle,CreateFloatRgn(),true);
            SkinTestWnd->Update();
        }
    }
    else if (skinFloating)
    {
        int x=(X*SkinTestWnd->Image->Picture->Bitmap->Width)/ClientWidth;
        int y=(Y*SkinTestWnd->Image->Picture->Bitmap->Height)/ClientHeight;
        int w=SkinTestWnd->Image->Picture->Bitmap->Width;
        int h=SkinTestWnd->Image->Picture->Bitmap->Height;
        int border=5*SkinTestWnd->Image->Picture->Bitmap->Width/ClientWidth;
        int leftX,rightX,topY,bottomY;
        unsigned color=SkinColor(0,0);
        for (leftX=0;leftX<SkinTestWnd->Image->Picture->Bitmap->Width;leftX++)
        {
            if (!IsCloseEnough(SkinColor(leftX,y),color))
                break;
        }
        for (rightX=SkinTestWnd->Image->Picture->Bitmap->Width-1;rightX>0;rightX--)
        {
            if (!IsCloseEnough(SkinColor(rightX,y),color))
                break;
        }
        for (topY=0;topY<SkinTestWnd->Image->Picture->Bitmap->Height;topY++)
        {
            if (!IsCloseEnough(SkinColor(x,topY),color))
                break;
        }
        for (bottomY=SkinTestWnd->Image->Picture->Bitmap->Height-1;bottomY>0;bottomY--)
        {
            if (!IsCloseEnough(SkinColor(x,bottomY),color))
                break;
        }
        if ((abs(x-leftX)<border)||(abs(x-rightX)<border)||(abs(y-topY)<border)||
            (abs(y-bottomY)<border))
        {
            if ((x<(w/2))&&(y>(x*h/w))&&(y<(h-x*h/w)))
            {
                Screen->Cursor=crSizeWE;
                sizeType=SIZE_LEFT;
            }
            else if ((x>=(w/2))&&(y>((w-x)*h/w))&&(y<(h-(w-x)*h/w)))
            {
                Screen->Cursor=crSizeWE;
                sizeType=SIZE_RIGHT;
            }
            else if ((y<(h/2))&&(x>(y*w/h))&&(x<(w-y*w/h)))
            {
                Screen->Cursor=crSizeNS;
                sizeType=SIZE_TOP;
            }
            else
            {
                Screen->Cursor=crSizeNS;
                sizeType=SIZE_BOTTOM;
            }
        }
        else
        {
            Screen->Cursor=crArrow;
            sizeType=SIZE_NONE;
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TSkinTestWnd::ImageMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    mouseClicked=0;
}
//---------------------------------------------------------------------------

void __fastcall TSkinTestWnd::ImageMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    mouseClicked=1;
    mouseX=X;
    mouseY=Y;
}
//---------------------------------------------------------------------------

void __fastcall TSkinTestWnd::PaintBoxMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
    Screen->Cursor=crArrow;
}
//---------------------------------------------------------------------------

void __fastcall TSkinTestWnd::UpdateFloat()
{
    if (CanSkinFloat())
    {
        BorderStyle=bsNone;
        SetWindowRgn(Handle,CreateFloatRgn(),true);
        skinFloating=1;
    }
    else
    {
        BorderStyle=bsSizeable;
        SetWindowRgn(Handle,CreateRectRgn(0,0,Width,Height),true);
        skinFloating=0;
    }
}

void __fastcall TSkinTestWnd::Close1Click(TObject *Sender)
{
    Close();    
}
//---------------------------------------------------------------------------

void __fastcall TSkinTestWnd::FormResize(TObject *Sender)
{
    if (!skinFloating)
        SetWindowRgn(Handle,CreateRectRgn(0,0,Width,Height),true);
}
//---------------------------------------------------------------------------

