/*
     Definitions for the CPU-Modules
*/

#include "memory.h"

#ifndef __m68000defs__
#define __m68000defs__


#include <stdlib.h>

#ifndef LSB_FIRST
#define LSB_FIRST
#endif

#ifndef NOTYPEREDEF
typedef signed char	BYTE;
typedef short		WORD;
typedef int		LONG;
typedef unsigned int	ULONG;
#endif
typedef unsigned char	UBYTE;
typedef unsigned short	UWORD;
typedef unsigned int	CPTR;

extern unsigned char cycletbl[65536];
extern const unsigned char exception_cycles[48];

#ifndef __WATCOMC__
#ifdef WIN32
#define __inline__   __inline
#else
#define __inline__  inline
#endif
#endif
#ifdef __WATCOMC__
#define __inline__
#endif


#ifdef __MWERKS__
#pragma require_prototypes off
#endif



/****************************************************************************/
/* Define a MC68K word. Upper bytes are always zero			    */
/****************************************************************************/
typedef union
{
#ifdef __128BIT__
 #ifdef LSB_FIRST
   struct { UBYTE l,h,h2,h3,h4,h5,h6,h7, h8,h9,h10,h11,h12,h13,h14,h15; } B;
   struct { UWORD l,h,h2,h3,h4,h5,h6,h7; } W;
   ULONG D;
 #else
   struct { UBYTE h15,h14,h13,h12,h11,h10,h9,h8, h7,h6,h5,h4,h3,h2,h,l; } B;
   struct { UWORD h7,h6,h5,h4,h3,h2,h,l; } W;
   ULONG D;
 #endif
#elif __64BIT__
 #ifdef LSB_FIRST
   struct { UBYTE l,h,h2,h3,h4,h5,h6,h7; } B;
   struct { UWORD l,h,h2,h3; } W;
   ULONG D;
 #else
   struct { UBYTE h7,h6,h5,h4,h3,h2,h,l; } B;
   struct { UWORD h3,h2,h,l; } W;
   ULONG D;
 #endif
#else
 #ifdef LSB_FIRST
   struct { UBYTE l,h,h2,h3; } B;
   struct { UWORD l,h; } W;
   ULONG D;
 #else
   struct { UBYTE h3,h2,h,l; } B;
   struct { UWORD h,l; } W;
   ULONG D;
 #endif
#endif
} pair68000;

extern void Exception(int nr, CPTR oldpc);


typedef void cpuop_func(void);
extern cpuop_func *cpufunctbl[65536];


typedef char flagtype;
#ifndef __WATCOMC__
#define READ_MEML(a,b) asm ("mov (%%esi),%%eax \n\t bswap %%eax \n\t" :"=a" (b) :"S" (a))
#define READ_MEMW(a,b) asm ("mov (%%esi),%%ax\n\t  xchg %%al,%%ah" :"=a" (b) : "S" (a))
#endif
#ifdef __WATCOMC__
LONG wat_readmeml(void *a);
#pragma aux wat_readmeml=\
       "mov eax,[esi]"\
       "bswap eax    "\
       parm [esi] \
       value [eax];
#define READ_MEML(a,b) b=wat_readmeml(a)
WORD wat_readmemw(void *a);
#pragma aux wat_readmemw=\
       "mov ax,[esi]"\
       "xchg al,ah    "\
       parm [esi] \
       value [ax];
#define READ_MEMW(a,b) b=wat_readmemw(a)
#endif

#define get_byte(a) cpu_readmem24((a)&0xffffff)
#define get_word(a) cpu_readmem24_word((a)&0xffffff)
#define get_long(a) cpu_readmem24_dword((a)&0xffffff)
#define put_byte(a,b) cpu_writemem24((a)&0xffffff,b)
#define put_word(a,b) cpu_writemem24_word((a)&0xffffff,b)
#define put_long(a,b) cpu_writemem24_dword((a)&0xffffff,b)

