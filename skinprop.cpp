//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "skinprop.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TSkinPropertiesDlg *SkinPropertiesDlg;
//---------------------------------------------------------------------
__fastcall TSkinPropertiesDlg::TSkinPropertiesDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------

void __fastcall TSkinPropertiesDlg::OKBtnClick(TObject *Sender)
{
    ModalResult=1;    
}
//---------------------------------------------------------------------------

void __fastcall TSkinPropertiesDlg::CancelBtnClick(TObject *Sender)
{
    ModalResult=2;    
}

void __fastcall TSkinPropertiesDlg::WhiteColorMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    ColorDialog1->Color=WhiteColor->Brush->Color;
    if (ColorDialog1->Execute())
    {
        WhiteColor->Brush->Color=ColorDialog1->Color;
        Custom->Checked=true;
        LowContrast->Checked=false;
        HighContrast->Checked=false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSkinPropertiesDlg::BlackColorMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    ColorDialog1->Color=BlackColor->Brush->Color;
    if (ColorDialog1->Execute())
    {
        BlackColor->Brush->Color=ColorDialog1->Color;
        Custom->Checked=true;
        LowContrast->Checked=false;
        HighContrast->Checked=false;
    }
}
//---------------------------------------------------------------------------

