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
class TSearchDlg : public TForm
{
__published:
    TLabel *Label1;
    TEdit *Value;
    TButton *CancelBtn;
    TButton *OKBtn;
    TLabel *Label2;
    TComboBox *Size;
    void __fastcall OKBtnClick(TObject *Sender);
    void __fastcall CancelBtnClick(TObject *Sender);
private:
public:
	virtual __fastcall TSearchDlg(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TSearchDlg *SearchDlg;
//----------------------------------------------------------------------------

