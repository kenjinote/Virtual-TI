//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Emu89.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TEmu89Wnd *Emu89Wnd;
extern void kup(int k);
extern void kdown(int k);
//---------------------------------------------------------------------------
__fastcall TEmu89Wnd::TEmu89Wnd(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