extern int __fastcall ReadRAMByte(int);
extern int __fastcall ReadRAMWord(int);
extern int __fastcall ReadRAMDWord(int);
extern int __fastcall ReadIntROMByte(int);
extern int __fastcall ReadIntROMWord(int);
extern int __fastcall ReadIntROMDWord(int);
extern int __fastcall ReadExtROMByte(int);
extern int __fastcall ReadExtROMWord(int);
extern int __fastcall ReadExtROMDWord(int);
extern int __fastcall ReadIOByte(int);
extern int __fastcall ReadIOWord(int);
extern int __fastcall ReadIODWord(int);

typedef int __fastcall readmem_func(int);
extern readmem_func *ReadByte[16];
extern readmem_func *ReadWord[16];
extern readmem_func *ReadDWord[16];

extern int readmem24(int addr);
extern int readmem24_word(int addr);
extern int readmem24_dword(int addr);

union flagu {
    struct {
        char v;
        char c;
        char n;
        char z;
    } flags;
    struct {
        unsigned short vc;
        unsigned short nz;
    } quickclear;
    ULONG longflags;
};

#define CLEARVC regflags.quickclear.vc=0;
#define CLEARFLGS regflags.longflags=0;

#define CTRUE 0x01
#define VTRUE 0x01
#define ZTRUE 0x01
#define NTRUE 0x01
#define XTRUE 0x01

extern int areg_byteinc[];
extern int movem_index1[256];
extern int movem_index2[256];
extern int movem_next[256];
extern int imm8_table[];
extern UBYTE *actadr;

#define ZFLG (regflags.flags.z)
#define NFLG (regflags.flags.n)
#define CFLG (regflags.flags.c)
#define VFLG (regflags.flags.v)

/*	Assembler Engine Register Structure */

typedef struct
{
    LONG Icount; 			/* 0x0000 Cycles to Process */

    pair68000 d[8];             /* 0x0004 8 Data registers */
	CPTR a[8];             /* 0x0024 8 Address registers */

    CPTR  usp;              /* 0x0044 Stack registers (They will overlap over reg_a7) */
    CPTR  isp;              /* 0x0048 */

    ULONG statusflags;		/* 0x004C System registers */

    /* byte sr_high;       	   The system register (byte most meaningful from SR) */
    /*                         T u S u u I I I */

    /* word r;         		   The CCR register except de extended carry in */
    /*                         INTEL format,  uuuuuVuuSZuuuuuC */
    /*                                        5432109876543210 */

    /* byte extended_carry;    The extended carry (X) -> uuuuuuuX */
    /*                                                   76543210 */

    ULONG pc;            	/* 0x0050 Program Counter */

    UBYTE IRQ_level;        /* IRQ level you want the MC68K process (0=None)  */
    UBYTE vector;           /* Interruption vector (From 0 to 255)            */

    /* Backward compatible with C emulator in case required */
    /* Use MakeSR to fill with correct info */

    UWORD  sr;

    /* Stuff for Debugger (it needs these fields) */

    ULONG  vbr,sfc,dfc,msp;
    double fp[8];
    ULONG  fpcr,fpsr,fpiar;

            flagtype t1;
            flagtype t0;
            flagtype s;
            flagtype m;
            flagtype x;
            flagtype stopped;
            int intmask;

} regstruct;

extern regstruct regs, lastint_regs;
extern int oldEmu;
extern union flagu regflags;

extern union flagu intel_flag_lookup[256];
extern union flagu regflags;
extern UBYTE aslmask_ubyte[];
extern UWORD aslmask_uword[];
extern ULONG aslmask_ulong[];

UWORD inline nextiword() { UWORD v=cpu_readmem24_word(regs.pc); regs.pc+=2; }
ULONG inline nextilong() { ULONG v=cpu_readmem24_dword(regs.pc); regs.pc+=4; }

extern int opcode;

ULONG inline get_disp_ea (ULONG base)
{
   UWORD dp = nextiword();
        int reg = (dp >> 12) & 7;
	LONG regd = dp & 0x8000 ? regs.a[reg] : regs.d[reg].D;
        if ((dp & 0x800) == 0)
            regd = (LONG)(WORD)regd;
        return base + (BYTE)(dp) + regd;
}

