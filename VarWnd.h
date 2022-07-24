//---------------------------------------------------------------------------
#ifndef VarWndH
#define VarWndH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TVariableWnd : public TForm
{
__published:	// IDE-managed Components
    TListView *ListView;
private:	// User declarations
public:		// User declarations
    __fastcall TVariableWnd(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TVariableWnd *VariableWnd;
//---------------------------------------------------------------------------
#endif
