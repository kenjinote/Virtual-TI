//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <stdio.h>

#include "Log.h"
#include "EditLog.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TLogDlg *LogDlg;
//---------------------------------------------------------------------
__fastcall TLogDlg::TLogDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
extern int debugLogEnable;
void __fastcall TLogDlg::EditClick(TObject *Sender)
{
    EditLogDlg->UpdateList();
    EditLogDlg->ShowModal();
    if (debugLogEnable)
        StartBtn->Caption="Stop Logging";
    else
        StartBtn->Caption="Start Logging";
}
//---------------------------------------------------------------------------

void __fastcall TLogDlg::ClearBtnClick(TObject *Sender)
{
    List->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TLogDlg::StartBtnClick(TObject *Sender)
{
    debugLogEnable=!debugLogEnable;    
    if (debugLogEnable)
        StartBtn->Caption="Stop Logging";
    else
        StartBtn->Caption="Start Logging";
}
//---------------------------------------------------------------------------

void __fastcall TLogDlg::OKBtnClick(TObject *Sender)
{
    ModalResult=1;    
}
//---------------------------------------------------------------------------

void __fastcall TLogDlg::FormShow(TObject *Sender)
{
    if (debugLogEnable)
        StartBtn->Caption="Stop Logging";
    else
        StartBtn->Caption="Start Logging";
}
//---------------------------------------------------------------------------
extern int useInitPath;
extern char initPath[256];
void __fastcall TLogDlg::SaveClick(TObject *Sender)
{
    TSaveDialog *dlog=new TSaveDialog(this);
    dlog->DefaultExt="txt";
    dlog->FileName="log.txt";
    dlog->Options.Clear();
    dlog->Filter="Text files (*.txt)|*.txt";
    dlog->FilterIndex=1;
    if (useInitPath)
    {
        useInitPath=0;
        dlog->InitialDir=initPath;
    }
    if (dlog->Execute())
    {
        FILE *fp=fopen(dlog->FileName.c_str(),"w");
        if (fp)
        {
            for (int i=0;i<List->Items->Count;i++)
                fprintf(fp,"%s\n",List->Items->Strings[i].c_str());
            fclose(fp);
        }
    }
    delete dlog;
}
//---------------------------------------------------------------------------

