#ifndef __CALCHW_H__
#define __CALCHW_H__

#undef setmem

typedef unsigned char UBYTE;

class CalcHW
{
public:
    int kbmatrix[10],onheld;
    int outRed,outWhite,getError;
    CalcHW() {}
    virtual void EnableDebug()=0;
    virtual void DisableDebug()=0;
    virtual int getmem(int addr)=0;
    virtual int getmem_word(int addr)=0;
    virtual int getmem_dword(int addr)=0;
    virtual void setmem(int addr,int v)=0;
    virtual void setmem_word(int addr,int v)=0;
    virtual void setmem_dword(int addr,int v)=0;
    virtual int readmem(int addr)=0;
    virtual int readmem_word(int addr)=0;
    virtual int readmem_dword(int addr)=0;
    virtual void writemem(int addr,int v)=0;
    virtual void writemem_word(int addr,int v)=0;
    virtual void writemem_dword(int addr,int v)=0;
    virtual int readport(int port)=0;
    virtual void writeport(int port,int v)=0;
    virtual void Execute()=0;
    virtual void OneInstruction()=0;
    virtual void Reset()=0;
    virtual void OnUp()=0;
    virtual void OnDown()=0;
    virtual void KeyUp(int r,int c)=0;
    virtual void KeyDown(int r,int c)=0;
    virtual void AlphaUp()=0;
    virtual void AlphaDown()=0;
    virtual void KBInt()=0;
    virtual int* GetKeyMap()=0;
    virtual int GetKeyMat(int r,int c)=0;
    virtual int GetKeyAlphaMat(int r,int c)=0;
    virtual int GetKeyID(int i)=0;
    virtual void SaveState(char *fn)=0;
    virtual void LoadState(char *fn)=0;
    virtual int SendByte(int c)=0;
    virtual int GetByte(int &c)=0;
    virtual int SendReady()=0;
    virtual int SendBuffer(int c)=0;
    virtual int CheckBuffer()=0;
    virtual int GetBuffer(int &c)=0;
    virtual int SendFile(char *fn,int more=0)=0;
    virtual int ReceiveFile()=0;
};

class TI89: public CalcHW
{
    int ioReadByte(int addr);
    void ioWriteByte(int addr,int v);
    void intRomWriteByte(int addr,int v);
    void extRomWriteByte(int addr,int v);
    int memprotect,ram128,timer,timerInit,int0Count;
    int io0Bit7,io0Bit2;
    int kbmask,ramWrap;
    int comError,transflag,transbyte,transnotready;
    int recvflag,recvbyte;
    int romWriteReady,romWritePhase;
    int directAccess;
public:
    TI89();
    virtual void EnableDebug();
    virtual void DisableDebug();
    virtual int getmem(int addr);
    virtual int getmem_word(int addr);
    virtual int getmem_dword(int addr);
    virtual void setmem(int addr,int v);
    virtual void setmem_word(int addr,int v);
    virtual void setmem_dword(int addr,int v);
    virtual int readmem(int addr);
    virtual int readmem_word(int addr);
    virtual int readmem_dword(int addr);
    virtual void writemem(int addr,int v);
    virtual void writemem_word(int addr,int v);
    virtual void writemem_dword(int addr,int v);
    virtual int readport(int port);
    virtual void writeport(int port,int v);
    virtual void Execute();
    virtual void OneInstruction();
    virtual void Reset();
    virtual void OnUp();
    virtual void OnDown();
    virtual void KeyUp(int r,int c);
    virtual void KeyDown(int r,int c);
    virtual void AlphaUp();
    virtual void AlphaDown();
    virtual void KBInt();
    virtual int* GetKeyMap();
    virtual int GetKeyMat(int r,int c);
    virtual int GetKeyAlphaMat(int r,int c);
    virtual int GetKeyID(int i);
    virtual void SaveState(char *fn);
    virtual void LoadState(char *fn);
    virtual int SendByte(int c);
    virtual int GetByte(int &c);
    virtual int SendReady();
    virtual int SendBuffer(int c);
    virtual int CheckBuffer();
    virtual int GetBuffer(int &c);
    virtual int SendFile(char *fn,int more=0);
    virtual int ReceiveFile();
};

