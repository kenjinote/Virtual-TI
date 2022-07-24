//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "WizardBmp.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TWizardBitmap *WizardBitmap;
//---------------------------------------------------------------------------
__fastcall TWizardBitmap::TWizardBitmap(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
