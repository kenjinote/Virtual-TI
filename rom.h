//---------------------------------------------------------------------------
#ifndef romH
#define romH

#define ROMFLAG_TI89 1
#define ROMFLAG_INTERNAL 2
#define ROMFLAG_UPDATE 4
#define ROMFLAG_FLASHROM 8
#define ROMFLAG_SPECIAL 16
#define ROMFLAG_Z80 32
#define ROMFLAG_TI82 64
#define ROMFLAG_TI83 128
#define ROMFLAG_TI85 256
#define ROMFLAG_TI86 512
#define ROMFLAG_TI73 1024
#define ROMFLAG_NEWUPDATE 2048

#define MAX_DEBUG_FILES 256

struct ROMImageDesc
{
    char file[32];
    char name[64];
    int type;
    int version;
    int size;
};
extern ROMImageDesc romImage[64];
extern int romImageCount;
extern int currentROM;

struct SkinDesc
{
    char file[32];
    char name[64];
    char author[64];
    int calc;
    unsigned short csum;
    int defW,defH;
};
extern SkinDesc skin[64];
extern int skinCount,currentSkin;
extern Graphics::TBitmap *skinImageLg;
extern Graphics::TBitmap *skinImageSm;
extern RECT skinLcd;
extern RECT skinKey[80];

typedef unsigned char UBYTE;
extern UBYTE *ram,*intRom,*extRom,*origRom,*rom;
extern UBYTE garbageMem[0x10000];
extern int initialPC;
extern int lcdBase,lcdRefresh,lcdWidth,lcdHeight;
extern int calc,isInternal,plusMod,ramSize;
extern int rom92Present,rom92PlusPresent,rom89Present;
extern int initIntTabOfs;
extern int romErase,romErasePhase;
extern int romChanged[32];
extern int lcdOff,contrast,lcdUpdateFreq,run;
extern char initPath[256];
extern char *ScreenLine[256];
extern int scaleFact;
extern int oldScreenBase[6];
extern int updateLink;
extern int sendInProgress,emuLink;
extern int transferAsGroup;
extern int scaleDefault[7];
extern int lastIRQ4;
extern int soundEnable;

typedef struct
{
    int num;
    char *name;
} ROMFunc;
extern ROMFunc *romFuncs;
extern int romFuncAddr[0x800];

extern int useInitPath;
extern char initPath[256];

typedef struct
{
    BYTE buf[256];
    int start;
    int end;
} LinkBuffer;
extern LinkBuffer recvBuf,sendBuf,*vSendBuf,*vRecvBuf;
extern int lastByteTicks,byteCount;

extern int db92Count;
extern DWORD db92Addr[MAX_DEBUG_FILES];
extern WORD db92VarOfs[MAX_DEBUG_FILES];
extern WORD db92LblOfs[MAX_DEBUG_FILES];

extern int otherInDebug;

extern int CheckRecvBuf();
extern BYTE GetRecvBuf();
extern int PutRecvBuf(BYTE b);
extern int PutRecvReady();
extern int CheckSendBuf();
extern BYTE GetSendBuf();
extern int PutSendBuf(BYTE b);
extern int PutSendReady();
extern int CheckVRecvBuf();
extern BYTE GetVRecvBuf();
extern int PutVSendBuf(BYTE b);
extern int PutVSendReady();

extern int LoadROM(int mode);
extern int AnalyzeROMs(int noCache=0);
extern void InitEmu(int rom);
extern void CloseEmu();
extern void RefreshLCD();
extern void RefreshLCDNow();
extern int ROMImageCalc(int rom);
extern int ROMImageVer(int rom);
extern void SaveState();
extern void LoadState();
extern void AnalyzeSkins();
extern void LoadSkin(int n);
extern void CloseSkin(int saveSkin=1);
extern int GetKeyAt(int x,int y);

extern "C" void HandleException();

extern Graphics::TBitmap* GetLCD_BW();
extern Graphics::TBitmap* GetLCD_True();
extern Graphics::TBitmap* GetLCD_Calc(Graphics::TBitmap* calc);

//---------------------------------------------------------------------------
#endif