class TI92: public CalcHW
{
    int ioReadByte(int addr);
    void ioWriteByte(int addr,int v);
    void intRomWriteByte(int addr,int v);
    void extRomWriteByte(int addr,int v);
    int memprotect,ram128,timer,timerInit,int0Count;
    int io0Bit7,io0Bit2;
    int kbmask,ramWrap;
    int comError,transflag,transbyte,transnotready;
    int recvflag,recvbyte;
    int romWriteReady,romWritePhase;
    int debugEnabled;
    int directAccess;
public:
    TI92();
    virtual void EnableDebug();
    virtual void DisableDebug();
    virtual int getmem(int addr);
    virtual int getmem_word(int addr);
    virtual int getmem_dword(int addr);
    virtual void setmem(int addr,int v);
    virtual void setmem_word(int addr,int v);
    virtual void setmem_dword(int addr,int v);
    virtual int readmem(int addr);
    virtual int readmem_word(int addr);
    virtual int readmem_dword(int addr);
    virtual void writemem(int addr,int v);
    virtual void writemem_word(int addr,int v);
    virtual void writemem_dword(int addr,int v);
    virtual int readport(int port);
    virtual void writeport(int port,int v);
    virtual void Execute();
    virtual void OneInstruction();
    virtual void Reset();
    virtual void OnUp();
    virtual void OnDown();
    virtual void KeyUp(int r,int c);
    virtual void KeyDown(int r,int c);
    virtual void AlphaUp();
    virtual void AlphaDown();
    virtual void KBInt();
    virtual int* GetKeyMap();
    virtual int GetKeyMat(int r,int c);
    virtual int GetKeyAlphaMat(int r,int c);
    virtual int GetKeyID(int i);
    virtual void SaveState(char *fn);
    virtual void LoadState(char *fn);
    virtual int SendByte(int c);
    virtual int GetByte(int &c);
    virtual int SendReady();
    virtual int SendBuffer(int c);
    virtual int CheckBuffer();
    virtual int GetBuffer(int &c);
    virtual int SendFile(char *fn,int more=0);
    virtual int ReceiveFile();
};

class TI92Plus: public CalcHW
{
    int ioReadByte(int addr);
    void ioWriteByte(int addr,int v);
    void intRomWriteByte(int addr,int v);
    void extRomWriteByte(int addr,int v);
    int memprotect,ram128,timer,timerInit,int0Count;
    int io0Bit7,io0Bit2;
    int kbmask,ramWrap;
    int comError,transflag,transbyte,transnotready;
    int recvflag,recvbyte;
    int romWriteReady,romWritePhase;
    int directAccess;
public:
    TI92Plus();
    virtual void EnableDebug();
    virtual void DisableDebug();
    virtual int getmem(int addr);
    virtual int getmem_word(int addr);
    virtual int getmem_dword(int addr);
    virtual void setmem(int addr,int v);
    virtual void setmem_word(int addr,int v);
    virtual void setmem_dword(int addr,int v);
    virtual int readmem(int addr);
    virtual int readmem_word(int addr);
    virtual int readmem_dword(int addr);
    virtual void writemem(int addr,int v);
    virtual void writemem_word(int addr,int v);
    virtual void writemem_dword(int addr,int v);
    virtual int readport(int port);
    virtual void writeport(int port,int v);
    virtual void Execute();
    virtual void OneInstruction();
    virtual void Reset();
    virtual void OnUp();
    virtual void OnDown();
    virtual void KeyUp(int r,int c);
    virtual void KeyDown(int r,int c);
    virtual void AlphaUp();
    virtual void AlphaDown();
    virtual void KBInt();
    virtual int* GetKeyMap();
    virtual int GetKeyMat(int r,int c);
    virtual int GetKeyAlphaMat(int r,int c);
    virtual int GetKeyID(int i);
    virtual void SaveState(char *fn);
    virtual void LoadState(char *fn);
    virtual int SendByte(int c);
    virtual int GetByte(int &c);
    virtual int SendReady();
    virtual int SendBuffer(int c);
    virtual int CheckBuffer();
    virtual int GetBuffer(int &c);
    virtual int SendFile(char *fn,int more=0);
    virtual int ReceiveFile();
};

