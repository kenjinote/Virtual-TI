//----------------------------------------------------------------------------
#ifndef TRAP_H
#define TRAP_H
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
class TTrapDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
    TGroupBox *GroupBox1;
    TGroupBox *GroupBox2;
    TGroupBox *GroupBox3;
    TCheckBox *AddressError;
    TCheckBox *IllegalInstruction;
    TCheckBox *DivZero;
    TCheckBox *CHK;
    TCheckBox *TRAPV;
    TCheckBox *PrivilegeViolation;
    TCheckBox *Trace;
    TCheckBox *Line1010;
    TCheckBox *Line1111;
    TCheckBox *AutoInt1;
    TCheckBox *AutoInt2;
    TCheckBox *AutoInt4;
    TCheckBox *AutoInt5;
    TCheckBox *AutoInt6;
    TCheckBox *AutoInt7;
    TCheckBox *Trap0;
    TCheckBox *Trap1;
    TCheckBox *Trap2;
    TCheckBox *Trap3;
    TCheckBox *Trap4;
    TCheckBox *Trap5;
    TCheckBox *Trap6;
    TCheckBox *Trap7;
    TCheckBox *Trap8;
    TCheckBox *Trap9;
    TCheckBox *Trap10;
    TCheckBox *Trap11;
    TCheckBox *Trap12;
    TCheckBox *Trap13;
    TCheckBox *Trap14;
    TCheckBox *Trap15;
private:
public:
	virtual __fastcall TTrapDlg(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TTrapDlg *TrapDlg;
//----------------------------------------------------------------------------
#endif
