//---------------------------------------------------------------------------
#ifndef DebugH
#define DebugH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>

#define DATABREAK_READ 1
#define DATABREAK_WRITE 2
#define DATABREAK_BOTH 3

#define LOG_VALUE 1
#define LOG_BIT 2

//---------------------------------------------------------------------------
class TDebugWnd : public TForm
{
__published:	// IDE-managed Components
    TGroupBox *GroupBox1;
    TListBox *Code;
    TGroupBox *GroupBox2;
    TLabel *D0;
    TLabel *D1;
    TLabel *D2;
    TLabel *D3;
    TLabel *D4;
    TLabel *D5;
    TLabel *D6;
    TLabel *D7;
    TLabel *A0;
    TLabel *A1;
    TLabel *A2;
    TLabel *A3;
    TLabel *A4;
    TLabel *A5;
    TLabel *A6;
    TLabel *A7;
    TLabel *SR;
    TLabel *TFlag;
    TLabel *SFlag;
    TLabel *MFlag;
    TLabel *IFlag;
    TLabel *XFlag;
    TLabel *NFlag;
    TLabel *ZFlag;
    TLabel *VFlag;
    TLabel *CFlag;
    TGroupBox *GroupBox3;
    TListBox *Stack;
    TGroupBox *GroupBox4;
    TListBox *Memory;
    TGroupBox *HandlesBox;
    TListBox *Handles;
    TMainMenu *MainMenu1;
    TMenuItem *Debug1;
    TMenuItem *Run1;
    TMenuItem *Step1;
    TMenuItem *Stepover1;
    TMenuItem *Setbreakpoint1;
    TMenuItem *Setdatabreakpoint1;
    TMenuItem *Breakpoints1;
    TMenuItem *N2;
    TMenuItem *Clearallbreakpoints1;
    TPopupMenu *CodePopupMenu1;
    TMenuItem *Gotoaddress1;
    TMenuItem *Setbreakpoint2;
    TMenuItem *SetPC1;
    TLabel *PC;
    TMenuItem *Break1;
    TListBox *MemAddr;
    TListBox *MemCol0;
    TListBox *MemCol1;
    TListBox *MemCol2;
    TListBox *MemCol3;
    TListBox *MemCol4;
    TListBox *MemCol5;
    TListBox *MemCol6;
    TListBox *MemCol7;
    TListBox *MemText;
    TPopupMenu *MemoryPopupMenu;
    TMenuItem *MemGoTo;
    TMenuItem *Setbreakpointonwrite1;
    TMenuItem *GotoPC1;
    TMenuItem *Setbreakpointonprogramentry1;
    TPopupMenu *HandlesPopupMenu;
    TMenuItem *Viewmemory1;
    TMenuItem *Viewdisassembly1;
    TMenuItem *Searchforvalue1;
    TMenuItem *Log1;
    TMenuItem *Editloggedaddresses1;
    TMenuItem *Viewlog1;
    TMenuItem *Clearall1;
    TMenuItem *N1;
    TMenuItem *Enablelogging1;
    TMenuItem *Loglinkportsend1;
    TMenuItem *Loglinkportreceive1;
    TMenuItem *Runtocursor1;
    TMenuItem *Setbreakpointontrapexception1;
    TMenuItem *Source1;
    TMenuItem *Enablesourceleveldebuggingifavailable1;
    TMenuItem *Loaddebuginformation1;
    TMenuItem *N3;
    TMenuItem *Nofilesloaded1;
    TMenuItem *View1;
    TMenuItem *ROMLabels1;
    TMenuItem *ROMVariables1;
    TMenuItem *N4;
    TMenuItem *Nodebuginformation1;
    void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
    