class TI82: public CalcHW
{
    int kbmask;
    int port3Left,intr;
    int curCol,curRow,dir,justSet,mode;
    unsigned char vidMem[768];
    int redWire,whiteWire;
    int maskTimer,maskOn,timerSpeed;
    int port2;
public:
    TI82();
    virtual void EnableDebug();
    virtual void DisableDebug();
    virtual int getmem(int addr);
    virtual int getmem_word(int addr);
    virtual int getmem_dword(int addr);
    virtual void setmem(int addr,int v);
    virtual void setmem_word(int addr,int v);
    virtual void setmem_dword(int addr,int v);
    virtual int readmem(int addr);
    virtual int readmem_word(int addr);
    virtual int readmem_dword(int addr);
    virtual void writemem(int addr,int v);
    virtual void writemem_word(int addr,int v);
    virtual void writemem_dword(int addr,int v);
    virtual int readport(int port);
    virtual void writeport(int port,int v);
    virtual void Execute();
    virtual void OneInstruction();
    virtual void Reset();
    virtual void OnUp();
    virtual void OnDown();
    virtual void KeyUp(int r,int c);
    virtual void KeyDown(int r,int c);
    virtual void AlphaUp();
    virtual void AlphaDown();
    virtual void KBInt();
    virtual int* GetKeyMap();
    virtual int GetKeyMat(int r,int c);
    virtual int GetKeyAlphaMat(int r,int c);
    virtual int GetKeyID(int i);
    virtual void SaveState(char *fn);
    virtual void LoadState(char *fn);
    virtual int SendByte(int c);
    virtual int GetByte(int &c);
    virtual int SendReady();
    virtual int SendBuffer(int c);
    virtual int CheckBuffer();
    virtual int GetBuffer(int &c);
    virtual int SendFile(char *fn,int more=0);
    virtual int ReceiveFile();
};

class TI83: public CalcHW
{
    int kbmask;
    int port3Left,intr;
    int curCol,curRow,dir,justSet,mode;
    unsigned char vidMem[768];
    int redWire,whiteWire;
    int maskTimer,maskOn,timerSpeed;
    int port2;
public:
    TI83();
    virtual void EnableDebug();
    virtual void DisableDebug();
    virtual int getmem(int addr);
    virtual int getmem_word(int addr);
    virtual int getmem_dword(int addr);
    virtual void setmem(int addr,int v);
    virtual void setmem_word(int addr,int v);
    virtual void setmem_dword(int addr,int v);
    virtual int readmem(int addr);
    virtual int readmem_word(int addr);
    virtual int readmem_dword(int addr);
    virtual void writemem(int addr,int v);
    virtual void writemem_word(int addr,int v);
    virtual void writemem_dword(int addr,int v);
    virtual int readport(int port);
    virtual void writeport(int port,int v);
    virtual void Execute();
    virtual void OneInstruction();
    virtual void Reset();
    virtual void OnUp();
    virtual void OnDown();
    virtual void KeyUp(int r,int c);
    virtual void KeyDown(int r,int c);
    virtual void AlphaUp();
    virtual void AlphaDown();
    virtual void KBInt();
    virtual int* GetKeyMap();
    virtual int GetKeyMat(int r,int c);
    virtual int GetKeyAlphaMat(int r,int c);
    virtual int GetKeyID(int i);
    virtual void SaveState(char *fn);
    virtual void LoadState(char *fn);
    virtual int SendByte(int c);
    virtual int GetByte(int &c);
    virtual int SendReady();
    virtual int SendBuffer(int c);
    virtual int CheckBuffer();
    virtual int GetBuffer(int &c);
    virtual int SendFile(char *fn,int more=0);
    virtual int ReceiveFile();
};

class TI83Plus: public CalcHW
{
    int kbmask;
    int port3Left,intr;
    int curCol,curRow,dir,justSet,mode;
    unsigned char vidMem[768];
    int redWire,whiteWire;
    int maskTimer,maskOn,timerSpeed;
    int port2,port4;
public:
    TI83Plus();
    virtual void EnableDebug();
    virtual void DisableDebug();
    virtual int getmem(int addr);
    virtual int getmem_word(int addr);
    virtual int getmem_dword(int addr);
    virtual void setmem(int addr,int v);
    virtual void setmem_word(int addr,int v);
    virtual void setmem_dword(int addr,int v);
    virtual int readmem(int addr);
    virtual int readmem_word(int addr);
    virtual int readmem_dword(int addr);
    virtual void writemem(int addr,int v);
    virtual void writemem_word(int addr,int v);
    virtual void writemem_dword(int addr,int v);
    virtual int readport(int port);
    virtual void writeport(int port,int v);
    virtual void Execute();
    virtual void OneInstruction();
    virtual void Reset();
    virtual void OnUp();
    virtual void OnDown();
    virtual void KeyUp(int r,int c);
    virtual void KeyDown(int r,int c);
    virtual void AlphaUp();
    virtual void AlphaDown();
    virtual void KBInt();
    virtual int* GetKeyMap();
    virtual int GetKeyMat(int r,int c);
    virtual int GetKeyAlphaMat(int r,int c);
    virtual int GetKeyID(int i);
    virtual void SaveState(char *fn);
    virtual void LoadState(char *fn);
    virtual int SendByte(int c);
    virtual int GetByte(int &c);
    virtual int SendReady();
    virtual int SendBuffer(int c);
    virtual int CheckBuffer();
    virtual int GetBuffer(int &c);
    virtual int SendFile(char *fn,int more=0);
    virtual int ReceiveFile();
};

