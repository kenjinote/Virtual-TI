#include <vcl\vcl.h>
#include <vcl\registry.hpp>
#define NOTYPEREDEF
#include "cpudefs.h"
#include <stdio.h>
#include <string.h>
#include <dir.h>
#include <io.h>
#include <dos.h>
#include "rom.h"
#include "Debug.h"
#include "Emu.h"
#include "jpegfmt.h"

UBYTE *ram,*intRom,*extRom,*origRom,*rom;
UBYTE garbageMem[0x10000];
int initialPC;
int lcdBase,lcdRefresh,lcdWidth,lcdHeight,lcdLineBytes;
int lcdUpdateFreq=1;
int calc,isInternal,plusMod,ramSize;
int rom92Present,rom92PlusPresent,rom89Present;
int initIntTabOfs;
int romErase,romErasePhase;
int romChanged[32];
int needPath=1;
int lastTI89ROM,lastTI92PlusROM;
char initPath[256];
int lcdOff,contrast;
char *ScreenLine[256];
int scaleFact=2;
int calcChanged;
BYTE oldScreen[6][3840];
int oldScreenBase[6];
int scrNum=0;
int updateLink=0;
int sendInProgress=0;
int transferAsGroup=1;
int soundEnable=0;

int otherInDebug;

int scaleDefault[7]={2,2,2,2,1,1,2};

ROMImageDesc romImage[64];
int romImageCount;
int currentROM;

SkinDesc skin[64];
int skinCount=0;
int currentSkin=-1;
Graphics::TBitmap *skinImageLg=NULL;
Graphics::TBitmap *skinImageSm=NULL;
RECT skinLcd;
RECT skinKey[80];

LinkBuffer recvBuf,sendBuf,*vSendBuf,*vRecvBuf;
int lastByteTicks=0;
int byteCount=0;

int irv[7],igv[7],ibv[7];

int irv89[2]={49,204};
int igv89[2]={46,224};
int ibv89[2]={34,207};

int irv92[2]={83,174};
int igv92[2]={111,204};
int ibv92[2]={138,176};

int irvC[2]={0,255};
int igvC[2]={0,255};
int ibvC[2]={0,255};

int contrastDelta[8]={0,-2,-4,-7,-10,-7,-4,-2};

int lcdUpdateNeeded=0;
extern HANDLE lcdUpdateEvent;

ROMFunc *romFuncs=NULL;
int romFuncAddr[0x800];

UBYTE* mem[256];
unsigned char* pageA;
unsigned char* pageB;
int pageAType,pageANum,pageBType,pageBNum;
int memAnd,flashProtect,romRetOr;
int (*asm_readmem)(int);
int (*asm_readmem_word)(int);
int (*asm_readmem_dword)(int);
void (*asm_writemem)(int,int);
void (*asm_writemem_word)(int,int);
void (*asm_writemem_dword)(int,int);

int debug_readmem(int a) { return hw->readmem(a); }
int debug_readmem_word(int a) { return hw->readmem_word(a); }
int debug_readmem_dword(int a) { return hw->readmem_dword(a); }
void debug_writemem(int a,int v) { hw->writemem(a,v); }
void debug_writemem_word(int a,int v) { hw->writemem_word(a,v); }
void debug_writemem_dword(int a,int v) { hw->writemem_dword(a,v); }

int inline IsNum(char c)
{
    if ((c>='0')&&(c<='9'))
        return 1;
    return 0;
}

int inline IsAlNum(char c)
{
    if ((c>='0')&&(c<='9'))
        return 1;
    if ((c>='a')&&(c<='z'))
        return 1;
    if ((c>='A')&&(c<='Z'))
        return 1;
    if (c==' ')
        return 1;
    return 0;
}

void RecordROMInfo(int n,int csum)
{
    TRegistry *reg=new TRegistry;
    reg->RootKey=HKEY_LOCAL_MACHINE;
    reg->OpenKey("\\Software\\ACZ\\Virtual TI\\ROMs",true);
    reg->OpenKey(romImage[n].file,true);
    reg->WriteInteger("Size",romImage[n].size);
    reg->WriteInteger("Checksum",csum);
    reg->WriteString("Name",romImage[n].name);
    reg->WriteInteger("Version",romImage[n].version);
    reg->WriteInteger("Flags",romImage[n].type);
    reg->CloseKey();
    delete reg;
}

void AddROM8285(char *name,int size)
{
    FILE *fp=fopen(name,"rb");
    if (!fp) return;
    char *buf=new char[size];
    fread(buf,size,1,fp);
    int csum=0;
    for (int i=0;i<1024;i++)
        csum+=((int)buf[i])&0xff;
    csum&=0xffff;
    int ver=0;
    int flags=ROMFLAG_Z80|ROMFLAG_INTERNAL;
    for (int i=0;i<size-16;i++)
    {
        if (!ver)
        {
            if (IsNum(buf[i])&&IsNum(buf[i+1])&&(buf[i+2]=='.')&&
                IsNum(buf[i+3]))
            {
                ver=i;
                buf[i+4]=0;
                i+=4;
                continue;
            }
            if (IsNum(buf[i])&&(buf[i+1]=='.')&&IsNum(buf[i+2]))
            {
                ver=i;
                buf[i+3]=0;
                i+=3;
                continue;
            }
        }
        if ((buf[i]=='C')&&(buf[i+1]=='A')&&(buf[i+2]=='T')&&
            (buf[i+3]=='A')&&(buf[i+4]=='L')&&(buf[i+5]=='O')&&
            (buf[i+6]=='G'))
            flags|=ROMFLAG_TI85;
    }
    if (!(flags&ROMFLAG_TI85))
        flags|=ROMFLAG_TI82;
    if (ver)
    {
        strcpy(romImage[romImageCount].file,name);
        if (flags&ROMFLAG_TI82)
        {
            sprintf(romImage[romImageCount].name,
                "TI-82 version %s",&buf[ver]);
        }
        else
        {
            sprintf(romImage[romImageCount].name,
                "TI-85 version %s",&buf[ver]);
        }
        romImage[romImageCount].type=flags;
        if (buf[ver+3]==0)
            ver=(buf[ver]<<16)+buf[ver+2];
        else
            ver=(buf[ver]<<16)+(buf[ver+2]<<8)+buf[ver+3];
        romImage[romImageCount].version=ver;
        romImage[romImageCount].size=size;
        RecordROMInfo(romImageCount,csum);
        romImageCount++;
    }
    else
    {
        char str[256];
        sprintf(str,"The ROM image %s is not valid.\n"
            "Try redownloading it from your calculator.",
            name);
        MessageBox(NULL,str,"Error",MB_OK|MB_ICONSTOP);
    }
    delete[] buf;
    fclose(fp);
}

void AddROM8386(char *name,int size)
{
    FILE *fp=fopen(name,"rb");
    if (!fp) return;
    char *buf=new char[size];
    fread(buf,size,1,fp);
    int csum=0;
    for (int i=0;i<1024;i++)
        csum+=((int)buf[i])&0xff;
    csum&=0xffff;
    int ver=0;
    int flags=ROMFLAG_Z80|ROMFLAG_INTERNAL;
    for (int i=0;i<size-16;i++)
    {
        if ((buf[i]=='E')&&(buf[i+1]=='X')&&(buf[i+2]=='P')&&
            (buf[i+3]=='L')&&(buf[i+4]=='O')&&(buf[i+5]=='R')&&
            (buf[i+6]=='E')&&(buf[i+7]=='R'))
        {
            ver=0x64;
            flags|=ROMFLAG_TI73|ROMFLAG_FLASHROM;
            break;
        }
    }
    for (int i=0;(i<size-16)&&(!(flags&ROMFLAG_TI73));i++)
    {
        if (!ver)
        {
            if (buf[i+3]=='O') buf[i+3]='0';
            if (buf[i+4]=='O') buf[i+4]='0';
            if (IsNum(buf[i])&&IsNum(buf[i+2])&&(buf[i+3]=='.')&&
                IsNum(buf[i+4]))
            {
                ver=i;
                buf[i+5]=0;
                i+=5;
                continue;
            }
            if (IsNum(buf[i])&&(buf[i+1]=='.')&&IsNum(buf[i+2])&&
                IsNum(buf[i+3]))
            {
                ver=i;
                buf[i+4]=0;
                i+=4;
                continue;
            }
            if (IsNum(buf[i])&&(buf[i+1]=='.')&&IsNum(buf[i+2]))
            {
                ver=i;
                buf[i+3]=0;
                i+=3;
                continue;
            }
        }
        if ((buf[i]=='T')&&(buf[i+1]=='I')&&(buf[i+2]=='8')&&
            (buf[i+3]=='2'))
            flags|=ROMFLAG_TI83;
        if ((buf[i]=='T')&&(buf[i+1]=='I')&&(buf[i+2]=='-')&&
            (buf[i+3]=='8')&&(buf[i+4]=='3')&&(buf[i+5]==' ')&&
            (buf[i+6]=='P')&&(buf[i+7]=='l'))
            flags|=ROMFLAG_FLASHROM;
    }
    if (!(flags&(ROMFLAG_TI83|ROMFLAG_TI73)))
        flags|=ROMFLAG_TI86;
    if (ver)
    {
        strcpy(romImage[romImageCount].file,name);
        if (flags&ROMFLAG_TI83)
        {
            if (flags&ROMFLAG_FLASHROM)
            {
                sprintf(romImage[romImageCount].name,
                    "TI-83 Plus version %s",&buf[ver]);
            }
            else
            {
                sprintf(romImage[romImageCount].name,
                    "TI-83 version %s",&buf[ver]);
            }
        }
        else if (flags&ROMFLAG_TI86)
        {
            sprintf(romImage[romImageCount].name,
                "TI-86 version %s",&buf[ver]);
        }
        else
        {
            sprintf(romImage[romImageCount].name,
                "TI-73 version %s",&buf[ver]);
        }
        romImage[romImageCount].type=flags;
        if (buf[ver+3]==0)
            ver=(buf[ver]<<16)+buf[ver+2];
        else
            ver=(buf[ver]<<16)+(buf[ver+2]<<8)+buf[ver+3];
        romImage[romImageCount].version=ver;
        romImage[romImageCount].size=size;
        RecordROMInfo(romImageCount,csum);
        romImageCount++;
    }
    else
    {
        char str[256];
        sprintf(str,"The ROM image %s is not valid.\n"
            "Try redownloading it from your calculator.",
            name);
        MessageBox(NULL,str,"Error",MB_OK|MB_ICONSTOP);
    }
    delete[] buf;
    fclose(fp);
}

