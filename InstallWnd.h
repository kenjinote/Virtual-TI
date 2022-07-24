//---------------------------------------------------------------------------
#ifndef InstallWndH
#define InstallWndH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <NMHttp.hpp>
#include <Psock.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TSetupWnd : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel1;
    TPanel *Panel2;
    TButton *BackButton;
    TButton *NextButton;
    TButton *CancelButton;
    TImage *Image1;
    TLabel *Label1;
    TCheckBox *SkinEdit;
    TCheckBox *Skin82;
    TCheckBox *Skin83;
    TCheckBox *Skin83P;
    TCheckBox *Skin85;
    TCheckBox *Skin86;
    TCheckBox *Skin89;
    TCheckBox *Skin92;
    TLabel *Label2;
    TLabel *Label3;
    TEdit *Folder;
    TButton *Button1;
    TPanel *StartPanel;
    TNMHTTP *HTTP;
    TPanel *InstallPanel;
    TLabel *Label4;
    TProgressBar *ProgressBar;
    TPanel *FinishPanel;
    TLabel *Label5;
    TLabel *Label6;
    void __fastcall NextButtonClick(TObject *Sender);
    void __fastcall HTTPFailure(CmdType Cmd);
    void __fastcall HTTPPacketRecvd(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
    void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TSetupWnd(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSetupWnd *SetupWnd;
//---------------------------------------------------------------------------
#endif
