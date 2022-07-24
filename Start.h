//---------------------------------------------------------------------------
#ifndef StartH
#define StartH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TStartWnd : public TForm
{
__published:	// IDE-managed Components
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Message;
    TBevel *Bevel1;
private:	// User declarations
public:		// User declarations
    __fastcall TStartWnd(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TStartWnd *StartWnd;
//---------------------------------------------------------------------------
#endif
