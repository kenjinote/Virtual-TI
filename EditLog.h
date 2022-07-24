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
class TEditLogDlg : public TForm
{
__published:
	TButton *OKBtn;
    TListBox *List;
    TButton *AddBtn;
    TButton *DelBtn;
    void __fastcall AddBtnClick(TObject *Sender);
    
    void __fastcall ListDblClick(TObject *Sender);
    void __fastcall DelBtnClick(TObject *Sender);
private:
public:
	virtual __fastcall TEditLogDlg(TComponent* AOwner);
    void __fastcall UpdateList(); 
};
//----------------------------------------------------------------------------
extern PACKAGE TEditLogDlg *EditLogDlg;
//----------------------------------------------------------------------------

