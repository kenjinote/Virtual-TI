//---------------------------------------------------------------------------
#include <vcl.h>
#include <dir.h>
#include <stdio.h>
#pragma hdrstop

#include "calchw.h"
#include "rom.h"
#define NOTYPEREDEF
#include "m68000.h"
#include "Debug.h"
#include "Log.h"
#include "Emu.h"

static int keymap92[]={VK_ESCAPE,k_esc,'1',k_1,'2',k_2,
    '3',k_3,'4',k_4,'5',k_5,'6',k_6,'7',k_7,
    '8',k_8,'9',k_9,'0',k_0,0xbd,k_neg,
    0xbb,k_equ,VK_BACK,k_bs,VK_TAB,k_sto,'Q',
    k_q,'W',k_w,'E',k_e,'R',k_r,'T',k_t,'Y',k_y,
    'U',k_u,'I',k_i,'O',k_o,'P',k_p,0xdb,
    k_lparan,0xdd,k_rparan,VK_RETURN,k_enter1,
    VK_CONTROL,k_dia,'A',k_a,'S',k_s,'D',k_d,
    'F',k_f,'G',k_g,'H',k_h,'J',k_j,'K',k_k,
    'L',k_l,0xc0,k_mode,VK_SHIFT,k_shift,
    0xdc,k_ln,'Z',k_z,'X',k_x,'C',k_c,
    'V',k_v,'B',k_b,'N',k_n,'M',k_m,0xbc,
    k_comma,0xbe,k_dot,0xbf,k_div,VK_MULTIPLY,
    k_mult,VK_MENU,k_2nd,VK_SPACE,k_space,VK_CAPITAL,
    k_hand,VK_F1,k_f1,VK_F2,k_f2,VK_F3,k_f3,VK_F4,
    k_f4,VK_F5,k_f5,VK_F6,k_f6,VK_F7,k_f7,VK_F8,
    k_f8,VK_F9,k_apps,VK_PRIOR,k_tan,VK_NEXT,k_power,VK_SCROLL,
    k_on,VK_NUMPAD7,k_7,VK_NUMPAD8,k_8,VK_NUMPAD9,k_9,
    VK_SUBTRACT,k_minus,VK_NUMPAD4,k_4,VK_NUMPAD5,k_5,
    VK_NUMPAD6,k_6,VK_ADD,k_plus,VK_NUMPAD1,k_1,
    VK_NUMPAD2,k_2,VK_NUMPAD3,k_3,VK_NUMPAD0,k_0,
    VK_DECIMAL,k_dot,VK_SEPARATOR,k_enter2,VK_DIVIDE,
    k_div,VK_HOME,k_cos,VK_UP,k_up,
    VK_LEFT,k_left,VK_RIGHT,k_right,VK_DOWN,k_down,
    VK_INSERT,k_sin,VK_DELETE,k_clear,0xba,
    k_theta,-1,-1};

static int ti92kmat[][8]=
    {k_down,k_right,k_up,k_left,k_hand,k_shift,k_dia,k_2nd,
     k_3,k_2,k_1,k_f8,k_w,k_s,k_z,kn,
     k_6,k_5,k_4,k_f3,k_e,k_d,k_x,kn,
     k_9,k_8,k_7,k_f7,k_r,k_f,k_c,k_sto,
     k_comma,k_rparan,k_lparan,k_f2,k_t,k_g,k_v,k_space,
     k_tan,k_cos,k_sin,k_f6,k_y,k_h,k_b,k_div,
     k_p,k_enter2,k_ln,k_f1,k_u,k_j,k_n,k_power,
     k_mult,k_apps,k_clear,k_f5,k_i,k_k,k_m,k_equ,
     kn,k_esc,k_mode,k_plus,k_o,k_l,k_theta,k_bs,
     k_neg,k_dot,k_0,k_f4,k_q,k_a,k_enter1,k_minus};

static int key[]=
    {k_hand,k_f1,k_f2,k_f3,k_f4,k_f5,k_f6,k_f7,k_f8,
     k_q,k_w,k_e,k_r,k_t,k_y,k_u,k_i,k_o,k_p,
     k_a,k_s,k_d,k_f,k_g,k_h,k_j,k_k,k_l,
     k_z,k_x,k_c,k_v,k_b,k_n,k_m,k_theta,
     k_shift,k_on,k_dia,k_2nd,k_sto,k_space,k_equ,
     k_bs,k_enter1,k_2nd,k_esc,k_mode,k_clear,k_ln,
     k_apps,k_enter2,k_left,k_right,k_up,k_down,
     k_sin,k_cos,k_tan,k_power,k_lparan,k_rparan,k_comma,
     k_div,k_7,k_8,k_9,k_mult,k_4,k_5,k_6,k_minus,
     k_1,k_2,k_3,k_plus,k_0,k_dot,k_neg,k_enter1};

char* defExt92[0x20]={"92e","92c","92c","92c","92l","92c",
    "92m","92c","92z","92c","92c","92t","92s","92d",
    "92a","92c","92i","92c","92p","92f","92x","92c",
    "92c","92c","92c","92c","92c","92c","92c","92c",
    "92c","92c"};

