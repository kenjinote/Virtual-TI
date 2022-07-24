//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "VarWnd.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TVariableWnd *VariableWnd;
//---------------------------------------------------------------------------
__fastcall TVariableWnd::TVariableWnd(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
