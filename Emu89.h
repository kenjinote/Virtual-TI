//---------------------------------------------------------------------------
#ifndef Emu89H
#define Emu89H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TEmu89Wnd : public TForm
{
__published:	// IDE-managed Components
    TPaintBox *LCD;
private:	// User declarations
public:		// User declarations
    __fastcall TEmu89Wnd(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TEmu89Wnd *Emu89Wnd;
//---------------------------------------------------------------------------
#endif
