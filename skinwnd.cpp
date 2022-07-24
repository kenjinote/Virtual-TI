//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include <dir.h>
#pragma hdrstop

#include "skinwnd.h"
#include "skinprop.h"
#include "skinabt.h"
#include "skinkeys.h"
#include "skintest.h"
#include "jpegfmt.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSkinEdit *SkinEdit;

int calc=89;
char name[64]="";
char author[64]="";
int state=0;
int mouseHeld;
RECT lcd,useLcd,key[80];
int curKey;
int modified=0;
TColor white,black;
int colorType=1;
char filename[256];

char *keynames[5][81]=
   {{NULL},
    {"Y=","Window","Zoom","Trace","Graph","2nd","Mode","Del","Left","Right","Up","Down","Alpha","X,T,theta","Stat","Math","Matrx","Prgm","Vars","Clear","x^-1","sin","cos","tan","^","x^2",", (comma)","(",")","÷","log","7","8","9","*","ln","4","5","6","-","Sto>","1","2","3","+","On","0",". (period)","(-)","Enter",NULL},
    {"F1","F2","F3","F4","F5","2nd","Exit","More","Left","Right","Up","Down","Alpha","x-var","Del","Graph","Stat","Prgm","Custom","Clear","log","sin","cos","tan","^","ln","ee","(",")","÷","x^2","7","8","9","*",", (comma)","4","5","6","-","Sto>","1","2","3","+","On","0",". (period)","(-)","Enter",NULL},
    {"F1","F2","F3","F4","F5","2nd","Shift","Esc","Left","Right","Up","Down","Diamond","Alpha","Apps","Home","Mode","Catalog","Backspace","Clear","X","Y","Z","T","^","=","(",")",", (comma)","÷","|","7","8","9","*","ee","4","5","6","-","Sto>","1","2","3","+","On","0",". (period)","(-)","Enter",NULL},
    {"Hand","F1","F2","F3","F4","F5","F6","F7","F8","Q","W","E","R","T","Y","U","I","O","P","A","S","D","F","G","H","J","K","L","Z","X","C","V","B","N","M","theta","Shift","On","Diamond","2nd (Alpha area)","Sto>","Space","=","Backspace","Enter (Alpha area)","2nd (Cursor area)","Esc","Mode","Clear","ln","Apps","Enter (Cursor area)","Left","Right","Up","Down","sin","cos","tan","^","(",")",", (comma)","÷","7","8","9","*","4","5","6","-","1","2","3","+","0",". (period)","(-)","Enter (Numeric area)",NULL}};

