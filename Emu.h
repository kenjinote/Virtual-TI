//---------------------------------------------------------------------------
#ifndef EmuH
#define EmuH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>

#define WM_HELLO WM_USER+101
#define WM_GOODBYE WM_USER+102
#define WM_ENABLE_LINK WM_USER+110
#define WM_DISABLE_LINK WM_USER+111
#define WM_SEND_BUFFER WM_USER+120
#define WM_ENTER_DEBUG WM_USER+130
#define WM_EXIT_DEBUG WM_USER+131
//---------------------------------------------------------------------------
class TEmuWnd : public TForm
{
__published:	// IDE-managed Components
    TPaintBox *LCD;
    TPopupMenu *PopupMenu1;
    TMenuItem *SendFile1;
    TMenuItem *Reset1;
    TMenuItem *EnableCOMLink1;
    TMenuItem *COM1;
    TMenuItem *COM2;
    TMenuItem *COM3;
    TMenuItem *COM4;
    TMenuItem *About1;
    TMenuItem *Setemulationmode1;
    TMenuItem *Enterdebugmode1;
    TMenuItem *N2;
    TMenuItem *Loadstateimage1;
    TMenuItem *Savestateimage1;
    TMenuItem *N3;
    TMenuItem *Setprogramentrybreakpoint1;
    TMenuItem *Exitwithoutsavingstate1;
    TMenuItem *DumpROM1;
    TMenuItem *Restricttoactualspeed1;
    TMenuItem *Large2xcalculatorimage1;
    TMenuItem *Emulationoptions1;
    TMenuItem *Receiveoptions1;
    TMenuItem *Transferindividually1;
    TMenuItem *Transferasgroup1;
    TMenuItem *Screenshot1;
    TMenuItem *LCDonlyblackwhiteBMP1;
    TMenuItem *LCDonlytruecolorsBMP1;
    TMenuItem *CalculatorimageBMP1;
    TMenuItem *N1;
    TMenuItem *LCDonlyblackwhiteClipboard1;
    TMenuItem *LCDonlytruecolorsClipboard1;
    TMenuItem *CalculatorimageClipboard1;
    TMenuItem *VirtualLink1;
    TMenuItem *N5;
    TMenuItem *Normal1xview1;
    TMenuItem *N6;
    TMenuItem *N4;
    TMenuItem *SerialLinkonCOM11;
    TMenuItem *SerialLinkonCOM21;
    TMenuItem *SerialLinkonCOM31;
    TMenuItem *SerialLinkonCOM41;
    TMenuItem *Currentskin1;
    TMenuItem *None1;
    TPaintBox *Image;
    TMenuItem *Fullscreenview1;
    TMenuItem *Exitemulator1;
    TMenuItem *N7;
    TMenuItem *Showpercentageofactualspeed1;
    TMenuItem *Stayontop1;
    TMenuItem *N8;
    TMenuItem *ParallelLinkonLPT11;
    TMenuItem *ParallelLinkonLPT21;
    TMenuItem *Exitwithoutsavingstate2;
    TMenuItem *N9;
    TMenuItem *N4FPSLCDUpdates1;
    TMenuItem *N10HzLCDUpdates1;
    TMenuItem *N30HzLCDUpdates1;
    TMenuItem *N10;
    TMenuItem *None2;
    TMenuItem *Enablesound1;
    TLabel *SpeedIndic;
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall LCDPaint(TObject *Sender);

    void __fastcall LCDMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall SendFile1Click(TObject *Sender);
    void __fastcall Reset1Click(TObject *Sender);
    void __fastcall FormDestroy(TObject *Sender);
    void __fastcall COM1Click(TObject *Sender);
    void __fastcall COM2Click(TObject *Sender);
    void __fastcall COM3Click(TObject *Sender);
    void __fastcall COM4Click(TObject *Sender);
    void __fastcall FormDeactivate(TObject *Sender);
    void __fastcall About1Click(TObject *Sender);
    void __fastcall Enterdebugmode1Click(TObject *Sender);
    void __fastcall Setemulationmode1Click(TObject *Sender);
    void __fastcall Setprogramentrybreakpoint1Click(TObject *Sender);
    void __fastcall Savestateimage1Click(TObject *Sender);
    void __fastcall Loadstateimage1Click(TObject *Sender);
    void __fastcall Exitwithoutsavingstate1Click(TObject *Sender);
    void __fastcall UseNewClick(TObject *Sender);
    void __fastcall DumpROM1Click(TObject *Sender);
    void __fastcall Restricttoactualspeed1Click(TObject *Sender);
    void __fastcall Redefinekeys1Click(TObject *Sender);
    void __fastcall Large2xcalculatorimage1Click(TObject *Sender);
    void __fastcall Showpercentageofactualspeed1Click(TObject *Sender);
    

    
    void __fastcall LCDonlyblackwhiteBMP1Click(TObject *Sender);
    void __fastcall LCDonlytruecolorsBMP1Click(TObject *Sender);

