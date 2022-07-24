//---------------------------------------------------------------------------
#ifndef DumpH
#define DumpH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class DumpThread : public TThread
{
private:
    int comPort;
protected:
    void __fastcall Execute();
public:
    __fastcall DumpThread(int port,bool CreateSuspended);
    void __fastcall UpdateProgress();
    void __fastcall UpdateLabel();
    void __fastcall CloseWindow();
};
//---------------------------------------------------------------------------
#endif
