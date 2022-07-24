//---------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop
#include "rom.h"
#include "RomWizard.h"
#include "WizardBmp.h"

#include "SetupWizard.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
TSetupWizardWnd *SetupWizardWnd;
//---------------------------------------------------------------------
__fastcall TSetupWizardWnd::TSetupWizardWnd(TComponent* AOwner)
	: TForm(AOwner)
{
    AnalyzeROMs();
    if (romImageCount==0)
    {
        ROMPresentIntroPanel->Hide();
        CopyROMPanel->Hide();
        FinishedPanel->Hide();
        ROMWizardDonePanel->Hide();
        NoROMIntroPanel->Show();
        BackButton->Enabled=false;
        NextButton->Caption="Next >";
        activePanel=NoROMIntroPanel;
    }
    else
    {
        CopyROMPanel->Hide();
        FinishedPanel->Hide();
        ROMWizardDonePanel->Hide();
        NoROMIntroPanel->Hide();
        ROMPresentIntroPanel->Show();
        BackButton->Enabled=false;
        NextButton->Caption="Next >";
        activePanel=ROMPresentIntroPanel;
    }
}
//---------------------------------------------------------------------
void __fastcall TSetupWizardWnd::CopyROMText1Click(TObject *Sender)
{
    CopyROM1->Checked=true;
    StartROMWizard1->Checked=false;
}
//---------------------------------------------------------------------------

void __fastcall TSetupWizardWnd::ROMWizardText1Click(TObject *Sender)
{
    CopyROM1->Checked=false;
    StartROMWizard1->Checked=true;
}
//---------------------------------------------------------------------------


void __fastcall TSetupWizardWnd::CancelButtonClick(TObject *Sender)
{
    ModalResult=2;
}
//---------------------------------------------------------------------------

