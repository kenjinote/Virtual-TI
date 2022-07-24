//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <stdio.h>
#include <dir.h>
#include <process.h>
#include <registry.hpp>
#include <shlwapi.h> 

#include "InstallWnd.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSetupWnd *SetupWnd;
TPanel *ActivePanel;
char filename[256];
bool downloadInProgress=false;
bool cancelled=false;
int completed=0;

#include <shlobj.h>
typedef IShellLink *LPSHELLLINK; // Reproduce interface pointer
typedef IPersistFile *LPPERSISTFILE;

BOOL CreateShortcut( AnsiString pszPath, AnsiString pszArguments, AnsiString pszLocation,
    AnsiString pszWorkingDir, int nCmdShow )
{
    LPSHELLLINK pShellLink;
    HRESULT hrCoInit; // OLE installation result
    HRESULT hr; // Result of object methods
    hrCoInit = CoInitialize( NULL ); // Initialize OLE
    hr = CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
        IID_IShellLink, (void**)&pShellLink );
    if( SUCCEEDED( hr ) )
    {
        LPPERSISTFILE pPersistFile;
        // Get PersistFile interface ---------------------------------------
        if(SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile,(void**)&pPersistFile)))
        {
            WORD wsz[ MAX_PATH ];
            // Set program path ----------------------------------------------
            hr = pShellLink->SetPath(pszPath.c_str());
            // Set arguments -------------------------------------------------
            if( SUCCEEDED( hr ) )
                hr = pShellLink->SetArguments(pszArguments.c_str() );
            // Set working directory -----------------------------------------
            if( SUCCEEDED( hr ) )
                hr = pShellLink->SetWorkingDirectory(pszWorkingDir.c_str() );
            // Set ShowCommand -----------------------------------------------
            if( SUCCEEDED( hr ) )
                hr = pShellLink->SetShowCmd(nCmdShow );
            // Set path where shortcut is to be saved ------------------------
            if( SUCCEEDED( hr ) )
            {
                MultiByteToWideChar( CP_ACP, 0, pszLocation.c_str(), -1, (wchar_t*)wsz, MAX_PATH);
                hr = pPersistFile->Save( (wchar_t*)wsz, TRUE );
                hr = pPersistFile->SaveCompleted( (wchar_t*)wsz );
            }
            hr = pPersistFile->Release( );
        }
        pShellLink->Release( );
    }
    // If OLE was initialized here, then we also have to uninitialize
    // it here.
    if( SUCCEEDED( hrCoInit ) ) CoUninitialize();
    return SUCCEEDED( hr );
}

//---------------------------------------------------------------------------
__fastcall TSetupWnd::TSetupWnd(TComponent* Owner)
    : TForm(Owner)
{
    ActivePanel=StartPanel;
}
//---------------------------------------------------------------------------
int DownloadFile(char *n)
{
    downloadInProgress=true;
    SetupWnd->HTTP->Body="";
    try
    {
        char url[256];
        sprintf(url,"http://rusty.acz.org/%s",n);
        SetupWnd->HTTP->Get(url);
        if (SetupWnd->HTTP->ReplyNumber!=200)
            throw "error";
    }
    catch(...)
    {
        if (cancelled)
        {
            downloadInProgress=false;
            SetupWnd->Close();
            return 0;
        }
        char url[256];
        sprintf(url,"http://rustyw.dhs.org/VTISetup/%s",n);
        try
        {
            SetupWnd->HTTP->Get(url);
            if (SetupWnd->HTTP->ReplyNumber!=200)
                throw "error";
        }
        catch(...)
        {
            downloadInProgress=false;
            if (cancelled)
            {
                SetupWnd->Close();
                return 0;
            }
            MessageBox(NULL,"Could not download file from server.","Error",MB_OK);
            SetupWnd->Close();
            return 0;
        }
    }
    downloadInProgress=false;
    return 1;
}

