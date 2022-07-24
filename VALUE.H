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
class TNewValueDlg : public TForm
{
__published:
    TLabel *Label1;
    TEdit *Address;
    TButton *CancelBtn;
    TButton *OKBtn;
    void __fastcall OKBtnClick(TObject *Sender);
    void __fastcall CancelBtnClick(TObject *Sender);
private:
public:
	virtual __fastcall TNewValueDlg(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TNewValueDlg *NewValueDlg;
//----------------------------------------------------------------------------

