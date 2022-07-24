//---------------------------------------------------------------------------
#ifndef LblWndH
#define LblWndH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TLabelWnd : public TForm
{
__published:	// IDE-managed Components
    TListView *ListView;
private:	// User declarations
public:		// User declarations
    __fastcall TLabelWnd(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TLabelWnd *LabelWnd;
//---------------------------------------------------------------------------
#endif
