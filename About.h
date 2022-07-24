//----------------------------------------------------------------------------
#ifndef __ABOUT_H__
#define __ABOUT_H__
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
class TAboutDlg : public TForm
{
__published:
	TButton *OKBtn;
    TLabel *Label2;
    TLabel *Label4;
    TLabel *Calc;
    TLabel *RAM;
    TLabel *ROM;
    TLabel *Flash;
    TLabel *Label5;
    TImage *Image1;
    TLabel *Label1;
    TLabel *Author;
    void __fastcall FormCreate(TObject *Sender);
    
    
    
private:
public:
	virtual __fastcall TAboutDlg(TComponent* AOwner);
    void __fastcall UpdateText();
};
//----------------------------------------------------------------------------
extern PACKAGE TAboutDlg *AboutDlg;
//----------------------------------------------------------------------------
#endif
