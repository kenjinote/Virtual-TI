//---------------------------------------------------------------------------
#ifndef WizardBmpH
#define WizardBmpH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TWizardBitmap : public TForm
{
__published:	// IDE-managed Components
    TImage *Bmp;
    TImage *vti1;
    TImage *vti1a;
    TImage *vti2;
    TImage *vti2a;
private:	// User declarations
public:		// User declarations
    __fastcall TWizardBitmap(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TWizardBitmap *WizardBitmap;
//---------------------------------------------------------------------------
#endif
