//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "skinkeys.h"
#include "skinwnd.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TKeyWnd *KeyWnd;
extern int curKey;
//---------------------------------------------------------------------------
__fastcall TKeyWnd::TKeyWnd(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TKeyWnd::KeyListClick(TObject *Sender)
{
    if (KeyList->ItemIndex<0) return;
    curKey=KeyList->ItemIndex;
    SkinEdit->Image->Repaint();
    SkinEdit->PaintBox->Repaint();
}
//---------------------------------------------------------------------------