static DWORD funcAddr[12][0x42]=
    {{0x4fac38,0x4fad0e,0x46c816,0x46c894,0x401326,0x40134c,0x4027e8,0x40285a,0x4028b2,0x4028ec,0x402970,0x46c7fc,0x4fabfc,0x4012fc,0x46c9e8,0x49f026,0x262f04,0x46384c,0x461990,0x461a58,0x461a96,0x461aa4,0x461ac8,0x4643f6,0x46478a,0x464f84,0x465796,0x75bc,0x4440,0x7644,0x46893c,0x468b22,0x468ce0,0x465c58,0x46c9b8,0x4fb684,0x46ec66,0x400000,0x4619e2,0x49e5b8,0x4fb594,0x4fb5ac,0x4fb600,0x4fb634,0x4fb658,0x431928,0x4faa34,0x4f3686,0x431f32,0x4fb880,0x4fb88c,0x4fb964,0x4fb9ac,0x4fbae0,0x4fbb38,0x4fb78c,0x4fb7ce,0x4fb6a8,0x4fb6d2,0x4fb768,0x4fb778,0x4fb7e4,0x4fb804,0x4fb858,0x4fb866,0x46e1b2},
     {0x4fae04,0x4faedc,0x46c53a,0x46c5b8,0x400f26,0x400f4c,0x4023e8,0x40245a,0x4024b2,0x4024ec,0x402570,0x46c520,0x4fadc8,0x400efc,0x46c70c,0x49ef7e,0x462ccc,0x463614,0x461758,0x461820,0x46185e,0x46186c,0x461890,0x4641be,0x464552,0x464d4c,0x46555e,0x7594,0x4440,0x761c,0x4686d0,0x4688b6,0x468a74,0x465a20,0x46c6dc,0x4fb84c,0x46e9b6,0x400000,0x4617aa,0x49e510,0x4fb75c,0x4fb774,0x4fb7c8,0x4fb7fc,0x4fb820,0x431584,0x4fabfc,0x4f3786,0x431b8e,0x4fba48,0x4fba54,0x4fbb2c,0x4fbb74,0x4fbca8,0x4fbd00,0x4fb954,0x4fb996,0x4fb870,0x4fb89a,0x4fb930,0x4fb940,0x4fb9ac,0x4fb9cc,0x4fba20,0x4fba2e,0x46df02},
     {0x4fae40,0x4faf18,0x46c55e,0x46c5dc,0x400f26,0x400f4c,0x4023ec,0x40245e,0x4024b6,0x4024f0,0x402574,0x46c544,0x4fae04,0x400efc,0x46c730,0x49efa2,0x462cd0,0x463618,0x46175c,0x461824,0x461862,0x461870,0x461894,0x4641c2,0x464556,0x464d50,0x465562,0x7594,0x4440,0x761c,0x4686d4,0x4688ba,0x468a78,0x465a24,0x46c700,0x4fb888,0x46e9da,0x400000,0x4617ae,0x49e534,0x4fb798,0x4fb7b0,0x4fb804,0x4fb838,0x4fb85c,0x431588,0x4fac38,0x4f37aa,0x431b92,0x4fba84,0x4fba90,0x4fbb68,0x4fbbb0,0x4fbce4,0x4fbd3c,0x4fb990,0x4fb9d2,0x4fb8ac,0x4fb8d6,0x4fb96c,0x4fb97c,0x4fb9e8,0x4fba08,0x4fba5c,0x4fba6a,0x46df26},
     {0x4faf38,0x4fb010,0x46c96e,0x46c9ec,0x400f26,0x400f4c,0x4023e0,0x402452,0x4024aa,0x4024e4,0x402568,0x46c954,0x4faefc,0x400efc,0x46cb40,0x49f166,0x4630e0,0x463a28,0x461b6c,0x461c34,0x461c72,0x461c80,0x461ca4,0x4645d2,0x464966,0x465160,0x465972,0x7594,0x4440,0x761c,0x468ae4,0x468cca,0x468e88,0x465e34,0x46cb10,0x4fb980,0x46edea,0x400000,0x461bbe,0x49e6f8,0x4fb890,0x4fb8a8,0x4fb8fc,0x4fb930,0x4fb954,0x431830,0x4fad30,0x4f390e,0x431e3a,0x4fbb7c,0x4fbb88,0x4fbc60,0x4fbca8,0x4fbddc,0x4fbe34,0x4fba88,0x4fbaca,0x4fb9a4,0x4fb9ce,0x4fba64,0x4fba74,0x4fbae0,0x4fbb00,0x4fbb54,0x4fbb62,0x46e336},
     {0x4faf40,0x4fb018,0x46c8ee,0x46c96c,0x400eb6,0x400efc,0x402390,0x402402,0x40245a,0x402494,0x402518,0x46c8d4,0x4faf04,0x400eac,0x46cac0,0x49f12a,0x46314c,0x463a94,0x461bd8,0x461ca0,0x461cde,0x461cec,0x461d10,0x46463e,0x4649d2,0x465146,0x465958,0x7594,0x4440,0x761c,0x468a78,0x468c5e,0x468e1c,0x465e1c,0x46ca90,0x4fb980,0x46ed62,0x400000,0x461c2a,0x49e6bc,0x4fb890,0x4fb8a8,0x4fb8fc,0x4fb930,0x4fb954,0x4317f8,0x4fad38,0x4f3912,0x431e02,0x4fbb7c,0x4fbb88,0x4fbc60,0x4fbca8,0x4fbddc,0x4fbe34,0x4fba88,0x4fbaca,0x4fb9a4,0x4fb9ce,0x4fba64,0x4fba74,0x4fbae0,0x4fbb00,0x4fbb54,0x4fbb62,0x46e2b6},
     {0x4faef4,0x4fafcc,0x46c9c6,0x46ca44,0x400ed6,0x400efc,0x402390,0x402402,0x40245a,0x402494,0x402518,0x46c9ac,0x4faeb8,0x400eac,0x46cb98,0x49f20e,0x463224,0x463b6c,0x461cb0,0x461d78,0x461db6,0x461dc4,0x461de8,0x464716,0x464aaa,0x46521e,0x465a30,0x7594,0x4440,0x761c,0x468b50,0x468d36,0x468ef4,0x465ef4,0x46cb68,0x4fb934,0x46ee3a,0x400000,0x461d02,0x49e7a0,0x4fb844,0x4fb85c,0x4fb8b0,0x4fb8e4,0x4fb908,0x431840,0x4facec,0x4f38c6,0x431e4a,0x4fbb30,0x4fbb3c,0x4fbc14,0x4fbc5c,0x4fbd90,0x4fbde8,0x4fba3c,0x4fba7e,0x4fb958,0x4fb982,0x4fba18,0x4fba28,0x4fba94,0x4fbab4,0x4fbb08,0x4fbb16,0x46e38e},
     {0x4fad98,0x4fae6e,0x46cb26,0x46cba4,0x400ed6,0x400efc,0x4023a0,0x402412,0x40246a,0x4024a4,0x402528,0x46cb0c,0x4fad5c,0x400eac,0x46ccf8,0x49f226,0x463384,0x463ccc,0x461e10,0x461ed8,0x461f16,0x461f24,0x461f48,0x464876,0x464c0a,0x46537e,0x465b90,0x7594,0x4440,0x761c,0x468cb0,0x468e96,0x469054,0x466054,0x46ccc8,0x4fb7d8,0x46ef9a,0x400000,0x461e62,0x49e7b8,0x4fb6e8,0x4fb700,0x4fb754,0x4fb788,0x4fb7ac,0x4318fc,0x4fab90,0x4f39f2,0x431f06,0x4fb9d4,0x4fb9e0,0x4fbab8,0x4fbb00,0x4fbc34,0x4fbc8c,0x4fb8e0,0x4fb922,0x4fb7fc,0x4fb826,0x4fb8bc,0x4fb8cc,0x4fb938,0x4fb958,0x4fb9ac,0x4fb9ba,0x46e4ee},
     {0x4fadf4,0x4faeca,0x46cb32,0x46cbb0,0x400ed6,0x400efc,0x4023a0,0x402412,0x40246a,0x4024a4,0x402528,0x46cb18,0x4fadb8,0x400eac,0x46cd04,0x49f262,0x463390,0x463cd8,0x461e1c,0x461ee4,0x461f22,0x461f30,0x461f54,0x464882,0x464c16,0x46538a,0x465b9c,0x7594,0x4440,0x761c,0x468cbc,0x468ea2,0x469060,0x466060,0x46ccd4,0x4fb834,0x46efa6,0x400000,0x461e6e,0x49e7f4,0x4fb744,0x4fb75c,0x4fb7b0,0x4fb7e4,0x4fb808,0x431908,0x4fabec,0x4f3a32,0x431f12,0x4fba30,0x4fba3c,0x4fbb14,0x4fbb5c,0x4fbc90,0x4fbce8,0x4fb93c,0x4fb97e,0x4fb858,0x4fb882,0x4fb918,0x4fb928,0x4fb994,0x4fb9b4,0x4fba08,0x4fba16,0x46e4fa},
     {0x4fad5c,0x4fae32,0x46ca8e,0x46cb0c,0x400ed6,0x400efc,0x4023a0,0x402412,0x40246a,0x4024a4,0x402528,0x46ca74,0x4fad20,0x400eac,0x46cc60,0x49f1be,0x4632ec,0x463c34,0x461d78,0x461e40,0x461e7e,0x461e8c,0x461eb0,0x4647de,0x464b72,0x4652e6,0x465af8,0x7594,0x4440,0x761c,0x468c18,0x468dfe,0x468fbc,0x465fbc,0x46cc30,0x4fb79c,0x46ef02,0x400000,0x461dca,0x49e750,0x4fb6ac,0x4fb6c4,0x4fb718,0x4fb74c,0x4fb770,0x4318bc,0x4fab54,0x4f398e,0x431ec6,0x4fb998,0x4fb9a4,0x4fba7c,0x4fbac4,0x4fbbf8,0x4fbc50,0x4fb8a4,0x4fb8e6,0x4fb7c0,0x4fb7ea,0x4fb880,0x4fb890,0x4fb8fc,0x4fb91c,0x4fb970,0x4fb97e,0x46e456},
     {0x4fad64,0x4fae3a,0x46ca96,0x46cb14,0x400ed6,0x400efc,0x4023a8,0x40241a,0x402472,0x4024ac,0x402530,0x46ca7c,0x4fad28,0x400eac,0x46cc68,0x49f1c6,0x4632f4,0x463c3c,0x461d80,0x461e48,0x461e86,0x461e94,0x461eb8,0x4647e6,0x464b7a,0x4652ee,0x465b00,0x7594,0x4440,0x761c,0x468c20,0x468e06,0x468fc4,0x465fc4,0x46cc38,0x4fb7a4,0x46ef0a,0x400000,0x461dd2,0x49e758,0x4fb6b4,0x4fb6cc,0x4fb720,0x4fb754,0x4fb778,0x4318c4,0x4fab5c,0x4f3996,0x431ece,0x4fb9a0,0x4fb9ac,0x4fba84,0x4fbacc,0x4fbc00,0x4fbc58,0x4fb8ac,0x4fb8ee,0x4fb7c8,0x4fb7f2,0x4fb888,0x4fb898,0x4fb904,0x4fb924,0x4fb978,0x4fb986,0x46e45e},
     {0x4fae10,0x4faee6,0x46caca,0x46cb48,0x400ed6,0x400efc,0x4023a8,0x40241a,0x402472,0x4024ac,0x402530,0x46cab0,0x4fadd4,0x400eac,0x46cc9c,0x49f272,0x463328,0x463c70,0x461db4,0x461e7c,0x461eba,0x461ec8,0x461eec,0x46481a,0x464bae,0x465322,0x465b34,0x7594,0x4440,0x761c,0x468c54,0x468e3a,0x468ff8,0x465ff8,0x46cc6c,0x4fb850,0x46ef3e,0x400000,0x461e06,0x49e804,0x4fb760,0x4fb778,0x4fb7cc,0x4fb800,0x4fb824,0x4318b8,0x4fac08,0x4f3a42,0x431ec2,0x4fba4c,0x4fba58,0x4fbb30,0x4fbb78,0x4fbcac,0x4fbd04,0x4fb958,0x4fb99a,0x4fb874,0x4fb89e,0x4fb934,0x4fb944,0x4fb9b0,0x4fb9d0,0x4fba24,0x4fba32,0x46e492}};

