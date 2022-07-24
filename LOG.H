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
class TLogDlg : public TForm
{
__published:
	TButton *OKBtn;
    TListBox *List;
    TButton *Edit;
    TButton *Save;
    TButton *ClearBtn;
    TButton *StartBtn;
    void __fastcall EditClick(TObject *Sender);
    void __fastcall ClearBtnClick(TObject *Sender);
    void __fastcall StartBtnClick(TObject *Sender);
    void __fastcall OKBtnClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall SaveClick(TObject *Sender);
private:
public:
	virtual __fastcall TLogDlg(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TLogDlg *LogDlg;
//----------------------------------------------------------------------------