int Decode(char *n,int size)
{
    char filename[256];
    if (SetupWnd->Folder->Text.c_str()[SetupWnd->Folder->Text.Length()-1]=='\\')
        sprintf(filename,"%s%s",SetupWnd->Folder->Text.c_str(),n);
    else
        sprintf(filename,"%s\\%s",SetupWnd->Folder->Text.c_str(),n);
    FILE *fp=fopen(filename,"wb");
    if (!fp)
    {
        MessageBox(NULL,"Could not write file to directory.","Error",MB_OK);
        SetupWnd->Close();
        return 0;
    }
    if (SetupWnd->HTTP->Body.Length()<size)
    {
        MessageBox(NULL,"Could not download file from server.","Error",MB_OK);
        SetupWnd->Close();
        return 0;
    }
    for (int i=0,j=0;j<size;i++)
    {
        int c=SetupWnd->HTTP->Body.c_str()[i];
        if (c==-1)
            c=SetupWnd->HTTP->Body.c_str()[++i];
        else
            c--;
        fputc(c,fp);
        j++;
    }
    fclose(fp);
    return 1;
}

void __fastcall TSetupWnd::NextButtonClick(TObject *Sender)
{
    if (ActivePanel==StartPanel)
    {
        ActivePanel=InstallPanel;
        StartPanel->Hide();
        InstallPanel->Show();
        NextButton->Enabled=false;
        if (!DownloadFile("VTISetup.dat.enc")) return;
        mkdir(Folder->Text.c_str());
        if (!Decode("VTISetup.dat",9*4*2)) return;
        chdir(SetupWnd->Folder->Text.c_str());
        FILE *fp=fopen("VTISetup.dat","rb");
        int sizes[9],finalSize[9];
        fread(sizes,4,9,fp);
        fread(finalSize,4,9,fp);
        fclose(fp);                                       
        remove("VTISetup.dat");
        int total=sizes[0];
        if (SkinEdit->Checked) total+=sizes[1];
        if (Skin82->Checked) total+=sizes[2];
        if (Skin83->Checked) total+=sizes[3];
        if (Skin83P->Checked) total+=sizes[4];
        if (Skin85->Checked) total+=sizes[5];
        if (Skin86->Checked) total+=sizes[6];
        if (Skin89->Checked) total+=sizes[7];
        if (Skin92->Checked) total+=sizes[8];
        ProgressBar->Max=total;
        char totalStr[32];
        sprintf(totalStr,"0k of %dk completed",total/1024);
        Label6->Caption=totalStr;
        completed=0;
        if (!DownloadFile("vti.exe.enc")) return;
        if (!Decode("vti.exe",finalSize[0])) return;
        completed+=sizes[0];
        if (SkinEdit->Checked)
        {
            Label4->Caption="Downloading Skin Editor...";
            if (!DownloadFile("skin.exe.enc")) return;
            if (!Decode("skin.exe",finalSize[1])) return;
            completed+=sizes[1];
        }
        if (Skin82->Checked)
        {
            Label4->Caption="Downloading TI-82 Skin...";
            if (!DownloadFile("ti82.skn.enc")) return;
            if (!Decode("ti82.skn",finalSize[2])) return;
            completed+=sizes[2];
        }
        if (Skin83->Checked)
        {
            Label4->Caption="Downloading TI-83 Skin...";
            if (!DownloadFile("ti83.skn.enc")) return;
            if (!Decode("ti83.skn",finalSize[3])) return;
            completed+=sizes[3];
        }
        if (Skin83P->Checked)
        {
            Label4->Caption="Downloading TI-83 Plus Skin...";
            if (!DownloadFile("ti83plus.skn.enc")) return;
            if (!Decode("ti83plus.skn",finalSize[4])) return;
            completed+=sizes[4];
        }
        if (Skin85->Checked)
        {
            Label4->Caption="Downloading TI-85 Skin...";
            if (!DownloadFile("ti85.skn.enc")) return;
            if (!Decode("ti85.skn",finalSize[5])) return;
            completed+=sizes[5];
        }
        if (Skin86->Checked)
        {
            Label4->Caption="Downloading TI-86 Skin...";
            if (!DownloadFile("ti86.skn.enc")) return;
            if (!Decode("ti86.skn",finalSize[6])) return;
            completed+=sizes[6];
        }
        if (Skin89->Checked)
        {
            Label4->Caption="Downloading TI-89 Skin...";
            if (!DownloadFile("ti89.skn.enc")) return;
            if (!Decode("ti89.skn",finalSize[7])) return;
            completed+=sizes[7];
        }
        if (Skin92->Checked)
        {
            Label4->Caption="Downloading TI-92 Skin...";
            if (!DownloadFile("ti92.skn.enc")) return;
            if (!Decode("ti92.skn",finalSize[8])) return;
            completed+=sizes[8];
        }
        TRegistry *reg=new TRegistry;
        reg->RootKey=HKEY_CURRENT_USER;
        reg->OpenKey("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",false);
        AnsiString startFolder=reg->ReadString("Start Menu");
        delete reg;
        startFolder+="\\Programs\\Virtual TI\\";
        mkdir(startFolder.c_str());
        if (SetupWnd->Folder->Text.c_str()[SetupWnd->Folder->Text.Length()-1]=='\\')
            CreateShortcut(Folder->Text+"vti.exe","",startFolder+"Virtual TI.lnk",
                startFolder,SW_SHOW);
        else
            CreateShortcut(Folder->Text+"\\vti.exe","",startFolder+"Virtual TI.lnk",
                startFolder,SW_SHOW);
        if (SkinEdit->Checked)
        {
            if (SetupWnd->Folder->Text.c_str()[SetupWnd->Folder->Text.Length()-1]=='\\')
                CreateShortcut(Folder->Text+"skin.exe","",startFolder+"Skin Editor.lnk",
                    startFolder,SW_SHOW);
            else
                CreateShortcut(Folder->Text+"\\skin.exe","",startFolder+"Skin Editor.lnk",
                    startFolder,SW_SHOW);
        }
        CreateShortcut("http://rusty.acz.org/bugreport.html","",startFolder+"Bug Report.lnk",
            startFolder,SW_SHOW);
        InstallPanel->Hide();
        FinishPanel->Show();
        ActivePanel=FinishPanel;
        NextButton->Caption="Finish";
        NextButton->Enabled=true;
    }
    else if (ActivePanel==FinishPanel)
    {
        char filename[256];
        if (SetupWnd->Folder->Text.c_str()[SetupWnd->Folder->Text.Length()-1]=='\\')
            sprintf(filename,"%svti.exe",SetupWnd->Folder->Text.c_str());
        else
            sprintf(filename,"%s\\vti.exe",SetupWnd->Folder->Text.c_str());
        spawnl(P_NOWAIT,filename,NULL,NULL);
        Close();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSetupWnd::HTTPFailure(CmdType Cmd)
{
    throw "error";
}
//---------------------------------------------------------------------------
void __fastcall TSetupWnd::HTTPPacketRecvd(TObject *Sender)
{
    ProgressBar->Position=completed+HTTP->BytesRecvd;
    char str[32];
    sprintf(str,"%dk of %dk completed",(completed+HTTP->BytesRecvd)/1024,
        ProgressBar->Max/1024);
    Label6->Caption=str;
}
//---------------------------------------------------------------------------
void __fastcall TSetupWnd::CancelButtonClick(TObject *Sender)
{
    if (downloadInProgress)
    {
        cancelled=true;
        HTTP->Abort();
    }
    else
        Close();
}
//---------------------------------------------------------------------------
void __fastcall TSetupWnd::FormCloseQuery(TObject *Sender, bool &CanClose)
{
    CanClose=true;
    if (downloadInProgress)
    {
        cancelled=true;
        HTTP->Abort();
    }
}

int CALLBACK BrowseCallback(HWND hWnd,UINT msg,LPARAM,LPARAM)
{
    if (msg==BFFM_INITIALIZED)
        SendMessage(hWnd,BFFM_SETSELECTION,TRUE,(LONG)(SetupWnd->Folder->Text.c_str()));
    return 0;
}
//---------------------------------------------------------------------------
void __fastcall TSetupWnd::Button1Click(TObject *Sender)
{
    BROWSEINFO bi;
    char path[MAX_PATH];
    strcpy(path,Folder->Text.c_str());
    bi.hwndOwner=Handle;
    bi.pidlRoot=NULL;
    bi.pszDisplayName=path;
    bi.lpszTitle="Select the folder in which Virtual TI will be installed:";
    bi.ulFlags=BIF_RETURNONLYFSDIRS;
    bi.lpfn=BrowseCallback;
    LPITEMIDLIST id=SHBrowseForFolder(&bi);
    SHGetPathFromIDList(id,path);
    if (id==NULL) return;
    LPMALLOC m;
    SHGetMalloc(&m);
    m->Free(id);
    Folder->Text=path;
}
//---------------------------------------------------------------------------