void AddROM(char *name,int size)
{
    if (size==(128*1024))
        AddROM8285(name,size);
    if (size==(256*1024))
        AddROM8386(name,size);
    if (size==(512*1024))
        AddROM8386(name,size);
    if ((size!=(1024*1024))&&(size!=(2048*1024)))
        return;
    FILE *fp=fopen(name,"rb");
    if (!fp) return;
    unsigned char *buf=new char[size];
    fread(buf,size,1,fp);
    if ((size==(1024*1024))&&(buf[0]!=0)||(buf[1]!=0))
    {
        fclose(fp);
        delete[] buf;
        AddROM8386(name,512*1024);
        return;
    }
    int csum=0;
    for (int i=0;i<1024;i++)
        csum+=((int)buf[i])&0xff;
    csum&=0xffff;
    int ver=0;
    int flags=0;
    if ((buf[5]&0x60)==0x20) flags|=ROMFLAG_INTERNAL;
    if (!(buf[0x65]&0xf)) flags|=ROMFLAG_FLASHROM;
    if ((flags&ROMFLAG_INTERNAL)&&(flags&ROMFLAG_FLASHROM))
        flags|=ROMFLAG_TI89;
    for (int i=0x10000;i<size-16;i+=2)
    {
        if (IsNum(buf[i])&&(buf[i+1]=='.')&&IsNum(buf[i+2])&&
            (buf[i+3]==0)&&IsAlNum(buf[i+4])&&IsAlNum(buf[i+5])&&
            (buf[i+6]=='/')&&IsAlNum(buf[i+7])&&IsAlNum(buf[i+8])&&
            (buf[i+9]=='/')&&IsAlNum(buf[i+10])&&IsAlNum(buf[i+11]))
        {
            ver=i;
            break;
        }
        if (IsNum(buf[i])&&(buf[i+1]=='.')&&IsNum(buf[i+2])&&
            IsNum(buf[i+3])&&(buf[i+4]==0)&&IsAlNum(buf[i+5])&&
            IsAlNum(buf[i+6])&&(buf[i+7]=='/')&&IsAlNum(buf[i+8])&&
            IsAlNum(buf[i+9])&&(buf[i+10]=='/')&&IsAlNum(buf[i+11])&&
            IsAlNum(buf[i+12]))
        {
            ver=i;
            break;
        }
        if (IsNum(buf[i])&&(buf[i+1]=='.')&&IsNum(buf[i+2])&&
            (buf[i+3]==0)&&IsAlNum(buf[i+4])&&IsAlNum(buf[i+5])&&
            IsAlNum(buf[i+6])&&IsAlNum(buf[i+7])&&IsAlNum(buf[i+8])&&
            IsAlNum(buf[i+9])&&IsAlNum(buf[i+10])&&IsAlNum(buf[i+11]))
        {
            ver=i;
            break;
        }
        if (IsNum(buf[i])&&(buf[i+1]=='.')&&IsNum(buf[i+2])&&
            IsAlNum(buf[i+3])&&(buf[i+4]==0)&&IsAlNum(buf[i+5])&&
            IsAlNum(buf[i+6])&&IsAlNum(buf[i+7])&&IsAlNum(buf[i+8])&&
            IsAlNum(buf[i+9])&&IsAlNum(buf[i+10])&&IsAlNum(buf[i+11]))
        {
            ver=i;
            break;
        }
    }
    if (ver)
    {
        strcpy(romImage[romImageCount].file,name);
        if (flags&ROMFLAG_TI89)
        {
            sprintf(romImage[romImageCount].name,
                "TI-89 version %s",&buf[ver]);
            if ((buf[0x10000]==0xff)&&(buf[0x10001]==0xf8))
                lastTI89ROM=romImageCount;
            else if (lastTI89ROM==-1)
                lastTI89ROM=romImageCount;
        }
        else
        {
            if (flags&ROMFLAG_FLASHROM)
            {
                sprintf(romImage[romImageCount].name,
                    "TI-92 Plus version %s",&buf[ver]);
                if ((buf[0x10000]==0xff)&&(buf[0x10001]==0xf8))
                    lastTI92PlusROM=romImageCount;
                else if (lastTI92PlusROM==-1)
                    lastTI92PlusROM=romImageCount;
            }
            else
            {
                if (flags&ROMFLAG_INTERNAL)
                {
                    sprintf(romImage[romImageCount].name,
                        "TI-92 version %s (internal)",&buf[ver]);
                }
                else
                {
                    sprintf(romImage[romImageCount].name,
                        "TI-92 version %s (external)",&buf[ver]);
                }
            }
        }
        romImage[romImageCount].type=flags;
        if (buf[ver+3]==0)
            ver=(buf[ver]<<16)+buf[ver+2];
        else
            ver=(buf[ver]<<16)+(buf[ver+2]<<8)+buf[ver+3];
        romImage[romImageCount].version=ver;
        romImage[romImageCount].size=size;
        RecordROMInfo(romImageCount,csum);
        romImageCount++;
    }
    else
    {
        char str[256];
        sprintf(str,"The ROM image %s is not valid.\n"
            "Try redownloading it from your calculator.",
            name);
        MessageBox(NULL,str,"Error",MB_OK|MB_ICONSTOP);
    }
    delete[] buf;
    fclose(fp);
}

void AddROMUpdate(char *name,int size)
{
    FILE *fp=fopen(name,"rb");
    if (!fp) return;
    char *buf=new char[size];
    fread(buf,size,1,fp);
    int csum=0;
    for (int i=0;i<1024;i++)
        csum+=((int)buf[i])&0xff;
    csum&=0xffff;
    int ver=0;
    int flags=ROMFLAG_UPDATE|ROMFLAG_FLASHROM;
    if ((buf[0x8d]&0x60)==0x20)
        flags|=ROMFLAG_INTERNAL|ROMFLAG_TI89;
    int nameStart=0;
    for (;;nameStart++)
    {
        if (buf[nameStart]==0x4d)
            break;
    }
    nameStart+=2;
    if (!strncmp(&buf[nameStart],"Advanced Mathematics Software",29))
    {
       for (int i=0;i<size-16;i+=2)
       {
            if (IsNum(buf[i])&&(buf[i+1]=='.')&&IsNum(buf[i+2])&&
                (buf[i+3]==0)&&IsAlNum(buf[i+4])&&IsAlNum(buf[i+5])&&
                (buf[i+6]=='/')&&IsAlNum(buf[i+7])&&IsAlNum(buf[i+8])&&
                (buf[i+9]=='/')&&IsAlNum(buf[i+10])&&IsAlNum(buf[i+11]))
            {
                ver=i;
                break;
            }
            if (IsNum(buf[i])&&(buf[i+1]=='.')&&IsNum(buf[i+2])&&
                IsNum(buf[i+3])&&(buf[i+4]==0)&&IsAlNum(buf[i+5])&&
                IsAlNum(buf[i+6])&&(buf[i+7]=='/')&&IsAlNum(buf[i+8])&&
                IsAlNum(buf[i+9])&&(buf[i+10]=='/')&&IsAlNum(buf[i+11])&&
                IsAlNum(buf[i+12]))
            {
                ver=i;
                break;
            }
        }
    }
    else
    {
        flags|=ROMFLAG_SPECIAL;
        ver=-1;
    }
    if (ver)
    {
        strcpy(romImage[romImageCount].file,name);
        if (flags&ROMFLAG_SPECIAL)
        {
            if (flags&ROMFLAG_TI89)
            {
                buf[nameStart+buf[nameStart-1]]=0;
                sprintf(romImage[romImageCount].name,
                    "%s (TI-89)",&buf[nameStart]);
            }
            else
            {
                buf[nameStart+buf[nameStart-1]]=0;
                sprintf(romImage[romImageCount].name,
                    "%s (TI-92 Plus)",&buf[nameStart]);
            }
        }
        else if (flags&ROMFLAG_TI89)
        {
            sprintf(romImage[romImageCount].name,
                "TI-89 version %s (update)",&buf[ver]);
        }
        else
        {
            sprintf(romImage[romImageCount].name,
                "TI-92 Plus version %s (update)",&buf[ver]);
        }
        romImage[romImageCount].type=flags;
        if (ver!=-1)
        {
            if (buf[ver+3]==0)
                ver=(buf[ver]<<16)+buf[ver+2];
            else
                ver=(buf[ver]<<16)+(buf[ver+2]<<8)+buf[ver+3];
        }
        romImage[romImageCount].version=ver;
        romImage[romImageCount].size=size;
        RecordROMInfo(romImageCount,csum);
        romImageCount++;
    }
    delete[] buf;
    fclose(fp);
}

