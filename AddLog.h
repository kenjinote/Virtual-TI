//----------------------------------------------------------------------------
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
#include <ComCtrls.hpp>
//----------------------------------------------------------------------------
class TAddLogDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
    TPageControl *Page;
    TTabSheet *ValuePage;
    TTabSheet *BitPage;
    TComboBox *Size;
    TEdit *Addr;
    TEdit *LowAddr;
    TLabel *Label2;
    TEdit *BitAddr;
    TLabel *Label3;
    TCheckBox *Check7;
    TCheckBox *Check6;
    TCheckBox *Check5;
    TCheckBox *Check4;
    TCheckBox *Check3;
    TCheckBox *Check2;
    TCheckBox *Check1;
    TCheckBox *Check0;
    TLabel *Label4;
    TLabel *Label5;
    TLabel *Label6;
    TLabel *Label7;
    TLabel *Label8;
    TLabel *Label9;
    TLabel *Label10;
    TLabel *Label11;
    TLabel *Label12;
    TLabel *Label13;
    TLabel *Label14;
    TLabel *Label15;
    TLabel *Label16;
    TLabel *Label17;
    TLabel *Label18;
    TLabel *Label19;
    TLabel *Label20;
    TCheckBox *State7;
    TCheckBox *State6;
    TCheckBox *State5;
    TCheckBox *State4;
    TCheckBox *State3;
    TCheckBox *State2;
    TCheckBox *State1;
    TCheckBox *State0;
    TEdit *HighAddr;
    TGroupBox *GroupBox1;
    TRadioButton *Read;
    TRadioButton *Write;
    TRadioButton *Either;
    TGroupBox *GroupBox2;
    TRadioButton *Single;
    TRadioButton *Range;
    TLabel *Label1;
    TLabel *Label21;
    TCheckBox *IOPort1;
    TCheckBox *IOPort2;
    void __fastcall OKBtnClick(TObject *Sender);
    void __fastcall CancelBtnClick(TObject *Sender);
private:
public:
	virtual __fastcall TAddLogDlg(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TAddLogDlg *AddLogDlg;
//----------------------------------------------------------------------------

