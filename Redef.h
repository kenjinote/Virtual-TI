//---------------------------------------------------------------------------
#ifndef RedefH
#define RedefH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TRedefineWnd : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel1;
    TLabel *Key;
private:	// User declarations
public:		// User declarations
    __fastcall TRedefineWnd(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TRedefineWnd *RedefineWnd;
//---------------------------------------------------------------------------
#endif
