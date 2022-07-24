/*
	Interface routine for 68kem <-> Mame

*/

#include "Debug.h"
#include <stdio.h>
#include <mem.h>
//#include "driver.h"
#define NOTYPEREDEF
#include "M68000.H"
#include "memory.h"
//#include "osd_dbg.h"
#pragma hdrstop
#include "cycletbl.h"
#include "emu.h"
#include "rom.h"
#include "Sound.h"

//extern "C" void M68KRUN(void);
//extern "C" void M68KRESET(void);

extern void BuildCPU(void);

//#define ROMTBL(x) readmem24_dword(readmem24_dword(0xc8)+(x)*4)
//#define HANDLE_TABLE readmem24_dword(ROMTBL(0x2f)+0x11a)
//#define HANDLE(x) readmem24_dword(HANDLE_TABLE+4*(x))

int opcode;

extern int debugCodeBreakCount,debugCodeBreak[256];
extern int run,debug,debugProgBreak,debugStartPC;
extern int debugProgBreakOfs;
extern int debugEndStepOver,debugBeginStepOver;
extern int debugStepOver,debugStepGoal,debugStepA7;
extern int debugDataBreak,debugException;
extern int debugRunToCursor,debugCursorAddr;
extern int exceptBP[48];
int endStepOver,cpuCompleteStop,previouspc,ignoreStack;
int MC68000_ICount;
int lastIRQ4=0;

extern HANDLE lcdUpdateEvent;
volatile extern bool pause;
volatile extern int emuRunning;
volatile extern bool showDebugWnd;

int oldEmu=1;

union flagu regflags;

int pending_interrupts;
/* LBO 090597 - added these lines and made them extern in cpudefs.h */
int movem_index1[256];
int movem_index2[256];
int movem_next[256];
UBYTE *actadr;

regstruct regs, lastint_regs;

union flagu intel_flag_lookup[256];

/********************************************/
/* Interface routines to link Mame -> 68KEM */
/********************************************/

static int InitStatus=0;

int oldPC;

void Exception(int nr, CPTR oldpc)
{
/*    if (!oldEmu)
    {
        if ((nr<24)||(nr>=32)) return;
        MC68000_Cause_Interrupt(nr-24);
        return;
    }*/
	MC68000_ICount -= exception_cycles[nr];

   MakeSR();

   if(!regs.s) {
   	  regs.usp=regs.a[7];
      regs.a[7]=regs.isp;
      regs.s=1;
   }

   if ((exceptBP[nr])&&((nr<24)||(nr>31)))
        debugStartPC=oldPC;
   else if (exceptBP[nr])
        debugStartPC=get_long(regs.vbr + 4*nr);

   regs.a[7] -= 4;
   put_long (regs.a[7], m68k_getpc ());
   regs.a[7] -= 2;
   put_word (regs.a[7], regs.sr);
   m68k_setpc(get_long(regs.vbr + 4*nr));

   regs.t1 = regs.t0 = regs.m = 0;

   if (exceptBP[nr])
   {
        run=0;
        debug=1;
        debugException=nr;
        if ((emuRunning)&&(!pause))
        {
            showDebugWnd=true;
            SetEvent(lcdUpdateEvent);
        }
        else
        {
            DebugWnd->Show();
            DebugWnd->Update();
        }
   }
}

static void initCPU(void)
{
    int i,j;

    for (i = 0 ; i < 256 ; i++) {
      for (j = 0 ; j < 8 ; j++) {
       if (i & (1 << j)) break;
      }
     movem_index1[i] = j;
     movem_index2[i] = 7-j;
     movem_next[i] = i & (~(1 << j));
    }
    for (i = 0; i < 256; i++) {
         intel_flag_lookup[i].flags.c = !!(i & 1);
         intel_flag_lookup[i].flags.z = !!(i & 64);
         intel_flag_lookup[i].flags.n = !!(i & 128);
         intel_flag_lookup[i].flags.v = 0;
    }

}

void Initialization(void) {
   /* Init 68000 emulator */
    BuildCPU();
   initCPU();
}