class TI73: public CalcHW
{
    int kbmask;
    int port3Left,intr;
    int curCol,curRow,dir,justSet,mode;
    unsigned char vidMem[768];
    int redWire,whiteWire;
    int maskTimer,maskOn,timerSpeed;
    int port2;
public:
    TI73();
    virtual void EnableDebug();
    virtual void DisableDebug();
    virtual int getmem(int addr);
    virtual int getmem_word(int addr);
    virtual int getmem_dword(int addr);
    virtual void setmem(int addr,int v);
    virtual void setmem_word(int addr,int v);
    virtual void setmem_dword(int addr,int v);
    virtual int readmem(int addr);
    virtual int readmem_word(int addr);
    virtual int readmem_dword(int addr);
    virtual void writemem(int addr,int v);
    virtual void writemem_word(int addr,int v);
    virtual void writemem_dword(int addr,int v);
    virtual int readport(int port);
    virtual void writeport(int port,int v);
    virtual void Execute();
    virtual void OneInstruction();
    virtual void Reset();
    virtual void OnUp();
    virtual void OnDown();
    virtual void KeyUp(int r,int c);
    virtual void KeyDown(int r,int c);
    virtual void AlphaUp();
    virtual void AlphaDown();
    virtual void KBInt();
    virtual int* GetKeyMap();
    virtual int GetKeyMat(int r,int c);
    virtual int GetKeyAlphaMat(int r,int c);
    virtual int GetKeyID(int i);
    virtual void SaveState(char *fn);
    virtual void LoadState(char *fn);
    virtual int SendByte(int c);
    virtual int GetByte(int &c);
    virtual int SendReady();
    virtual int SendBuffer(int c);
    virtual int CheckBuffer();
    virtual int GetBuffer(int &c);
    virtual int SendFile(char *fn,int more=0);
    virtual int ReceiveFile();
};

class TI85: public CalcHW
{
    int kbmask;
    int port3Left,intr,port6;
    int redWire,whiteWire;
    int unstop;
public:
    TI85();
    virtual void EnableDebug();
    virtual void DisableDebug();
    virtual int getmem(int addr);
    virtual int getmem_word(int addr);
    virtual int getmem_dword(int addr);
    virtual void setmem(int addr,int v);
    virtual void setmem_word(int addr,int v);
    virtual void setmem_dword(int addr,int v);
    virtual int readmem(int addr);
    virtual int readmem_word(int addr);
    virtual int readmem_dword(int addr);
    virtual void writemem(int addr,int v);
    virtual void writemem_word(int addr,int v);
    virtual void writemem_dword(int addr,int v);
    virtual int readport(int port);
    virtual void writeport(int port,int v);
    virtual void Execute();
    virtual void OneInstruction();
    virtual void Reset();
    virtual void OnUp();
    virtual void OnDown();
    virtual void KeyUp(int r,int c);
    virtual void KeyDown(int r,int c);
    virtual void AlphaUp();
    virtual void AlphaDown();
    virtual void KBInt();
    virtual int* GetKeyMap();
    virtual int GetKeyMat(int r,int c);
    virtual int GetKeyAlphaMat(int r,int c);
    virtual int GetKeyID(int i);
    virtual void SaveState(char *fn);
    virtual void LoadState(char *fn);
    virtual int SendByte(int c);
    virtual int GetByte(int &c);
    virtual int SendReady();
    virtual int SendBuffer(int c);
    virtual int CheckBuffer();
    virtual int GetBuffer(int &c);
    virtual int SendFile(char *fn,int more=0);
    virtual int ReceiveFile();
};