ROMFunc romFuncs92[]=
    {{0,"ST_eraseHelp"},{1,"ST_showHelp"},{2,"HeapFree"},
     {3,"HeapAlloc"},{4,"ER_catch"},{5,"ER_success"},
     {6,"reset_link"},{7,"flush_link"},{8,"tx_free"},
     {9,"transmit"},{0xa,"receive"},{0xb,"HeapFreeIndir"},
     {0xc,"ST_busy"},{0xd,"ER_throwVar"},
     {0xe,"HeapRealloc"},{0xf,"sprintf"},
     {0x10,"DrawStrXY"},{0x11,"DrawCharXY"},
     {0x12,"FontSetSys"},{0x13,"DrawTo"},{0x14,"MoveTo"},
     {0x15,"PortSet"},{0x16,"PortRestore"},
     {0x17,"WinActivate"},{0x18,"WinClose"},
     {0x19,"WinOpen"},{0x1a,"WinStrXY"},
     {0x1b,"kb_globals"},{0x1c,"LCD_MEM"},
     {0x1d,"ST_flags"},{0x1e,"MenuPopup"},
     {0x1f,"MenuBegin"},{0x20,"MenuOn"},{0x21,"SF_font"},
     {0x22,"HeapAllocThrow"},{0x23,"strcmp"},
     {0x24,"FindSymEntry"},{0x25,"ROM_base"},
     {0x26,"FontGetSys"},{0x27,"vcbprintf"},
     {0x28,"strlen"},{0x29,"strncmp"},{0x2a,"strncpy"},
     {0x2b,"strcat"},{0x2c,"strchr"},
     {0x2d,"push_quantum"},{0x2e,"OSAlexOut"},
     {0x2f,"ERD_dialog"},{0x30,"check_estack_size"},
     {0x31,"labs"},{0x32,"memset"},{0x33,"memcmp"},
     {0x34,"memcpy"},{0x35,"memmove"},{0x36,"abs"},
     {0x37,"rand"},{0x38,"srand"},{0x39,"_du32u32"},
     {0x3a,"_ds32s32"},{0x3b,"_du16u16"},{0x3c,"_ds16u16"},
     {0x3d,"_ru32u32"},{0x3e,"_rs32s32"},{0x3f,"_ru16u16"},
     {0x40,"_rs16u16"},{0x41,"DerefSym"},{0,NULL}};

static BYTE fontCompare[]={0x4,0x50,0xa0,0x50,0xa0,0x50,
    0x7,0x3a,0x4a,0x2e,0x1a,0x6a,0x7,0x3a,0x4a,0x24};

void IRQ(int level)
{
    if (cpuCompleteStop&&(level==6))
    {
        cpuCompleteStop=0;
        return;
    }
    MC68000_Cause_Interrupt(level);
//	pending_interrupts|=1<<(level+24);
}

TI92::TI92()
{
    romFuncs=romFuncs92;
    asm_readmem=asm_89_readmem;
    asm_readmem_word=asm_89_readmem_word;
    asm_readmem_dword=asm_89_readmem_dword;
    asm_writemem=asm_89_writemem;
    asm_writemem_word=asm_89_writemem_word;
    asm_writemem_dword=asm_89_writemem_dword;
}

void TI92::EnableDebug()
{
    debugEnabled=1;
    asm_readmem=debug_readmem;
    asm_readmem_word=debug_readmem_word;
    asm_readmem_dword=debug_readmem_dword;
    asm_writemem=debug_writemem;
    asm_writemem_word=debug_writemem_word;
    asm_writemem_dword=debug_writemem_dword;
}