void AddROMUpdateTIFL(char *name,int _size)
{
    FILE *fp=fopen(name,"rb");
    if (!fp) return;
    int size;
    while (1)
    {
        fseek(fp,0x10,SEEK_CUR);
        int len=fgetc(fp);
        char name[32];
        fread(name,len+1,1,fp);
        fseek(fp,0x38-len,SEEK_CUR);
        fread(&size,4,1,fp);
        if (!strcmp(name,"basecode"))
            break;
        fseek(fp,size,SEEK_CUR);
    }
    char *buf=new char[size];
    fread(buf,size,1,fp);
    int csum=0;
    for (int i=0;i<1024;i++)
        csum+=((int)buf[i])&0xff;
    csum&=0xffff;
    int ver=0;
    int flags=ROMFLAG_UPDATE|ROMFLAG_FLASHROM;
    if ((buf[0x8d]&0x60)==0x20)
        flags|=ROMFLAG_INTERNAL|ROMFLAG_TI89;
    int nameStart=0;
    for (;;nameStart++)
    {
        if (buf[nameStart]==0x4d)
            break;
    }
    nameStart+=2;
    if (!strncmp(&buf[nameStart],"Advanced Mathematics Software",29))
    {
       for (int i=0;i<size-16;i+=2)
       {
            if (IsNum(buf[i])&&(buf[i+1]=='.')&&IsNum(buf[i+2])&&
                (buf[i+3]==0)&&IsAlNum(buf[i+4])&&IsAlNum(buf[i+5])&&
                (buf[i+6]=='/')&&IsAlNum(buf[i+7])&&IsAlNum(buf[i+8])&&
                (buf[i+9]=='/')&&IsAlNum(buf[i+10])&&IsAlNum(buf[i+11]))
            {
                ver=i;
                break;
            }
            if (IsNum(buf[i])&&(buf[i+1]=='.')&&IsNum(buf[i+2])&&
                IsNum(buf[i+3])&&(buf[i+4]==0)&&IsAlNum(buf[i+5])&&
                IsAlNum(buf[i+6])&&(buf[i+7]=='/')&&IsAlNum(buf[i+8])&&
                IsAlNum(buf[i+9])&&(buf[i+10]=='/')&&IsAlNum(buf[i+11])&&
                IsAlNum(buf[i+12]))
            {
                ver=i;
                break;
            }
        }
    }
    else
    {
        flags|=ROMFLAG_SPECIAL;
        ver=-1;
    }
    if (ver)
    {
        strcpy(romImage[romImageCount].file,name);
        if (flags&ROMFLAG_SPECIAL)
        {
            if (flags&ROMFLAG_TI89)
            {
                buf[nameStart+buf[nameStart-1]]=0;
                sprintf(romImage[romImageCount].name,
                    "%s (TI-89)",&buf[nameStart]);
            }
            else
            {
                buf[nameStart+buf[nameStart-1]]=0;
                sprintf(romImage[romImageCount].name,
                    "%s (TI-92 Plus)",&buf[nameStart]);
            }
        }
        else if (flags&ROMFLAG_TI89)
        {
            sprintf(romImage[romImageCount].name,
                "TI-89 version %s (update)",&buf[ver]);
        }
        else
        {
            sprintf(romImage[romImageCount].name,
                "TI-92 Plus version %s (update)",&buf[ver]);
        }
        romImage[romImageCount].type=flags|ROMFLAG_NEWUPDATE;
        if (ver!=-1)
        {
            if (buf[ver+3]==0)
                ver=(buf[ver]<<16)+buf[ver+2];
            else
                ver=(buf[ver]<<16)+(buf[ver+2]<<8)+buf[ver+3];
        }
        romImage[romImageCount].version=ver;
        romImage[romImageCount].size=_size;
        RecordROMInfo(romImageCount,csum);
        romImageCount++;
    }
    delete[] buf;
    fclose(fp);
}

int QuickCheckROM(char *name,int size)
{
    TRegistry *reg=new TRegistry;
    reg->RootKey=HKEY_LOCAL_MACHINE;
    reg->OpenKey("\\Software\\ACZ\\Virtual TI\\ROMs",true);
    reg->OpenKey(name,true);
    if (reg->ValueExists("Size")&&
        reg->ValueExists("Checksum")&&
        reg->ValueExists("Name")&&
        reg->ValueExists("Version")&&
        reg->ValueExists("Flags"))
    {
        if (reg->ReadInteger("Size")==size)
        {
            FILE *fp=fopen(name,"rb");
            if (fp)
            {
                int csum=0;
                for (int i=0;i<1024;i++)
                    csum+=fgetc(fp)&0xff;
                fclose(fp);
                csum&=0xffff;
                if (reg->ReadInteger("Checksum")==csum)
                {
                    strcpy(romImage[romImageCount].file,name);
                    strcpy(romImage[romImageCount].name,
                        reg->ReadString("Name").c_str());
                    romImage[romImageCount].version=
                        reg->ReadInteger("Version");
                    romImage[romImageCount].type=
                        reg->ReadInteger("Flags");
                    romImage[romImageCount].size=size;
                    romImageCount++;
                    reg->CloseKey();
                    delete reg;
                    return 1;
                }
            }
        }
    }
    reg->CloseKey();
    delete reg;
    return 0;
}

int AnalyzeROMs(int noCache)
{
    ffblk ff;
    int done,i,use,type,ver,ver89,verPlus;
    char oldPath[256];

    getcwd(oldPath,256);
    chdir(initPath);
    romImageCount=0;
    lastTI89ROM=-1; lastTI92PlusROM=-1;
    done=findfirst("*.rom",&ff,FA_RDONLY|FA_ARCH);
    while (!done)
    {
        if (noCache)
            AddROM(ff.ff_name,ff.ff_fsize);
        else if (!QuickCheckROM(ff.ff_name,ff.ff_fsize))
            AddROM(ff.ff_name,ff.ff_fsize);
        done=findnext(&ff);
    }
    done=findfirst("*.dmp",&ff,FA_RDONLY|FA_ARCH);
    while (!done)
    {
        if (noCache)
            AddROM(ff.ff_name,ff.ff_fsize);
        else if (!QuickCheckROM(ff.ff_name,ff.ff_fsize))
            AddROM(ff.ff_name,ff.ff_fsize);
        done=findnext(&ff);
    }
    done=findfirst("*.bin",&ff,FA_RDONLY|FA_ARCH);
    while (!done)
    {
        if (noCache)
            AddROM(ff.ff_name,ff.ff_fsize);
        else if (!QuickCheckROM(ff.ff_name,ff.ff_fsize))
            AddROM(ff.ff_name,ff.ff_fsize);
        done=findnext(&ff);
    }
    done=findfirst("*.clc",&ff,FA_RDONLY|FA_ARCH);
    while (!done)
    {
        if (noCache)
            AddROM(ff.ff_name,ff.ff_fsize);
        else if (!QuickCheckROM(ff.ff_name,ff.ff_fsize))
            AddROM(ff.ff_name,ff.ff_fsize);
        done=findnext(&ff);
    }
    done=findfirst("*.tib",&ff,FA_RDONLY|FA_ARCH);
    while (!done)
    {
        if (noCache)
            AddROMUpdate(ff.ff_name,ff.ff_fsize);
        else if (!QuickCheckROM(ff.ff_name,ff.ff_fsize))
            AddROMUpdate(ff.ff_name,ff.ff_fsize);
        done=findnext(&ff);
    }
    done=findfirst("*.89u",&ff,FA_RDONLY|FA_ARCH);
    while (!done)
    {
        if (noCache)
            AddROMUpdateTIFL(ff.ff_name,ff.ff_fsize);
        else if (!QuickCheckROM(ff.ff_name,ff.ff_fsize))
            AddROMUpdateTIFL(ff.ff_name,ff.ff_fsize);
        done=findnext(&ff);
    }
    done=findfirst("*.9xu",&ff,FA_RDONLY|FA_ARCH);
    while (!done)
    {
        if (noCache)
            AddROMUpdateTIFL(ff.ff_name,ff.ff_fsize);
        else if (!QuickCheckROM(ff.ff_name,ff.ff_fsize))
            AddROMUpdateTIFL(ff.ff_name,ff.ff_fsize);
        done=findnext(&ff);
    }
    chdir(oldPath);
    if (!romImageCount) return -1;
    use=0; type=romImage[0].type; ver=romImage[0].version;
    return use;
}

int read_dword(unsigned char* ptr)
{
    return (ptr[0]<<24)|(ptr[1]<<16)|(ptr[2]<<8)|ptr[3];
}