    void __fastcall Step1Click(TObject *Sender);
    void __fastcall Run1Click(TObject *Sender);
    void __fastcall CodeKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall Gotoaddress1Click(TObject *Sender);
    void __fastcall MemGotoClick(TObject *Sender);
    void __fastcall SetPC1Click(TObject *Sender);
    void __fastcall Break1Click(TObject *Sender);
    void __fastcall MemCol0KeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall MemCol0Click(TObject *Sender);
    void __fastcall MemCol1Click(TObject *Sender);
    void __fastcall MemCol2Click(TObject *Sender);
    void __fastcall MemCol3Click(TObject *Sender);
    void __fastcall MemCol4Click(TObject *Sender);
    void __fastcall MemCol5Click(TObject *Sender);
    void __fastcall MemCol6Click(TObject *Sender);
    void __fastcall MemCol7Click(TObject *Sender);
    void __fastcall MemCol0DblClick(TObject *Sender);
    void __fastcall MemCol1DblClick(TObject *Sender);
    void __fastcall MemCol2DblClick(TObject *Sender);
    void __fastcall MemCol3DblClick(TObject *Sender);
    void __fastcall MemCol4DblClick(TObject *Sender);
    void __fastcall MemCol5DblClick(TObject *Sender);
    void __fastcall MemCol6DblClick(TObject *Sender);
    void __fastcall MemCol7DblClick(TObject *Sender);
    void __fastcall Setbreakpoint1Click(TObject *Sender);
    void __fastcall CodeDrawItem(TWinControl *Control, int Index,
          TRect &Rect, TOwnerDrawState State);
    void __fastcall GotoPC1Click(TObject *Sender);
    void __fastcall Clearallbreakpoints1Click(TObject *Sender);
    void __fastcall Setbreakpointonprogramentry1Click(TObject *Sender);
    void __fastcall HandlesDblClick(TObject *Sender);
    void __fastcall Viewmemory1Click(TObject *Sender);
    void __fastcall Viewdisassembly1Click(TObject *Sender);
    void __fastcall Setdatabreakpoint1Click(TObject *Sender);
    void __fastcall Searchforvalue1Click(TObject *Sender);
    void __fastcall Stepover1Click(TObject *Sender);
    void __fastcall D0Click(TObject *Sender);
    void __fastcall D1Click(TObject *Sender);
    void __fastcall D2Click(TObject *Sender);
    void __fastcall D3Click(TObject *Sender);
    void __fastcall D4Click(TObject *Sender);
    void __fastcall D5Click(TObject *Sender);
    void __fastcall D6Click(TObject *Sender);
    void __fastcall D7Click(TObject *Sender);
    void __fastcall A0Click(TObject *Sender);
    void __fastcall A1Click(TObject *Sender);
    void __fastcall A2Click(TObject *Sender);
    void __fastcall A3Click(TObject *Sender);
    void __fastcall A4Click(TObject *Sender);
    void __fastcall A5Click(TObject *Sender);
    void __fastcall A6Click(TObject *Sender);
    void __fastcall A7Click(TObject *Sender);
    void __fastcall PCClick(TObject *Sender);
    void __fastcall SRClick(TObject *Sender);
    void __fastcall TFlagClick(TObject *Sender);
    void __fastcall SFlagClick(TObject *Sender);
    void __fastcall MFlagClick(TObject *Sender);
    void __fastcall IFlagClick(TObject *Sender);
    void __fastcall XFlagClick(TObject *Sender);
    void __fastcall NFlagClick(TObject *Sender);
    void __fastcall ZFlagClick(TObject *Sender);
    void __fastcall VFlagClick(TObject *Sender);
    void __fastcall CFlagClick(TObject *Sender);
    void __fastcall Viewlog1Click(TObject *Sender);
    void __fastcall Editloggedaddresses1Click(TObject *Sender);
    void __fastcall Enablelogging1Click(TObject *Sender);
    void __fastcall Clearall1Click(TObject *Sender);
    void __fastcall Loglinkportsend1Click(TObject *Sender);
    void __fastcall Loglinkportreceive1Click(TObject *Sender);
    
    
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall Setbreakpointontrapexception1Click(TObject *Sender);
    void __fastcall Runtocursor1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TDebugWnd(TComponent* Owner);
    void __fastcall Update();
    void __fastcall UpdateMem();
    void __fastcall MemDblClick(int col,int selindx);
};
//---------------------------------------------------------------------------
extern PACKAGE TDebugWnd *DebugWnd;

extern int CheckAddr(int addr,int size,int type,int v);
extern void DataBreak();

extern void UpdateDebugCheckEnable();

extern int debugCheckEnable;
extern int debugLogLinkSend,debugLogLinkRecv;
extern int debugCodeBreakCount,debugDataBreakCount;
extern int debugProgBreak;
//---------------------------------------------------------------------------
#endif