void TI92::DisableDebug()
{
    debugEnabled=0;
    if (!ram128) return;
    asm_readmem=asm_89_readmem;
    asm_readmem_word=asm_89_readmem_word;
    asm_readmem_dword=asm_89_readmem_dword;
    asm_writemem=asm_89_writemem;
    asm_writemem_word=asm_89_writemem_word;
    asm_writemem_dword=asm_89_writemem_dword;
}

int* TI92::GetKeyMap()
{
    return keymap92;
}

int TI92::GetKeyMat(int r,int c)
{
    return ti92kmat[r][c];
}

int TI92::GetKeyAlphaMat(int r,int c)
{
    return kn;
}

void TI92::Reset()
{
    chdir(initPath);
	LoadROM(currentROM);
    int i;
    for (i=0;i<0x40000;i++)
        ram[i]=0;
	for (i=0;i<256;i++)
	{
		if (initialPC<0x400000)
			ram[i]=intRom[i+initIntTabOfs];
		else
			ram[i]=extRom[i+initIntTabOfs];
	}
    for (i=0;i<32;i++)
        mem[i]=&ram[(i&3)<<16];
    for (i=0;i<32;i++)
        mem[i+32]=&intRom[i<<16];
    for (i=0;i<32;i++)
        mem[i+64]=&extRom[i<<16];
    for (i=96;i<256;i++)
        mem[i]=garbageMem;
    for (i=0;i<0x10000;i++)
        garbageMem[i]=0x14;
	MC68000_Reset();
	regs.pc=initialPC;
	contrast=16;
	ram128=1;
	memprotect=0;
	kbmask=0xff;
    for (i=0;i<10;i++) kbmatrix[i]=0xff;
	ramWrap=0; memAnd=0x1ffff;
	onheld=1; lcdBase=((calc==89)||plusMod)?0x4c00:0x4440;
    for (i=0;i<6;i++) oldScreenBase[i]=lcdBase;
	timer=0; timerInit=0xb2; int0Count=0;
	transflag=0; recvflag=0;
	transnotready=0;
	lcdRefresh=0;
    io0Bit2=1; io0Bit7=1;
    romWriteReady=0; romWritePhase=0; romRetOr=0;
    cpuCompleteStop=0; lcdOff=0;
    romErase=0;
    directAccess=0; outRed=1; outWhite=1; getError=0;
    for (i=0;i<32;i++) romChanged[i]=0;
    memset(romFuncAddr,0,sizeof(int)*0x800);
    for (i=0;i<12;i++)
    {
        DWORD addr=funcAddr[i][0x21];
		if (initialPC<0x400000) addr-=0x200000;
        int j;
        for (j=0;j<16;j++)
        {
            if (getmem(addr+j)!=fontCompare[j])
                break;
        }
        if (j==16)
        {
            for (j=0;romFuncs[j].name;j++)
            {
                romFuncAddr[j]=funcAddr[i][j];
        		if ((initialPC<0x400000)&&
                    (romFuncAddr[j]>=0x400000))
                    romFuncAddr[j]-=0x200000;
            }
            break;
        }
    }
}

int TI92::ioReadByte(int p)
{
	int v,i;
	switch (p)
	{
		case 0: v=((contrast&1)<<5)|(io0Bit7<<7)|(io0Bit2<<2);
            io0Bit2=1;
            return v;
		case 1: return (memprotect<<2)|ram128;
		case 2: case 4: case 6: case 8: case 0xa: return 0x14;
		case 3: case 5: case 7: case 9: case 0xb: return 1;
		case 0xc: return 5|((1-transflag)<<1)|(directAccess?0x40:0);
		case 0xd:
            if ((emuLink)&&(!recvflag)&&(!transflag))
            {
                if (CheckSendBuf())
                {
                    recvflag=1;
                    recvbyte=GetSendBuf();
                    lastByteTicks=0;
                    lastIRQ4=0;
                    byteCount++;
                    if (byteCount<4)
                        IRQ(4);
                    else
                        byteCount=0;
                }
            }
            return (recvflag<<5)|((1-transflag)<<6);
		case 0xe:
            if (directAccess)
                return 0x50|(outWhite<<1)|outRed;
            return 0x10;
		case 0xf: recvflag=0; return recvbyte;
		case 0x10: case 0x12: return 0x14;
		case 0x11: case 0x13: return 1;
		case 0x14: return 0x14;
		case 0x15: return 0x1b;
		case 0x16: return 0x14;
		case 0x17: return timer;
		case 0x18: return (kbmask>>8)&3;
		case 0x19: return kbmask&0xff;
		case 0x1a: return 0x14|(onheld<<1);
		case 0x1b: for (v=0xff,i=0;i<10;i++)
			   {
			   	if (!(kbmask&(1<<i)))
			   		v&=kbmatrix[i];
			   }
			   return v;
		case 0x1c: case 0x1e: return 0x14;
		case 0x1d: case 0x1f: return 1;
	}
    return 0;
}

void TI92::ioWriteByte(int p,int v)
{
	switch(p)
	{
		case 0: contrast=(contrast&(~1))|((v>>5)&1);
            io0Bit7=(v>>7)&1;
            io0Bit2=(v>>2)&1;
            break;
		case 1: ram128=v&1; memprotect=(v>>2)&1;
			memAnd=(ramWrap)?0x1ffff:(ram128?0x1ffff:0x3ffff);
            for (int i=0;i<32;i++)
                mem[i]=&ram[(i&(memAnd>>16))<<16];
            if ((!ram128)||(debugEnabled))
            {
                asm_readmem=debug_readmem;
                asm_readmem_word=debug_readmem_word;
                asm_readmem_dword=debug_readmem_dword;
            }
            else
            {
                asm_readmem=asm_89_readmem;
                asm_readmem_word=asm_89_readmem_word;
                asm_readmem_dword=asm_89_readmem_dword;
            }
			break;
		case 5: if (!(v&0x10)) cpuCompleteStop=1; break;
		case 0xc:
            directAccess=(v>>6)&1;
            break;
		case 0xd: break;
		case 0xe:
            directAccess=(v>>6)&1;
            if (directAccess)
            {
                outWhite=(v>>1)&1;
                outRed=v&1;
            }
            break;
		case 0xf:
            if ((emuLink)&&(!transflag))
            {
                int timeout=10;
                while (1)
                {
                    if (!PutRecvReady())
                    {
                        Sleep(30);
                        timeout--;
                        if (timeout<=0) break;
                        continue;
                    }
                    PutRecvBuf(v);
                    break;
                }
            }
            else
            {
                transflag=1;
                transbyte=v;
            }
            if (debugLogLinkSend)
            {
                char str[32];
                sprintf(str,"Send of %2.2X to link port",v&0xff);
                LogDlg->List->Items->Add(str);
            }
            IRQ(4);
            updateLink=1;
            if ((!sendInProgress)&&(!emuLink))
                ReceiveFile();
            break;
		case 0x10: lcdBase=(((lcdBase>>3)&0xff)|((v&0xff)<<8))<<3;
			   break;
		case 0x11: lcdBase=(((lcdBase>>3)&0xff00)|(v&0xff))<<3;
		           break;
		case 0x17: timerInit=v; break;
		case 0x18: kbmask=(kbmask&0xff)|((v&3)<<8); break;
		case 0x19: kbmask=(kbmask&0x300)|(v&0xff); break;
        case 0x1c: lcdOff=(v&0x80)>>7; break;
		case 0x1d: contrast=(contrast&1)|((v&15)<<1); break;
		default: break;
	}
}

