//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "LblWnd.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TLabelWnd *LabelWnd;
//---------------------------------------------------------------------------
__fastcall TLabelWnd::TLabelWnd(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
