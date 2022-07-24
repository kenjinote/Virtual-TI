//----------------------------------------------------------------------------
#ifndef OCBH
#define OCBH
//----------------------------------------------------------------------------
#include <System.hpp>
#include <Windows.hpp>
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Graphics.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Controls.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
//----------------------------------------------------------------------------
class TLinkTypeDialog : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
    TGroupBox *GroupBox1;
    TRadioButton *COM1;
    TRadioButton *COM2;
    TRadioButton *COM3;
    TRadioButton *COM4;
    TRadioButton *Ser1;
    TRadioButton *Ser2;
    TRadioButton *Ser3;
    TRadioButton *Ser4;
    TLabel *Label1;
    TComboBox *Calc;
    TRadioButton *Parallel;
    TLabel *Time;
    void __fastcall OKBtnClick(TObject *Sender);
    void __fastcall CalcChange(TObject *Sender);
private:
public:
	virtual __fastcall TLinkTypeDialog(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TLinkTypeDialog *LinkTypeDialog;
//----------------------------------------------------------------------------
#endif