int TI92::getmem(int addr)
{
    addr&=0xffffff;
    if ((addr<0x200000)&&(!ram128))
        return 0;
	if (addr<0x600000)
		return (mem[addr>>16][addr&0xffff]);
	else
		return (ioReadByte(addr&0x1f))&0xff;
}

int TI92::getmem_word(int addr)
{
	return ((getmem(addr)<<8)|getmem(addr+1))&0xffff;
}

int TI92::getmem_dword(int addr)
{
	return (getmem(addr)<<24)|
	       (getmem(addr+1)<<16)|
	       (getmem(addr+2)<<8)|
	       getmem(addr+3);
}

void TI92::setmem(int addr,int v)
{
    addr&=0xffffff;
	if (addr<0x200000)
		ram[addr&memAnd]=v;
    else if (addr<0x400000)
        intRomWriteByte(addr&0x1fffff,v);
    else if (addr<0x600000)
        extRomWriteByte(addr&0x1fffff,v);
	else if (addr<0x800000)
		ioWriteByte(addr&0x1f,v);
}

void TI92::setmem_word(int addr,int v)
{
	setmem(addr,(v>>8)&0xff);
	setmem(addr+1,v&0xff);
}

void TI92::setmem_dword(int addr,int v)
{
	setmem(addr,(v>>24)&0xff);
	setmem(addr+1,(v>>16)&0xff);
    setmem(addr+2,(v>>8)&0xff);
	setmem(addr+3,v&0xff);
}

int TI92::readmem(int addr)
{
    addr&=0xffffff;
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,1,DATABREAK_READ,0))
            DataBreak();
    }
	if (addr<0x600000)
		return (mem[addr>>16][addr&0xffff]);
	else
		return (ioReadByte(addr&0x1f))&0xff;
}

int TI92::readmem_word(int addr)
{
    addr&=0xffffff;
    if (addr&1) { ::Exception(3,0); HandleException(); }
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,2,DATABREAK_READ,0))
            DataBreak();
    }
	return ((getmem(addr)<<8)|getmem(addr+1))&0xffff;
}

int TI92::readmem_dword(int addr)
{
    addr&=0xffffff;
    if (addr&1) { ::Exception(3,0); HandleException(); }
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,4,DATABREAK_READ,0))
            DataBreak();
    }
	return (getmem(addr)<<24)|
	       (getmem(addr+1)<<16)|
	       (getmem(addr+2)<<8)|
	       getmem(addr+3);
}

void TI92::intRomWriteByte(int addr,int v)
{
    if (calc!=89) return;
    if (romWriteReady)
    {
//        if ((intRom[addr]==0xff)||(romWriteReady==1))
        {
            intRom[addr]=v;
            romChanged[addr>>16]=1;
        }
//        else
//            romWriteReady--;
        romWriteReady--;
        romRetOr=0xffffffff;
    }
    else if (v==0x50)
        romWritePhase=0x50;
    else if (v==0x10)
    {
        if (romWritePhase==0x50)
            romWritePhase=0x51;
        else if (romWritePhase==0x51)
        {
            romWriteReady=2;
            romWritePhase=0x50;
        }
    }
    else if (v==0x20)
    {
        if (romWritePhase==0x50)
            romWritePhase=0x20;
    }
    else if (v==0xd0)
    {
        if (romWritePhase==0x20)
        {
            romWritePhase=0xd0;
            romRetOr=0xffffffff;
            romErase=0xffffffff;
            romErasePhase=0;
            for (int i=0;i<0x10000;i++)
                intRom[(addr&0x1f0000)+i]=0xff;
            romChanged[addr>>16]=1;
        }
    }
    else if (v==0xff)
    {
        if (romWritePhase==0x50)
        {
            romWriteReady=0;
            romRetOr=0;
        }
    }
}

void TI92::extRomWriteByte(int addr,int v)
{
    if ((calc!=92)||(!plusMod)) return;
    if (romWriteReady)
    {
        if ((extRom[addr]==0xff)||(romWriteReady==1))
        {
            extRom[addr]=v;
            romChanged[addr>>16]=1;
        }
        else
            romWriteReady--;
        romWriteReady--;
        romRetOr=0xffffffff;
    }
    else if (v==0x50)
        romWritePhase=0x50;
    else if (v==0x10)
    {
        if (romWritePhase==0x50)
            romWritePhase=0x51;
        else if (romWritePhase==0x51)
        {
            romWriteReady=2;
            romWritePhase=0x50;
        }
    }
    else if (v==0x20)
    {
        if (romWritePhase==0x50)
            romWritePhase=0x20;
    }
    else if (v==0xd0)
    {
        if (romWritePhase==0x20)
        {
            romWritePhase=0xd0;
            romRetOr=0xffffffff;
            romErase=0xffffffff;
            romErasePhase=0;
            for (int i=0;i<0x10000;i++)
                extRom[(addr&0x1f0000)+i]=0xff;
            romChanged[addr>>16]=1;
        }
    }
    else if (v==0xff)
    {
        if (romWritePhase==0x50)
        {
            romWriteReady=0;
            romRetOr=0;
        }
    }
}

void TI92::writemem(int addr,int v)
{
    addr&=0xffffff;
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,1,DATABREAK_WRITE,v))
            DataBreak();
    }
    if ((addr<0x100)&&(memprotect))
    {
        ::Exception(24+7,0);
        HandleException();
    }
	if (addr<0x200000)
		ram[addr&memAnd]=v;
    else if (addr<0x400000)
        intRomWriteByte(addr&0x1fffff,v);
    else if (addr<0x600000)
        extRomWriteByte(addr&0x1fffff,v);
	else if (addr<0x800000)
		ioWriteByte(addr&0x1f,v);
}

void TI92::writemem_word(int addr,int v)
{
    addr&=0xffffff;
    if (addr&1) { ::Exception(3,0); HandleException(); }
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,2,DATABREAK_WRITE,v))
            DataBreak();
    }
    if ((addr<0x100)&&(memprotect))
    {
        ::Exception(24+7,0);
        HandleException();
    }
	setmem(addr,(v>>8)&0xff);
	setmem(addr+1,v&0xff);
}

void TI92::writemem_dword(int addr,int v)
{
    addr&=0xffffff;
    if (addr&1) { ::Exception(3,0); HandleException(); }
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,4,DATABREAK_WRITE,v))
            DataBreak();
    }
    if ((addr<0x100)&&(memprotect))
    {
        ::Exception(24+7,0);
        HandleException();
    }
	setmem(addr,(v>>24)&0xff);
	setmem(addr+1,(v>>16)&0xff);
	setmem(addr+2,(v>>8)&0xff);
	setmem(addr+3,v&0xff);
}

