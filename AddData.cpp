//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "AddData.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TAddDataBreakDlg *AddDataBreakDlg;
//---------------------------------------------------------------------
__fastcall TAddDataBreakDlg::TAddDataBreakDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------



void __fastcall TAddDataBreakDlg::OKBtnClick(TObject *Sender)
{
    ModalResult=1;    
}
//---------------------------------------------------------------------------

void __fastcall TAddDataBreakDlg::CancelBtnClick(TObject *Sender)
{
    ModalResult=2;
}
//---------------------------------------------------------------------------

void __fastcall TAddDataBreakDlg::LowAddrChange(TObject *Sender)
{
    Range->Checked=true;
    Single->Checked=false;
}
//---------------------------------------------------------------------------

void __fastcall TAddDataBreakDlg::HighAddrChange(TObject *Sender)
{
    Range->Checked=true;
    Single->Checked=false;
}
//---------------------------------------------------------------------------

void __fastcall TAddDataBreakDlg::SizeChange(TObject *Sender)
{
    Single->Checked=true;
    Range->Checked=false;
}
//---------------------------------------------------------------------------

void __fastcall TAddDataBreakDlg::AddrChange(TObject *Sender)
{
    Single->Checked=true;
    Range->Checked=false;    
}
//---------------------------------------------------------------------------

