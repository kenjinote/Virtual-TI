#include "Debug.h"
#define NOTYPEREDEF
#include "M68000.h"
//#include "driver.h"
//#include "cpuintrf.h"
#include "readcpu.h"
//#include "osd_dbg.h"
#include <stdio.h>

#define ROMTBL(x) readmem24_dword(readmem24_dword(0xc8)+(x)*4)
#define HANDLE_TABLE readmem24_dword(ROMTBL(0x2f)+0x11a)
#define HANDLE(x) readmem24_dword(HANDLE_TABLE+4*(x))

extern "C" wrapper_M68KRESET();
extern "C" wrapper_M68KRUN();

/* LBO 090597 - added these lines and made them extern in cpudefs.h */
int movem_index1[256];
int movem_index2[256];
int movem_next[256];
UBYTE *actadr;

regstruct regs, lastint_regs;

int previouspc;

union flagu intel_flag_lookup[256];
union flagu regflags;

extern int cpu_interrupt(void);
extern void BuildCPU(void);
extern int debugCodeBreakCount,debugCodeBreak[256];
extern int run,debug,debugProgBreak,debugStartPC;
extern int debugProgBreakOfs;
extern int debugEndStepOver,debugBeginStepOver;
extern int debugStepOver,debugStepGoal;
extern int debugDataBreak;
int endStepOver;

#define MC68000_interrupt() (cpu_interrupt())

#define ReadMEM(A) (readmem24(A))
#define WriteMEM(A,V) (writemem24(A,V))

int MC68000_ICount;
int pending_interrupts;

static int InitStatus=0;

extern FILE * errorlog;
extern int ExitEmulator;

int cpuCompleteStop=0;

void m68k_dumpstate(void)
{
   int i;
   CPTR nextpc;
   for(i = 0; i < 8; i++){
      printf("D%d: %08x ", i, regs.d[i].D);
      if ((i & 3) == 3) printf("\n");
   }
   for(i = 0; i < 8; i++){
      printf("A%d: %08x ", i, regs.a[i]);
      if ((i & 3) == 3) printf("\n");
   }
    if (regs.s == 0) regs.usp = regs.a[7];
   if (regs.s && regs.m) regs.msp = regs.a[7];
   if (regs.s && regs.m == 0) regs.isp = regs.a[7];
   printf("USP=%08x ISP=%08x MSP=%08x VBR=%08x SR=%08x\n",
	  regs.usp,regs.isp,regs.msp,regs.vbr,regs.sr);
   printf ("T=%d%d S=%d M=%d N=%d Z=%d V=%d C=%d IMASK=%d\n",
	   regs.t1, regs.t0, regs.s, regs.m,
	   NFLG, ZFLG, VFLG, CFLG, regs.intmask);
   for(i = 0; i < 8; i++){
	printf("FP%d: %g ", i, regs.fp[i]);
      if ((i & 3) == 3) printf("\n");
   }
   printf("N=%d Z=%d I=%d NAN=%d\n",
	  (regs.fpsr & 0x8000000) != 0,
	  (regs.fpsr & 0x4000000) != 0,
	  (regs.fpsr & 0x2000000) != 0,
	  (regs.fpsr & 0x1000000) != 0);
   MC68000_disasm(m68k_getpc(), &nextpc, 1);
   printf("Next PC = 0x%0x\n", nextpc);
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

inline void Interrupt68k(int level)
{
   int ipl=(regs.sr&0xf00)>>8;
   if(level>ipl)
   {
   	::Exception(24+level,0);
   	pending_interrupts &= ~(1 << (level + 24));
   	regs.intmask = level;
   	MakeSR();
   }
}

void Initialization(void) {
   /* Init 68000 emulator */
   initCPU();
}

void MC68000_Reset(void)
{
if (!InitStatus)
{
        Initialization();
	InitStatus=1;
}

   regs.a[7]=regs.isp=readmem24_dword(0);
   m68k_setpc(readmem24_dword(4));

   regs.s = 1;
   regs.m = 0;
   regs.stopped = 0;
   regs.t1 = 0;
   regs.t0 = 0;
   ZFLG = CFLG = NFLG = VFLG = 0;
   regs.intmask = 7;
   regs.vbr = regs.sfc = regs.dfc = 0;
   regs.fpcr = regs.fpsr = regs.fpiar = 0;

   pending_interrupts = 0;

   wrapper_M68KRESET();
}


void MC68000_SetRegs(MC68000_Regs *src)
{
	regs = src->regs;
	NFLG = (regs.sr >> 3) & 1;
	ZFLG = (regs.sr >> 2) & 1;
	VFLG = (regs.sr >> 1) & 1;
	CFLG = regs.sr & 1;
	pending_interrupts = src->pending_interrupts;
}

void MC68000_GetRegs(MC68000_Regs *dst)
{
	regs.sr = (regs.sr & 0xfff0) | (NFLG << 3) | (ZFLG << 2) | (VFLG << 1) |
	CFLG;
	dst->regs = regs;
	dst->pending_interrupts = pending_interrupts;
}


void MC68000_Cause_Interrupt(int level)
{
	if (level >= 1 && level <= 7)
		pending_interrupts |= 1 << (24 + level);
}


void MC68000_Clear_Pending_Interrupts(void)
{
	pending_interrupts &= ~0xff000000;
}


int  MC68000_GetPC(void)
{
	return regs.pc;
}



/* Execute one 68000 instruction */

int MC68000_Execute(int cycles)
{
	UWORD opcode;

    if (debugStepOver&&debugEndStepOver)
        return 0;
	if ((pending_interrupts & MC68000_STOP) && !(pending_interrupts & 0xff000000))
		return cycles;

	MC68000_ICount = cycles;
    endStepOver=0;
    int bp,i,oldPC;
	do
	{
        if (cpuCompleteStop)
            break;
        oldPC=regs.pc;
#ifdef MAME_DEBUG
		{
			extern int mame_debug;
			if (mame_debug)
				MAME_Debug();
		}
#endif

		if (pending_interrupts & 0xff000000)
		{
			int level, mask = 0x80000000;
			for (level = 7; level; level--, mask >>= 1)
				if (pending_interrupts & mask)
					break;
			Interrupt68k (level);
		}

//{       /* ASG 980413 */
//        extern int previouspc;
//        previouspc = regs.pc;
//}
	#if MC68000ASM_DEBUG
	{
	void MC68000_MiniTrace(unsigned long *d, unsigned long *a, unsigned long pc, unsigned long sr, int icount);
	MakeSR();
	MC68000_MiniTrace(regs.d, regs.a, regs.pc, regs.sr, MC68000_ICount);
	}
	#endif

/*		#ifdef ASM_MEMORY
			opcode=nextiword_opcode();
		#else
			opcode=nextiword();
		#endif*/

//		MC68000_ICount -= 12;
//		cpufunctbl[opcode](opcode);
       wrapper_M68KRUN();

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
            debugStartPC=oldPC;
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

   return (cycles - MC68000_ICount);
}








UBYTE *allocmem(long size, UBYTE init_value) {
   UBYTE *p;
   p=(UBYTE *)malloc(size);
   if (!p) {
      printf("No enough memory !\n");
   }
   return(p);
}