int TI92::readport(int port)
{
    return 0;
}

void TI92::writeport(int port,int v)
{
}

void TI92::Execute()
{
	static i=0;
	int j,f;

    if (!run) return;
    UpdateDebugCheckEnable();
   	i++;
    if (i>=8)
   	{
    	i=0;
	    int0Count++;
   		if (int0Count>3)
    	{
	    	int0Count=0;
   			IRQ(1);
    	}
	    if (int0Count==1) IRQ(4);
   		if (int0Count==2)
    	{
	    	for (j=0,f=0;j<10;j++)
   			{
    			if (kbmatrix[j]!=0xff)
	    			f=1;
   			}
    		if (f) { IRQ(2); }
	    }
   		lcdRefresh++;
    	if (lcdRefresh>15)
	    {
   			lcdRefresh=0;
    		RefreshLCD();
	    }
   		if (timer!=0)
    	{
	    	timer++;
   			timer&=0xff;
    	}
	    else
   		{
    		timer=timerInit;
   			IRQ(5);
    	}
   	}
    static int leftOver=0;
	leftOver=MC68000_Execute(625-leftOver)-(625-leftOver);
}

void TI92::OneInstruction()
{
    UpdateDebugCheckEnable();
    MC68000_Execute(1);
}

void TI92::OnUp()
{
    onheld=1;
}

void TI92::OnDown()
{
    onheld=0;
    IRQ(6);
    if (cpuCompleteStop)
        cpuCompleteStop=0;
}

void TI92::KeyUp(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]|=1<<c;
}

void TI92::KeyDown(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]&=~(1<<c);
}

void TI92::AlphaUp()
{
    KeyUp(0,7);
}

void TI92::AlphaDown()
{
    KeyDown(0,7);
}

void TI92::KBInt()
{
    IRQ(1);
}

int TI92::GetKeyID(int i)
{
    return key[i];
}

void TI92::SaveState(char *fn)
{
    FILE *fp=fopen(fn,"wb");
    fprintf(fp,"VTIv2.0 ");
    fwrite(romImage[currentROM].name,56,1,fp);
    MakeSR();
    fwrite(&regs,sizeof(regstruct),1,fp);
    fwrite(ram,256,1024,fp);
    fwrite(&memprotect,4,1,fp); fwrite(&ram128,4,1,fp);
    fwrite(&timer,4,1,fp); fwrite(&timerInit,4,1,fp);
    fwrite(&int0Count,4,1,fp);
    fwrite(&io0Bit7,4,1,fp); fwrite(&io0Bit2,4,1,fp);
    fwrite(&kbmask,4,1,fp);
    fwrite(&ramWrap,4,1,fp); fwrite(&(memAnd),4,1,fp);
    fwrite(&comError,4,1,fp);
    fwrite(&transflag,4,1,fp); fwrite(&transbyte,4,1,fp);
    fwrite(&transnotready,4,1,fp);
    fwrite(&recvflag,4,1,fp); fwrite(&recvbyte,4,1,fp);
    fwrite(&romRetOr,4,1,fp);
    fwrite(&lcdBase,4,1,fp); fwrite(&lcdOff,4,1,fp);
    fwrite(&contrast,4,1,fp);
    fwrite(&cpuCompleteStop,4,1,fp);
    fclose(fp);
}

void TI92::LoadState(char *fn)
{
    FILE *fp=fopen(fn,"rb");
    if (!fp) return;
    char str[9],romName[56];
    fread(str,8,1,fp);
    str[8]=0;
    if (strcmp(str,"VTIv2.0 "))
        return;
    fread(romName,56,1,fp);
    if (strcmp(romName,romImage[currentROM].name))
        return;
    fread(&regs,sizeof(regstruct),1,fp);
    MakeFromSR();
    fread(ram,256,1024,fp);
    fread(&memprotect,4,1,fp); fread(&ram128,4,1,fp);
    fread(&timer,4,1,fp); fread(&timerInit,4,1,fp);
    fread(&int0Count,4,1,fp);
    fread(&io0Bit7,4,1,fp); fread(&io0Bit2,4,1,fp);
    fread(&kbmask,4,1,fp);
    fread(&ramWrap,4,1,fp); fread(&(memAnd),4,1,fp);
    fread(&comError,4,1,fp);
    fread(&transflag,4,1,fp); fread(&transbyte,4,1,fp);
    fread(&transnotready,4,1,fp);
    fread(&recvflag,4,1,fp); fread(&recvbyte,4,1,fp);
    fread(&romRetOr,4,1,fp);
    fread(&lcdBase,4,1,fp); fread(&lcdOff,4,1,fp);
    fread(&contrast,4,1,fp);
    fread(&cpuCompleteStop,4,1,fp);
    for (int i=0;i<32;i++)
        mem[i]=&ram[(i&(memAnd>>16))<<16];
    fclose(fp);
}

int TI92::SendByte(int c)
{
    recvflag=1;
    recvbyte=c;
    IRQ(4);
    int count=10000;
    while ((recvflag)&&(count--))
        Execute();
    if (recvflag)
        return 0;
    return 1;
}

int TI92::GetByte(int &c)
{
    int count=10000;
    while ((!transflag)&&(count--))
        Execute();
    if (transflag)
    {
        c=transbyte&0xff;
        transflag=0;
    }
    else
    {
        getError=1;
        return 0;
    }
    return 1;
}

int TI92::SendReady()
{
    return !recvflag;
}

int TI92::SendBuffer(int c)
{
    if (recvflag) return 0;
    recvflag=1;
    recvbyte=c;
    IRQ(4);
    return 1;
}

int TI92::CheckBuffer()
{
    return transflag;
}

int TI92::GetBuffer(int &c)
{
    if (!transflag) return 0;
    transflag=0;
    c=transbyte&0xff;
    return 1;
}

#define WaitForAck() \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; }

