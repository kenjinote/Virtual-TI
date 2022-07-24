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
#include <Dialogs.hpp>
//----------------------------------------------------------------------------
class TSkinPropertiesDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
    TGroupBox *GroupBox1;
    TRadioButton *TI73;
    TRadioButton *TI82;
    TRadioButton *TI83;
    TRadioButton *TI83Plus;
    TRadioButton *TI85;
    TRadioButton *TI86;
    TRadioButton *TI89;
    TRadioButton *TI92;
    TRadioButton *TI92Plus;
    TLabel *Label1;
    TEdit *Name;
    TGroupBox *GroupBox2;
    TRadioButton *LowContrast;
    TRadioButton *HighContrast;
    TShape *Shape1;
    TShape *Shape2;
    TShape *Shape3;
    TShape *Shape4;
    TRadioButton *Custom;
    TShape *WhiteColor;
    TShape *BlackColor;
    TColorDialog *ColorDialog1;
    TLabel *Label2;
    TEdit *Author;
    void __fastcall OKBtnClick(TObject *Sender);
    void __fastcall CancelBtnClick(TObject *Sender);
    
    
    void __fastcall WhiteColorMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall BlackColorMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
private:
public:
	virtual __fastcall TSkinPropertiesDlg(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TSkinPropertiesDlg *SkinPropertiesDlg;
//----------------------------------------------------------------------------
#endif
