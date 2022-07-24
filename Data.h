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
class TDataBreakDlg : public TForm
{
__published:
	TButton *OKBtn;
    TButton *AddBtn;
    TListBox *List;
    TLabel *Label1;
    TButton *Button1;
    void __fastcall AddBtnClick(TObject *Sender);
    void __fastcall OKBtnClick(TObject *Sender);
    void __fastcall Button1Click(TObject *Sender);
    void __fastcall ListDblClick(TObject *Sender);
private:
public:
	virtual __fastcall TDataBreakDlg(TComponent* AOwner);
    void __fastcall UpdateList();
};
//----------------------------------------------------------------------------
extern PACKAGE TDataBreakDlg *DataBreakDlg;
//----------------------------------------------------------------------------