int LoadROM(int mode)
{
	FILE *fp;
    char oldPath[256];

    getcwd(oldPath,256);
    chdir(initPath);
	initialPC=0;
	initIntTabOfs=0;
	calc=92; isInternal=0; ramSize=256;
    plusMod=0;
    currentROM=mode;
    if (romImage[mode].type&ROMFLAG_UPDATE)
    {
        if (romImage[mode].type&ROMFLAG_TI89)
        {
            if (lastTI89ROM!=-1)
                currentROM=lastTI89ROM;
            else
                currentROM=-1;
        }
        else
        {
            if (lastTI92PlusROM!=-1)
                currentROM=lastTI92PlusROM;
            else
                currentROM=-1;
        }
    }
    if (currentROM!=-1)
    {
      	fp=fopen(romImage[currentROM].file,"rb");
        if (!fp)
        {
            MessageBox(NULL,"Could not open ROM image","Error",MB_OK);
            chdir(oldPath);
            return 1;
        }
        if (romImage[currentROM].type&ROMFLAG_INTERNAL)
        {
           	fread(intRom,romImage[currentROM].size,1,fp);
            if (romImage[currentROM].size<(2048*1024))
            {
                for (int i=0;i<0x100000;i++)
                    intRom[i+0x100000]=intRom[i];
            }
            if (romImage[currentROM].type&ROMFLAG_FLASHROM)
            {
                if (romImage[currentROM].type&ROMFLAG_Z80)
                    initialPC=7;
                else
                {
                    initIntTabOfs=0x12000;
                    for (;;initIntTabOfs+=4)
                    {
                        if ((intRom[initIntTabOfs]==0xcc)&&
                            (intRom[initIntTabOfs+1]==0xcc)&&
                            (intRom[initIntTabOfs+2]==0xcc)&&
                            (intRom[initIntTabOfs+3]==0xcc))
                            break;
                    }
                    initIntTabOfs+=4;
                    initialPC=read_dword(&intRom[initIntTabOfs+4]);
                }
            }
            else
            {
                if (romImage[currentROM].type&ROMFLAG_Z80)
                    initialPC=0;
                else
                    initialPC=read_dword(&intRom[4]);
                initIntTabOfs=0;
            }
            isInternal=1;
            rom=intRom;
        }
        else
        {
       	    fread(extRom,romImage[currentROM].size,1,fp);
            if (romImage[currentROM].size<(2048*1024))
            {
                for (int i=0;i<0x100000;i++)
                    extRom[i+0x100000]=extRom[i];
            }
            if (romImage[currentROM].type&ROMFLAG_FLASHROM)
            {
                initIntTabOfs=0x12000;
                for (;;initIntTabOfs+=4)
                {
                    if ((extRom[initIntTabOfs]==0xcc)&&
                        (extRom[initIntTabOfs+1]==0xcc)&&
                        (extRom[initIntTabOfs+2]==0xcc)&&
                        (extRom[initIntTabOfs+3]==0xcc))
                        break;
                }
                initIntTabOfs+=4;
                initialPC=read_dword(&extRom[initIntTabOfs+4]);
                plusMod=1;
            }
            else
            {
                initialPC=read_dword(&extRom[4]);
                initIntTabOfs=0;
            }
            rom=extRom;
        }
        if (romImage[currentROM].type&ROMFLAG_TI89)
        {
            lcdWidth=160;
            lcdHeight=100;
            lcdLineBytes=30;
            calc=89;
        }
        else if (romImage[currentROM].type&ROMFLAG_TI82)
        {
            lcdWidth=96;
            lcdHeight=64;
            lcdLineBytes=12;
            calc=82;
        }
        else if (romImage[currentROM].type&ROMFLAG_TI83)
        {
            lcdWidth=96;
            lcdHeight=64;
            lcdLineBytes=12;
            if (romImage[currentROM].type&ROMFLAG_FLASHROM)
            {
                calc=84;
                plusMod=1;
            }
            else
            {
                calc=83;
                plusMod=0;
            }
        }
        else if (romImage[currentROM].type&ROMFLAG_TI85)
        {
            lcdWidth=128;
            lcdHeight=64;
            lcdLineBytes=16;
            calc=85;
        }
        else if (romImage[currentROM].type&ROMFLAG_TI86)
        {
            lcdWidth=128;
            lcdHeight=64;
            lcdLineBytes=16;
            calc=86;
        }
        else if (romImage[currentROM].type&ROMFLAG_TI73)
        {
            lcdWidth=96;
            lcdHeight=64;
            lcdLineBytes=12;
            calc=73;
        }
        else
        {
            lcdWidth=240;
            lcdHeight=128;
            lcdLineBytes=30;
            calc=plusMod?94:((romImage[currentROM].
                size==1048576)?92:93);
        }
        fclose(fp);
    }
    currentROM=mode;
    if (romImage[mode].type&ROMFLAG_UPDATE)
    {
    	fp=fopen(romImage[currentROM].file,"rb");
	    if (!fp)
        {
            chdir(oldPath);
            return 0;
        }
        int size=romImage[mode].size;
        if (romImage[mode].type&ROMFLAG_NEWUPDATE)
        {
            while (1)
            {
                fseek(fp,0x10,SEEK_CUR);
                int len=fgetc(fp);
                char name[32];
                fread(name,len+1,1,fp);
                fseek(fp,0x38-len,SEEK_CUR);
                fread(&size,4,1,fp);
                if (!strcmp(name,"basecode"))
                    break;
                fseek(fp,size,SEEK_CUR);
            }
        }
        if (romImage[mode].type&ROMFLAG_TI89)
        {
            for (int i=0x12000;i<0x200000;i++)
            {
                if (((i&0xffff)!=1)||(i<0x1a0000))
                    intRom[i]=0xff;
                else
                    intRom[i]=0xfe;
            }
           	fread(&intRom[0x12000],size,1,fp);
            initIntTabOfs=0x12000;
            for (;;initIntTabOfs+=4)
            {
                if ((intRom[initIntTabOfs]==0xcc)&&
                    (intRom[initIntTabOfs+1]==0xcc)&&
                    (intRom[initIntTabOfs+2]==0xcc)&&
                    (intRom[initIntTabOfs+3]==0xcc))
                    break;
            }
            initIntTabOfs+=4;
            initialPC=read_dword(&intRom[initIntTabOfs+4]);
            lcdWidth=160;
            lcdHeight=100;
            lcdLineBytes=30;
		    calc=89;
            rom=intRom;
        }
        else
        {
            for (int i=0x12000;i<0x200000;i++)
            {
                if (((i&0xffff)!=1)||(i<0x1a0000))
                    extRom[i]=0xff;
                else
                    extRom[i]=0xfe;
            }
           	fread(&extRom[0x12000],size,1,fp);
            initIntTabOfs=0x12000;
            for (;;initIntTabOfs+=4)
            {
                if ((extRom[initIntTabOfs]==0xcc)&&
                    (extRom[initIntTabOfs+1]==0xcc)&&
                    (extRom[initIntTabOfs+2]==0xcc)&&
                    (extRom[initIntTabOfs+3]==0xcc))
                    break;
            }
            initIntTabOfs+=4;
            initialPC=read_dword(&extRom[initIntTabOfs+4]);
            plusMod=1;
            lcdWidth=240;
            lcdHeight=128;
            lcdLineBytes=30;
            calc=94;
            rom=extRom;
        }
    }
    chdir(oldPath);
    if (initialPC==0) return 1;
	return 0;
}

void InitEmu(int rom)
{
    if (needPath)
    {
        needPath=0;
        getcwd(initPath,256);
    }
	ram=(UBYTE*)malloc(256*1024+3);
	intRom=(UBYTE*)malloc(2048*1024+3);
	extRom=(UBYTE*)malloc(2048*1024+3);
    origRom=(UBYTE*)malloc(2048*1024+3);
    recvBuf.start=0; recvBuf.end=0;
    sendBuf.start=0; sendBuf.end=0;
    currentROM=rom;
    int i;
	for (i=0;i<2048*1024;i++)
	{
		if (i&1)
		{
			intRom[i]=0;
			extRom[i]=0;
		}
		else
		{
			intRom[i]=0x14;
			extRom[i]=0x14;
		}
	}
    if (romImage[currentROM].type&ROMFLAG_TI82)
        hw=new TI82;
    else if ((romImage[currentROM].type&ROMFLAG_TI83)&&
        (romImage[currentROM].type&ROMFLAG_FLASHROM))
        hw=new TI83Plus;
    else if (romImage[currentROM].type&ROMFLAG_TI83)
        hw=new TI83;
    else if (romImage[currentROM].type&ROMFLAG_TI73)
        hw=new TI73;
    else if (romImage[currentROM].type&ROMFLAG_TI85)
        hw=new TI85;
    else if (romImage[currentROM].type&ROMFLAG_TI86)
        hw=new TI86;
    else if (romImage[currentROM].type&ROMFLAG_TI89)
        hw=new TI89;
    else if (romImage[currentROM].size==1048576)
        hw=new TI92;
    else
        hw=new TI92Plus;
    hw->Reset();
    debugCodeBreakCount=0;
    debugDataBreakCount=0;
    debugProgBreak=0;
    if (calc==82) scaleFact=scaleDefault[0];
    else if ((calc==83)||(calc==84)) scaleFact=scaleDefault[1];
    else if (calc==73) scaleFact=scaleDefault[6];
    else if (calc==85) scaleFact=scaleDefault[2];
    else if (calc==86) scaleFact=scaleDefault[3];
    else if (calc==89) scaleFact=scaleDefault[4];
    else scaleFact=scaleDefault[5];
    if (EmuWnd)
    {
        if (!EmuWnd->Fullscreenview1->Checked)
        {
            if (scaleFact==1)
            {
                EmuWnd->Normal1xview1->Checked=true;
                EmuWnd->Large2xcalculatorimage1->Checked=false;
            }
            else
            {
                EmuWnd->Normal1xview1->Checked=false;
                EmuWnd->Large2xcalculatorimage1->Checked=true;
            }
        }
    }
}

void CloseEmu()
{
    if (calc==82) scaleDefault[0]=scaleFact;
    else if ((calc==83)||(calc==84)) scaleDefault[1]=scaleFact;
    else if (calc==73) scaleDefault[6]=scaleFact;
    else if (calc==85) scaleDefault[2]=scaleFact;
    else if (calc==86) scaleDefault[3]=scaleFact;
    else if (calc==89) scaleDefault[4]=scaleFact;
    else scaleDefault[5]=scaleFact;
	free(ram); free(intRom); free(extRom);
    delete hw;
}

