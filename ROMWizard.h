//----------------------------------------------------------------------------
#ifndef ROMWIZARD_H
#define ROMWIZARD_H
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
class TROMWizardWnd : public TForm
{
__published:
    TPanel *Panel2;
    TImage *Image1;
    TBevel *Bevel1;
    TButton *BackButton;
    TButton *NextButton;
    TButton *CancelButton;
    TPanel *StartPanel;
    TLabel *Label1;
    TRadioButton *TI82None;
    TRadioButton *TI82Ash;
    TRadioButton *TI83None;
    TRadioButton *TI85None;
    TRadioButton *TI85Usgard;
    TRadioButton *TI85ZShell;
    TRadioButton *TI86;
    TRadioButton *TI89;
    TRadioButton *TI92FargoII;
    TRadioButton *TI92None;
    TRadioButton *TI92IIFargoII;
    TRadioButton *TI92Plus;
    TPanel *LinkTypePanel;
    TLabel *Label3;
    TRadioButton *GraphLink1;
    TRadioButton *GraphLink2;
    TRadioButton *GraphLink3;
    TRadioButton *GraphLink4;
    TRadioButton *Serial1;
    TRadioButton *Serial2;
    TRadioButton *Serial3;
    TRadioButton *Serial4;
    TRadioButton *Parallel1;
    TRadioButton *Parallel2;
    TPanel *Send89Panel;
    TLabel *Send89Text1;
    TLabel *Send89Text2;
    TProgressBar *Send89Progress;
    TLabel *Get89TimeText;
    TPanel *FinishedPanel;
    TLabel *Label4;
    TLabel *Label5;
    TRadioButton *VirtualLink;
    TPanel *Send86Panel;
    TLabel *Send86Text1;
    TLabel *Send86Text2;
    TLabel *Get86TimeText;
    TProgressBar *Send86Progress;
    TRadioButton *TI83AShell;
    TRadioButton *TI83Plus;
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall NextButtonClick(TObject *Sender);
    void __fastcall BackButtonClick(TObject *Sender);
    void __fastcall FormDestroy(TObject *Sender);
    void __fastcall CalcClick(TObject *Sender);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall FormCreate(TObject *Sender);
private:
    TPanel *activePanel;
    TProgressBar *progress;
    TLabel *transferText,*timeText;
    int sending,linkType,stage,pos,left,blockStart,romSize;
    char calcName[4];
    unsigned char *buf;
public:
	virtual __fastcall TROMWizardWnd(TComponent* AOwner);
    void __fastcall OnIdle(TObject *,bool&);
};
//----------------------------------------------------------------------------
extern PACKAGE TROMWizardWnd *ROMWizardWnd;
//----------------------------------------------------------------------------
#endif
