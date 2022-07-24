//---------------------------------------------------------------------------
#ifndef skinwndH
#define skinwndH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TSkinEdit : public TForm
{
__published:	// IDE-managed Components
    TMainMenu *MainMenu1;
    TMenuItem *File1;
    TMenuItem *New1;
    TMenuItem *Open1;
    TMenuItem *Save1;
    TMenuItem *Saveas1;
    TMenuItem *N1;
    TMenuItem *Exit1;
    TStatusBar *StatusBar1;
    TMenuItem *View1;
    TMenuItem *Halfsize1;
    TMenuItem *Fullsize1;
    TMenuItem *Doublesize1;
    TMenuItem *Fittowindow1;
    TScrollBox *ScrollBox1;
    TImage *Image;
    TMenuItem *Edit1;
    TMenuItem *Properties1;
    TMenuItem *N2;
    TMenuItem *LCDposition1;
    TMenuItem *Keypositions1;
    TMenuItem *N3;
    TMenuItem *Gammacorrection1;
    TMenuItem *Brightnesscontrast1;
    TMenuItem *Crop1;
    TMenuItem *Help1;
    TMenuItem *About1;
    TPaintBox *PaintBox;
    TMenuItem *N4;
    TMenuItem *Test1;
    void __fastcall Exit1Click(TObject *Sender);
    void __fastcall New1Click(TObject *Sender);
    void __fastcall Halfsize1Click(TObject *Sender);
    void __fastcall Fittowindow1Click(TObject *Sender);
    void __fastcall Fullsize1Click(TObject *Sender);
    void __fastcall Doublesize1Click(TObject *Sender);
    void __fastcall Properties1Click(TObject *Sender);
    void __fastcall About1Click(TObject *Sender);
    void __fastcall LCDposition1Click(TObject *Sender);
    void __fastcall Keypositions1Click(TObject *Sender);
    void __fastcall PaintBoxPaint(TObject *Sender);
    void __fastcall PaintBoxMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall PaintBoxMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall PaintBoxMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
    void __fastcall FormResize(TObject *Sender);
    
    void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
    void __fastcall Save1Click(TObject *Sender);
    void __fastcall Saveas1Click(TObject *Sender);
    void __fastcall Open1Click(TObject *Sender);
    void __fastcall Test1Click(TObject *Sender);
    
private:	// User declarations
    Graphics::TBitmap *bmp;
public:		// User declarations
    __fastcall TSkinEdit(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSkinEdit *SkinEdit;
//---------------------------------------------------------------------------
#endif