int inline cctrue(const int cc)
{
            switch(cc){
              case 0: return 1;                       /* T */
              case 1: return 0;                       /* F */
              case 2: return !CFLG && !ZFLG;          /* HI */
              case 3: return CFLG || ZFLG;            /* LS */
              case 4: return !CFLG;                   /* CC */
              case 5: return CFLG;                    /* CS */
              case 6: return !ZFLG;                   /* NE */
              case 7: return ZFLG;                    /* EQ */
              case 8: return !VFLG;                   /* VC */
              case 9: return VFLG;                    /* VS */
              case 10:return !NFLG;                   /* PL */
              case 11:return NFLG;                    /* MI */
              case 12:return NFLG == VFLG;            /* GE */
              case 13:return NFLG != VFLG;            /* LT */
              case 14:return !ZFLG && (NFLG == VFLG); /* GT */
              case 15:return ZFLG || (NFLG != VFLG);  /* LE */
             }
             abort();
             return 0;
}

static __inline__ void MakeSR(void)
{
    if (oldEmu)
    {

    	int sr ;

    	sr = regs.t1 ;
    	sr <<= 1 ;
    	sr |= regs.t0 ;
    	sr <<= 1 ;
    	sr |= regs.s ;
    	sr <<= 1 ;
    	sr |= regs.m ;
    	sr <<= 4 ;
    	sr |= regs.intmask ;
    	sr <<= 4 ;
    	sr |= regs.x ;
    	sr <<= 1 ;
    	sr |= NFLG ;
    	sr <<= 1 ;
    	sr |= ZFLG ;
    	sr <<= 1 ;
    	sr |= VFLG ;
    	sr <<= 1 ;
    	sr |= CFLG ;

    	regs.sr = sr ;
        return;
    }
	/* Make Status Register compatible with C emulator */

	extern regstruct regs;

    regs.sr = ((regs.statusflags & 0xa7) << 8);

    if (regs.statusflags & 0x0008000) regs.sr |= 8;     /* Negative */
    if (regs.statusflags & 0x0004000) regs.sr |= 4;		/* Zero */
    if (regs.statusflags & 0x0000100) regs.sr |= 1;		/* Carry */

    if (regs.statusflags & 0x1000000) regs.sr |= 16;	/* Ex Carry */
    if (regs.statusflags & 0x0080000) regs.sr |= 2;		/* Overflow */
}

void inline MakeFromSR(void)
{
  /*  int oldm = regs.m; */
	int olds = regs.s;


	int sr = regs.sr ;

	CFLG = sr & 1 ;
	sr >>= 1 ;
	VFLG = sr & 1 ;
	sr >>= 1 ;
	ZFLG = sr & 1 ;
	sr >>= 1 ;
	NFLG = sr & 1 ;
	sr >>= 1 ;
	regs.x = sr & 1 ;
	sr >>= 4 ;
	regs.intmask = sr & 7 ;
	sr >>= 4 ;
	regs.m = sr & 1 ;
	sr >>= 1 ;
	regs.s = sr & 1 ;
	sr >>= 1 ;
	regs.t0 = sr & 1 ;
	sr >>= 1 ;
	regs.t1 = sr & 1 ;

/*
    regs.t1 = (regs.sr >> 15) & 1;
    regs.t0 = (regs.sr >> 14) & 1;
    regs.s = (regs.sr >> 13) & 1;
    regs.m = (regs.sr >> 12) & 1;
    regs.intmask = (regs.sr >> 8) & 7;
    regs.x = (regs.sr >> 4) & 1;
    NFLG = (regs.sr >> 3) & 1;
    ZFLG = (regs.sr >> 2) & 1;
    VFLG = (regs.sr >> 1) & 1;
    CFLG = regs.sr & 1;
*/
    if (oldEmu)
    {
        if (olds != regs.s) {
           if (olds) {
             regs.isp = regs.a[7];
             regs.a[7] = regs.usp;
            } else {
               regs.usp = regs.a[7];
               regs.a[7] = regs.isp;
            }
         }
    }

}

static inline int m68k_getpc() { return regs.pc; }
static inline void m68k_setpc(int pc) { regs.pc=pc; }

extern int MC68000_ICount;
extern void MC68000_Cause_Interrupt(int level);
extern void Exception(int nr, CPTR oldpc);

#define change_pc24(x) ;


#endif
