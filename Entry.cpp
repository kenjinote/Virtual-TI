//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Entry.h"
#define NOTYPEREDEF
#include "cpudefs.h"
#include "rom.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"

#define ROMTBL(x) cpu_readmem24_dword(cpu_readmem24_dword(0xc8)+(x)*4)
#define HANDLE_TABLE cpu_readmem24_dword(ROMTBL(0x2f)+0x11a)
#define HANDLE_TABLE_92 cpu_readmem24_dword(romFuncAddr[0x1c]+0x1902)
#define HANDLE(x) cpu_readmem24_dword(HANDLE_TABLE+4*(x))
#define HANDLE92(x) cpu_readmem24_dword(HANDLE_TABLE_92+4*(x))
#define GETBYTE(x) (cpu_readmem24(x)&0xff)
#define GETWORD(x) (cpu_readmem24_word(x)&0xffff)

TProgramEntryDlg *ProgramEntryDlg;
extern int debugProgBreak;
//---------------------------------------------------------------------
__fastcall TProgramEntryDlg::TProgramEntryDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
void __fastcall TProgramEntryDlg::UpdateTree()
{
    Programs->Items->Clear();
    if ((calc==89)||(calc==94))
    {
        int addr=HANDLE(8)+4;
        char name[10];
        for (int i=0;i<GETWORD(addr-2);i++)
        {
            int j;
            for (j=0;j<9;j++)
            {
                name[j]=GETBYTE(addr+14*i+j);
                if ((name[j]<=' ')&&(name[j]))
                    return;
            }
            TTreeNode *p=Programs->Items->AddChild(NULL,name);
            p->Data=NULL;
            int fa=HANDLE(GETWORD(addr+14*i+12))+4;
            for (int k=0;k<GETWORD(fa-2);k++)
            {
                for (j=0;j<9;j++)
                {
                    name[j]=GETBYTE(fa+14*k+j);
                    if ((name[j]<=' ')&&(name[j]))
                        return;
                }
                int pa=HANDLE(GETWORD(fa+14*k+12));
                pa+=1+GETWORD(pa);
                if (GETBYTE(pa)==0xf3)
                {
                    TTreeNode *tn=Programs->Items->AddChild(p,
                        name);
                    int handle=GETWORD(fa+14*k+12);
                    int addr=HANDLE(handle)+2;
                    int ofs=2;
                    if ((GETBYTE(addr+4)=='6')&&(GETBYTE(addr+
                        5)=='8')&&(GETBYTE(addr+6)=='k'))
                        ofs+=GETWORD(addr+0xc);
                    tn->Data=(void*)((ofs<<16)+handle);
                    if ((HANDLE_TABLE+(handle<<2))==debugProgBreak)
                        Programs->Selected=tn;
                }
            }
        }
        Programs->FullExpand();
    }
    else if ((calc==92)||(calc==93))
    {
        int addr=HANDLE92(0xb)+4;
        char name[10];
        for (int i=0;i<GETWORD(addr-2);i++)
        {
            int j;
            for (j=0;j<8;j++)
            {
                name[j]=GETBYTE(addr+12*i+j);
                if ((name[j]<=' ')&&(name[j]))
                    return;
            }
            name[8]=0;
            TTreeNode *p=Programs->Items->AddChild(NULL,name);
            p->Data=NULL;
            int fa=HANDLE92(GETWORD(addr+12*i+10))+4;
            for (int k=0;k<GETWORD(fa-2);k++)
            {
                for (j=0;j<8;j++)
                {
                    name[j]=GETBYTE(fa+12*k+j);
                    if ((name[j]<=' ')&&(name[j]))
                        return;
                }
                name[8]=0;
                int pa=HANDLE92(GETWORD(fa+12*k+10));
                pa+=1+GETWORD(pa);
                if (GETBYTE(pa)==0xdc)
                {
                    int handle=GETWORD(fa+12*k+10);
                    int addr=HANDLE92(handle)+2;
                    if ((GETBYTE(addr)==0)&&
                        (GETBYTE(addr+1)=='2')&&
                        (GETBYTE(addr+2)=='E')&&
                        (GETBYTE(addr+3)=='X')&&
                        (GETBYTE(addr+4)=='E')&&
                        (GETBYTE(addr+5)==' '))
                    {
                        if ((GETBYTE(addr+6)=='D')&&
                            (GETBYTE(addr+7)=='L')&&
                            (GETBYTE(addr+8)=='L')&&
                            (GETBYTE(addr+9)==' '))
                            continue;
                        int ofs=GETWORD(addr+0x12);
                        if (ofs!=0)
                        {
                            ofs=GETWORD(addr+ofs);
                            TTreeNode *tn=Programs->Items->AddChild(p,
                                name);
                            tn->Data=(void*)((ofs<<16)+handle);
                            if ((HANDLE_TABLE+(handle<<2))==debugProgBreak)
                                Programs->Selected=tn;
                        }
                    }
                }
            }
        }
        Programs->FullExpand();
    }
    else if (calc==86)
    {
        int i,j,k;
        for (i=0x2bfff,j=0;i>(0x20000|hw->getmem_word(0xd298));)
        {
            int type=hw->getmem(i--);
            int addr=hw->getmem(i--);
            addr|=hw->getmem(i--)<<8;
            addr|=hw->getmem(i--)<<16;
            i--;
            k=hw->getmem(i--);
            char name[9];
            for (j=0;j<k;j++)
                name[j]=hw->getmem(i--);
            name[k]=0;
            if ((name[0]<'0')&&(k==1))
                continue;
            if (type==0x12)
            {
                TTreeNode *p=Programs->Items->AddChild(NULL,name);
                p->Data=(void*)addr;
                if (debugProgBreak==addr)
                    Programs->Selected=p;
            }
        }
        Programs->FullExpand();
    }
    else if (calc==85)
    {
        int i,j,k;
        for (i=0xfa6f,j=0;i>hw->getmem_word(0x8be5);)
        {
            int type=hw->getmem(i--);
            int iAddr=i;
            int addr=hw->getmem(i--);
            addr|=hw->getmem(i--)<<8;
            k=hw->getmem(i--);
            char name[9];
            for (j=0;j<k;j++)
                name[j]=hw->getmem(i--);
            name[k]=0;
            if ((name[0]<'0')&&(k==1))
                continue;
            if (type==0xc)
            {
                TTreeNode *p=Programs->Items->AddChild(NULL,name);
                int len=hw->getmem_word(addr);
                int csum=0;
                for (int i=addr+5;i<(addr+len);i++)
                    csum+=hw->getmem(i);
                p->Data=(void*)((csum<<16)|iAddr);
                if (debugProgBreak==((csum<<16)|iAddr))
                    Programs->Selected=p;
            }
        }
        Programs->FullExpand();
    }
    else if ((calc==82)||(calc==83))
    {
        int i,j,k;
        int prgm=0;
        for (i=0xfe6e,j=0;i>hw->getmem_word(0x8d10);)
        {
            int type=hw->getmem(i--)&0xf;
            int addr=hw->getmem(i--);
            addr|=hw->getmem(i--)<<8;
            if ((type==5)||(type==6))
            {
                prgm=1;
                k=hw->getmem(i--);
                char name[9];
                for (j=0;j<k;j++)
                    name[j]=hw->getmem(i--);
                name[k]=0;
                if ((name[0]<'0')&&(k==1))
                    continue;
                TTreeNode *p=Programs->Items->AddChild(NULL,name);
                int len=hw->getmem_word(addr);
                int csum=0;
                for (int i=addr+((calc==83)?2:5);i<(addr+len);i++)
                    csum+=hw->getmem(i);
                p->Data=(void*)((csum<<16)|len);
                if (debugProgBreak==((csum<<16)|len))
                    Programs->Selected=p;
            }
            else if ((prgm)&&(type==0))
                break;
            else
                i-=3;
        }
        Programs->FullExpand();
    }
    else if (calc==84)
    {
        int i,j,k;
        int prgm=0;
        for (i=0xfe66,j=0;;)
        {
            int type=hw->getmem(i--)&0xf;
            i-=2;
            int addr=hw->getmem(i--);
            addr|=hw->getmem(i--)<<8;
            i--;
            if ((type==5)||(type==6))
            {
                prgm=1;
                k=hw->getmem(i--);
                char name[9];
                for (j=0;j<k;j++)
                    name[j]=hw->getmem(i--);
                name[k]=0;
                if ((name[0]<'0')&&(k==1))
                    continue;
                TTreeNode *p=Programs->Items->AddChild(NULL,name);
                int len=hw->getmem_word(addr);
                int csum=0;
                for (int i=addr+4;i<(addr+len);i++)
                    csum+=hw->getmem(i);
                p->Data=(void*)((csum<<16)|len);
                if (debugProgBreak==((csum<<16)|len))
                    Programs->Selected=p;
            }
            else if ((prgm)&&(type==0))
                break;
            else if (type==1)
                i-=hw->getmem(i--);
            else
                i-=3;
        }
        Programs->FullExpand();
    }
}



void __fastcall TProgramEntryDlg::OKBtnClick(TObject *Sender)
{
    ModalResult=1;
}
//---------------------------------------------------------------------------

void __fastcall TProgramEntryDlg::CancelBtnClick(TObject *Sender)
{
    ModalResult=2;    
}
//---------------------------------------------------------------------------

