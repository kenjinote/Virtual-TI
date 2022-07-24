//---------------------------------------------------------------------------
#ifndef EmuThreadH
#define EmuThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class EmuThread : public TThread
{
private:
protected:
    void __fastcall Execute();
public:
    __fastcall EmuThread(bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