void RefreshLCD()
{
	int x,y,i,j,b,v0,v1,v2,v3,v4,v5,v,ofs,lofs;
	unsigned long *sptr,*sptr1,*sptr2;
    int rv[7],gv[7],bv[7];
    unsigned long rgbv[7];
    int cl,w,h;
    static int lcdUpdateCount=0;

    lcdUpdateCount++;
    if (lcdUpdateCount>=lcdUpdateFreq)
        lcdUpdateCount=0;
    else
        return;
    lcdUpdateNeeded++;
    SetEvent(lcdUpdateEvent);
    w=lcdWidth*scaleFact;
    h=lcdHeight*scaleFact;
    irv[0]=irvC[0]; igv[0]=igvC[0]; ibv[0]=ibvC[0];
    irv[6]=irvC[1]; igv[6]=igvC[1]; ibv[6]=ibvC[1];
    if (calc==89)
        cl=contrast;
    else if (calc>=92)
        cl=31-contrast;
    else if ((calc==82)||(calc==85))
        cl=contrast+((calc==85)?4:0);
    else if (calc==83)
        cl=((contrast-12)*32)/52;
    else
        cl=contrast;
    if (cpuCompleteStop) cl+=6;
    if (romErase)
    {
        if (calc!=89)
            cl+=contrastDelta[romErasePhase>>3];
        romErasePhase++;
        if (romErasePhase==64)
        {
            romErasePhase=0;
            romErase=0;
        }
    }
    if (cl>31) cl=31;
    if (cl<0) cl=0;
    if (!lcdOff)
    {
        if (cl>16)
        {
            rv[0]=irv[0]; rv[6]=irv[0]+((irv[6]-irv[0])*(32-cl))/16;
            gv[0]=igv[0]; gv[6]=igv[0]+((igv[6]-igv[0])*(32-cl))/16;
            bv[0]=ibv[0]; bv[6]=ibv[0]+((ibv[6]-ibv[0])*(32-cl))/16;
            rv[1]=rv[0]+(rv[6]-rv[0])/6;
            rv[2]=rv[0]+((rv[6]-rv[0])*2)/6;
            rv[3]=rv[0]+((rv[6]-rv[0])*3)/6;
            rv[4]=rv[0]+((rv[6]-rv[0])*4)/6;
            rv[5]=rv[0]+((rv[6]-rv[0])*5)/6;
            gv[1]=gv[0]+(gv[6]-gv[0])/6;
            gv[2]=gv[0]+((gv[6]-gv[0])*2)/6;
            gv[3]=gv[0]+((gv[6]-gv[0])*3)/6;
            gv[4]=gv[0]+((gv[6]-gv[0])*4)/6;
            gv[5]=gv[0]+((gv[6]-gv[0])*5)/6;
            bv[1]=bv[0]+(bv[6]-bv[0])/6;
            bv[2]=bv[0]+((bv[6]-bv[0])*2)/6;
            bv[3]=bv[0]+((bv[6]-bv[0])*3)/6;
            bv[4]=bv[0]+((bv[6]-bv[0])*4)/6;
            bv[5]=bv[0]+((bv[6]-bv[0])*5)/6;
        }
        else
        {
            rv[6]=irv[6]; rv[0]=irv[6]-((irv[6]-irv[0])*cl)/16;
            gv[6]=igv[6]; gv[0]=igv[6]-((igv[6]-igv[0])*cl)/16;
            bv[6]=ibv[6]; bv[0]=ibv[6]-((ibv[6]-ibv[0])*cl)/16;
            rv[1]=rv[0]+(rv[6]-rv[0])/6;
            rv[2]=rv[0]+((rv[6]-rv[0])*2)/6;
            rv[3]=rv[0]+((rv[6]-rv[0])*3)/6;
            rv[4]=rv[0]+((rv[6]-rv[0])*4)/6;
            rv[5]=rv[0]+((rv[6]-rv[0])*5)/6;
            gv[1]=gv[0]+(gv[6]-gv[0])/6;
            gv[2]=gv[0]+((gv[6]-gv[0])*2)/6;
            gv[3]=gv[0]+((gv[6]-gv[0])*3)/6;
            gv[4]=gv[0]+((gv[6]-gv[0])*4)/6;
            gv[5]=gv[0]+((gv[6]-gv[0])*5)/6;
            bv[1]=bv[0]+(bv[6]-bv[0])/6;
            bv[2]=bv[0]+((bv[6]-bv[0])*2)/6;
            bv[3]=bv[0]+((bv[6]-bv[0])*3)/6;
            bv[4]=bv[0]+((bv[6]-bv[0])*4)/6;
            bv[5]=bv[0]+((bv[6]-bv[0])*5)/6;
        }
        for (j=0;j<7;j++)
            rgbv[j]=bv[j]|(gv[j]<<8)|(rv[j]<<16);
        int size=lcdLineBytes*lcdHeight;
    	for (i=0;i<size;i++)
    		oldScreen[scrNum][i]=cpu_readmem24(lcdBase+i);
        oldScreenBase[scrNum]=lcdBase;
    	scrNum=(scrNum+1)%6;
        if (scrNum!=0) return;
        if (scaleFact==1)
        {
        	for (y=0,lofs=0;y<lcdHeight;y++,lofs+=lcdLineBytes)
        	{
    	    	ofs=lofs;
        		for (x=0,sptr=(unsigned long*)ScreenLine[y];x<lcdWidth;x+=8,ofs++)
        		{
    	    		v0=oldScreen[0][ofs];
    		    	v1=oldScreen[1][ofs];
        			v2=oldScreen[2][ofs];
        			v3=oldScreen[3][ofs];
    	    		v4=oldScreen[4][ofs];
    		    	v5=oldScreen[5][ofs];
        			for (i=7,b=0x80;i>=0;i--,b>>=1)
        			{
                        j=6-(((v0&b)+(v1&b)+(v2&b)+(v3&b)+(v4&b)+(v5&b))>>i);
                        *(sptr++)=rgbv[j];
    	    		}
    		    }
            }
    	}
        else
        {
        	for (y=0,lofs=0;y<lcdHeight;y++,lofs+=lcdLineBytes)
        	{
    	    	ofs=lofs;
        		for (x=0,sptr1=(unsigned long*)ScreenLine[y<<1],sptr2=
                    (unsigned long*)ScreenLine[(y<<1)+1];x<lcdWidth;x+=8,
                    ofs++)
        		{
    	    		v0=oldScreen[0][ofs];
    		    	v1=oldScreen[1][ofs];
        			v2=oldScreen[2][ofs];
        			v3=oldScreen[3][ofs];
    	    		v4=oldScreen[4][ofs];
    		    	v5=oldScreen[5][ofs];
        			for (i=7,b=0x80;i>=0;i--,b>>=1)
        			{
                        j=6-(((v0&b)+(v1&b)+(v2&b)+(v3&b)+(v4&b)+(v5&b))>>i);
                        unsigned long v=rgbv[j];
                        *(sptr1++)=v; *(sptr1++)=v;
                        *(sptr2++)=v; *(sptr2++)=v;
    	    		}
    		    }
            }
        }
    }
    else
    {
        for (j=0;j<7;j++)
            rgbv[j]=ibv[j]|(igv[j]<<8)|(irv[j]<<16);
        for (y=0,lofs=0;y<h;y++,
            lofs+=lcdLineBytes)
        {
    		for (x=0,sptr=(unsigned long*)ScreenLine[y];x<w;x++)
	    	{
                *(sptr++)=rgbv[6];
	    	}
    	}
        if ((!cpuCompleteStop)&&(calc>=89))
        {
    		for (x=0,sptr=(unsigned long*)ScreenLine[43*scaleFact];x<w;x++)
	    	{
                *(sptr++)=rgbv[0];
	    	}
        }
    }
}