class TI86: public CalcHW
{
    int kbmask;
    int port3Left,intr;
    int redWire,whiteWire;
    int unstop;
public:
    TI86();
    virtual void EnableDebug();
    virtual void DisableDebug();
    virtual int getmem(int addr);
    virtual int getmem_word(int addr);
    virtual int getmem_dword(int addr);
    virtual void setmem(int addr,int v);
    virtual void setmem_word(int addr,int v);
    virtual void setmem_dword(int addr,int v);
    virtual int readmem(int addr);
    virtual int readmem_word(int addr);
    virtual int readmem_dword(int addr);
    virtual void writemem(int addr,int v);
    virtual void writemem_word(int addr,int v);
    virtual void writemem_dword(int addr,int v);
    virtual int readport(int port);
    virtual void writeport(int port,int v);
    virtual void Execute();
    virtual void OneInstruction();
    virtual void Reset();
    virtual void OnUp();
    virtual void OnDown();
    virtual void KeyUp(int r,int c);
    virtual void KeyDown(int r,int c);
    virtual void AlphaUp();
    virtual void AlphaDown();
    virtual void KBInt();
    virtual int* GetKeyMap();
    virtual int GetKeyMat(int r,int c);
    virtual int GetKeyAlphaMat(int r,int c);
    virtual int GetKeyID(int i);
    virtual void SaveState(char *fn);
    virtual void LoadState(char *fn);
    virtual int SendByte(int c);
    virtual int GetByte(int &c);
    virtual int SendReady();
    virtual int SendBuffer(int c);
    virtual int CheckBuffer();
    virtual int GetBuffer(int &c);
    virtual int SendFile(char *fn,int more=0);
    virtual int ReceiveFile();
};

extern CalcHW* hw;

extern UBYTE* mem[256];
extern unsigned char* pageA;
extern unsigned char* pageB;
extern int pageAType,pageANum,pageBType,pageBNum;
extern int memAnd,flashProtect,romRetOr;
extern int (*asm_readmem)(int);
extern int (*asm_readmem_word)(int);
extern int (*asm_readmem_dword)(int);
extern void (*asm_writemem)(int,int);
extern void (*asm_writemem_word)(int,int);
extern void (*asm_writemem_dword)(int,int);

extern "C" int asm_86_readmem(int);
extern "C" int asm_86_readmem_word(int);
extern "C" int asm_86_readmem_dword(int);
extern "C" void asm_86_writemem(int,int);
extern "C" void asm_86_writemem_word(int,int);
extern "C" void asm_86_writemem_dword(int,int);
extern "C" int asm_89_readmem(int);
extern "C" int asm_89_readmem_word(int);
extern "C" int asm_89_readmem_dword(int);
extern "C" void asm_89_writemem(int,int);
extern "C" void asm_89_writemem_word(int,int);
extern "C" void asm_89_writemem_dword(int,int);
extern "C" int debug_readmem(int);
extern "C" int debug_readmem_word(int);
extern "C" int debug_readmem_dword(int);
extern "C" void debug_writemem(int,int);
extern "C" void debug_writemem_word(int,int);
extern "C" void debug_writemem_dword(int,int);

extern void IRQ(int level);
extern int cpuCompleteStop;

enum keys {kn,k_f1,k_f2,k_f3,k_f4,k_f5,k_f6,k_f7,k_f8,k_hand,
    k_q,k_w,k_e,k_r,k_t,k_y,k_u,k_i,k_o,k_p,k_a,k_s,k_d,k_f,
    k_g,k_h,k_j,k_k,k_l,k_z,k_x,k_c,k_v,k_b,k_n,k_m,k_shift,
    k_theta,k_dia,k_2nd,k_sto,k_space,k_equ,k_bs,k_enter1,
    k_esc,k_mode,k_clear,k_ln,k_sin,k_cos,k_tan,k_power,
    k_apps,k_enter2,k_up,k_down,k_left,k_right,k_lparan,
    k_rparan,k_comma,k_div,k_7,k_8,k_9,k_mult,k_4,k_5,k_6,
    k_minus,k_1,k_2,k_3,k_plus,k_0,k_dot,k_neg,k_on,k_graph,
    k_table,k_prgm,k_custom,k_log,k_square,k_ee,k_xvar};

#define VIDMEM_ADDR 0x84848000

#endif