int TI92::SendFile(char *fn,int more)
{
	FILE *fp;
	int csum,type,size,id=0x89;
	int i,j,c,n,pos;
	char calcName[32],folderName[9],buf[16];
    unsigned char hdr[16];

    sendInProgress=1;
	for (i=0;i<100;i++)
        Execute();
    if ((fn[strlen(fn)-1]=='B')||(fn[strlen(fn)-1]=='b'))
    {
        fp=fopen(fn,"rb");
        fseek(fp,0x4c,SEEK_SET);
        fread(&size,4,1,fp);
        size-=0x54;
        fseek(fp,0x52,SEEK_SET);
        SendByte(id); SendByte(6);
    	SendByte(5); SendByte(0);
        SendByte(size&0xff);
        SendByte((size>>8)&0xff);
	    SendByte((size>>16)&0xff);
	    SendByte((size>>24)&0xff);
        csum=size&0xff; csum+=(size>>8)&0xff;
        csum+=(size>>16)&0xff; csum+=(size>>24)&0xff;
        SendByte(0x1d);
	    csum+=0x1d;
	    SendByte(csum&0xff); SendByte((csum>>8)&0xff);
        int pos=0;
        while (1)
        {
			WaitForAck();
            int segSize=1024;
            if (segSize>size) segSize=size;
	   	    SendByte(id); SendByte(6);
            SendByte(5); SendByte(0);
	        SendByte(segSize&0xff); SendByte((segSize>>8)&0xff);
            SendByte(0); SendByte(0);
	        SendByte(0x1d);
            csum=0x1d+(segSize&0xff)+((segSize>>8)&0xff);
            SendByte(csum&0xff); SendByte((csum>>8)&0xff);
	        WaitForAck(); WaitForAck();
	        SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
			SendByte(id); SendByte(0x15);
            SendByte(segSize&0xff); SendByte((segSize>>8)&0xff);
            csum=0;
			for (j=0;j<segSize;j++)
            {
		        i=fgetc(fp);
		    	if (!SendByte(i)) { fclose(fp); sendInProgress=0; return 0; }
			    csum+=i;
            }
	        csum&=0xffff;
		    SendByte(csum&0xff); SendByte((csum>>8)&0xff);
            size-=segSize;
            pos+=1024;
            if (size==0) break;
        }
        WaitForAck();
        SendByte(id); SendByte(0x92); SendByte(0); SendByte(0);
        fclose(fp);
        sendInProgress=0;
        return 1;
    }
	for (i=0;i<100;i++)
        Execute();
	fp=fopen(fn,"rb");
	if (!fp)
    {
        sendInProgress=0;
		return 1;
    }
    fseek(fp,0x3a,SEEK_SET);
    n=fgetc(fp);
    n|=fgetc(fp)<<8;
   	fseek(fp,0xa,SEEK_SET);
    fread(buf,8,1,fp);
  	buf[8]=0; strcpy(folderName,buf);
    pos=0x3c;
    for (;n;n--)
    {
        fseek(fp,pos,SEEK_SET);
        fread(hdr,16,1,fp);
        pos+=16;
	    type=hdr[0xc];
        memcpy(buf,&hdr[4],8);
        buf[8]=0;
        if (type==0x1f)
        {
            strcpy(folderName,buf);
            continue;
        }
	    fseek(fp,(hdr[0]&0xff)+((hdr[1]&0xff)<<8)+
            ((hdr[2]&0xff)<<16)+4,SEEK_SET);
    	size=fgetc(fp)<<8;
	    size|=fgetc(fp);
        strcpy(calcName,folderName);
	    strcat(calcName,"\\"); strcat(calcName,buf);
    	id=0x89;
	    SendByte(id); SendByte(6);
    	SendByte(6+strlen(calcName));
	    SendByte(0); SendByte((size+2)&0xff);
    	SendByte(((size+2)>>8)&0xff);
	    SendByte(0); SendByte(0);
    	csum=(size+2)&0xff; csum+=((size+2)>>8)&0xff;
	    SendByte(type);
    	csum+=type;
	    SendByte(strlen(calcName));
    	csum+=strlen(calcName);
	    for (i=0;i<strlen(calcName);i++)
    	{
	    	SendByte(calcName[i]);
    		csum+=calcName[i];
	    }
    	SendByte(csum&0xff); SendByte((csum>>8)&0xff);
	    WaitForAck();
    	WaitForAck();
	    SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
    	SendByte(id); SendByte(0x15);
	    SendByte((size+6)&0xff); SendByte(((size+6)>>8)&0xff);
    	SendByte(0); SendByte(0); SendByte(0); SendByte(0);
	    fseek(fp,(hdr[0]&0xff)+((hdr[1]&0xff)<<8)+
            ((hdr[2]&0xff)<<16)+4,SEEK_SET);
    	csum=0;
	    for (j=0;j<(size+2);j++)
    	{
	    	 i=fgetc(fp)&0xff;
    		 if (!SendByte(i)) { fclose(fp); sendInProgress=0; return 0; }
	    	 csum+=i;
    	}
	    csum&=0xffff;
    	SendByte(csum&0xff); SendByte((csum>>8)&0xff);
	    WaitForAck();
    	SendByte(id); SendByte(0x92); SendByte(0); SendByte(0);
	    WaitForAck();
	    for (i=0;i<1000;i++)
            Execute();
    }
   	fclose(fp);
    sendInProgress=0;
    return 1;
}