void __fastcall TSetupWizardWnd::NextButtonClick(TObject *Sender)
{
    if (activePanel==NoROMIntroPanel)
    {
        if (StartROMWizard1->Checked==true)
        {
            Hide();
            TROMWizardWnd *rw=new TROMWizardWnd(NULL);
            if (rw->ShowModal()!=1)
            {
                delete rw;
                Show();
                return;
            }
            delete rw;
            NoROMIntroPanel->Hide();
            ROMWizardDonePanel->Show();
            Show();
            WizardDoneInstalledROMList->Items->Clear();
            Application->ProcessMessages();
            Screen->Cursor=crHourGlass;
            AnalyzeROMs();
            for (int i=0;i<romImageCount;i++)
            {
                WizardDoneInstalledROMList->Items->Add(
                    romImage[i].name);
            }
            Screen->Cursor=crDefault;
            activePanel=ROMWizardDonePanel;
            romListPanel=ROMWizardDonePanel;
            if (romImageCount)
                NextButton->Enabled=true;
            else
                NextButton->Enabled=false;
        }
        else if (CopyROM1->Checked==true)
        {
            NoROMIntroPanel->Hide();
            CopyROMPanel->Show();
            CopyROMInstalledList->Items->Clear();
            Application->ProcessMessages();
            Screen->Cursor=crHourGlass;
            AnalyzeROMs();
            for (int i=0;i<romImageCount;i++)
            {
                CopyROMInstalledList->Items->Add(
                    romImage[i].name);
            }
            Screen->Cursor=crDefault;
            activePanel=CopyROMPanel;
            romListPanel=CopyROMPanel;
            if (romImageCount)
                NextButton->Enabled=true;
            else
                NextButton->Enabled=false;
        }
        BackButton->Enabled=true;
        romPresent=0;
    }
    else if (activePanel==ROMPresentIntroPanel)
    {
        ROMPresentIntroPanel->Hide();
        CopyROMPanel->Show();
        CopyROMInstalledList->Items->Clear();
        Application->ProcessMessages();
        Screen->Cursor=crHourGlass;
        AnalyzeROMs();
        for (int i=0;i<romImageCount;i++)
        {
            CopyROMInstalledList->Items->Add(
                romImage[i].name);
        }
        Screen->Cursor=crDefault;
        activePanel=CopyROMPanel;
        romListPanel=CopyROMPanel;
        BackButton->Enabled=true;
        romPresent=1;
    }
    else if (activePanel==ROMWizardDonePanel)
    {
        ROMWizardDonePanel->Hide();
        FinishedPanel->Show();
        activePanel=FinishedPanel;
        NextButton->Caption="Finish";
    }
    else if (activePanel==CopyROMPanel)
    {
        CopyROMPanel->Hide();
        FinishedPanel->Show();
        activePanel=FinishedPanel;
        NextButton->Caption="Finish";
    }
    else if (activePanel==FinishedPanel)
    {
        chdir(initPath);
        ModalResult=1;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSetupWizardWnd::BackButtonClick(TObject *Sender)
{
    if (activePanel==CopyROMPanel)
    {
        if (romPresent)
        {
            CopyROMPanel->Hide();
            ROMPresentIntroPanel->Show();
            activePanel=ROMPresentIntroPanel;
        }
        else
        {
            CopyROMPanel->Hide();
            NoROMIntroPanel->Show();
            activePanel=NoROMIntroPanel;
        }
        BackButton->Enabled=false;
        NextButton->Enabled=true;
    }
    else if (activePanel==ROMWizardDonePanel)
    {
        ROMWizardDonePanel->Hide();
        NoROMIntroPanel->Show();
        activePanel=NoROMIntroPanel;
        BackButton->Enabled=false;
        NextButton->Enabled=true;
    }
    else if (activePanel==FinishedPanel)
    {
        FinishedPanel->Hide();
        romListPanel->Show();
        activePanel=romListPanel;
        NextButton->Caption="Next >";
    }
}
//---------------------------------------------------------------------------

void __fastcall TSetupWizardWnd::CopyROMWizardButtonClick(TObject *Sender)
{
    Hide();
    TROMWizardWnd *rw=new TROMWizardWnd(NULL);
    if (rw->ShowModal()==1)
    {
        delete rw;
        Show();
        CopyROMInstalledList->Items->Clear();
        Application->ProcessMessages();
        Screen->Cursor=crHourGlass;
        AnalyzeROMs();
        for (int i=0;i<romImageCount;i++)
        {
            CopyROMInstalledList->Items->Add(
                romImage[i].name);
        }
        Screen->Cursor=crDefault;
        if (romImageCount)
            NextButton->Enabled=true;
        else
            NextButton->Enabled=false;
    }
    else
    {
        delete rw;
        Show();
    }
}
//---------------------------------------------------------------------------

void __fastcall TSetupWizardWnd::WizardDoneRestartButtonClick(
      TObject *Sender)
{
    Hide();
    TROMWizardWnd *rw=new TROMWizardWnd(NULL);
    if (rw->ShowModal()==1)
    {
        delete rw;
        Show();
        WizardDoneInstalledROMList->Items->Clear();
        Application->ProcessMessages();
        Screen->Cursor=crHourGlass;
        AnalyzeROMs();
        for (int i=0;i<romImageCount;i++)
        {
            WizardDoneInstalledROMList->Items->Add(
                romImage[i].name);
        }
        Screen->Cursor=crDefault;
        if (romImageCount)
            NextButton->Enabled=true;
        else
            NextButton->Enabled=false;
    }
    else
    {
        delete rw;
        Show();
    }
}
//---------------------------------------------------------------------------

void __fastcall TSetupWizardWnd::FormCreate(TObject *Sender)
{
    Image1->Picture=WizardBitmap->Bmp->Picture;
}
//---------------------------------------------------------------------------

void __fastcall TSetupWizardWnd::CopyROMAddButtonClick(TObject *Sender)
{
    TOpenDialog *dlog=new TOpenDialog(this);
    dlog->DefaultExt="rom";
    dlog->FileName="*.rom;*.bin;*.dmp;*.tib";
    dlog->FilterIndex=1;
    dlog->Options.Clear();
    dlog->Options << ofFileMustExist << ofPathMustExist << ofAllowMultiSelect;
    dlog->Filter="ROM images (*.rom;*.bin;*.dmp;*.tib)|*.rom;*.bin;*.dmp;*.tib";
    char cwd[256];
    getcwd(cwd,256);
    dlog->InitialDir=cwd;
    if (dlog->Execute())
    {
        Screen->Cursor=crHourGlass;
        for (int i=0;i<dlog->Files->Count;i++)
        {
            char fname[MAXFILE],fext[MAXEXT],path[256];
            FILE *fp=fopen(dlog->Files->Strings[i].c_str(),"rb");
            fnsplit(dlog->Files->Strings[i].c_str(),NULL,
                NULL,fname,fext);
            if (initPath[0])
            {
                sprintf(path,"%s%s%s%s",initPath,(initPath[strlen(
                    initPath)-1]=='\\')?"":"\\",fname,fext);
            }
            else
                sprintf(path,"%s%s",fname,fext);
            FILE *out=fopen(path,"wb");
            char buf[1024];
            while (!feof(fp))
            {
                int n=fread(buf,1,1024,fp);
                fwrite(buf,1,n,out);
            }
            fclose(fp);
            fclose(out);
        }
        Screen->Cursor=crDefault;
        CopyROMInstalledList->Items->Clear();
        Application->ProcessMessages();
        Screen->Cursor=crHourGlass;
        AnalyzeROMs();
        for (int i=0;i<romImageCount;i++)
        {
            CopyROMInstalledList->Items->Add(
                romImage[i].name);
        }
        Screen->Cursor=crDefault;
        if (romImageCount)
            NextButton->Enabled=true;
        else
            NextButton->Enabled=false;
    }
    delete dlog;
}
//---------------------------------------------------------------------------