    void __fastcall LCDonlyblackwhiteClipboard1Click(TObject *Sender);
    void __fastcall LCDonlytruecolorsClipboard1Click(TObject *Sender);
    void __fastcall CalculatorimageBMP1Click(TObject *Sender);
    void __fastcall CalculatorimageClipboard1Click(TObject *Sender);
    void __fastcall ShowLCDonly1Click(TObject *Sender);
    void __fastcall VirtualLink1Click(TObject *Sender);
    void __fastcall Normal1xview1Click(TObject *Sender);
    void __fastcall Usecalculatorkeypad1Click(TObject *Sender);
    void __fastcall Transferindividually1Click(TObject *Sender);
    void __fastcall Transferasgroup1Click(TObject *Sender);
    
    void __fastcall SerialLinkonCOM11Click(TObject *Sender);
    void __fastcall SerialLinkonCOM21Click(TObject *Sender);
    void __fastcall SerialLinkonCOM31Click(TObject *Sender);
    void __fastcall SerialLinkonCOM41Click(TObject *Sender);

    void __fastcall OnKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall OnKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall None1Click(TObject *Sender);
    void __fastcall ImagePaint(TObject *Sender);
    void __fastcall Fullscreenview1Click(TObject *Sender);
    void __fastcall Exitemulator1Click(TObject *Sender);
    void __fastcall Stayontop1Click(TObject *Sender);
    void __fastcall ImageMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall ParallelLinkonLPT11Click(TObject *Sender);
    void __fastcall ParallelLinkonLPT21Click(TObject *Sender);
    void __fastcall Exitwithoutsavingstate2Click(TObject *Sender);
    void __fastcall N4FPSLCDUpdates1Click(TObject *Sender);
    void __fastcall N10HzLCDUpdates1Click(TObject *Sender);
    void __fastcall N30HzLCDUpdates1Click(TObject *Sender);
    
    void __fastcall None2Click(TObject *Sender);
    void __fastcall Enablesound1Click(TObject *Sender);
    void __fastcall ImageMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall ImageMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
    void __fastcall LCDMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y);
    void __fastcall FormResize(TObject *Sender);
private:	// User declarations
    int keyHeld,genTab,enforceRatio,nonFullSkin;
    Graphics::TBitmap *calcImage;
public:		// User declarations
    Graphics::TBitmap *bmp;
    __fastcall TEmuWnd(TComponent* Owner);
    void __fastcall OnIdle(TObject *,bool&);
    void __fastcall Make();
    void __fastcall DoLink();
    void __fastcall OnSkinSelect(TObject *Sender);
protected:
    void __fastcall WMHello(TMessage &Msg);
    void __fastcall WMGoodbye(TMessage &Msg);
    void __fastcall WMEnableLink(TMessage &Msg);
    void __fastcall WMDisableLink(TMessage &Msg);
    void __fastcall WMSendBuffer(TMessage &Msg);
    void __fastcall WMEnterDebug(TMessage &Msg);
    void __fastcall WMExitDebug(TMessage &Msg);
    void __fastcall FormKeyDown(TMessage &Msg);
    void __fastcall FormKeyUp(TMessage &Msg);
    void __fastcall WMSizing(TMessage &Msg);
    void __fastcall WMDropFiles(TMessage &Msg);
BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_HELLO,TMessage,WMHello)
    MESSAGE_HANDLER(WM_GOODBYE,TMessage,WMGoodbye)
    MESSAGE_HANDLER(WM_ENABLE_LINK,TMessage,WMEnableLink)
    MESSAGE_HANDLER(WM_DISABLE_LINK,TMessage,WMDisableLink)
    MESSAGE_HANDLER(WM_SEND_BUFFER,TMessage,WMSendBuffer)
    MESSAGE_HANDLER(WM_ENTER_DEBUG,TMessage,WMEnterDebug)
    MESSAGE_HANDLER(WM_EXIT_DEBUG,TMessage,WMExitDebug)
    MESSAGE_HANDLER(WM_KEYDOWN,TMessage,FormKeyDown)
    MESSAGE_HANDLER(WM_KEYUP,TMessage,FormKeyUp)
    MESSAGE_HANDLER(WM_SIZING,TMessage,WMSizing)
    MESSAGE_HANDLER(WM_DROPFILES,TMessage,WMDropFiles)
END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TEmuWnd *EmuWnd;
extern HWND otherWnd;

extern int cl_getport();
extern void cl_setport(int b);
extern int openLink,graphLink;

//---------------------------------------------------------------------------
#endif
