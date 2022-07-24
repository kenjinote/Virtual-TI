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
class TEmuModeDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
    TLabel *Label1;
    TTabControl *Tab;
    TListBox *Calc;
    TPanel *Panel1;
    TPanel *Panel2;
    TPanel *Panel3;
    TPanel *Panel4;
    void __fastcall OKBtnClick(TObject *Sender);
    void __fastcall CancelBtnClick(TObject *Sender);
    void __fastcall TabChange(TObject *Sender);
private:
    void FillList();
public:
    int* romID;
	virtual __fastcall TEmuModeDlg(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TEmuModeDlg *EmuModeDlg;
//----------------------------------------------------------------------------

