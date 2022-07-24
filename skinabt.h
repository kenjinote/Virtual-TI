//----------------------------------------------------------------------------
#ifndef __SKINABT_H__
#define __SKINABT_H__
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
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label5;
private:
public:
	virtual __fastcall TAboutDlg(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TAboutDlg *AboutDlg;
//----------------------------------------------------------------------------
#endif
