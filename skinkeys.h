//---------------------------------------------------------------------------
#ifndef skinkeysH
#define skinkeysH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TKeyWnd : public TForm
{
__published:	// IDE-managed Components
    TListBox *KeyList;
    void __fastcall KeyListClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TKeyWnd(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TKeyWnd *KeyWnd;
//---------------------------------------------------------------------------
#endif
