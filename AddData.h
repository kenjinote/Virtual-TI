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
//----------------------------------------------------------------------------
class TAddDataBreakDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
    TGroupBox *GroupBox1;
    TRadioButton *Read;
    TRadioButton *Write;
    TRadioButton *Either;
    TGroupBox *GroupBox2;
    TRadioButton *Single;
    TRadioButton *Range;
    TComboBox *Size;
    TLabel *Label1;
    TEdit *Addr;
    TEdit *LowAddr;
    TEdit *HighAddr;
    TLabel *Label2;
    TCheckBox *IOPort;
    void __fastcall OKBtnClick(TObject *Sender);
    void __fastcall CancelBtnClick(TObject *Sender);
    void __fastcall LowAddrChange(TObject *Sender);
    void __fastcall HighAddrChange(TObject *Sender);
    void __fastcall SizeChange(TObject *Sender);
    void __fastcall AddrChange(TObject *Sender);
private:
public:
	virtual __fastcall TAddDataBreakDlg(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TAddDataBreakDlg *AddDataBreakDlg;
//----------------------------------------------------------------------------