void RefreshLCDNow()
{
	int x,y,i,j,b,v0,v1,v2,v3,v4,v5,v,ofs,lofs;
	unsigned long *sptr,*sptr1,*sptr2;
    int rv[7],gv[7],bv[7];
    unsigned long rgbv[7];
    int cl,w,h;

    w=lcdWidth*scaleFact;
    h=lcdHeight*scaleFact;
    irv[0]=irvC[0]; igv[0]=igvC[0]; ibv[0]=ibvC[0];
    irv[6]=irvC[1]; igv[6]=igvC[1]; ibv[6]=ibvC[1];
    if (calc==89)
        cl=contrast;
    else if (calc>=92)
        cl=31-contrast;
    else if ((calc==82)||(calc==85))
        cl=contrast+((calc==85)?4:0);
    else if (calc==83)
        cl=((contrast-12)*32)/52;
    else
        cl=contrast;
    if (cpuCompleteStop) cl+=6;
    if (romErase)
    {
        if (calc!=89)
            cl+=contrastDelta[romErasePhase>>3];
        romErasePhase++;
        if (romErasePhase==64)
        {
            romErasePhase=0;
            romErase=0;
        }
    }
    if (cl>31) cl=31;
    if (cl<0) cl=0;
    if (!lcdOff)
    {
        if (cl>16)
        {
            rv[0]=irv[0]; rv[6]=irv[0]+((irv[6]-irv[0])*(32-cl))/16;
            gv[0]=igv[0]; gv[6]=igv[0]+((igv[6]-igv[0])*(32-cl))/16;
            bv[0]=ibv[0]; bv[6]=ibv[0]+((ibv[6]-ibv[0])*(32-cl))/16;
            rv[1]=rv[0]+(rv[6]-rv[0])/6;
            rv[2]=rv[0]+((rv[6]-rv[0])*2)/6;
            rv[3]=rv[0]+((rv[6]-rv[0])*3)/6;
            rv[4]=rv[0]+((rv[6]-rv[0])*4)/6;
            rv[5]=rv[0]+((rv[6]-rv[0])*5)/6;
            gv[1]=gv[0]+(gv[6]-gv[0])/6;
            gv[2]=gv[0]+((gv[6]-gv[0])*2)/6;
            gv[3]=gv[0]+((gv[6]-gv[0])*3)/6;
            gv[4]=gv[0]+((gv[6]-gv[0])*4)/6;
            gv[5]=gv[0]+((gv[6]-gv[0])*5)/6;
            bv[1]=bv[0]+(bv[6]-bv[0])/6;
            bv[2]=bv[0]+((bv[6]-bv[0])*2)/6;
            bv[3]=bv[0]+((bv[6]-bv[0])*3)/6;
            bv[4]=bv[0]+((bv[6]-bv[0])*4)/6;
            bv[5]=bv[0]+((bv[6]-bv[0])*5)/6;
        }
        else
        {
            rv[6]=irv[6]; rv[0]=irv[6]-((irv[6]-irv[0])*cl)/16;
            gv[6]=igv[6]; gv[0]=igv[6]-((igv[6]-igv[0])*cl)/16;
            bv[6]=ibv[6]; bv[0]=ibv[6]-((ibv[6]-ibv[0])*cl)/16;
            rv[1]=rv[0]+(rv[6]-rv[0])/6;
            rv[2]=rv[0]+((rv[6]-rv[0])*2)/6;
            rv[3]=rv[0]+((rv[6]-rv[0])*3)/6;
            rv[4]=rv[0]+((rv[6]-rv[0])*4)/6;
            rv[5]=rv[0]+((rv[6]-rv[0])*5)/6;
            gv[1]=gv[0]+(gv[6]-gv[0])/6;
            gv[2]=gv[0]+((gv[6]-gv[0])*2)/6;
            gv[3]=gv[0]+((gv[6]-gv[0])*3)/6;
            gv[4]=gv[0]+((gv[6]-gv[0])*4)/6;
            gv[5]=gv[0]+((gv[6]-gv[0])*5)/6;
            bv[1]=bv[0]+(bv[6]-bv[0])/6;
            bv[2]=bv[0]+((bv[6]-bv[0])*2)/6;
            bv[3]=bv[0]+((bv[6]-bv[0])*3)/6;
            bv[4]=bv[0]+((bv[6]-bv[0])*4)/6;
            bv[5]=bv[0]+((bv[6]-bv[0])*5)/6;
        }
        for (j=0;j<7;j++)
            rgbv[j]=bv[j]|(gv[j]<<8)|(rv[j]<<16);
        for (j=0;j<6;j++)
        {
        	for (i=0;i<(lcdLineBytes*lcdHeight);i++)
	        	oldScreen[j][i]=cpu_readmem24(oldScreenBase[j]+i);
        }
        if (scaleFact==1)
        {
        	for (y=0,lofs=0;y<lcdHeight;y++,lofs+=lcdLineBytes)
        	{
    	    	ofs=lofs;
        		for (x=0,sptr=(unsigned long*)ScreenLine[y];x<lcdWidth;x+=8,ofs++)
        		{
    	    		v0=oldScreen[0][ofs];
    		    	v1=oldScreen[1][ofs];
        			v2=oldScreen[2][ofs];
        			v3=oldScreen[3][ofs];
    	    		v4=oldScreen[4][ofs];
    		    	v5=oldScreen[5][ofs];
        			for (i=7,b=0x80;i>=0;i--,b>>=1)
        			{
                        j=6-(((v0&b)+(v1&b)+(v2&b)+(v3&b)+(v4&b)+(v5&b))>>i);
                        *(sptr++)=rgbv[j];
    	    		}
    		    }
            }
    	}
        else
        {
        	for (y=0,lofs=0;y<lcdHeight;y++,lofs+=lcdLineBytes)
        	{
    	    	ofs=lofs;
        		for (x=0,sptr1=(unsigned long*)ScreenLine[y<<1],sptr2=
                    (unsigned long*)ScreenLine[(y<<1)+1];x<lcdWidth;x+=8,
                    ofs++)
        		{
    	    		v0=oldScreen[0][ofs];
    		    	v1=oldScreen[1][ofs];
        			v2=oldScreen[2][ofs];
        			v3=oldScreen[3][ofs];
    	    		v4=oldScreen[4][ofs];
    		    	v5=oldScreen[5][ofs];
        			for (i=7,b=0x80;i>=0;i--,b>>=1)
        			{
                        j=6-(((v0&b)+(v1&b)+(v2&b)+(v3&b)+(v4&b)+(v5&b))>>i);
                        unsigned long v=rgbv[j];
                        *(sptr1++)=v; *(sptr1++)=v;
                        *(sptr2++)=v; *(sptr2++)=v;
    	    		}
    		    }
            }
        }
    }
    else
    {
        for (j=0;j<7;j++)
            rgbv[j]=ibv[j]|(igv[j]<<8)|(irv[j]<<16);
        for (y=0,lofs=0;y<h;y++,
            lofs+=lcdLineBytes)
        {
    		for (x=0,sptr=(unsigned long*)ScreenLine[y];x<w;x++)
	    	{
                *(sptr++)=rgbv[6];
	    	}
    	}
        if ((!cpuCompleteStop)&&(calc>=89))
        {
    		for (x=0,sptr=(unsigned long*)ScreenLine[43*scaleFact];x<w;x++)
	    	{
                *(sptr++)=rgbv[0];
	    	}
        }
    }
}

int ROMImageCalc(int rom)
{
    if (romImage[rom].type&ROMFLAG_TI82)
        return 82;
    else if (romImage[rom].type&ROMFLAG_TI83)
        return 83;
    else if (romImage[rom].type&ROMFLAG_TI85)
        return 85;
    else if (romImage[rom].type&ROMFLAG_TI86)
        return 86;
    else if (romImage[rom].type&ROMFLAG_TI89)
        return 89;
    else if (romImage[rom].type&ROMFLAG_FLASHROM)
        return 94;
    else if (romImage[rom].size==1048576)
        return 92;
    else
        return 93;
}

int ROMImageVer(int rom)
{
    return romImage[rom].version;
}

void SaveState()
{
    char str[256];
    char oldPath[256];

    getcwd(oldPath,256);
    chdir(initPath);
    strcpy(str,romImage[currentROM].file);
    str[strlen(str)-3]='s';
    str[strlen(str)-2]='a';
    str[strlen(str)-1]='v';
    hw->SaveState(str);
    chdir(oldPath);
}

void LoadState()
{
    char str[256];
    char oldPath[256];

    getcwd(oldPath,256);
    chdir(initPath);
    strcpy(str,romImage[currentROM].file);
    str[strlen(str)-3]='s';
    str[strlen(str)-2]='a';
    str[strlen(str)-1]='v';
    hw->LoadState(str);
    chdir(oldPath);
}

Graphics::TBitmap* GetLCD_BW()
{
	int x,y,i,j,b,v0,v1,v2,v3,v4,v5,v,ofs,lofs;
	unsigned long *sptr,*sptr1,*sptr2;
    unsigned long rgbv[7];
    unsigned long* ScreenLine[256];
    int cl,w,h;
    Graphics::TBitmap *bmp;

    for (x=0;x<7;x++)
    {
        y=(255*x)/6;
        rgbv[x]=(y<<16)|(y<<8)|y;
    }
    {
        bmp=new Graphics::TBitmap;
        bmp->Width=lcdWidth;
        bmp->Height=lcdHeight;
        bmp->PixelFormat=pf32bit;
        for (int i=0;i<lcdHeight;i++)
            ScreenLine[i]=(unsigned long *)bmp->ScanLine[i];
    	for (y=0,lofs=0;y<lcdHeight;y++,lofs+=lcdLineBytes)
    	{
	    	ofs=lofs;
    		for (x=0,sptr=(unsigned long*)ScreenLine[y];x<lcdWidth;x+=8,ofs++)
    		{
	    		v0=oldScreen[0][ofs];
		    	v1=oldScreen[1][ofs];
    			v2=oldScreen[2][ofs];
       			v3=oldScreen[3][ofs];
   	    		v4=oldScreen[4][ofs];
   		    	v5=oldScreen[5][ofs];
       			for (i=7,b=0x80;i>=0;i--,b>>=1)
       			{
                    j=6-(((v0&b)+(v1&b)+(v2&b)+(v3&b)+(v4&b)+(v5&b))>>i);
                    *(sptr++)=rgbv[j];
   	    		}
   		    }
        }
  	}
    return bmp;
}

