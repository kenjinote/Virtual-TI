//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("skin.res");
USEFORM("skinwnd.cpp", SkinEdit);
USEFORM("skinprop.cpp", SkinPropertiesDlg);
USEFORM("skinabt.cpp", AboutDlg);
USEFORM("skinkeys.cpp", KeyWnd);
USEUNIT("jpenquan.cpp");
USEUNIT("jfif.cpp");
USEUNIT("jpdecomp.cpp");
USEUNIT("jpdedu.cpp");
USEUNIT("JPDEHUFF.cpp");
USEUNIT("jpdequan.cpp");
USEUNIT("jpeg.cpp");
USEUNIT("jpegdeco.cpp");
USEUNIT("jpegenco.cpp");
USEUNIT("Jpegfmt.cpp");
USEUNIT("jpencomp.cpp");
USEUNIT("jpendu.cpp");
USEUNIT("jpenhuff.cpp");
USEUNIT("bitimage.cpp");
USEFORM("skintest.cpp", SkinTestWnd);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        Application->Initialize();
        Application->Title = "Virtual TI Skin Creator";
        Application->CreateForm(__classid(TSkinEdit), &SkinEdit);
        Application->CreateForm(__classid(TSkinPropertiesDlg), &SkinPropertiesDlg);
        Application->CreateForm(__classid(TAboutDlg), &AboutDlg);
        Application->CreateForm(__classid(TKeyWnd), &KeyWnd);
        Application->CreateForm(__classid(TSkinTestWnd), &SkinTestWnd);
        Application->Run();
    }
    catch (Exception &exception)
    {
        Application->ShowException(&exception);
    }
    return 0;
}
//---------------------------------------------------------------------------
