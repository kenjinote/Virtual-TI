//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Redef.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TRedefineWnd *RedefineWnd;
//---------------------------------------------------------------------------
__fastcall TRedefineWnd::TRedefineWnd(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
