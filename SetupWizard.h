//----------------------------------------------------------------------------
#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H
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
class TSetupWizardWnd : public TForm
{
__published:
    TPanel *NoROMIntroPanel;
    TPanel *Panel2;
    TImage *Image1;
    TLabel *Label1;
    TLabel *ROMWizardText1;
    TLabel *CopyROMText1;
    TLabel *Label4;
    TRadioButton *CopyROM1;
    TRadioButton *StartROMWizard1;
    TBevel *Bevel1;
    TButton *BackButton;
    TButton *NextButton;
    TButton *CancelButton;
    TPanel *CopyROMPanel;
    TLabel *Label3;
    TListBox *CopyROMInstalledList;
    TLabel *Label5;
    TButton *CopyROMAddButton;
    TButton *CopyROMRemoveButton;
    TButton *CopyROMWizardButton;
    TPanel *ROMPresentIntroPanel;
    TLabel *Label6;
    TLabel *Label7;
    TPanel *ROMWizardDonePanel;
    TLabel *Label8;
    TLabel *Label9;
    TListBox *WizardDoneInstalledROMList;
    TButton *WizardDoneAddButton;
    TButton *WizardDoneRemoveButton;
    TButton *WizardDoneRestartButton;
    TPanel *FinishedPanel;
    TLabel *Label10;
    TLabel *Label11;
    TLabel *Label12;
    TLabel *Label13;
    TLabel *Label14;
    TLabel *Label15;
    TLabel *Label16;
    TLabel *Label17;
    TLabel *Label18;
    TLabel *Label19;
    void __fastcall CopyROMText1Click(TObject *Sender);
    void __fastcall ROMWizardText1Click(TObject *Sender);
    
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall NextButtonClick(TObject *Sender);
    
    void __fastcall BackButtonClick(TObject *Sender);
    void __fastcall CopyROMWizardButtonClick(TObject *Sender);
    void __fastcall WizardDoneRestartButtonClick(TObject *Sender);
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall CopyROMAddButtonClick(TObject *Sender);
private:
    TPanel *activePanel,*romListPanel;
    int romPresent;
public:
	virtual __fastcall TSetupWizardWnd(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TSetupWizardWnd *SetupWizardWnd;
//----------------------------------------------------------------------------
#endif
