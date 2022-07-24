//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "DumpWnd.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TDumpWindow *DumpWindow;
int dumpCancel=0;
//---------------------------------------------------------------------------
__fastcall TDumpWindow::TDumpWindow(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TDumpWindow::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    dumpCancel=1;
}
//---------------------------------------------------------------------------
