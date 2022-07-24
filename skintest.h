//---------------------------------------------------------------------------
#ifndef skintestH
#define skintestH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
class TSkinTestWnd : public TForm
{
__published:	// IDE-managed Components
    TImage *Image;
    TPaintBox *PaintBox;
    TPopupMenu *PopupMenu1;
    TMenuItem *Normal1xview1;
    TMenuItem *Large2xview1;
    TMenuItem *N1;
    TMenuItem *Close1;
    void __fastcall PaintBoxPaint(TObject *Sender);
    void __fastcall Normal1xview1Click(TObject *Sender);
    void __fastcall Large2xview1Click(TObject *Sender);
    void __fastcall ImageMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
    void __fastcall ImageMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall ImageMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall PaintBoxMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
    void __fastcall Close1Click(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);
private:	// User declarations
public:		// User declarations
    int enforceRatio;
    Graphics::TBitmap *lcdBmp,*sLcdBmp;
    TRect lcd;
    __fastcall TSkinTestWnd(TComponent* Owner);
    void __fastcall WMSizing(TMessage &Msg);
    void __fastcall UpdateFloat();
BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_SIZING,TMessage,WMSizing)
END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TSkinTestWnd *SkinTestWnd;
//---------------------------------------------------------------------------
#endif