#define LCD_EDIT 1
#define KEY_EDIT 2
//---------------------------------------------------------------------------
__fastcall TSkinEdit::TSkinEdit(TComponent* Owner)
    : TForm(Owner)
{
    bmp=NULL;
    filename[0]=0;
    colorType=1;
    white=(TColor)0xcfe0cc;
    black=(TColor)0x222e31;
}
//---------------------------------------------------------------------------
void __fastcall TSkinEdit::Exit1Click(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::New1Click(TObject *Sender)
{
    if (modified)
    {
        int but=MessageBox(Handle,"Save changes to current file?",
            "File modified",MB_YESNOCANCEL|MB_ICONQUESTION);
        if (but==IDCANCEL)
            return;
        if (but==IDYES)
            Save1Click(Sender);
    }
    TOpenDialog *dlg=new TOpenDialog(this);
    dlg->DefaultExt="bmp";
    dlg->FileName="*.bmp";
    dlg->Filter="Bitmap files (*.bmp)|*.bmp";
    dlg->FilterIndex=1;
    dlg->Options.Clear();
    dlg->Options << ofFileMustExist << ofHideReadOnly;
    dlg->Title="Select Image Bitmap";
    if (dlg->Execute())
    {
        if (bmp) { delete bmp; bmp=NULL; }
        bmp=new Graphics::TBitmap;
        TFileStream *fs=new TFileStream(dlg->FileName,fmOpenRead);
        bmp->LoadFromStream(fs);
        delete fs;
        Image->Picture->Bitmap=bmp;
        Fittowindow1->Checked=true;
        Halfsize1->Checked=false;
        Fullsize1->Checked=false;
        Doublesize1->Checked=false;
        int w=bmp->Width;
        int h=bmp->Height;
        if (w>640)
        {
            int nh=(h*640)/w;
            if (nh>480)
            {
                int nw=(w*480)/h;
                ClientWidth=nw;
                ClientHeight=480;
            }
            else
            {
                ClientWidth=640;
                ClientHeight=nh;
            }
        }
        else if (h>480)
        {
            int nw=(w*480)/h;
            if (nw>640)
            {
                int nh=(h*640)/w;
                ClientWidth=640;
                ClientHeight=nh;
            }
            else
            {
                ClientWidth=nw;
                ClientHeight=480;
            }
        }
        else
        {
            ClientWidth=bmp->Width;
            ClientHeight=bmp->Height;
        }
        Image->Align=alNone;
        PaintBox->Align=alNone;
        StatusBar1->Panels->Items[0]->Text="Select action from edit menu";
        name[0]=0;
        Caption="Virtual TI Skin Creator - Untitled";
        Halfsize1->Enabled=true;
        Fittowindow1->Enabled=true;
        Fullsize1->Enabled=true;
        Doublesize1->Enabled=true;
        Properties1->Enabled=true;
        LCDposition1->Enabled=true;
        Keypositions1->Enabled=true;
//        Gammacorrection1->Enabled=true;
//        Brightnesscontrast1->Enabled=true;
//        Crop1->Enabled=true;
        Save1->Enabled=true;
        Saveas1->Enabled=true;
        Test1->Enabled=true;
        lcd.left=-1; lcd.right=-1;
        lcd.top=-1; lcd.bottom=-1;
        useLcd.left=-1; useLcd.right=-1;
        useLcd.top=-1; useLcd.bottom=-1;
        for (int i=0;i<80;i++)
        {
            key[i].left=-1; key[i].right=-1;
            key[i].top=-1; key[i].bottom=-1;
        }
        modified=1;
        filename[0]=0;
        state=0;
        LCDposition1->Checked=false;
        Keypositions1->Checked=false;
        KeyWnd->Hide();
        Properties1Click(Sender);
    }
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::Halfsize1Click(TObject *Sender)
{
    if (!bmp) return;
    Fittowindow1->Checked=false;
    Halfsize1->Checked=true;
    Fullsize1->Checked=false;
    Doublesize1->Checked=false;
    Image->Align=alNone;
    PaintBox->Align=alNone;
    Image->Width=bmp->Width>>1;
    PaintBox->Width=bmp->Width>>1;
    Image->Height=bmp->Height>>1;
    PaintBox->Height=bmp->Height>>1;
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::Fittowindow1Click(TObject *Sender)
{
    if (!bmp) return;
    Fittowindow1->Checked=true;
    Halfsize1->Checked=false;
    Fullsize1->Checked=false;
    Doublesize1->Checked=false;
    Image->Align=alNone;
    PaintBox->Align=alNone;
    FormResize(NULL);
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::Fullsize1Click(TObject *Sender)
{
    if (!bmp) return;
    Fittowindow1->Checked=false;
    Halfsize1->Checked=false;
    Fullsize1->Checked=true;
    Doublesize1->Checked=false;
    Image->Align=alNone;
    PaintBox->Align=alNone;
    Image->Width=bmp->Width;
    PaintBox->Width=bmp->Width;
    Image->Height=bmp->Height;
    PaintBox->Height=bmp->Height;
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::Doublesize1Click(TObject *Sender)
{
    if (!bmp) return;
    Fittowindow1->Checked=false;
    Halfsize1->Checked=false;
    Fullsize1->Checked=false;
    Doublesize1->Checked=true;
    Image->Align=alNone;
    PaintBox->Align=alNone;
    Image->Width=bmp->Width<<1;
    PaintBox->Width=bmp->Width<<1;
    Image->Height=bmp->Height<<1;
    PaintBox->Height=bmp->Height<<1;
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::Properties1Click(TObject *Sender)
{
    SkinPropertiesDlg->TI73->Checked=(calc==73);
    SkinPropertiesDlg->TI82->Checked=(calc==82);
    SkinPropertiesDlg->TI83->Checked=(calc==83);
    SkinPropertiesDlg->TI83Plus->Checked=(calc==84);
    SkinPropertiesDlg->TI85->Checked=(calc==85);
    SkinPropertiesDlg->TI86->Checked=(calc==86);
    SkinPropertiesDlg->TI89->Checked=(calc==89);
    SkinPropertiesDlg->TI92->Checked=(calc==92);
    SkinPropertiesDlg->TI92Plus->Checked=(calc==94);
    SkinPropertiesDlg->Name->Text=name;
    SkinPropertiesDlg->Author->Text=author;
    SkinPropertiesDlg->LowContrast->Checked=(colorType==0);
    SkinPropertiesDlg->HighContrast->Checked=(colorType==1);
    SkinPropertiesDlg->Custom->Checked=(colorType==2);
    if (colorType==2)
    {
        SkinPropertiesDlg->WhiteColor->Brush->Color=white;
        SkinPropertiesDlg->BlackColor->Brush->Color=black;
    }
    if (SkinPropertiesDlg->ShowModal()==1)
    {
        strcpy(name,SkinPropertiesDlg->Name->Text.c_str());
        strcpy(author,SkinPropertiesDlg->Author->Text.c_str());
        if (name[0]!=0)
            Caption="Virtual TI Skin Creator - "+AnsiString(name);
        else
            Caption="Virtual TI Skin Creator - Untitled";
        if (author[0]!=0)
            Caption="Virtual TI Skin Creator - "+AnsiString(name)+" by "+AnsiString(author);
        if (SkinPropertiesDlg->TI73->Checked) calc=73;
        else if (SkinPropertiesDlg->TI82->Checked) calc=82;
        else if (SkinPropertiesDlg->TI83->Checked) calc=83;
        else if (SkinPropertiesDlg->TI83Plus->Checked) calc=84;
        else if (SkinPropertiesDlg->TI85->Checked) calc=85;
        else if (SkinPropertiesDlg->TI86->Checked) calc=86;
        else if (SkinPropertiesDlg->TI89->Checked) calc=89;
        else if (SkinPropertiesDlg->TI92->Checked) calc=92;
        else if (SkinPropertiesDlg->TI92Plus->Checked) calc=94;
        if (SkinPropertiesDlg->LowContrast->Checked)
        {
            colorType=0;
            white=(TColor)0xb0ccae;
            black=(TColor)0x8a6f53;
        }
        else if (SkinPropertiesDlg->HighContrast->Checked)
        {
            colorType=1;
            white=(TColor)0xcfe0cc;
            black=(TColor)0x222e31;
        }
        else if (SkinPropertiesDlg->Custom->Checked)
        {
            colorType=2;
            white=SkinPropertiesDlg->WhiteColor->Brush->Color;
            black=SkinPropertiesDlg->BlackColor->Brush->Color;
        }
        modified=1;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::About1Click(TObject *Sender)
{
    AboutDlg->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::LCDposition1Click(TObject *Sender)
{
    if (state==LCD_EDIT)
    {
        state=0;
        Image->Repaint();
        PaintBox->Repaint();
        LCDposition1->Checked=false;
        StatusBar1->Panels->Items[0]->Text="Select action from edit menu";
        return;
    }
    KeyWnd->Hide();
    state=LCD_EDIT;
    Image->Repaint();
    PaintBox->Repaint();
    LCDposition1->Checked=true;
    Keypositions1->Checked=false;
    StatusBar1->Panels->Items[0]->Text="Left click & drag to define LCD area";
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::Keypositions1Click(TObject *Sender)
{
    if (state==KEY_EDIT)
    {
        KeyWnd->Hide();
        state=0;
        Image->Repaint();
        PaintBox->Repaint();
        Keypositions1->Checked=false;
        StatusBar1->Panels->Items[0]->Text="Select action from edit menu";
        return;
    }
    KeyWnd->KeyList->Clear();
    int n=(calc==73)?0:(calc<=84)?1:(calc<=86)?2:(calc==89)?3:4;
    for (int i=0;keynames[n][i];i++)
        KeyWnd->KeyList->Items->Add(keynames[n][i]);
    KeyWnd->KeyList->ItemIndex=0;
    curKey=0;
    KeyWnd->Show();
    state=KEY_EDIT;
    Image->Repaint();
    PaintBox->Repaint();
    LCDposition1->Checked=false;
    Keypositions1->Checked=true;
    StatusBar1->Panels->Items[0]->Text="Left drag to define, right click for next";
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::PaintBoxPaint(TObject *Sender)
{
    if (state==LCD_EDIT)
    {
        TRect r;
/*        PaintBox->Canvas->Brush->Color=white;
        PaintBox->Canvas->Brush->Style=bsSolid;
        PaintBox->Canvas->Pen->Style=psDash;
        PaintBox->Canvas->Pen->Mode=pmCopy;
        PaintBox->Canvas->Pen->Color=(TColor)((~PaintBox->Canvas->Brush->Color)&0xffffff);
        r.Left=lcd.left*Image->Width/bmp->Width;
        r.Top=lcd.top*Image->Height/bmp->Height;
        r.Right=lcd.right*Image->Width/bmp->Width;
        r.Bottom=lcd.bottom*Image->Height/bmp->Height;
        PaintBox->Canvas->Rectangle(r.Left,r.Top,r.Right,r.Bottom);*/
        PaintBox->Canvas->Brush->Color=black;
        PaintBox->Canvas->Brush->Style=bsBDiagonal;
        PaintBox->Canvas->Pen->Style=psSolid;
        PaintBox->Canvas->Pen->Width=1;
        PaintBox->Canvas->Pen->Mode=pmCopy;
        PaintBox->Canvas->Pen->Color=black;
        r.Left=useLcd.left*Image->Width/bmp->Width;
        r.Top=useLcd.top*Image->Height/bmp->Height;
        r.Right=useLcd.right*Image->Width/bmp->Width;
        r.Bottom=useLcd.bottom*Image->Height/bmp->Height;
        PaintBox->Canvas->Rectangle(r.Left,r.Top,r.Right,r.Bottom);
    }
    else if (state==KEY_EDIT)
    {
        POINT p[5];
        PaintBox->Canvas->Pen->Style=psSolid;
        PaintBox->Canvas->Pen->Width=1;
        PaintBox->Canvas->Pen->Mode=pmNot;
        p[0].x=key[curKey].left*Image->Width/bmp->Width;
        p[0].y=key[curKey].top*Image->Height/bmp->Height;
        p[1].x=key[curKey].right*Image->Width/bmp->Width;
        p[1].y=key[curKey].top*Image->Height/bmp->Height;
        p[2].x=key[curKey].right*Image->Width/bmp->Width;
        p[2].y=key[curKey].bottom*Image->Height/bmp->Height;
        p[3].x=key[curKey].left*Image->Width/bmp->Width;
        p[3].y=key[curKey].bottom*Image->Height/bmp->Height;
        p[4].x=key[curKey].left*Image->Width/bmp->Width;
        p[4].y=key[curKey].top*Image->Height/bmp->Height;
        PaintBox->Canvas->Polyline(p,4);
    }
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::PaintBoxMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (Button!=mbLeft) return;
    mouseHeld=1;
    if (state==LCD_EDIT)
    {
        useLcd.left=useLcd.right=X*bmp->Width/Image->Width;
        useLcd.top=useLcd.bottom=Y*bmp->Height/Image->Height;
        modified=1;
    }
    else if (state==KEY_EDIT)
    {
        POINT p[5];
        PaintBox->Canvas->Pen->Style=psSolid;
        PaintBox->Canvas->Pen->Width=1;
        PaintBox->Canvas->Pen->Mode=pmNot;
        p[0].x=key[curKey].left*Image->Width/bmp->Width;
        p[0].y=key[curKey].top*Image->Height/bmp->Height;
        p[1].x=key[curKey].right*Image->Width/bmp->Width;
        p[1].y=key[curKey].top*Image->Height/bmp->Height;
        p[2].x=key[curKey].right*Image->Width/bmp->Width;
        p[2].y=key[curKey].bottom*Image->Height/bmp->Height;
        p[3].x=key[curKey].left*Image->Width/bmp->Width;
        p[3].y=key[curKey].bottom*Image->Height/bmp->Height;
        p[4].x=key[curKey].left*Image->Width/bmp->Width;
        p[4].y=key[curKey].top*Image->Height/bmp->Height;
        PaintBox->Canvas->Polyline(p,4);
        key[curKey].left=key[curKey].right=X*bmp->Width/Image->Width;
        key[curKey].top=key[curKey].bottom=Y*bmp->Height/Image->Height;
        modified=1;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::PaintBoxMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if ((Button==mbRight)&&(state==KEY_EDIT))
    {
        curKey++;
        if (curKey>=KeyWnd->KeyList->Items->Count)
            curKey=0;
        KeyWnd->KeyList->ItemIndex=curKey;
        Image->Repaint();
    }
    else if ((Button==mbRight)&&(mouseHeld==2))
        mouseHeld=0;
    else if ((Button==mbLeft)&&(mouseHeld==1))
    {
        if (useLcd.right<useLcd.left)
        {
            int tmp=useLcd.left;
            useLcd.left=useLcd.right;
            useLcd.right=tmp;
        }
        if (useLcd.bottom<useLcd.top)
        {
            int tmp=useLcd.top;
            useLcd.top=useLcd.bottom;
            useLcd.bottom=tmp;
        }
        mouseHeld=0;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::PaintBoxMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
    if (!mouseHeld) return;
    if ((state==LCD_EDIT)&&(mouseHeld==1))
    {
        useLcd.right=X*bmp->Width/Image->Width;
        useLcd.bottom=Y*bmp->Height/Image->Height;
        Image->Repaint();
    }
    else if (state==KEY_EDIT)
    {
        POINT p[5];
        PaintBox->Canvas->Pen->Style=psSolid;
        PaintBox->Canvas->Pen->Width=1;
        PaintBox->Canvas->Pen->Mode=pmNot;
        p[0].x=key[curKey].left*Image->Width/bmp->Width;
        p[0].y=key[curKey].top*Image->Height/bmp->Height;
        p[1].x=key[curKey].right*Image->Width/bmp->Width;
        p[1].y=key[curKey].top*Image->Height/bmp->Height;
        p[2].x=key[curKey].right*Image->Width/bmp->Width;
        p[2].y=key[curKey].bottom*Image->Height/bmp->Height;
        p[3].x=key[curKey].left*Image->Width/bmp->Width;
        p[3].y=key[curKey].bottom*Image->Height/bmp->Height;
        p[4].x=key[curKey].left*Image->Width/bmp->Width;
        p[4].y=key[curKey].top*Image->Height/bmp->Height;
        PaintBox->Canvas->Polyline(p,4);
        key[curKey].right=X*bmp->Width/Image->Width;
        key[curKey].bottom=Y*bmp->Height/Image->Height;
        p[0].x=key[curKey].left*Image->Width/bmp->Width;
        p[0].y=key[curKey].top*Image->Height/bmp->Height;
        p[1].x=key[curKey].right*Image->Width/bmp->Width;
        p[1].y=key[curKey].top*Image->Height/bmp->Height;
        p[2].x=key[curKey].right*Image->Width/bmp->Width;
        p[2].y=key[curKey].bottom*Image->Height/bmp->Height;
        p[3].x=key[curKey].left*Image->Width/bmp->Width;
        p[3].y=key[curKey].bottom*Image->Height/bmp->Height;
        p[4].x=key[curKey].left*Image->Width/bmp->Width;
        p[4].y=key[curKey].top*Image->Height/bmp->Height;
        PaintBox->Canvas->Polyline(p,4);
    }
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::FormResize(TObject *Sender)
{
    if (!Fittowindow1->Checked) return;
    if (!bmp) return;
    int w=ClientWidth;
    int h=ClientHeight-StatusBar1->Height;
    int newW=Image->Picture->Bitmap->Width*h/Image->Picture->Bitmap->Height;
    if (newW>ClientWidth)
    {
        int newH=Image->Picture->Bitmap->Height*w/Image->Picture->Bitmap->Width;
        Image->Width=w;
        Image->Height=newH;
        PaintBox->Width=w;
        PaintBox->Height=newH;
    }
    else
    {
        Image->Width=newW;
        Image->Height=h;
        PaintBox->Width=newW;
        PaintBox->Height=h;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::FormCloseQuery(TObject *Sender, bool &CanClose)
{
    if (modified)
    {
        int but=MessageBox(Handle,"Save changes to current file?",
            "File modified",MB_YESNOCANCEL|MB_ICONQUESTION);
        if (but==IDCANCEL)
        {
            CanClose=false;
            return;
        }
        if (but==IDYES)
            Save1Click(Sender);
    }
    CanClose=true;
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::Save1Click(TObject *Sender)
{
    if (filename[0]==0)
    {
        Saveas1Click(Sender);
        return;
    }
    FILE *fp=fopen(filename,"wb");
    if (!fp) return;
    Graphics::TBitmap *tmp=new Graphics::TBitmap;
    int lcdW=(calc<=84)?96:(calc<89)?128:(calc==89)?160:240;
    int lcdH=(calc<89)?64:(calc==89)?100:128;
    tmp->Width=(lcdW<<1)*bmp->Width/(useLcd.right-useLcd.left);
    tmp->Height=(lcdH<<1)*bmp->Height/(useLcd.bottom-useLcd.top);
    tmp->PixelFormat=pf24bit;
    TRect r;
    r.Left=0; r.Right=tmp->Width;
    r.Top=0; r.Bottom=tmp->Height;
    tmp->Canvas->StretchDraw(r,bmp);
    fprintf(fp,"VTIv2.5 ");
    fwrite(name,64,1,fp);
    fwrite(author,64,1,fp);
    fwrite(&calc,4,1,fp);
    fwrite(&colorType,4,1,fp);
    fwrite(&white,4,1,fp);
    fwrite(&black,4,1,fp);
    RECT nr;
    nr.left=useLcd.left*tmp->Width/bmp->Width;
    nr.right=useLcd.right*tmp->Width/bmp->Width;
    nr.top=useLcd.top*tmp->Height/bmp->Height;
    nr.bottom=useLcd.bottom*tmp->Height/bmp->Height;
    fwrite(&nr,sizeof(RECT),1,fp);
    for (int i=0;i<80;i++)
    {
        nr.left=key[i].left*tmp->Width/bmp->Width;
        nr.right=key[i].right*tmp->Width/bmp->Width;
        nr.top=key[i].top*tmp->Height/bmp->Height;
        nr.bottom=key[i].bottom*tmp->Height/bmp->Height;
        fwrite(&nr,sizeof(RECT),1,fp);
    }
    Screen->Cursor=crHourGlass;
    WriteJPEG(fp,tmp,tmp->Width,tmp->Height,1);
    delete tmp;
    Screen->Cursor=crDefault;
    fclose(fp);
    modified=0;
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::Saveas1Click(TObject *Sender)
{
    TSaveDialog *dlog=new TSaveDialog(this);
    dlog->DefaultExt="skn";
    dlog->FileName=(filename[0])?filename:"untitled.skn";
    dlog->Options.Clear();
    dlog->Options << ofHideReadOnly << ofPathMustExist << ofOverwritePrompt;
    dlog->Filter="Virtual TI Skin Files (*.skn)|*.skn";
    dlog->FilterIndex=1;
    char cwd[256];
    getcwd(cwd,256);
    dlog->InitialDir=cwd;
    if (dlog->Execute())
    {
        strcpy(filename,dlog->FileName.c_str());
        Save1Click(Sender);
    }
    delete dlog;
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::Open1Click(TObject *Sender)
{
    if (modified)
    {
        int but=MessageBox(Handle,"Save changes to current file?",
            "File modified",MB_YESNOCANCEL|MB_ICONQUESTION);
        if (but==IDCANCEL)
            return;
        if (but==IDYES)
            Save1Click(Sender);
    }
    TOpenDialog *dlog=new TOpenDialog(this);
    dlog->DefaultExt="skn";
    dlog->FileName="*.skn";
    dlog->Options.Clear();
    dlog->Options << ofFileMustExist << ofPathMustExist;
    dlog->Filter="Virtual TI Skin Files (*.skn)|*.skn";
    dlog->FilterIndex=1;
    char cwd[256];
    getcwd(cwd,256);
    dlog->InitialDir=cwd;
    if (dlog->Execute())
    {
        FILE *fp=fopen(dlog->FileName.c_str(),"rb");
        if (!fp) return;
        char str[9];
        fread(str,8,1,fp);
        str[8]=0;
        //if (strcmp(str,"VTIv2.1 "))
        //{
        //    MessageBox(Handle,"File is not a valid Virtual TI skin",
        //        "Error",MB_OK|MB_ICONSTOP);
        //    delete dlog;
        //    return;
        //}
        fread(name,64,1,fp);
        if (!strcmp(str,"VTIv2.5 "))
        {
            fread(author,64,1,fp);
        }
        else
         MessageBox(NULL,"This skin does not have an author attached to it and is from the older Virtual TI Skin Creator. Might not yet contain certain new skin features","Alert",MB_OK);
        fread(&calc,4,1,fp);
        fread(&colorType,4,1,fp);
        fread(&white,4,1,fp);
        fread(&black,4,1,fp);
        fread(&useLcd,sizeof(RECT),1,fp);
        fread(key,sizeof(RECT),80,fp);
        if (bmp) { delete bmp; bmp=NULL; }
        bmp=new Graphics::TBitmap;
        Screen->Cursor=crHourGlass;
        ReadJPEG(fp,bmp);
        Screen->Cursor=crDefault;
        fclose(fp);
        strcpy(filename,dlog->FileName.c_str());
        Image->Picture->Bitmap=bmp;
        Fittowindow1->Checked=true;
        Halfsize1->Checked=false;
        Fullsize1->Checked=false;
        Doublesize1->Checked=false;
        int w=bmp->Width;
        int h=bmp->Height;
        if (w>640)
        {
            int nh=(h*640)/w;
            if (nh>480)
            {
                int nw=(w*480)/h;
                ClientWidth=nw;
                ClientHeight=480;
            }
            else
            {
                ClientWidth=640;
                ClientHeight=nh;
            }
        }
        else if (h>480)
        {
            int nw=(w*480)/h;
            if (nw>640)
            {
                int nh=(h*640)/w;
                ClientWidth=640;
                ClientHeight=nh;
            }
            else
            {
                ClientWidth=nw;
                ClientHeight=480;
            }
        }
        else
        {
            ClientWidth=bmp->Width;
            ClientHeight=bmp->Height;
        }
        Image->Align=alNone;
        PaintBox->Align=alNone;
        StatusBar1->Panels->Items[0]->Text="Select action from edit menu";
        if (name[0]!=0)
            Caption="Virtual TI Skin Creator - "+AnsiString(name);
        else
            Caption="Virtual TI Skin Creator - Untitled";
        if (author[0]!=0)
            Caption=Caption+" by "+AnsiString(author);
        Halfsize1->Enabled=true;
        Fittowindow1->Enabled=true;
        Fullsize1->Enabled=true;
        Doublesize1->Enabled=true;
        Properties1->Enabled=true;
        LCDposition1->Enabled=true;
        Keypositions1->Enabled=true;
        Gammacorrection1->Enabled=true;
        Brightnesscontrast1->Enabled=true;
        Crop1->Enabled=true;
        Save1->Enabled=true;
        Saveas1->Enabled=true;
        Test1->Enabled=true;
        lcd.left=-1; lcd.right=-1;
        lcd.top=-1; lcd.bottom=-1;
        modified=0;
    }
    delete dlog;
}
//---------------------------------------------------------------------------

void __fastcall TSkinEdit::Test1Click(TObject *Sender)
{
    if (!bmp) return;
    if (useLcd.left==-1) return;
    if (!SkinTestWnd->Image->Picture->Bitmap)
        SkinTestWnd->Image->Picture->Bitmap=new Graphics::TBitmap;
    int lcdW=(calc<=84)?96:(calc<89)?128:(calc==89)?160:240;
    int lcdH=(calc<89)?64:(calc==89)?100:128;
    SkinTestWnd->lcdBmp->Width=lcdW<<1;
    SkinTestWnd->lcdBmp->Height=lcdH<<1;
    SkinTestWnd->sLcdBmp->Width=lcdW;
    SkinTestWnd->sLcdBmp->Height=lcdH;
    SkinTestWnd->sLcdBmp->PixelFormat=pf24bit;
    SkinTestWnd->sLcdBmp->Canvas->Brush->Color=white;
    SkinTestWnd->sLcdBmp->Canvas->Brush->Style=bsSolid;
    TRect r;
    r.Left=0; r.Right=lcdW; r.Top=0; r.Bottom=lcdH;
    SkinTestWnd->sLcdBmp->Canvas->FillRect(r);
    SkinTestWnd->sLcdBmp->Canvas->Font->Name="MS Sans Serif";
    SkinTestWnd->sLcdBmp->Canvas->Font->Size=8;
    SkinTestWnd->sLcdBmp->Canvas->Font->Style.Clear();
    SkinTestWnd->sLcdBmp->Canvas->Font->Color=black;
    SkinTestWnd->sLcdBmp->Canvas->TextOut((lcdW>>1)-20,(lcdH>>1)-12,"Virtual TI");
    SkinTestWnd->sLcdBmp->Canvas->TextOut((lcdW>>1)-22,(lcdH>>1),"Skin Test");
    for (int y=0;y<lcdH;y++)
    {
        char *src=(char*)SkinTestWnd->sLcdBmp->ScanLine[y];
        char *dest1=(char*)SkinTestWnd->lcdBmp->ScanLine[y<<1];
        char *dest2=(char*)SkinTestWnd->lcdBmp->ScanLine[(y<<1)+1];
        for (int x=0;x<lcdW;x++)
        {
            dest1[0]=src[0]; dest1[1]=src[1]; dest1[2]=src[2];
            dest1[3]=src[0]; dest1[4]=src[1]; dest1[5]=src[2];
            dest2[0]=src[0]; dest2[1]=src[1]; dest2[2]=src[2];
            dest2[3]=src[0]; dest2[4]=src[1]; dest2[5]=src[2];
            dest1+=6; dest2+=6; src+=3;
        }
    }
    SkinTestWnd->Image->Picture->Bitmap->Width=(lcdW<<1)*bmp->Width/(useLcd.right-useLcd.left);
    SkinTestWnd->Image->Picture->Bitmap->Height=(lcdH<<1)*bmp->Height/(useLcd.bottom-useLcd.top);
    SkinTestWnd->lcd.Left=(useLcd.left)*(SkinTestWnd->Image->Picture->Bitmap->Width)/bmp->Width;
    SkinTestWnd->lcd.Top=(useLcd.top)*(SkinTestWnd->Image->Picture->Bitmap->Height)/bmp->Height;
    SkinTestWnd->lcd.Right=SkinTestWnd->lcd.Left+(lcdW<<1);
    SkinTestWnd->lcd.Bottom=SkinTestWnd->lcd.Top+(lcdH<<1);
    SkinTestWnd->Image->Picture->Bitmap->PixelFormat=pf24bit;
    r.Left=0; r.Right=SkinTestWnd->Image->Picture->Bitmap->Width;
    r.Top=0; r.Bottom=SkinTestWnd->Image->Picture->Bitmap->Height;
    SkinTestWnd->Image->Picture->Bitmap->Canvas->StretchDraw(r,bmp);
    SkinTestWnd->Image->Width=SkinTestWnd->Image->Picture->Bitmap->Width>>1;
    SkinTestWnd->Image->Height=SkinTestWnd->Image->Picture->Bitmap->Height>>1;
    SkinTestWnd->PaintBox->Left=SkinTestWnd->lcd.Left*SkinTestWnd->Image->Width/SkinTestWnd->Image->Picture->Bitmap->Width;
    SkinTestWnd->PaintBox->Width=lcdW;
    SkinTestWnd->PaintBox->Top=SkinTestWnd->lcd.Top*SkinTestWnd->Image->Height/SkinTestWnd->Image->Picture->Bitmap->Height;
    SkinTestWnd->PaintBox->Height=lcdH;
    SkinTestWnd->Show();
    SkinTestWnd->UpdateFloat();
    SkinTestWnd->enforceRatio=0;
    SkinTestWnd->ClientWidth=SkinTestWnd->Image->Picture->Bitmap->Width>>1;
    SkinTestWnd->ClientHeight=SkinTestWnd->Image->Picture->Bitmap->Height>>1;
    SkinTestWnd->ClientWidth=SkinTestWnd->Image->Picture->Bitmap->Width>>1;
    SkinTestWnd->ClientHeight=SkinTestWnd->Image->Picture->Bitmap->Height>>1;
    SkinTestWnd->Hide();
    SkinTestWnd->enforceRatio=1;
    SkinTestWnd->Normal1xview1->Checked=true;
    SkinTestWnd->Large2xview1->Checked=false;
    SkinTestWnd->UpdateFloat();
    SkinTestWnd->ShowModal();
}
//---------------------------------------------------------------------------



