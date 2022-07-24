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
class TGoToDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
    TLabel *Label1;
    TEdit *Address;
    void __fastcall OKBtnClick(TObject *Sender);
    void __fastcall CancelBtnClick(TObject *Sender);
    
private:
public:
	virtual __fastcall TGoToDlg(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TGoToDlg *GoToDlg;
//----------------------------------------------------------------------------
#endif