Graphics::TBitmap* GetLCD_True()
{
	int x,y,i,j,b,v0,v1,v2,v3,v4,v5,v,ofs,lofs;
	unsigned long *sptr,*sptr1,*sptr2;
    int rv[7],gv[7],bv[7];
    unsigned long rgbv[7];
    unsigned long* ScreenLine[256];
    int cl,w,h;
    Graphics::TBitmap *bmp;

    irv[0]=irvC[0]; igv[0]=igvC[0]; ibv[0]=ibvC[0];
    irv[6]=irvC[1]; igv[6]=igvC[1]; ibv[6]=ibvC[1];
    if (calc==89)
        cl=contrast;
    else if (calc>=92)
        cl=31-contrast;
    else if ((calc==82)||(calc==85))
        cl=contrast+((calc==85)?4:0);
    else if (calc==83)
        cl=((contrast-12)*32)/52;
    else
        cl=contrast;
    if (cpuCompleteStop) cl+=6;
    if (romErase)
    {
        if (calc!=89)
            cl+=contrastDelta[romErasePhase>>3];
        romErasePhase++;
        if (romErasePhase==64)
        {
            romErasePhase=0;
            romErase=0;
        }
    }
    if (cl>31) cl=31;
    if (cl<0) cl=0;
    if (cl>16)
    {
        rv[0]=irv[0]; rv[6]=irv[0]+((irv[6]-irv[0])*(32-cl))/16;
        gv[0]=igv[0]; gv[6]=igv[0]+((igv[6]-igv[0])*(32-cl))/16;
        bv[0]=ibv[0]; bv[6]=ibv[0]+((ibv[6]-ibv[0])*(32-cl))/16;
        rv[1]=rv[0]+(rv[6]-rv[0])/6;
        rv[2]=rv[0]+((rv[6]-rv[0])*2)/6;
        rv[3]=rv[0]+((rv[6]-rv[0])*3)/6;
        rv[4]=rv[0]+((rv[6]-rv[0])*4)/6;
        rv[5]=rv[0]+((rv[6]-rv[0])*5)/6;
        gv[1]=gv[0]+(gv[6]-gv[0])/6;
        gv[2]=gv[0]+((gv[6]-gv[0])*2)/6;
        gv[3]=gv[0]+((gv[6]-gv[0])*3)/6;
        gv[4]=gv[0]+((gv[6]-gv[0])*4)/6;
        gv[5]=gv[0]+((gv[6]-gv[0])*5)/6;
        bv[1]=bv[0]+(bv[6]-bv[0])/6;
        bv[2]=bv[0]+((bv[6]-bv[0])*2)/6;
        bv[3]=bv[0]+((bv[6]-bv[0])*3)/6;
        bv[4]=bv[0]+((bv[6]-bv[0])*4)/6;
        bv[5]=bv[0]+((bv[6]-bv[0])*5)/6;
    }
    else
    {
        rv[6]=irv[6]; rv[0]=irv[6]-((irv[6]-irv[0])*cl)/16;
        gv[6]=igv[6]; gv[0]=igv[6]-((igv[6]-igv[0])*cl)/16;
        bv[6]=ibv[6]; bv[0]=ibv[6]-((ibv[6]-ibv[0])*cl)/16;
        rv[1]=rv[0]+(rv[6]-rv[0])/6;
        rv[2]=rv[0]+((rv[6]-rv[0])*2)/6;
        rv[3]=rv[0]+((rv[6]-rv[0])*3)/6;
        rv[4]=rv[0]+((rv[6]-rv[0])*4)/6;
        rv[5]=rv[0]+((rv[6]-rv[0])*5)/6;
        gv[1]=gv[0]+(gv[6]-gv[0])/6;
        gv[2]=gv[0]+((gv[6]-gv[0])*2)/6;
        gv[3]=gv[0]+((gv[6]-gv[0])*3)/6;
        gv[4]=gv[0]+((gv[6]-gv[0])*4)/6;
        gv[5]=gv[0]+((gv[6]-gv[0])*5)/6;
        bv[1]=bv[0]+(bv[6]-bv[0])/6;
        bv[2]=bv[0]+((bv[6]-bv[0])*2)/6;
        bv[3]=bv[0]+((bv[6]-bv[0])*3)/6;
        bv[4]=bv[0]+((bv[6]-bv[0])*4)/6;
        bv[5]=bv[0]+((bv[6]-bv[0])*5)/6;
    }
    for (j=0;j<7;j++)
        rgbv[j]=bv[j]|(gv[j]<<8)|(rv[j]<<16);
    {
        bmp=new Graphics::TBitmap;
        bmp->Width=lcdWidth;
        bmp->Height=lcdHeight;
        bmp->PixelFormat=pf32bit;
        for (int i=0;i<lcdHeight;i++)
            ScreenLine[i]=(unsigned long *)bmp->ScanLine[i];
    	for (y=0,lofs=0;y<lcdHeight;y++,lofs+=lcdLineBytes)
    	{
	    	ofs=lofs;
    		for (x=0,sptr=(unsigned long*)ScreenLine[y];x<lcdWidth;x+=8,ofs++)
    		{
	    		v0=oldScreen[0][ofs];
		    	v1=oldScreen[1][ofs];
    			v2=oldScreen[2][ofs];
       			v3=oldScreen[3][ofs];
   	    		v4=oldScreen[4][ofs];
   		    	v5=oldScreen[5][ofs];
       			for (i=7,b=0x80;i>=0;i--,b>>=1)
       			{
                    j=6-(((v0&b)+(v1&b)+(v2&b)+(v3&b)+(v4&b)+(v5&b))>>i);
                    *(sptr++)=rgbv[j];
   	    		}
   		    }
        }
  	}
    return bmp;
}

Graphics::TBitmap* GetLCD_Calc(Graphics::TBitmap *calcImage)
{
	int x,y,i,j,b,v0,v1,v2,v3,v4,v5,v,ofs,lofs;
	unsigned long *sptr,*sptr1,*sptr2;
    int rv[7],gv[7],bv[7];
    unsigned long rgbv[7];
    unsigned long* ScreenLine[256];
    int cl,w,h;
    Graphics::TBitmap *bmp,*lcd;

    irv[0]=irvC[0]; igv[0]=igvC[0]; ibv[0]=ibvC[0];
    irv[6]=irvC[1]; igv[6]=igvC[1]; ibv[6]=ibvC[1];
    if (calc==89)
        cl=contrast;
    else if (calc>=92)
        cl=31-contrast;
    else if ((calc==82)||(calc==85))
        cl=contrast+((calc==85)?4:0);
    else if (calc==83)
        cl=((contrast-12)*32)/52;
    else
        cl=contrast;
    if (cpuCompleteStop) cl+=6;
    if (romErase)
    {
        if (calc!=89)
            cl+=contrastDelta[romErasePhase>>3];
        romErasePhase++;
        if (romErasePhase==64)
        {
            romErasePhase=0;
            romErase=0;
        }
    }
    if (cl>31) cl=31;
    if (cl<0) cl=0;
    if (cl>16)
    {
        rv[0]=irv[0]; rv[6]=irv[0]+((irv[6]-irv[0])*(32-cl))/16;
        gv[0]=igv[0]; gv[6]=igv[0]+((igv[6]-igv[0])*(32-cl))/16;
        bv[0]=ibv[0]; bv[6]=ibv[0]+((ibv[6]-ibv[0])*(32-cl))/16;
        rv[1]=rv[0]+(rv[6]-rv[0])/6;
        rv[2]=rv[0]+((rv[6]-rv[0])*2)/6;
        rv[3]=rv[0]+((rv[6]-rv[0])*3)/6;
        rv[4]=rv[0]+((rv[6]-rv[0])*4)/6;
        rv[5]=rv[0]+((rv[6]-rv[0])*5)/6;
        gv[1]=gv[0]+(gv[6]-gv[0])/6;
        gv[2]=gv[0]+((gv[6]-gv[0])*2)/6;
        gv[3]=gv[0]+((gv[6]-gv[0])*3)/6;
        gv[4]=gv[0]+((gv[6]-gv[0])*4)/6;
        gv[5]=gv[0]+((gv[6]-gv[0])*5)/6;
        bv[1]=bv[0]+(bv[6]-bv[0])/6;
        bv[2]=bv[0]+((bv[6]-bv[0])*2)/6;
        bv[3]=bv[0]+((bv[6]-bv[0])*3)/6;
        bv[4]=bv[0]+((bv[6]-bv[0])*4)/6;
        bv[5]=bv[0]+((bv[6]-bv[0])*5)/6;
    }
    else
    {
        rv[6]=irv[6]; rv[0]=irv[6]-((irv[6]-irv[0])*cl)/16;
        gv[6]=igv[6]; gv[0]=igv[6]-((igv[6]-igv[0])*cl)/16;
        bv[6]=ibv[6]; bv[0]=ibv[6]-((ibv[6]-ibv[0])*cl)/16;
        rv[1]=rv[0]+(rv[6]-rv[0])/6;
        rv[2]=rv[0]+((rv[6]-rv[0])*2)/6;
        rv[3]=rv[0]+((rv[6]-rv[0])*3)/6;
        rv[4]=rv[0]+((rv[6]-rv[0])*4)/6;
        rv[5]=rv[0]+((rv[6]-rv[0])*5)/6;
        gv[1]=gv[0]+(gv[6]-gv[0])/6;
        gv[2]=gv[0]+((gv[6]-gv[0])*2)/6;
        gv[3]=gv[0]+((gv[6]-gv[0])*3)/6;
        gv[4]=gv[0]+((gv[6]-gv[0])*4)/6;
        gv[5]=gv[0]+((gv[6]-gv[0])*5)/6;
        bv[1]=bv[0]+(bv[6]-bv[0])/6;
        bv[2]=bv[0]+((bv[6]-bv[0])*2)/6;
        bv[3]=bv[0]+((bv[6]-bv[0])*3)/6;
        bv[4]=bv[0]+((bv[6]-bv[0])*4)/6;
        bv[5]=bv[0]+((bv[6]-bv[0])*5)/6;
    }
    for (j=0;j<7;j++)
        rgbv[j]=bv[j]|(gv[j]<<8)|(rv[j]<<16);
    {
        bmp=new Graphics::TBitmap;
        bmp->Width=calcImage->Width;
        bmp->Height=calcImage->Height;
        bmp->PixelFormat=pf32bit;
        lcd=new Graphics::TBitmap;
        lcd->Width=lcdWidth<<1;
        lcd->Height=lcdHeight<<1;
        lcd->PixelFormat=pf32bit;
        for (int i=0;i<(lcdHeight<<1);i++)
            ScreenLine[i]=(unsigned long *)lcd->ScanLine[i];
    	for (y=0,lofs=0;y<lcdHeight;y++,lofs+=lcdLineBytes)
    	{
  	    	ofs=lofs;
      		for (x=0,sptr1=(unsigned long*)ScreenLine[y<<1],sptr2=
                (unsigned long*)ScreenLine[(y<<1)+1];x<lcdWidth;x+=8,
                ofs++)
     		{
 	    		v0=oldScreen[0][ofs];
  		    	v1=oldScreen[1][ofs];
      			v2=oldScreen[2][ofs];
      			v3=oldScreen[3][ofs];
  	    		v4=oldScreen[4][ofs];
   		    	v5=oldScreen[5][ofs];
       			for (i=7,b=0x80;i>=0;i--,b>>=1)
       			{
                    j=6-(((v0&b)+(v1&b)+(v2&b)+(v3&b)+(v4&b)+(v5&b))>>i);
                    unsigned long v=rgbv[j];
                    *(sptr1++)=v; *(sptr1++)=v;
                    *(sptr2++)=v; *(sptr2++)=v;
   	    		}
   		    }
        }
        if (calcImage)
            bmp->Canvas->Draw(0,0,calcImage);
        bmp->Canvas->Draw(skinLcd.left,skinLcd.top,lcd);
        delete lcd;
    }
    return bmp;
}

