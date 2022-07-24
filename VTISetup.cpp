//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("VTISetup.res");
USEFORM("InstallWnd.cpp", SetupWnd);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        Application->Initialize();
        Application->Title = "Virtual TI Setup";
        Application->CreateForm(__classid(TSetupWnd), &SetupWnd);
        Application->Run();
    }
    catch (Exception &exception)
    {
        Application->ShowException(&exception);
    }
    return 0;
}
//---------------------------------------------------------------------------
