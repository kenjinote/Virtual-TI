//---------------------------------------------------------------------------
#ifndef DumpWndH
#define DumpWndH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TDumpWindow : public TForm
{
__published:	// IDE-managed Components
    TProgressBar *ProgressBar1;
    TLabel *Label1;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
public:		// User declarations
    __fastcall TDumpWindow(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TDumpWindow *DumpWindow;
//---------------------------------------------------------------------------
#endif