void AddSkin(char *name,int size)
{
    FILE *fp=fopen(name,"rb");
    if (!fp) return;
    char str[9];
    fread(str,8,1,fp);
    str[8]=0;
    int version;
    if (strcmp(str,"VTIv2.1 "))
    {
        if (strcmp(str,"VTIv2.5 "))
        {
            fclose(fp);
            return;
        }
        version=25;
    }
    else
        version=21;
    strcpy(skin[skinCount].file,name);
    fread(skin[skinCount].name,64,1,fp);
    if (version>=25)
        fread(skin[skinCount].author,64,1,fp);
    else
        skin[skinCount].author[0]=0;
    fread(&skin[skinCount].calc,4,1,fp);
    fseek(fp,0,SEEK_SET);
    skin[skinCount].csum=0;
    for (int i=0;i<size;i++)
        skin[skinCount].csum+=fgetc(fp)&0xff;
    TRegistry *reg=new TRegistry;
    reg->RootKey=HKEY_LOCAL_MACHINE;
    reg->OpenKey("\\Software\\ACZ\\Virtual TI\\Skins",true);
    char keyName[96];
    sprintf(keyName,"%s",name);
    skin[skinCount].defW=0;
    skin[skinCount].defH=0;
    if (reg->OpenKey(keyName,false))
    {
        if (reg->ValueExists("Checksum"))
        {
            if (reg->ReadInteger("Checksum")==
                skin[skinCount].csum)
            {
                if ((reg->ValueExists("Width"))&&
                    (reg->ValueExists("Height")))
                {
                    skin[skinCount].defW=reg->ReadInteger("Width");
                    skin[skinCount].defH=reg->ReadInteger("Height");
                }
            }
        }
    }
    reg->CloseKey();
    delete reg;
    fclose(fp);
    skinCount++;
}

void AnalyzeSkins()
{
    ffblk ff;
    int done;

    skinCount=0;
    done=findfirst("*.skn",&ff,FA_RDONLY|FA_ARCH);
    while (!done)
    {
        AddSkin(ff.ff_name,ff.ff_fsize);
        done=findnext(&ff);
    }
}

void LoadSkin(int n)
{
    char filename[256];
    if (initPath[0])
    {
        sprintf(filename,"%s%s%s",initPath,(initPath[strlen(
            initPath)-1]=='\\')?"":"\\",skin[n].file);
    }
    else
        strcpy(filename,skin[n].file);
    FILE *fp=fopen(filename,"rb");
    if (!fp) return;
    char str[9];
    fread(str,8,1,fp);
    str[8]=0;
    if (!strcmp(str,"VTIv2.1 "))
        fseek(fp,80,SEEK_SET);
    else
        fseek(fp,144,SEEK_SET);
    TColor white,black;
    fread(&white,4,1,fp);
    fread(&black,4,1,fp);
    irvC[0]=((int)black)&0xff; irvC[1]=((int)white)&0xff;
    igvC[0]=(((int)black)>>8)&0xff; igvC[1]=(((int)white)>>8)&0xff;
    ibvC[0]=(((int)black)>>16)&0xff; ibvC[1]=(((int)white)>>16)&0xff;
    fread(&skinLcd,sizeof(RECT),1,fp);
    fread(skinKey,sizeof(RECT),80,fp);
    Screen->Cursor=crHourGlass;
    if (!skinImageLg) skinImageLg=new Graphics::TBitmap;
    skinImageLg->PixelFormat=pf24bit;
    ReadJPEG(fp,skinImageLg);
    if (!skinImageSm) skinImageSm=new Graphics::TBitmap;
    skinImageSm->Width=skinImageLg->Width>>1;
    skinImageSm->Height=skinImageLg->Height>>1;
    skinImageSm->PixelFormat=pf24bit;
    TRect r;
    r.Left=0; r.Right=skinImageSm->Width;
    r.Top=0; r.Bottom=skinImageSm->Height;
    skinImageSm->Canvas->StretchDraw(r,skinImageLg);
    Screen->Cursor=crDefault;
    fclose(fp);
    currentSkin=n;
}

void CloseSkin(int saveSkin)
{
    if (saveSkin)
    {
        TRegistry *reg=new TRegistry;
        reg->RootKey=HKEY_LOCAL_MACHINE;
        if ((currentSkin!=-1)&&(!EmuWnd->Fullscreenview1->Checked))
        {
            reg->OpenKey("\\Software\\ACZ\\Virtual TI\\Skins",true);
            char keyName[96];
            sprintf(keyName,"%s",skin[currentSkin].file);
            reg->OpenKey(keyName,true);
            reg->WriteInteger("Checksum",(int)((unsigned long)
                skin[currentSkin].csum));
            reg->WriteInteger("Width",EmuWnd->Image->Width);
            reg->WriteInteger("Height",EmuWnd->Image->Height);
            skin[currentSkin].defW=EmuWnd->Image->Width;
            skin[currentSkin].defH=EmuWnd->Image->Height;
            reg->CloseKey();
        }
        reg->OpenKey("\\Software\\ACZ\\Virtual TI\\Skins",true);
        char keyName[24];
        switch (calc)
        {
            case 73: strcpy(keyName,"DefaultSkin73"); break;
            case 82: strcpy(keyName,"DefaultSkin82"); break;
            case 83: strcpy(keyName,"DefaultSkin83"); break;
            case 84: strcpy(keyName,"DefaultSkin83Plus"); break;
            case 85: strcpy(keyName,"DefaultSkin85"); break;
            case 86: strcpy(keyName,"DefaultSkin86"); break;
            case 89: strcpy(keyName,"DefaultSkin89"); break;
            case 92: case 93: strcpy(keyName,"DefaultSkin92"); break;
            case 94: strcpy(keyName,"DefaultSkin92Plus"); break;
        }
        if (currentSkin!=-1)
            reg->WriteString(keyName,skin[currentSkin].file);
        else
            reg->WriteString(keyName,"");
        reg->CloseKey();
        delete reg;
    }
    if (skinImageLg) { delete skinImageLg; skinImageLg=NULL; }
    if (skinImageSm) { delete skinImageSm; skinImageSm=NULL; }
    skinLcd.left=0; skinLcd.right=lcdWidth<<1;
    skinLcd.top=0; skinLcd.bottom=lcdHeight<<1;
    currentSkin=-1;
    irvC[0]=0; irvC[1]=255;
    igvC[0]=0; igvC[1]=255;
    ibvC[0]=0; ibvC[1]=255;
}

int GetKeyAt(int x,int y)
{
    int i,found;

    for (i=0,found=-1;skinKey[i].left>0;i++)
    {
        if ((x>=skinKey[i].left)&&(x<skinKey[i].right)&&
            (y>=skinKey[i].top)&&(y<skinKey[i].bottom))
        {
            found=i;
            break;
        }
    }
    if (found!=-1)
        return hw->GetKeyID(found);
    return -1;
}

int CheckRecvBuf()
{
    if (recvBuf.start==recvBuf.end)
        return 0;
    return 1;
}

BYTE GetRecvBuf()
{
    BYTE ret=recvBuf.buf[recvBuf.start];
    recvBuf.start=(recvBuf.start+1)&255;
    return ret;
}

int PutRecvBuf(BYTE b)
{
    if (((recvBuf.end+1)&255)==recvBuf.start)
        return 0;
    recvBuf.buf[recvBuf.end]=b;
    recvBuf.end=(recvBuf.end+1)&255;
    return 1;
}

int PutRecvReady()
{
    if (((recvBuf.end+1)&255)==recvBuf.start)
        return 0;
    return 1;
}

int CheckSendBuf()
{
    if (sendBuf.start==sendBuf.end)
        return 0;
    return 1;
}

BYTE GetSendBuf()
{
    BYTE ret=sendBuf.buf[sendBuf.start];
    sendBuf.start=(sendBuf.start+1)&255;
    return ret;
}

int PutSendBuf(BYTE b)
{
    if (((sendBuf.end+1)&255)==sendBuf.start)
        return 0;
    sendBuf.buf[sendBuf.end]=b;
    sendBuf.end=(sendBuf.end+1)&255;
    return 1;
}

int PutSendReady()
{
    if (((sendBuf.end+1)&255)==sendBuf.start)
        return 0;
    return 1;
}

int CheckVRecvBuf()
{
    if (vRecvBuf->start==vRecvBuf->end)
        return 0;
    return 1;
}

BYTE GetVRecvBuf()
{
    BYTE ret=vRecvBuf->buf[vRecvBuf->start];
    vRecvBuf->start=(vRecvBuf->start+1)&255;
    return ret;
}

int PutVSendBuf(BYTE b)
{
    if (((vSendBuf->end+1)&255)==vSendBuf->start)
        return 0;
    vSendBuf->buf[vSendBuf->end]=b;
    vSendBuf->end=(vSendBuf->end+1)&255;
    return 1;
}

int PutVSendReady()
{
    if (((vSendBuf->end+1)&255)==vSendBuf->start)
        return 0;
    return 1;
}