void MC68000_Reset(void)
{
    SoundSetCycles(896);

if (!InitStatus)
{
        Initialization();
	InitStatus=1;
}

	memset(&regs,0,sizeof(regs));

    regs.a[7] = regs.isp = get_long(0);
    regs.pc   = get_long(4) & 0xffffff;
    regs.statusflags = 0x27;
    regs.IRQ_level = 0;
   regs.s = 1;
   regs.m = 0;
   regs.stopped = 0;
   regs.t1 = 0;
   regs.t0 = 0;
   ZFLG = CFLG = NFLG = VFLG = 0;
   regs.intmask = 7;
   regs.vbr = regs.sfc = regs.dfc = 0;
   regs.fpcr = regs.fpsr = regs.fpiar = 0;

   pending_interrupts=0;

    cpuCompleteStop=0;

    lastIRQ4=-1;

//    M68KRESET();
}


inline void Interrupt68k(int level)
{
   int ipl=(regs.sr&0xf00)>>8;
   if(level>ipl)
   {
   	::Exception(24+level,0);
   	regs.IRQ_level &= ~(1 << (level - 1));
   	regs.intmask = level;
   	MakeSR();
   }
}

extern "C" int SetupExceptionHandler();

int  MC68000_Execute(int cycles)
{
    if (pending_interrupts&MC68000_STOP) regs.IRQ_level|=0x80;
	if (regs.IRQ_level == 0x80)
    {
        if (soundEnable)
            SoundOutBits(hw->outWhite,hw->outRed,cycles);
        return cycles;		/* STOP with no IRQs */
    }
    if (cpuCompleteStop)
    {
        if (soundEnable)
            SoundOutBits(hw->outWhite,hw->outRed,cycles);
        return cycles;
    }

    int i,bp;

    static int soundTicks=0;

    endStepOver=0;
    ignoreStack=1;
//        if (updateLink)
//            EmuWnd->DoLink();
//    if (oldEmu)
    {
	MC68000_ICount = cycles;
    int bp,i;
	do
	{
        if (cpuCompleteStop)
            break;
        if ((lastIRQ4==2)&&(lastByteTicks>200))
        {
            IRQ(4);
            lastIRQ4=0;
            lastByteTicks=0;
        }
        if ((!lastIRQ4)&&(lastByteTicks>200))
        {
            if (CheckSendBuf())
            {
                IRQ(4);
                lastIRQ4=1;
            }
        }
        oldPC=regs.pc;

		if (regs.IRQ_level)
		{
			int level, mask = 0x40;
			for (level = 7; level>0; level--, mask >>= 1)
				if (regs.IRQ_level & mask)
					break;
            if (level>0)
    			Interrupt68k (level);
		}

	#if MC68000ASM_DEBUG
	{
	void MC68000_MiniTrace(unsigned long *d, unsigned long *a, unsigned long pc, unsigned long sr, int icount);
	MakeSR();
	MC68000_MiniTrace(regs.d, regs.a, regs.pc, regs.sr, MC68000_ICount);
	}
	#endif
        if (!run)
            break;

		opcode=nextiword();

		MC68000_ICount -= cycletbl[opcode];
        lastByteTicks+=cycletbl[opcode];
        if (soundEnable)
            SoundOutBits(hw->outWhite,hw->outRed,cycletbl[opcode]);
        if (!SetupExceptionHandler())
        {
    		cpufunctbl[opcode]();
            if (!debugCheckEnable)
            {
            	while (MC68000_ICount > 0)
                {
                    if (cpuCompleteStop)
                        break;
                    oldPC=regs.pc;
            		if (regs.IRQ_level)
            		{
            			int level, mask = 0x40;
            			for (level = 7; level>0; level--, mask >>= 1)
            				if (regs.IRQ_level & mask)
            					break;
                        if (level>0)
                			Interrupt68k (level);
            		}
            		opcode=nextiword();
            		MC68000_ICount -= cycletbl[opcode];
            		cpufunctbl[opcode]();
                    if (soundEnable)
                        SoundOutBits(hw->outWhite,hw->outRed,cycletbl[opcode]);
                }
            }
            HandleException();
        }

        if (!run)
            break;

        if (debugRunToCursor)
        {
            if (regs.pc==debugCursorAddr)
            {
                run=0;
                debug=1;
                debugStartPC=regs.pc;
                debugRunToCursor=0;
                if ((emuRunning)&&(!pause))
                {
                    showDebugWnd=true;
                    SetEvent(lcdUpdateEvent);
                }
                else
                {
                    DebugWnd->Show();
                    DebugWnd->Update();
                }
                break;
            }
        }

        if (debugCheckEnable)
        {
        for (i=0,bp=0;i<debugCodeBreakCount;i++)
        {
            if (regs.pc==debugCodeBreak[i])
            {
                run=0;
                bp=1;
            }
        }
        if (debugProgBreak)
        {
            int addr=hw->getmem_dword(debugProgBreak)+
                debugProgBreakOfs;
            if (regs.pc==addr)
            {
                run=0;
                bp=1;
                debugProgBreak=0;
           }
        }
        if (bp)
        {
            run=0;
            debug=1;
            debugStartPC=regs.pc;
            if ((emuRunning)&&(!pause))
            {
                showDebugWnd=true;
                SetEvent(lcdUpdateEvent);
            }
            else
            {
                DebugWnd->Show();
                DebugWnd->Update();
            }
            break;
        }
        if (debugDataBreak)
        {
            run=0;
            debug=1;
            debugStartPC=oldPC;
            if ((emuRunning)&&(!pause))
            {
                showDebugWnd=true;
                SetEvent(lcdUpdateEvent);
            }
            else
            {
                DebugWnd->Show();
                DebugWnd->Update();
            }
            break;
        }
        if (debugStepOver)
        {
            if (debugBeginStepOver)
            {
                debugBeginStepOver=0;
                if (endStepOver)
                {
                    run=0;
                    debug=1;
                    debugStartPC=regs.pc;
                    debugEndStepOver=1;
                    if ((emuRunning)&&(!pause))
                    {
                        showDebugWnd=true;
                        SetEvent(lcdUpdateEvent);
                    }
                    else
                    {
                        DebugWnd->Show();
                        DebugWnd->Update();
                    }
                    break;
                }
            }
            if (debugStepOver&&(regs.pc==debugStepGoal)&&
                ((regs.a[7]==debugStepA7)||(ignoreStack)))
            {
                run=0;
                debug=1;
                debugStartPC=regs.pc;
                debugEndStepOver=1;
                if ((emuRunning)&&(!pause))
                {
                    showDebugWnd=true;
                    SetEvent(lcdUpdateEvent);
                }
                else
                {
                    DebugWnd->Show();
                    DebugWnd->Update();
                }
                break;
            }
        }
        }
	}
	while (MC68000_ICount > 0);

   return (cycles - MC68000_ICount);
    }

/*//    if (!debugger)
    {
      	MC68000_ICount = cycles;
        regs.Icount=cycles;
        do
        {
            previouspc=regs.pc;
           M68KRUN();
        } while (regs.Icount>0);
        return (cycles - regs.Icount);
    }

  	MC68000_ICount = cycles;

    do
    {
        previouspc=regs.pc;

        regs.Icount=1;
       M68KRUN();
        MC68000_ICount-=1-regs.Icount;

        for (i=0,bp=0;i<debugCodeBreakCount;i++)
        {
            if (regs.pc==debugCodeBreak[i])
            {
                run=0;
                bp=1;
            }
        }
        if (debugProgBreak)
        {
            int addr=readmem24_dword(debugProgBreak)+
                debugProgBreakOfs;
            if (regs.pc==addr)
            {
                run=0;
                bp=1;
                debugProgBreak=0;
            }
        }
        if (bp)
        {
            run=0;
            debug=1;
            debugStartPC=regs.pc;
            DebugWnd->Show();
            DebugWnd->Update();
            break;
        }
        if (debugDataBreak)
        {
            run=0;
            debug=1;
            debugStartPC=previouspc;
            DebugWnd->Show();
            DebugWnd->Update();
            break;
        }
        if (debugStepOver)
        {
            if (debugBeginStepOver)
            {
                debugBeginStepOver=0;
                if (endStepOver)
                {
                    run=0;
                    debug=1;
                    debugStartPC=regs.pc;
                    DebugWnd->Show();
                    DebugWnd->Update();
                    debugEndStepOver=1;
                    break;
                }
            }
            if (debugStepOver&&(regs.pc==debugStepGoal))
            {
                run=0;
                debug=1;
                debugStartPC=regs.pc;
                DebugWnd->Show();
                DebugWnd->Update();
                debugEndStepOver=1;
                break;
            }
        }
    }
	while (MC68000_ICount > 0);

    return (cycles - MC68000_ICount);*/
}

void MC68000_SetRegs(MC68000_Regs *src)
{
	regs = src->regs;
}

void MC68000_GetRegs(MC68000_Regs *dst)
{
	dst->regs = regs;
}

void MC68000_Cause_Interrupt(int level)
{
	if (level >= 1 && level <= 7)
        regs.IRQ_level |= 1 << (level-1);
}

void MC68000_Clear_Pending_Interrupts(void)
{
	regs.IRQ_level = 0;
}

int  MC68000_GetPC(void)
{
    return regs.pc;
}