int TI92::ReceiveFile()
{
    int i,j,c,len,size,type,id,csum,n,bufLen;
    int curOfs,folderSpecified,folderPos,group;
    char calcName[32],folder[9],name[9],*list,*buf,*newBuf;
    char oldFolder[9];

    for (i=0;i<10;i++) kbmatrix[i]=0xff;
    sendInProgress=1;
    getError=0;
    GetByte(c); id=c;
    GetByte(c); if (c!=6) { sendInProgress=0; return 0; }
    if (getError) { sendInProgress=0; getError=0; return 0; }
    Screen->Cursor=crHourGlass;
    oldFolder[0]=0; n=0; folderPos=-1;
    buf=NULL; bufLen=0;
    if (transferAsGroup)
        list=new char[32768];
    else
        list=NULL;
    while (1)
    {
        GetByte(c); len=c;
        GetByte(c); len|=c<<8;
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        GetByte(c); size=c;
        GetByte(c); size|=c<<8;
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        GetByte(c); GetByte(c);
        GetByte(c); type=c;
        GetByte(c); j=c;
        if ((len-6)>8) { Screen->Cursor=crDefault; sendInProgress=0; return 0; }
        if (j>8) { Screen->Cursor=crDefault; sendInProgress=0; return 0; }
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        for (i=0;i<(len-6);i++)
        {
            GetByte(c);
            calcName[i]=c;
        }
        calcName[j]=0;
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
        SendByte(id); SendByte(9); SendByte(0); SendByte(0);
        GetByte(c); GetByte(c); GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        GetByte(c);
        GetByte(c);
        if (c!=0x15)
        {
            Screen->Cursor=crDefault;
            sendInProgress=0;
            if (buf) delete[] buf;
            if (list) delete[] list;
            return 0;
        }
        GetByte(c); GetByte(c); GetByte(c); GetByte(c);
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        newBuf=new char[bufLen+size+6];
        if (buf)
        {
            memcpy(newBuf,buf,bufLen);
            delete[] buf;
        }
        buf=newBuf;
        curOfs=bufLen+4;
        buf[bufLen]=0; buf[bufLen+1]=0; buf[bufLen+2]=0;
        buf[bufLen+3]=0;
        csum=0;
        for (i=0;i<size;i++)
        {
            GetByte(c);
            buf[curOfs+i]=c;
            csum+=c&0xff;
            if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        }
        buf[curOfs+i]=csum&0xff;
        buf[curOfs+i+1]=csum>>8;
        bufLen+=size+6;
        GetByte(c); GetByte(c);
        SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        folder[5]=0x8b; folder[6]=0x76; folder[7]=0xa;
        if (!strchr(calcName,'\\'))
        {
            strcpy(folder,"main");
            memset(name,0,8);
            strcpy(name,calcName);
            folderSpecified=0;
        }
        else
        {
            *(strchr(calcName,'\\'))=0;
            strcpy(folder,calcName);
            memset(name,0,8);
            strcpy(name,strchr(calcName,0)+1);
            folderSpecified=1;
        }
        if (!transferAsGroup)
        {
            TSaveDialog *dlog=new TSaveDialog(EmuWnd);
            dlog->DefaultExt=defExt92[type&0x1f];
            dlog->FileName=AnsiString(name)+AnsiString(".")+
                dlog->DefaultExt;
            dlog->FilterIndex=6;
            dlog->Options.Clear();
            dlog->Filter="TI-82 files (*.82?)|*.82?|TI-83 files (*.83?)|*.83?|TI-85 files (*.85?)|*.85?|TI-86 files (*.86?)|*.86?|TI-89 files (*.89?)|*.89?|TI-92 files (*.92?)|*.92?|TI-92 Plus files (*.9x?)|*.9x?";
            char cwd[256];
            getcwd(cwd,256);
            dlog->InitialDir=cwd;
            if (useInitPath)
            {
                useInitPath=0;
                dlog->InitialDir=initPath;
            }
            Screen->Cursor=crDefault;
            if (dlog->Execute())
            {
                FILE *fp=fopen(dlog->FileName.c_str(),"wb");
                fprintf(fp,"**TI92**");
                fputc(1,fp); fputc(0,fp);
                for (i=0;i<8;i++)
                    fputc(folder[i],fp);
                for (i=0;i<40;i++)
                    fputc(0,fp);
                fputc(1,fp); fputc(0,fp); fputc(0x52,fp);
                fputc(0,fp); fputc(0,fp); fputc(0,fp);
                for (i=0;i<8;i++)
                    fputc(name[i],fp);
                fputc(type,fp); fputc(0,fp); fputc(0,fp);
                fputc(0,fp);
                fputc((size+0x5a)&0xff,fp);
                fputc((size+0x5a)>>8,fp);
                fputc(0,fp); fputc(0,fp);
                fputc(0xa5,fp); fputc(0x5a,fp);
                for (i=0;i<bufLen;i++)
                    fputc(buf[i],fp);
                fclose(fp);
            }
            Screen->Cursor=crHourGlass;
            delete dlog;
            delete[] buf; buf=NULL; bufLen=0;
        }
        else
        {
            if (folderSpecified)
            {
                if (strcmp(folder,oldFolder))
                {
                    list[(n<<4)]=(curOfs-4)&0xff;
                    list[(n<<4)+1]=((curOfs-4)>>8)&0xff;
                    list[(n<<4)+2]=((curOfs-4)>>16)&0xff;
                    list[(n<<4)+3]=(curOfs-4)>>24;
                    memset(&list[(n<<4)+4],0,8);
                    memcpy(&list[(n<<4)+4],folder,strlen(folder));
                    list[(n<<4)+12]=0x1f;
                    list[(n<<4)+13]=0;
                    list[(n<<4)+14]=0;
                    list[(n<<4)+15]=0;
                    folderPos=n;
                    n++;
                    strcpy(oldFolder,folder);
                }
                if (folderPos!=-1)
                {
                    i=((list[(folderPos<<4)+14]&0xff)|((list[
                        (folderPos<<4)+15]&0xff)<<8))+1;
                    list[(folderPos<<4)+14]=i&0xff;
                    list[(folderPos<<4)+15]=i>>8;
                }
            }
            list[(n<<4)]=(curOfs-4)&0xff;
            list[(n<<4)+1]=((curOfs-4)>>8)&0xff;
            list[(n<<4)+2]=((curOfs-4)>>16)&0xff;
            list[(n<<4)+3]=(curOfs-4)>>24;
            memcpy(&list[(n<<4)+4],name,8);
            list[(n<<4)+12]=type;
            list[(n<<4)+13]=0;
            list[(n<<4)+14]=0;
            list[(n<<4)+15]=0;
            n++;
        }
        if (c!=6) break;
    }
    GetByte(c); GetByte(c);
    SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
    getError=0;
    Screen->Cursor=crDefault;
    if (transferAsGroup)
    {
        for (i=0;i<n;i++)
        {
            j=((list[(i<<4)]&0xff)|((list[(i<<4)+1]&
                0xff)<<8)|((list[(i<<4)+2]&0xff)<<16)|
                ((list[(i<<4)+3]&0xff)<<24))+(n<<4)+0x42;
            list[(i<<4)]=j&0xff;
            list[(i<<4)+1]=(j>>8)&0xff;
            list[(i<<4)+2]=(j>>16)&0xff;
            list[(i<<4)+3]=j>>24;
        }
        TSaveDialog *dlog=new TSaveDialog(EmuWnd);
        if ((n==1)||((n==2)&&(list[12]==0x1f)))
        {
            dlog->DefaultExt=defExt92[type&0x1f];
            group=0;
            if (n==2)
            {
                memcpy(list,&list[16],16);
                n=1;
            }
        }
        else
        {
            dlog->DefaultExt="92g";
            strcpy(name,"group");
            group=1;
        }
        dlog->FileName=AnsiString(name)+AnsiString(".")+
            dlog->DefaultExt;
        dlog->FilterIndex=6;
        dlog->Options.Clear();
        dlog->Options << ofHideReadOnly << ofOverwritePrompt << ofPathMustExist;
        dlog->Filter="TI-82 files (*.82?)|*.82?|TI-83 files (*.83?)|*.83?|TI-85 files (*.85?)|*.85?|TI-86 files (*.86?)|*.86?|TI-89 files (*.89?)|*.89?|TI-92 files (*.92?)|*.92?|TI-92 Plus files (*.9x?)|*.9x?";
        char cwd[256];
        getcwd(cwd,256);
        dlog->InitialDir=cwd;
        if (useInitPath)
        {
            useInitPath=0;
            dlog->InitialDir=initPath;
        }
        if (dlog->Execute())
        {
            FILE *fp=fopen(dlog->FileName.c_str(),"wb");
            fprintf(fp,"**TI92**");
            fputc(1,fp); fputc(0,fp);
            if (group)
            {
                for (i=0;i<8;i++)
                    fputc(0,fp);
            }
            else
            {
                for (i=0;i<8;i++)
                    fputc(folder[i],fp);
            }
            for (i=0;i<40;i++)
                fputc(0,fp);
            fputc(n&0xff,fp); fputc(n>>8,fp);
            for (i=0;i<(n<<4);i++)
                fputc(list[i],fp);
            fputc((bufLen+(n<<4)+0x42)&0xff,fp);
            fputc(((bufLen+(n<<4)+0x42)>>8)&0xff,fp);
            fputc(((bufLen+(n<<4)+0x42)>>16)&0xff,fp);
            fputc((bufLen+(n<<4)+0x42)>>24,fp);
            fputc(0xa5,fp); fputc(0x5a,fp);
            for (i=0;i<bufLen;i++)
                fputc(buf[i],fp);
            fclose(fp);
        }
        delete dlog;
    }
    sendInProgress=0;
    if (list) delete[] list;
    if (buf) delete[] buf;
    return 1;
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
