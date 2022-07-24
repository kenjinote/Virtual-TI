//--------------------------------------------------------------------------
#include <vcl.h>
#include <dir.h>
#include <stdio.h>
#pragma hdrstop

#include "calchw.h"
#include "rom.h"
#define NOTYPEREDEF
#include "Z80.h"
#include "Debug.h"
#include "Emu.h"

extern Z80_Regs R;
static int keymap86[]={VK_ESCAPE,k_esc,'1',k_1,'2',k_2,
    '3',k_3,'4',k_4,'5',k_5,'6',k_6,'7',k_7,
    '8',k_8,'9',k_9,'0',k_0,0xbd,k_neg,
    0xbb,k_equ,VK_BACK,k_left,VK_TAB,k_sto,'Q',
    k_q,'W',k_w,'E',k_e,'R',k_r,'T',k_t,'Y',k_y,
    'U',k_u,'I',k_i,'O',k_o,'P',k_p,0xdb,
    k_lparan,0xdd,k_rparan,VK_RETURN,k_enter1,
    'A',k_a,'S',k_s,'D',k_d,VK_CONTROL,k_dia,
    'F',k_f,'G',k_g,'H',k_h,'J',k_j,'K',k_k,
    'L',k_l,0xc0,k_mode,VK_SHIFT,k_shift,
    0xdc,k_ln,'Z',k_z,'X',k_x,'C',k_c,
    'V',k_v,'B',k_b,'N',k_n,'M',k_m,0xbc,
    k_comma,0xbe,k_dot,0xbf,k_div,VK_MULTIPLY,
    k_mult,VK_MENU,k_2nd,VK_SPACE,k_space,VK_CAPITAL,
    k_hand,VK_F1,k_f1,VK_F2,k_f2,VK_F3,k_f3,VK_F4,
    k_f4,VK_F5,k_f5,VK_F6,k_graph,VK_F7,k_table,VK_F8,
    k_prgm,VK_F9,k_custom,VK_NUMLOCK,k_power,VK_SCROLL,
    k_on,VK_NUMPAD7,k_7,VK_NUMPAD8,k_8,VK_NUMPAD9,k_9,
    VK_SUBTRACT,k_minus,VK_NUMPAD4,k_4,VK_NUMPAD5,k_5,
    VK_NUMPAD6,k_6,VK_ADD,k_plus,VK_NUMPAD1,k_1,
    VK_NUMPAD2,k_2,VK_NUMPAD3,k_3,VK_NUMPAD0,k_0,
    VK_DECIMAL,k_dot,VK_SEPARATOR,k_enter2,VK_DIVIDE,
    k_div,VK_HOME,k_cos,VK_UP,k_up,VK_PRIOR,k_tan,
    VK_LEFT,k_left,VK_RIGHT,k_right,VK_DOWN,k_down,
    VK_INSERT,k_sin,VK_DELETE,k_bs,0xba,
    k_square,0xde,k_log,VK_END,k_ee,VK_NEXT,k_clear,-1,-1};

static int ti86kmat[][8]=
    {kn,kn,kn,kn,k_up,k_right,k_left,k_down,
     kn,k_clear,k_power,k_div,k_mult,k_minus,k_plus,k_enter1,
     kn,k_custom,k_tan,k_rparan,k_9,k_6,k_3,k_neg,
     k_bs,k_prgm,k_cos,k_lparan,k_8,k_5,k_2,k_dot,
     k_dia,k_table,k_sin,k_ee,k_7,k_4,k_1,k_0,
     k_hand,k_graph,k_log,k_ln,k_square,k_comma,k_sto,kn,
     k_mode,k_esc,k_2nd,k_f1,k_f2,k_f3,k_f4,k_f5,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn};

static int ti86alphakmat[][8]=
    {kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,k_e,k_j,k_o,k_t,k_x,kn,
     kn,kn,k_d,k_i,k_n,k_s,k_w,k_space,
     kn,kn,k_c,k_h,k_m,k_r,k_v,k_z,
     kn,kn,k_b,k_g,k_l,k_q,k_u,k_y,
     kn,kn,k_a,k_f,k_k,k_p,k_equ,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn};

static int key[]=
    {k_f1,k_f2,k_f3,k_f4,k_f5,
     k_2nd,k_esc,k_mode,k_left,k_right,
     k_up,k_down,k_hand,k_dia,k_bs,
     k_graph,k_table,k_prgm,k_custom,k_clear,
     k_log,k_sin,k_cos,k_tan,k_power,
     k_ln,k_ee,k_lparan,k_rparan,k_div,
     k_square,k_7,k_8,k_9,k_mult,
     k_comma,k_4,k_5,k_6,k_minus,
     k_sto,k_1,k_2,k_3,k_plus,
     k_on,k_0,k_dot,k_neg,k_enter1};

char* defExt86[0x20]={"86n","86c","86v","86v","86l",
    "86l","86m","86m","86k","86k","86e","86r","86s",
    "86d","86d","86d","86d","86i","86p","86r","86p",
    "86p","86p","86r","86r","86r","86r","86r","86p",
    "86b","86p","86p"};

ROMFunc romFuncs86[]=
   {{0x4000,"_bitNUMOP1"},{0x4004,"_bitModeSolve"},
    {0x4008,"_isParseError"},{0x400c,"_clrError"},
    {0x4010,"_ldHLind"},{0x4014,"_intrptOn"},
    {0x4018,"_SETGRAPHDRAW"},{0x401c,"_setTBLGraphDraw"},
    {0x4020,"_setTBLGraphonly"},
    {0x4024,"_SETTBLGRAPHONLY_nof"},
    {0x4028,"_bitOnInterrupt"},{0x402c,"_bitGrfFuncM"},
    {0x4030,"_bitGrfPolarM"},{0x4034,"_bitGrfParamM"},
    {0x4038,"_bitGrfDifeqM"},{0x403c,"_cphlde"},
    {0x4040,"_dispColon"},{0x4044,"_divHLby10"},
    {0x4048,"_divHLbyA"},{0x404c,"_divHLby10_3b"},
    {0x4050,"_updateRunIndic"},{0x4054,"_alphaRCL"},
    {0x4058,"_APDHandler"},{0x405c,"_idle"},
    {0x4060,"_apdSetup"},{0x4064,"_kbdScan"},
    {0x4068,"_getcsc"},{0x406c,"_unshiftAlpha"},
    {0x4070,"_canAlphIns"},{0x4074,"_canAlpha"},
    {0x4078,"_forceAlpha"},{0x407c,"_setAlphaLock"},
    {0x4080,"_setAlpha"},{0x4084,"_mon"},
    {0x4088,"_monForceKey"},{0x408c,"_sendKPress"},
    {0x4090,"_popQueue"},{0x4094,"_pushQueue"},
    {0x4098,"_popCx"},{0x409c,"_JforceCmdNoChar"},
    {0x409f,"_JforceCmd"},{0x40a2,"_JforceKey"},
    {0x40a5,"_resetStacks"},{0x40a9,"_sysErrHandler"},
    {0x40ad,"_newContext"},{0x40b1,"_newContext0"},
    {0x40b5,"_PPutAwayPrompt"},{0x40b9,"_resPromptFlags"},
    {0x40bd,"_PPutAway"},{0x40c1,"_PutAway"},
    {0x40c5,"_SizeWind"},{0x40c9,"_ErrorEP"},
    {0x40cd,"_callMain"},{0x40d1,"_putAwayNoOp"},
    {0x40d5,"_exit"},{0x40d9,"_clearSolveMode"},
    {0x40dd,"_monErrHand"},{0x40e1,"_appInitIfMemx2"},
    {0x40e5,"_appInitIfMem"},{0x40e9,"_appInitIfDec"},
    {0x40ed,"_appInit"},{0x40f1,"_iINITC"},
    {0x40f5,"_iINITCED"},{0x40f9,"_ifQUIT"},
    {0x40fd,"_onKeyHandler"},{0x4101,"_setChecksum"},
    {0x4105,"_turn86off"},{0x4109,"_initialize"},
    {0x410d,"_resetdone0"},{0x4111,"_ResetDone"},
    {0x4115,"_initMem"},{0x4119,"_INIT_ASAP_RAM"},
    {0x411d,"_INITGS"},{0x4121,"_initlistvar"},
    {0x4125,"_setName"},{0x4129,"_warmup"},
    {0x412d,"_ERRAXES"},{0x4130,"_ERR1STORDER"},
    {0x4133,"_errstatplot"},{0x4136,"_errOverflow"},
    {0x4139,"_errDivBy0"},{0x413c,"_errSingularMat"},
    {0x413f,"_errDomain"},{0x4142,"_errIncrement"},
    {0x4145,"_errSyntax"},{0x4148,"_errNumberBase"},
    {0x414b,"_errMode"},{0x414e,"_errDataType"},
    {0x4151,"_errArgument"},{0x4154,"_errDimMismatch"},
    {0x4157,"_errDimension"},{0x415a,"_errUndefined"},
    {0x415d,"_ERREVSTKOVFL"},{0x4160,"_errMemory"},
    {0x4163,"_checkTokMem"},{0x4166,"_errMemFail"},
    {0x4169,"_errReserved"},{0x416c,"_errInvalid"},
    {0x416f,"_errIllegalNest"},{0x4172,"_errBound"},
    {0x4175,"_errGraphRange"},{0x4178,"_errZoom"},
    {0x417b,"_errBreak"},{0x417e,"_errStat"},
    {0x4181,"_errConversion"},{0x4184,"_errSolver"},
    {0x4187,"_errIterations"},{0x418a,"_errBadGuess"},
    {0x418d,"_errDifEqSetup"},{0x4190,"_errPoly"},
    {0x4193,"_errTolTooSmall"},{0x4196,"_errLinkXmit"},
    {0x4199,"_JError"},{0x419c,"_JErrorNo"},
    {0x419f,"_noErrorEntry"},{0x41a1,"_pushErrorHandler"},
    {0x41a4,"_popErrorHandler"},{0x41a7,"_C10BNUM1"},
    {0x41ab,"_C9BNUM1"},{0x41af,"_C10BNUM0"},
    {0x41b3,"_R10NUM0"},{0x41b7,"_SET10BNUM0"},
    {0x41bb,"_SET10BNUM1"},{0x41bf,"_HLTIMES10"},
    {0x41c3,"_CKOP1CPLX"},{0x41c7,"_CKCPLX"},
    {0x41cb,"_CKOP1REAL"},{0x41cf,"_ANGLE"},
    {0x41d3,"_IMAG"},{0x41d7,"_SETOP1REAL"},
    {0x41db,"_REALX"},{0x41df,"_SETREAL"},
    {0x41e3,"_COP1SET0"},{0x41e7,"_OP2SET0CPLX"},
    {0x41eb,"_SETOP1CPLX"},{0x41ef,"_CPOP4OP3"},
    {0x41f3,"_MOV10OP2CP"},{0x41f7,"_ABSO1O2CP"},
    {0x41fb,"_CPOP1OP2"},{0x41ff,"_CP6"},
    {0x4203,"_OP3TOOP4"},{0x4207,"_OP1TOOP4"},
    {0x420b,"_OP2TOOP4"},{0x420f,"_MOVTOOP4"},
    {0x4213,"_OP4TOOP2"},{0x4217,"_OP4TOOP3"},
    {0x421b,"_OP3TOOP2"},{0x421f,"_OP1TOOP3"},
    {0x4223,"_MOVOP1TO"},{0x4227,"_OP5TOOP2"},
    {0x422b,"_OP5TOOP6"},{0x422f,"_OP5TOOP4"},
    {0x4233,"_OP1TOOP2"},{0x4237,"_MOVTOOP2"},
    {0x423b,"_OP6TOOP2"},{0x423f,"_OP6TOOP1"},
    {0x4243,"_OP4TOOP1"},{0x4247,"_OP5TOOP1"},
    {0x424b,"_OP3TOOP1"},{0x424f,"_OP4TOOP5"},
    {0x4253,"_OP3TOOP5"},{0x4257,"_OP2TOOP5"},
    {0x425b,"_MOVTOOP5"},{0x425f,"_OP2TOOP6"},
    {0x4263,"_OP1TOOP6"},{0x4267,"_MOVTOOP6"},
    {0x426b,"_OP1TOOP5"},{0x426f,"_OP2TOOP1"},
    {0x4273,"_MOVTOOP1"},{0x4277,"_MOVREG"},
    {0x427b,"_MOV10B"},{0x427f,"_MOV18"},
    {0x4283,"_mov9b"},{0x4287,"_MOV8B"},
    {0x428b,"_MOV7B"},{0x428f,"_MOV14"},
    {0x4293,"_MOV6B"},{0x4297,"_MOV5B"},
    {0x429b,"_MOV4B"},{0x429f,"_MOV3B"},
    {0x42a3,"_MOV2B"},{0x42a7,"_O2TOO4D16"},
    {0x42ab,"_O1TOO4D16"},{0x42af,"_O4TOO1D16"},
    {0x42b3,"_O2TOO1D16"},{0x42b7,"_O3TOO2D14"},
    {0x42bb,"_O5TOO1D14"},{0x42bf,"_O3TOO1D14"},
    {0x42c3,"_O3TOO1D16"},{0x42c7,"_TOOP3D14"},
    {0x42cb,"_OP2TOOP3"},{0x42cf,"_MOVTOOP3"},
    {0x42d3,"_OP4TOOP6"},{0x42d7,"_MOV10TOOP1"},
    {0x42db,"_MOV10OP1OP2"},{0x42df,"_MOV10TOOP2"},
    {0x42e3,"_MOVFROP1"},{0x42e7,"_OP4SET1"},
    {0x42eb,"_OP3SET1"},{0x42ef,"_OP2SET8"},
    {0x42f3,"_SETNUM8"},{0x42f7,"_OP2SET5"},
    {0x42fb,"_OP2SET4"},{0x42ff,"_SETNUM4"},
    {0x4303,"_OP2SET3"},{0x4307,"_SETNUM3"},
    {0x430b,"_OP1SET1A"},{0x430f,"_op1set1"},
    {0x4313,"_OP1SET4"},{0x4317,"_OP1SET3"},
    {0x431b,"_OP3SET2"},{0x431f,"_OP1SET2"},
    {0x4323,"_OP2SET2"},{0x4327,"_SETNUM2"},
    {0x432b,"_SETMANT1"},{0x432f,"_OP2SET1"},
    {0x4333,"_SETNUM1"},{0x4337,"_SETNUM"},
    {0x433b,"_SETNUMA"},{0x433f,"_SETMANT"},
    {0x4343,"_ZERO16D"},{0x4347,"_SET16A"},
    {0x434b,"_SET14A"},{0x434f,"_SET14D"},
    {0x4353,"_OP4SET0"},{0x4357,"_OP3SET0"},
    {0x435b,"_OP2SET0"},{0x435f,"_OP1SET0"},
    {0x4363,"_SETNUM0"},{0x4367,"_ZEROOP1M"},
    {0x436b,"_ZEROOP1"},{0x436f,"_ZEROOP2"},
    {0x4373,"_ZEROOP3"},{0x4377,"_ZEROOP"},
    {0x437b,"_CLRLP"},{0x437f,"_SHRCTOACC"},
    {0x4383,"_SHRACC"},{0x4387,"_SHLCTOACC"},
    {0x438b,"_SHLACC"},{0x438f,"_SHROP1D14"},
    {0x4393,"_SHRO1D18"},{0x4397,"_SHROP1D18"},
    {0x439b,"_SHR18"},{0x439f,"_SHR18A"},
    {0x43a3,"_SHROP1D16"},{0x43a7,"_SHR16"},
    {0x43ab,"_SHRATOOPM16"},{0x43af,"_SHR14"},
    {0x43b3,"_SHLOP3D14"},{0x43b7,"_SHLOP1D14"},
    {0x43bb,"_SHR1TOOP1M"},{0x43bf,"_SHR1TOOPM"},
    {0x43c3,"_SHLOP3D16"},{0x43c7,"_SHLOP1D16"},
    {0x43cb,"_SHL16"},{0x43cf,"_SHL14"},
    {0x43d3,"_SHRSIN"},{0x43d7,"_SRDRO4D16"},
    {0x43db,"_SRDO1ONCE"},{0x43df,"_SRDO1"},
    {0x43e3,"_SHRCCON"},{0x43e7,"_SRDRO2D16"},
    {0x43eb,"_SHRDRND"},{0x43ef,"_MANTP1"},
    {0x43f3,"_MANTPA"},{0x43f7,"ADDPROP"},
    {0x43fb,"_ADDPROPLP"},{0x43ff,"_APROPOE"},
    {0x4403,"_APROP10"},{0x4407,"_OP2POP4D16"},
    {0x440b,"_OP2POP1D16"},{0x440f,"_OP1POP2D16"},
    {0x4413,"_OP2POP2D14"},{0x4417,"_OP1POP2D14"},
    {0x441b,"_OP1POP3D16"},{0x441f,"_ADD16D"},
    {0x4423,"_ADD14D"},{0x4427,"_ADD2D"},
    {0x442b,"_CSVAS"},{0x442f,"_CSVAS1"},
    {0x4433,"_CSVASH"},{0x4437,"_OP1POP3D14"},
    {0x443b,"_OP2MOP1D16"},{0x443f,"_OP1MOP2D16"},
    {0x4443,"_OP3MOP1D16"},{0x4447,"_OP1MOP3D16"},
    {0x444b,"_SUB16D"},{0x444f,"_SUB14D"},
    {0x4453,"_SUB10D"},{0x4457,"_SUB8D"},
    {0x445b,"_SUB6D"},{0x445f,"_OP4MD14"},
    {0x4463,"_OP4MD16"},{0x4467,"_OP3MOP1D14"},
    {0x446b,"_OP1MOP3D14"},{0x446f,"_OP2EXOP6"},
    {0x4473,"_OP5EXOP6"},{0x4477,"_OP1EXOP5"},
    {0x447b,"_OP1EXOP6"},{0x447f,"_OP2EXOP4"},
    {0x4483,"_OP2EXOP5"},{0x4487,"_OP1EXOP3"},
    {0x448b,"_OP1EXOP4"},{0x448f,"_OP1EXOP2"},
    {0x4493,"_EXLP"},{0x4497,"_CKOP1C0"},
    {0x449b,"_CKOP1FP0"},{0x449f,"_CKFP0"},
    {0x44a3,"_CKOP2FP0"},{0x44a7,"_POSRNO0INT"},
    {0x44ab,"_POSNO0INT"},{0x44af,"_CKPOSINT"},
    {0x44b3,"CKOP1INT"},{0x44b7,"_CKINT"},
    {0x44bb,"_CKOP1ODD"},{0x44bf,"_CKODD"},
    {0x44c3,"_CKOP1M1"},{0x44c7,"_CKOP1M0"},
    {0x44cb,"_CKOP1M"},{0x44cf,"_CK15ZERO"},
    {0x44d3,"_CKZERO"},{0x44d7,"_CK0S"},
    {0x44db,"_LDCCONB"},{0x44df,"_GETCONB"},
    {0x44e3,"_GETCON1"},{0x44e7,"_GETCON"},
    {0x44eb,"_PIDIV2"},{0x44ef,"_PIDIV4"},
    {0x44f3,"_TWOPI"},{0x44f7,"_PICON"},
    {0x44fb,"_PIDIV4A"},{0x44ff,"_PIDIV2A"},
    {0x4503,"_OP1EXPTODEC"},{0x4507,"_DECTOHEX"},
    {0x450b,"_CKOP2POS"},{0x450f,"_CKOP1POS"},
    {0x4513,"_CLROP2S"},{0x4517,"_CLROP1S"},
    {0x451b,"_FDIV100"},{0x451f,"_FDIV10"},
    {0x4523,"_DECO1EXP"},{0x4527,"_INCO1EXP"},
    {0x452b,"_INCEXP"},{0x452f,"_CKVALIDNUM"},
    {0x4533,"_GETEXP"},{0x4537,"_GETEXP1"},
    {0x453b,"_ISDOLLARPO1"},{0x453f,"_ISATEMPO1"},
    {0x4543,"_ISATEMP"},{0x4547,"_HTIMESL"},
    {0x454b,"_OP1STATLST"},{0x454f,"_OP1PREGC"},
    {0x4553,"_EOP1NOTRC"},{0x4557,"_EOP1NOTREAL"},
    {0x455b,"_EXCNAME"},{0x455f,"_LBNAME"},
    {0x4563,"_ATNAME"},{0x4567,"_ASTNAME"},
    {0x456b,"_THETANAME"},{0x456f,"_RNAME"},
    {0x4573,"_XNAME"},{0x4577,"_YNAME"},
    {0x457b,"_TNAME"},{0x457f,"_SETesTOfps"},
    {0x4583,"_CHKTEMPDIRT"},{0x4587,"_ISO1XYFSTAT"},
    {0x458b,"_ISO2XYFSTAT"},{0x458f,"_CHECKGRAPH"},
    {0x4593,"_CHECKGRAPH1"},{0x4597,"_CHKMATTYPE"},
    {0x459b,"_SETMATCPLX"},{0x459f,"_OP1MOP2EXP"},
    {0x45a3,"_OP1EXPMEXP1"},{0x45a7,"_OP1EXPMDE"},
    {0x45ab,"_OP1EXPMDE1"},{0x45af,"_OP1EXPMEXPM1"},
    {0x45b3,"_CKEXP12"},{0x45b7,"_CHKNUMOP1ERR"},
    {0x45bb,"_CHKERRBREAK"},{0x45bf,"_BITSTAT1"},
    {0x45c3,"_RESMATTYPE"},{0x45c7,"_ERRD_OP1NOTPOS"},
    {0x45cb,"_ERRD_OP1NOT_R"},
    {0x45cf,"_ERRD_OP1NOTPOSINT"},
    {0x45d3,"_ERRD_OP1_LE_0"},{0x45d7,"_ERRD_OP1_0"},
    {0x45db,"_EQU_or_NEWEQU"},{0x45df,"_SMARTER_ACTIVE"},
    {0x45e3,"_INVSUB"},{0x45e7,"_TIMES2"},
    {0x45eb,"_PLUS1"},{0x45ef,"_inct_ade"},
    {0x45f3,"_swapt"},{0x45f7,"_ldtir_ade"},
    {0x45fb,"_ldtir_bde"},{0x45ff,"_ldtir_ahl"},
    {0x4603,"_sttir_ade"},{0x4607,"_sttir_bde"},
    {0x460b,"_DATA_SIZE_TO_DE"},
    {0x460f,"_LOAD_EDITSYM_ASIC"},
    {0x4613,"_SetXXOP1"},{0x4617,"_SetXXOP2"},
    {0x461b,"_SetXXXXOP2"},{0x461f,"_ATODEC"},
    {0x4623,"_reset_currlast"},{0x4627,"_GETLASTENTRY"},
    {0x462b,"_GETLASTENTRYPTR"},{0x462f,"_load_ram_ahl"},
    {0x4633,"_abs_to_page_asic"},{0x4637,"_INC_PTR_AHL"},
    {0x463b,"_dec_ptr_ahl"},{0x463f,"_INC_PTR_BDE"},
    {0x4643,"_DEC_PTR_BDE"},{0x4647,"_SET_ABS_SRC_ADDR"},
    {0x464b,"_LOAD_FREE_MEM"},{0x464f,"_SET_MM_NUM_BYTES"},
    {0x4653,"_LJRND"},{0x4657,"_LJRND2A"},
    {0x465b,"_LJUNDERF"},{0x465f,"_DelResLst"},
    {0x4663,"_Delres"},{0x4667,"_chkgraphprog"},
    {0x466b,"_eqn"},{0x466f,"_exp"},{0x4673,"_expName"},
    {0x4677,"_defaultLimits"},{0x467b,"_ONE_E99"},
    {0x467f,"_XAXISDIF_TO_ACC"},
    {0x4683,"_YAXISDIF_TO_ACC"},{0x4687,"_fldpicname"},
    {0x468b,"_chk_new_size"},
    {0x468f,"_FIND_PARSE_FORMULA"},
    {0x4693,"_PARSE_FORMULA"},{0x4697,"_GET_FORM_NUM"},
    {0x469b,"_chktextcurs"},{0x469f,"_SYSEQNAM"},
    {0x46a3,"_SYSEQNAM_HL"},{0x46a7,"_POINT5"},
    {0x46ab,"_ok_to_edit_prog"},{0x46af,"_HAVE_EXTEND"},
    {0x46b3,"_skip_asap_tok"},{0x46b7,"_EXEC_ASAP_IND"},
    {0x46bb,"_MEMCHK"},{0x46bf,"_DEC_PTR_ADE"},
    {0x46c3,"_GETB_AHL"},{0x46c7,"_AHL_CMP_BDE"},
    {0x46cb,"_FINDSYM"},{0x46cf,"_INSERTMEM"},
    {0x46d3,"_INSERTMEMA"},{0x46d7,"_CLEANALL"},
    {0x46db,"_ENOUGHMEM"},{0x46df,"CTEMP5"},
    {0x46e3,"_CMPMEMNEED"},{0x46e7,"_CREATEVAR3"},
    {0x46eb,"_CREATEREAL"},{0x46ef,"_CREATERCONST"},
    {0x46f3,"_CREATECCONST"},{0x46f7,"_CREATECPLX"},
    {0x46fb,"_CREATETRVECT"},{0x46ff,"_CREATERVECT"},
    {0x4703,"_CREATETCVECT"},{0x4707,"_CREATECVECT"},
    {0x470b,"_CREATETRLIST"},{0x470f,"_CREATERLIST"},
    {0x4713,"_CREATELCLIST"},{0x4717,"_CREATECLIST"},
    {0x471b,"_CREATETRMAT"},{0x471f,"_CREATERMAT"},
    {0x4723,"_CREATETCMAT"},{0x4727,"_CREATECMAT"},
    {0x472b,"_CREATETSTRNG"},{0x472f,"_CREATESTRNG"},
    {0x4733,"_CREATETEQU"},{0x4737,"_CREATEEQU"},
    {0x473b,"_CREATEPICT"},{0x473f,"_CREATESTDB"},
    {0x4743,"_CREATEPODB"},{0x4747,"_CREATEPADB"},
    {0x474b,"_CREATEDEDB"},{0x474f,"_CREATEPROG"},
    {0x4753,"_CHKDEL"},{0x4757,"_CHKDELA"},
    {0x475b,"_DELMEMA"},{0x475f,"_delvar"},
    {0x4763,"_DELVARIO"},{0x4767,"_DELMEM"},
    {0x476b,"_DELVAR3D"},{0x476f,"_DELVAR3C"},
    {0x4773,"_DELVAR3C0"},{0x4777,"_DELVAR3DC"},
    {0x477b,"_datasizea"},{0x477f,"_DATASIZE"},
    {0x4783,"_PUSH2BOPER"},{0x4787,"_CHKMEM_PG1"},
    {0x478b,"_POP2BOPER"},{0x478f,"_PUSHOPER"},
    {0x4793,"_POPOPER"},{0x4797,"_POPOP5"},
    {0x479b,"_POPOP3"},{0x479f,"_POPOP1"},
    {0x47a3,"_POPREALO6"},{0x47a7,"_POPREALO5"},
    {0x47ab,"_POPREALO4"},{0x47af,"_POPREALO3"},
    {0x47b3,"_POPREALO2"},{0x47b7,"_POPREALO1"},
    {0x47bb,"_POPREAL"},{0x47bf,"_POPMCPLXO1"},
    {0x47c3,"_POPMCPLX"},{0x47c7,"_MOVCPLX"},
    {0x47cb,"_FPOPCPLX"},{0x47cf,"_FPOPREAL"},
    {0x47d3,"_FPOPFPS"},{0x47d7,"_DEALLOCFPS"},
    {0x47db,"_DEALLOCFPS1"},{0x47df,"_PAGE_1_TO_ASIC"},
    {0x47e3,"_RAM_PAGE_1"},{0x47e7,"_LOAD_OPS_ASIC"},
    {0x47eb,"_LOAD_ES_ASIC"},{0x47ef,"_LOAD_FPS_ASIC"},
    {0x47f3,"_RAM_PAGE_7"},{0x47f7,"_ALLOCFPS"},
    {0x47fb,"_ALLOCFPS1"},{0x47ff,"_PUSHREALO6"},
    {0x4803,"_PUSHREALO5"},{0x4807,"_PUSHREALO4"},
    {0x480B,"_PUSHREALO3"},{0x480F,"_PUSHREALO2"},
    {0x4813,"_PUSHREALO1"},{0x4817,"_PUSHREAL"},
    {0x481B,"_PUSHOP5"},{0x481F,"_PUSHOP3"},
    {0x4823,"_PUSHMCPLXO3"},{0x4827,"_PUSHOP1"},
    {0x482B,"_PUSHMCPLXO1"},{0x482F,"_PUSHMCPLX"},
    {0x4833,"_EXMCPLXO1"},{0x4837,"_EXREALO1"},
    {0x483B,"_CPYTO1FPS11"},{0x483F,"_CPYTO1FPS6"},
    {0x4843,"_CPYFPS6"},{0x4847,"_CPYTO1FPS8"},
    {0x484B,"_CPYCTO1FPS5"},{0x484F,"_CPYTO1FPS5"},
    {0x4853,"_CPYFPS5"},{0x4857,"_CPYTO2FPS4"},
    {0x485B,"_CPYTO2FPS5"},{0x485F,"_CPYTO6FPS3"},
    {0x4863,"_CPYTO6FPS2"},{0x4867,"_CPYTO2FPS3"},
    {0x486B,"_CPYCTO1FPS3"},{0x486F,"_CPYTO1FPS3"},
    {0x4873,"_CPYFPS3"},{0x4877,"_CPYS3"},
    {0x487B,"_CPYTO1FPS4"},{0x487F,"_CPYTO3FPS2"},
    {0x4883,"_CPYTO5FPST"},{0x4887,"_CPYTO6FPST"},
    {0x488B,"_CPYTO4FPST"},{0x488F,"_CPYTO3FPST"},
    {0x4893,"_CPYTO2FPST"},{0x4897,"_CPYTO1FPST"},
    {0x489B,"_CPYFPST"},{0x489F,"_CPYSTOP"},
    {0x48A3,"_CPYSTACK"},{0x48A7,"_CPYCTO3FPS1"},
    {0x48AB,"_CPYTO3FPS1"},{0x48AF,"_CPYTO2FPS1"},
    {0x48B3,"_CPYCTO1FPS1"},{0x48B7,"_CPYTO1FPS1"},
    {0x48BB,"_CPYFPS1"},{0x48BF,"_CPYS1"},
    {0x48C3,"_CPYTO2FPS2"},{0x48C7,"_CPYTO1FPS2"},
    {0x48CB,"_CPYFPS2"},{0x48CF,"_CPYS2"},
    {0x48D3,"_CPYO3TOFPST"},{0x48D7,"_CPYO2TOFPST"},
    {0x48DB,"_CPYO6TOFPST"},{0x48DF,"_CPYO1TOFPST"},
    {0x48E3,"_CPYTOFPST"},{0x48E7,"_CPYTOSTOP"},
    {0x48EB,"_CPYTOSTACK"},{0x48EF,"_CPYO5TOFPS1"},
    {0x48F3,"_CPYCO1TOFPS1"},{0x48F7,"_CPYO1TOFPS1"},
    {0x48FB,"_CPYTOFPS1"},{0x48FF,"_CPYTOS1"},
    {0x4903,"_CPYO2TOFPS2"},{0x4907,"_CPYO3TOFPS2"},
    {0x490B,"_CPYO6TOFPS2"},{0x490F,"_CPYO1TOFPS2"},
    {0x4913,"_CPYTOFPS2"},{0x4917,"_CPYTOS2"},
    {0x491B,"_CPYO5TOFPS3"},{0x491F,"_CPYCO1TOFPS3"},
    {0x4923,"_CPYO1TOFPS3"},{0x4927,"_CPYTOFPS3"},
    {0x492B,"_CPYTOS3"},{0x492F,"_CPYO1TOFPS4"},
    {0x4933,"_CPYTOFPS4"},{0x4937,"_CPYTOS4"},
    {0x493B,"_CPYO1TOFPS6"},{0x493F,"_CPYO1TOFPS7"},
    {0x4943,"_CPYO1TOFPS8"},{0x4947,"_ERRNOTENOUGHMEM"},
    {0x494B,"_FPSMINUS10"},{0x494F,"_selfTest"},
    {0x4953,"_ROMTest"},{0x4957,"_strLength"},
    {0x495B,"_strCopy"},{0x495F,"_strCat"},
    {0x4963,"_pstrCmp"},{0x4967,"_ISINFORMSET"},
    {0x496B,"_isInSet"},{0x496F,"___bank_call"},
    {0x4973,"___bank_ret"},{0x4976,"___bank_jump"},
    {0x4979,"___bank_entry"},{0x497D,"_JcatApp"},
    {0x4980,"_CustomInit"},{0x4984,"_lcustom"},
    {0x4988,"_curBlink"},{0x498C,"_cursorOff"},
    {0x4990,"_hideCursor"},{0x4994,"_cursorOn"},
    {0x4998,"_showCursor"},{0x499C,"_init_math_menu"},
    {0x49A0,"_bkup_init_math"},{0x49A4,"_RstrSmallText"},
    {0x49A8,"_installAppMenu"},{0x49AC,"_changeGrMenu"},
    {0x49B0,"_installGrMenu"},{0x49B4,"_menuPress"},
    {0x49B8,"_installMenu"},{0x49BC,"_loadGrMenu"},
    {0x49C0,"_loadAppMenu"},{0x49C4,"_loadMenu"},
    {0x49C8,"_ASAPloadmenu"},{0x49CC,"_hideMenu"},
    {0x49D0,"_popSys"},{0x49D4,"_popApp"},
    {0x49D8,"_ckNamesMenu"},{0x49DC,"_flushAllMenus"},
    {0x49E0,"_flushSys"},{0x49E4,"_flushApp"},
    {0x49E8,"_funcDisp"},{0x49EC,"_DispMenus"},
    {0x49F0,"_computeWinBtm"},{0x49F4,"_nextKey"},
    {0x49F7,"_getMenuPtr"},{0x49FB,"_getMenuPtrL7"},
    {0x49FF,"_aGrMenu"},{0x4A02,"_FindConvCode"},
    {0x4A06,"_mStatNames"},{0x4A0A,"_exec_mode"},
    {0x4A0E,"_JmodeApp"},{0x4A11,"_JformtApp"},
    {0x4A14,"_statplotmodeapp"},
    {0x4A18,"_statplotsetdisp"},{0x4A1C,"_JTbl1ModeApp"},
    {0x4A1F,"_TblSet1Disp"},{0x4A23,"_SetIndDep"},
    {0x4A27,"_putmap"},{0x4A2B,"_putc"},{0x4A2F,"_putb"},
    {0x4A33,"_dispHL"},{0x4A37,"_puts"},{0x4A3B,"_putps"},
    {0x4A3F,"_putbuf"},{0x4A43,"_wputc"},{0x4A47,"_wputs"},
    {0x4A4B,"_wputsEOL"},{0x4A4F,"_wdispEOL"},
    {0x4A53,"_whomeUp"},{0x4A57,"_setNumWindow"},
    {0x4A5B,"_checkCurRow"},{0x4A5F,"_newline"},
    {0x4A63,"_lineUp"},{0x4A67,"_moveDown"},
    {0x4A6B,"_scrollUp"},{0x4A6F,"_JshrinkWindow"},
    {0x4A72,"_shrinkWindow"},{0x4A76,"_moveUp"},
    {0x4A7A,"_scrollDown"},{0x4A7E,"_clrLCD"},
    {0x4A82,"_clrScrn"},{0x4A86,"_clrWindow"},
    {0x4A8A,"_eraseEOL"},{0x4A8E,"_JeraseEOW"},
    {0x4A91,"_eraseEOW"},{0x4A95,"_homeUp"},
    {0x4A99,"_getcurloc"},{0x4A9D,"_vputmap_g"},
    {0x4AA1,"_vputmap"},{0x4AA5,"_vputs"},
    {0x4AA9,"_vputsn"},{0x4AAD,"_runIndicOn"},
    {0x4AB1,"_runIndicOff"},{0x4AB5,"_saveCmdShadow"},
    {0x4AB9,"_saveShadow"},{0x4ABD,"_JsaveShadow"},
    {0x4AC0,"_JrstrShadow"},{0x4AC3,"_rstrShadow"},
    {0x4AC7,"_rstrPartial"},{0x4ACB,"_rstrCurRow"},
    {0x4ACF,"_rstrUnderMenu"},{0x4AD3,"_saveTR"},
    {0x4AD7,"_restoretr"},{0x4ADB,"_accent"},
    {0x4ADF,"_CSUB"},{0x4AE3,"_CADD"},{0x4AE7,"_CSQUARE"},
    {0x4AEB,"_CMULT"},{0x4AEF,"_CDIV"},{0x4AF3,"_CABS"},
    {0x4AF7,"_CABS1"},{0x4AFB,"_CSQROOT"},
    {0x4AFF,"_CACOSH"},{0x4B03,"_CACOS"},{0x4B07,"_CASIN"},
    {0x4B0B,"_CATANH"},{0x4B0F,"_CONJ"},
    {0x4B13,"_CMLTBYREAL"},{0x4B17,"_CDIVBYREAL"},
    {0x4B1B,"_BINOPEXEC"},{0x4B1F,"_BINOPEXEC1"},
    {0x4B23,"_SET2MVLPTRS"},{0x4B27,"_SETMAT1"},
    {0x4B2B,"_SETUNOPMAT2"},{0x4B2F,"_CREATETLIST"},
    {0x4B33,"_LIST_CONVERT"},{0x4B37,"_UNOPEXEC"},
    {0x4B3B,"_MARKTEMPS"},{0x4B3F,"_THREEEXEC"},
    {0x4B43,"_FOUREXEC"},{0x4B47,"_FIVEEXEC"},
    {0x4B4B,"_EXMEAN1"},{0x4B4F,"_fofx3A"},
    {0x4B53,"_ADRLELE"},{0x4B57,"_GETL1TOOP1"},
    {0x4B5B,"_GETL1TOP1A"},{0x4B5F,"_GETLTOOP1"},
    {0x4B63,"_GETL1TOOP2"},{0x4B67,"_GETL1TOP2A"},
    {0x4B6B,"_GETL2TOOP1"},{0x4B6F,"_GETL2TOP1A"},
    {0x4B73,"_PUTTOLA"},{0x4B77,"_PUTTOLA1"},
    {0x4B7B,"_PUTTOL"},{0x4B7F,"_LLOW"},{0x4B83,"_LHIGH"},
    {0x4B87,"_LSUM"},{0x4B8B,"_LISTCONST"},
    {0x4B8F,"_CLR_ALL_LST"},{0x4B93,"_TOFRAC"},
    {0x4B97,"_INTERPOLATE"},{0x4B9B,"_ARC"},
    {0x4B9F,"_GFUDYDX"},{0x4BA3,"_GPODRDo"},
    {0x4BA7,"_GPAD?DT"},{0x4BAB,"_GPODYDX"},
    {0x4BAF,"_GPADYDX"},{0x4BB3,"_ADRMROW"},
    {0x4BB7,"_ADRMELE"},{0x4BBB,"_GETMATOP1A"},
    {0x4BBF,"_GETM1TOOP1"},{0x4BC3,"_GETM1TOP1A"},
    {0x4BC7,"_GETMTOOP1"},{0x4BCB,"_GETM1TOP2A"},
    {0x4BCF,"_GETMTOOP2"},{0x4BD3,"_PUTTOM1A"},
    {0x4BD7,"_PUTTOMA"},{0x4BDB,"_PUTTOMA1"},
    {0x4BDF,"_PUTTOMAT"},{0x4BE3,"_STOSETNEXT"},
    {0x4BE7,"_MOP1SET0"},{0x4BEB,"_IDNMAT"},
    {0x4BEF,"_MINVOP1S"},{0x4BF3,"_CYLTOREC"},
    {0x4BF7,"_POLTOREC"},{0x4BFB,"_SPHTOREC"},
    {0x4BFF,"_MATCONST"},{0x4C03,"_MCKOP1FP0"},
    {0x4C07,"_ROWECH_POLY"},{0x4C0B,"_FORSEQINIT"},
    {0x4C0F,"_INC_ERRNOLPAREN"},{0x4C13,"_set_curpc"},
    {0x4C17,"_PARENSYN"},{0x4C1B,"_PARENSYNB"},
    {0x4C1F,"_BC2NONREAL"},{0x4C23,"_ERRNONREAL"},
    {0x4C27,"_READ_LIST_ONLY"},
    {0x4C2B,"_READ_LIST_ONLY_P"},{0x4C2F,"_INTOP1"},
    {0x4C33,"_PARSEINC"},{0x4C37,"_PARSEarg"},
    {0x4C3B,"_LOAD_ABS_DEST_ADDR"},
    {0x4C3F,"_AHL_PLUS_2_PG3"},{0x4C43,"_GRPHPARS"},
    {0x4C47,"_PLOTPARS"},{0x4C4B,"_CKPLOTRES"},
    {0x4C4F,"_EXECQPA"},{0x4C53,"_PARSEON"},
    {0x4C57,"_PARSEINP"},{0x4C5B,"_PARSEOFF"},
    {0x4C5F,"_PARSEAXIS"},{0x4C63,"_PARSESCAN"},
    {0x4C67,"_GETPARSE"},{0x4C6B,"_SAVEPARSE"},
    {0x4C6F,"_INITPFLGS"},{0x4C73,"_INITFFLGS"},
    {0x4C77,"_SET_CURPC_BDE"},{0x4C7B,"_CKENDLINERR"},
    {0x4C7F,"_OP2SET60"},{0x4C83,"_FIND_SET_PTR1_3"},
    {0x4C87,"_SET_PTR1_3"},{0x4C8B,"_SET_CHKDELPTR1_BDE"},
    {0x4C8F,"_RCLSTAT"},{0x4C93,"_RCLCONST"},
    {0x4C97,"_VARSYSADR"},{0x4C9B,"_stosystok"},
    {0x4C9F,"_STOANS"},{0x4CA3,"_STOQA"},
    {0x4CA7,"_STOOTHER_form"},{0x4CAB,"_STOTHETA"},
    {0x4CAF,"_STOR"},{0x4CB3,"_STOY"},{0x4CB7,"_STOT"},
    {0x4CBB,"_STOX"},{0x4CBF,"_STOOTHER"},
    {0x4CC3,"_VAR12A"},{0x4CC7,"_STOTYPE"},
    {0x4CCB,"_STOTYPEC"},{0x4CCF,"_SYSEQSEL"},
    {0x4CD3,"_RCLQIA"},{0x4CD7,"_RCLQA"},
    {0x4CDB,"_RCLANS"},{0x4CDF,"_RCLY"},{0x4CE3,"_RCLX"},
    {0x4CE7,"_RCLVARSYM"},{0x4CEB,"_RCLSYSTOK"},
    {0x4CEF,"_StMatEl"},{0x4CF3,"_StLstVecEl"},
    {0x4CF7,"_ERRIFBASE"},{0x4CFB,"_INCCURPCERREND"},
    {0x4CFF,"_COMMAERRF"},{0x4D03,"_COMMAERR"},
    {0x4D07,"_STEQARG2"},{0x4D0B,"_STEQARG"},
    {0x4D0F,"_BRKINC"},{0x4D13,"_incfetch"},
    {0x4D17,"_TOPOP"},{0x4D1B,"_CKFETCHVAR"},
    {0x4D1F,"_FETCHVARA"},{0x4D23,"_fetchvar"},
    {0x4D27,"_ckendlin"},{0x4D2B,"_grvalvar1"},
    {0x4D2F,"_DEL_LIST_FORM"},{0x4D33,"_HAVE_QIVAR"},
    {0x4D37,"_HAVE_QVAR"},{0x4D3B,"_delPlusName"},
    {0x4D3F,"_disp"},{0x4D43,"_PAUSE"},
    {0x4D47,"_XYRNDBOTH"},{0x4D4B,"_XYRND"},
    {0x4D4F,"_CheckTOP"},{0x4D53,"_CheckXY"},
    {0x4D57,"_DARKPNT"},{0x4D5B,"_CPointS"},
    {0x4D5F,"_WtoV"},{0x4D63,"_XItoF"},{0x4D67,"_YFtoI"},
    {0x4D6B,"_XFtoI"},{0x4D6F,"_PDspGrph"},
    {0x4D73,"_HORIZCMD"},{0x4D77,"_VERTCMD"},
    {0x4D7B,"_LINECMD"},{0x4D7F,"_POINTCMD"},
    {0x4D83,"_PIXELTEST"},{0x4D87,"_PIXELCMD"},
    {0x4D8B,"_TANLNF"},{0x4D8F,"_DRAWCMD"},
    {0x4D93,"_SHADECMD"},{0x4D97,"_INVCMD"},
    {0x4D9B,"_CIRCCMD"},{0x4D9F,"_LOAD_XOUTDAT_GR"},
    {0x4DA3,"_LOAD_YOUTDAT_GR"},
    {0x4DA7,"_LOAD_INPUTDAT_GR"},{0x4DAB,"_putEquNo"},
    {0x4DAF,"_divAby10"},{0x4DB3,"_DISP_DREQ_PRMPT"},
    {0x4DB7,"_JgraphStat"},{0x4DBA,"_JgraphApp"},
    {0x4DBD,"_TraceOff"},{0x4DC1,"_GRAPH_DSP_CHAR"},
    {0x4DC5,"_GRDECODA"},{0x4DC9,"_LABCOOR"},
    {0x4DCD,"_COORDISP"},{0x4DD1,"_CKEVALEQ"},
    {0x4DD5,"_EVALVARINIT"},{0x4DD9,"_EVAL_DIF"},
    {0x4DDD,"_GRLABELS"},{0x4DE1,"_YPIXSET"},
    {0x4DE5,"_XPIXSET"},{0x4DE9,"_COPYRNG"},
    {0x4DED,"_VALCUR"},{0x4DF1,"_grputaway_mon"},
    {0x4DF5,"_RSTGFLAGS"},{0x4DF9,"_grReset"},
    {0x4DFD,"_XYCENT"},{0x4E01,"_ZOOMXYCMD"},
    {0x4E05,"_ZFRNG"},{0x4E09,"_CKVALDELTA"},
    {0x4E0D,"_SETFUNCM"},{0x4E11,"_SETPOLM"},
    {0x4E15,"_SETPARM"},{0x4E19,"_SETDIFM"},
    {0x4E1D,"_ZMDECML"},{0x4E21,"_ZMPREV"},
    {0x4E25,"_ZMUSR"},{0x4E29,"_ZMSQUARE"},
    {0x4E2D,"_ZMTRIG"},{0x4E31,"_USERDEFAULT"},
    {0x4E35,"_ZooDefault"},{0x4E39,"_GRBUFCPY"},
    {0x4E3D,"_BLINKGCUR"},{0x4E41,"_NBCURSOR"},
    {0x4E45,"_STATMARK"},{0x4E49,"_YBar"},
    {0x4E4D,"_DARKLINE"},{0x4E51,"_ILine"},
    {0x4E55,"_POINTON"},{0x4E59,"_IPoint"},
    {0x4E5D,"_fstatname"},{0x4E61,"_yStatName"},
    {0x4E65,"_xStatName"},{0x4E69,"_STATNAME"},
    {0x4E6D,"_EQNNAME"},{0x4E71,"_GEQNAMEA"},
    {0x4E75,"_GEQNAME"},{0x4E79,"_QPANAME"},
    {0x4E7D,"_QIANAME"},{0x4E81,"_QANAME"},
    {0x4E85,"_RegeqName"},{0x4E89,"_ReGraph"},
    {0x4E8D,"_REGRAPH1"},{0x4E91,"_DOREFFLAGS02"},
    {0x4E95,"_INITDNEQ"},{0x4E99,"_INITDNEQ1"},
    {0x4E9D,"_INPUTDAT_TOOP2_GR"},{0x4EA1,"_NEXTTDIF"},
    {0x4EA5,"_INPTRS_GR"},{0x4EA9,"_SET_INPUTDAT_GR"},
    {0x4EAD,"_ResMem"},{0x4EB0,"_ResDef"},
    {0x4EB3,"_ResAll"},{0x4EB6,"_DFMAX"},{0x4EBA,"_DFMIN"},
    {0x4EBE,"_difFEQRESET"},{0x4EC2,"_diffeqset_trcup"},
    {0x4EC6,"_DIFFEQSET"},{0x4ECA,"_DIFFEQSET_trcp"},
    {0x4ECE,"_set_qnews"},{0x4ED2,"_DSOLVE"},
    {0x4ED6,"_CMPQA"},{0x4EDA,"_DRAW_SLOPE_F"},
    {0x4EDE,"_DRAW_DIR_FIELD"},{0x4EE2,"_NXTDIFVAL"},
    {0x4EE6,"_NXT_cur_DIFVAL"},{0x4EEA,"_EIGEN"},
    {0x4EEE,"_PUTTOAIJ"},{0x4EF2,"_PUTTOAJJ"},
    {0x4EF6,"_CPYTO2ES1"},{0x4EFA,"_CPYTO1ES1"},
    {0x4EFE,"_CPYES1"},{0x4F02,"_CPYTO3ES2"},
    {0x4F06,"_CPYTO2ES2"},{0x4F0A,"_CPYTO1ES2"},
    {0x4F0E,"_CPYTO2ES3"},{0x4F12,"_CPYTO1ES3"},
    {0x4F16,"_CPYTO3ES4"},{0x4F1A,"_CPYTO2ES4"},
    {0x4F1E,"_CPYTO1ES4"},{0x4F22,"_CPYTO2ES5"},
    {0x4F26,"_CPYTO1ES5"},{0x4F2A,"_CPYTO2EST"},
    {0x4F2E,"_CPYTO1EST"},{0x4F32,"_CPYEST"},
    {0x4F36,"_CPYTO2ES6"},{0x4F3A,"_CPYTO1ES6"},
    {0x4F3E,"_CPYTO2ES7"},{0x4F42,"_CPYTO1ES7"},
    {0x4F46,"_CPYTO2ES8"},{0x4F4A,"_CPYTO1ES8"},
    {0x4F4E,"_CPYTO1ES9"},{0x4F52,"_CPYTO1ES11"},
    {0x4F56,"_CPYO1TOEST"},{0x4F5A,"_CPYO1TOES1"},
    {0x4F5E,"_CPYO1TOES2"},{0x4F62,"_CPYO1TOES3"},
    {0x4F66,"_CPYO1TOES4"},{0x4F6A,"_CPYO1TOES5"},
    {0x4F6E,"_CPYO1TOES6"},{0x4F72,"_CPYO1TOES7"},
    {0x4F76,"_GETVJ"},{0x4F7A,"_EVALTOKEN"},
    {0x4F7E,"_INDEPVARNAME"},{0x4F82,"_QUAD"},
    {0x4F86,"_GRAPHQUAD"},{0x4F8A,"_ITSOLVER"},
    {0x4F8E,"_gritsolver"},{0x4F92,"_ITSOLVERB"},
    {0x4F96,"_ITSOLVERNB"},{0x4F9A,"_SET_EDITCURSOR"},
    {0x4F9E,"_JconEdtApp"},{0x4FA1,"_SET10BNUM0_BDE"},
    {0x4FA5,"_JInterEditApp"},{0x4FA8,"_setTitle"},
    {0x4FAC,"_dispNumVars"},{0x4FB0,"_setEdit2nd"},
    {0x4FB4,"_setEdit1st"},{0x4FB8,"_RecalledOp1"},
    {0x4FBC,"_RecallEd"},{0x4FC0,"_PosEditCur"},
    {0x4FC4,"_setupBuffer"},{0x4FC8,"_createNumEditBuf"},
    {0x4FCC,"_numHandleKey"},{0x4FD0,"_CallCommon"},
    {0x4FD4,"_CommonKeys"},{0x4FD7,"_fDel"},
    {0x4FDB,"_ScrollUpifBot"},{0x4FDF,"_ScrollDownIfTop"},
    {0x4FE3,"_fClear"},{0x4FE7,"_fClearEqu"},
    {0x4FEB,"_fcondBlank"},{0x4FEF,"_finsDisp"},
    {0x4FF3,"_fcurAdjust"},{0x4FF7,"_fHome"},
    {0x4FFB,"_setLeftEdge"},{0x4FFF,"_setUpArrow"},
    {0x5003,"_setDownArrow"},{0x5007,"_rmvDownArrow"},
    {0x500B,"_rmvThisArrow"},{0x500F,"_setIndicator"},
    {0x5012,"_fdispEOL"},{0x5016,"_closeEditBufNoR"},
    {0x501A,"_releaseBuffer"},{0x501E,"_nameToOP1"},
    {0x5022,"_nameToOP1hl"},{0x5026,"_cursorBOL"},
    {0x5029,"_cursorEOL"},{0x502C,"_numPPutAway"},
    {0x5030,"_numPutAway"},{0x5034,"_numSizeWind"},
    {0x5038,"_NumRedisp"},{0x503C,"_numerrorc"},
    {0x5040,"_JnumError"},{0x5043,"_NumError"},
    {0x5047,"_numError02"},{0x504A,"_TableTbl"},
    {0x504E,"_POLYROOT3"},{0x5052,"_DOPEVAL"},
    {0x5056,"_JpolyApp"},{0x5059,"_notEnterpmt"},
    {0x505C,"_outputsubnum"},{0x5060,"_JprgmEditApp"},
    {0x5063,"_ZifRclHandler"},{0x5067,"_initNameBuffer"},
    {0x506B,"_REINSTATE"},{0x506F,"_scrollPromptUp"},
    {0x5073,"_scrollPromptDown"},{0x5077,"_rclKey"},
    {0x507B,"_validateName"},{0x507F,"_InitNamePrompt"},
    {0x5083,"_namePrompt2"},{0x5087,"_promptKey"},
    {0x508B,"_JgrRangeApp"},{0x508E,"_JsimulApp"},
    {0x5091,"_SetupForStore"},{0x5095,"_closematField"},
    {0x5099,"_redispmatField"},{0x509D,"_CommonSizeWind"},
    {0x50A1,"_JSlistEdApp"},{0x50A4,"_SpeedBump"},
    {0x50A8,"_JstatCALCApp"},{0x50AB,"_JStatCApp"},
    {0x50AE,"_ShowStCmd"},{0x50B2,"_jstatup"},
    {0x50B5,"_jstatdown"},{0x50B8,"_jstatsizelarger"},
    {0x50BB,"_jstatsizesmaller"},{0x50BE,"_JstatFCSTApp"},
    {0x50C1,"_op1namtoedit"},{0x50C5,"_dspnameOp1"},
    {0x50C9,"_DspItEol"},{0x50CD,"_clrline"},
    {0x50D1,"_ErrOp2Xstat"},{0x50D5,"_ErrOp2Ystat"},
    {0x50D9,"_chkop2fstat"},{0x50DD,"_CHKOP2YSTAT"},
    {0x50E1,"_CHKOP2XSTAT"},{0x50E5,"_CMP5OP2OP1"},
    {0x50E9,"_Getname"},{0x50ED,"_namError"},
    {0x50F1,"_MAKE_FORM_NAME"},{0x50F5,"_NAMEINOP1"},
    {0x50F9,"_NAMEINOP1f"},{0x50FD,"_JtolApp"},
    {0x5100,"_JgrZfactApp"},{0x5103,"_CONVMCTOMR"},
    {0x5107,"_REDIMMAT"},{0x510B,"_DELCOLS"},
    {0x510F,"_INSCOLS"},{0x5113,"_INSROWS"},
    {0x5117,"_DELROWS"},{0x511B,"_CONVVRTOVC"},
    {0x511F,"_CONVMRTOMC"},{0x5123,"_CONVLRTOLC"},
    {0x5127,"_INCLSTSIZE"},{0x512B,"_insertlist"},
    {0x512F,"_dellistel"},{0x5133,"_EDITPROG"},
    {0x5137,"_CLOSEPROG"},{0x513B,"_CLRGRAPHREF"},
    {0x513F,"_FIXTEMPCNT"},{0x5143,"_SAVEDATA"},
    {0x5147,"_RESTOREDATA"},{0x514B,"_FINDALPHAUP"},
    {0x514F,"_findalphaDn"},{0x5153,"_READ_SYM_NAME"},
    {0x5157,"_ADJTYPE"},{0x515B,"_formDisp"},
    {0x515F,"_dispDone"},{0x5163,"_formMatrix"},
    {0x5167,"_jwscrollpregcleft"},
    {0x516A,"_jwscrollpregcright"},{0x516D,"_wscrollLeft"},
    {0x5171,"_JwscrollLeft"},{0x5174,"_wscrollUp"},
    {0x5178,"_JwscrollUp"},{0x517B,"_wscrollDown"},
    {0x517F,"_JwscrollDown"},{0x5182,"_wscrollRight"},
    {0x5186,"_JwscrollRight"},{0x5189,"_FormRedispmore"},
    {0x518D,"_formEReal"},{0x5191,"_formReal"},
    {0x5195,"_formdCplx"},{0x5199,"_ClearScrollFlags"},
    {0x519D,"_formScrollUp"},{0x51A1,"_initwindow"},
    {0x51A5,"_disarmScroll"},{0x51A9,"_op1toedit"},
    {0x51AD,"_RCLENTRYTOEDIT"},{0x51B1,"_rclVarToEdit"},
    {0x51B5,"_rclToEdit"},{0x51B9,"_rclToQueue"},
    {0x51BD,"_tokenize"},{0x51C1,"_sSearch"},
    {0x51C5,"_ASAPSEARCH"},{0x51C9,"_textLen"},
    {0x51CD,"_EqToSt"},{0x51D1,"_StToEq"},
    {0x51D5,"_fillSysPrompt"},{0x51D9,"_detok"},
    {0x51DD,"_SET_EDITTAIL"},{0x51E1,"_setTokTemp"},
    {0x51E5,"_COMPILE"},{0x51E9,"_CLine"},
    {0x51ED,"_CLINES"},{0x51F1,"_YONOFF"},
    {0x51F5,"_ADJMEMPTR"},{0x51F9,"_ADJfreemem"},
    {0x51FD,"_ADJMATH"},{0x5201,"_ADJM7"},
    {0x5205,"_SET_CHKDELPTR2"},
    {0x5209,"_LOAD_ABS_SRC_ADDR"},
    {0x520D,"_SET_INSDELPTR"},{0x5211,"_LOAD_INSDELPTR"},
    {0x5215,"_LOAD_CHKDELPTR2"},
    {0x5219,"_LOAD_CHKDELPTR1"},
    {0x521D,"_GET_DATA_SIZE_DE_INC"},
    {0x5221,"_WRITE_SIZE_DE_INC"},{0x5225,"_ADJSYMPTRS"},
    {0x5229,"_ADJSYMP1"},{0x522D,"_ADJPARSER"},
    {0x5231,"_SET_ENDPC"},{0x5235,"_ABS_MOV10TOOP1"},
    {0x5239,"_ABS_MOV10TOOP1_noset"},
    {0x523D,"_ABS_MOV10B_SET_D"},
    {0x5241,"_ABS_MOVFROP1_SET_D"},
    {0x5245,"_ABS_MOVFROP1"},{0x5249,"_ABS_MOV10B"},
    {0x524D,"_MM_LDIR_SET_SIZE"},
    {0x5251,"_ABS_MOV10TOOP2_noset"},
    {0x5255,"_load_imathptr1"},{0x5259,"_LOAD_IMATHPTR2"},
    {0x525D,"_LOAD_IMATHPTR3"},{0x5261,"_LOAD_IMATHPTR4"},
    {0x5265,"_LOAD_IMATHPTR5"},{0x5269,"_set_imathptr1"},
    {0x526D,"_set_imathptr3"},{0x5271,"_SET_IMATHPTR4"},
    {0x5275,"_SET_IMATHPTR5"},{0x5279,"_BLOCK0"},
    {0x527D,"_BLOCK0LP"},{0x5281,"_SET_CHKDELPTR1"},
    {0x5285,"_SET_ABS_DEST_ADDR"},{0x5289,"_NCR"},
    {0x528D,"_NPR"},{0x5291,"_ZMINT"},{0x5295,"_RNDRANGE"},
    {0x5299,"_ZMFIT"},{0x529D,"_STEPCK"},
    {0x52A1,"_GETRECT"},{0x52A5,"_FINDT1"},
    {0x52A9,"_STPICTOK"},{0x52AD,"_STPICGRPH"},
    {0x52B1,"_RCPICTOK"},{0x52B5,"_RCPICGRPH"},
    {0x52B9,"_RCfieldpic"},{0x52BD,"_STODBTOK"},
    {0x52C1,"_STODBGRPH"},{0x52C5,"_RCLDBTOK"},
    {0x52C9,"_RCLDBGRPH"},{0x52CD,"_VARFIND"},
    {0x52D1,"_ALLEQ"},{0x52D5,"_FNDALLSELEQ"},
    {0x52D9,"_FNDALLEQ"},{0x52DD,"_FNDSELEQ"},
    {0x52E1,"_initfpeq"},{0x52E5,"_NEXTEQ"},
    {0x52E9,"_PREVEQ"},{0x52ED,"_mm_lDIR"},
    {0x52F1,"_mm_lDDR"},{0x52F5,"_ZMSTATS"},
    {0x52F9,"_P_HISTPLOT"},{0x52FD,"_P_XYLINPLOT"},
    {0x5301,"_P_SCATPLOT"},{0x5305,"_P_BOXPLOT"},
    {0x5309,"_P_MBOXPLOT"},{0x530D,"_DRAWSPLOT"},
    {0x5311,"_INITNEWTRACEP"},{0x5315,"_SPLOTCOORD"},
    {0x5319,"_splotright"},{0x531D,"_SPLOTLEFT"},
    {0x5321,"_BOX_AT_MIN"},{0x5325,"_BOX_AT_MAX"},
    {0x5329,"_CMPBOXINFO"},{0x532D,"_nextplot"},
    {0x5331,"_NEXTPLOT1"},{0x5335,"_PREVPLOT"},
    {0x5339,"_CLRPREVPLOT"},{0x533D,"_put_index_lst"},
    {0x5341,"_GET_INDEX_LST"},{0x5345,"_HEAP_SORT"},
    {0x5349,"_TWOVARSET"},{0x534D,"_ONEVAR"},
    {0x5351,"_ExCmdStat"},{0x5355,"_SET_IMATHPTR2"},
    {0x5359,"_FCSTX"},{0x535D,"_FCSTY"},
    {0x5361,"_ORDSTAT"},{0x5365,"_INIT_SORT_LIST_AD"},
    {0x5369,"_rcl_statvar"},{0x536D,"_GETSTATPTR"},
    {0x5371,"_GetKy"},{0x5375,"_SEND1BERR"},
    {0x5379,"_SENDACK"},{0x537D,"_PRINTSCREEN"},
    {0x5381,"_KEYSCNLNK"},{0x5385,"_IORESETALL"},
    {0x5389,"_JLinkApp"},{0x538C,"_GETVARCMD"},
    {0x5390,"_GETVARCMD1"},{0x5394,"_SENDVARCMD"},
    {0x5398,"_low_bat_disp"},{0x539B,"_msgcommon"},
    {0x539F,"_JvarsApp"},{0x53A2,"_delvartok"},
    {0x53A6,"_bufLeft"},{0x53AA,"_bufRight"},
    {0x53AE,"_bufInsert"},{0x53B2,"_bufQueueChar"},
    {0x53B6,"_bufQueueString"},{0x53BA,"_bufReplace"},
    {0x53BE,"_bufDelete"},{0x53C2,"_bufPeek"},
    {0x53C6,"_bufToBtm"},{0x53CA,"_setupEditEqu"},
    {0x53CE,"_setupEditEquPtr"},{0x53D2,"_bufToTop"},
    {0x53D6,"_isEditFull"},{0x53DA,"_isEditEmpty"},
    {0x53DE,"_isAtTop"},{0x53E2,"_isAtBtm"},
    {0x53E6,"_bufClear"},{0x53EA,"_JcursorFirst"},
    {0x53ED,"_JcursorLast"},{0x53F0,"_cursorLeft"},
    {0x53F4,"_JcursorLeft"},{0x53F7,"_JcursorRight"},
    {0x53FA,"_cursorRight"},{0x53FE,"_cursorUp"},
    {0x5402,"_JcursorUp"},{0x5405,"_cursorDown"},
    {0x5409,"_JcursorDown"},{0x540C,"_cursorToOffset"},
    {0x5410,"_insertString"},{0x5414,"_needPreBlank"},
    {0x5418,"_condBlank"},{0x541C,"_insDisp"},
    {0x5420,"_JinsDisp"},{0x5423,"_JdispEOW"},
    {0x5426,"_dispEOL"},{0x542A,"_dispHead"},
    {0x542E,"_dispTail"},{0x5432,"_setupEditCmd"},
    {0x5436,"_SET_EDITTOP"},{0x543A,"_setEmptyEditEqu"},
    {0x543E,"_setEmptyEditPtr"},{0x5442,"_SET_EDITBTM"},
    {0x5446,"_setEditDetok"},{0x544A,"_closeEditEqu"},
    {0x544E,"_toggleIns"},{0x5452,"_JtoggleIns"},
    {0x5455,"_JerrorApp"},{0x5458,"_assertDec"},
    {0x545C,"_assertEnough"},{0x5460,"_dispErrNum"},
    {0x5464,"_MOV10OP2ADD"},{0x5468,"_INTGR"},
    {0x546C,"_MOV10TO2SUB"},{0x5470,"_MINUS1"},
    {0x5474,"_FPSUB"},{0x5478,"_FPADD"},{0x547C,"_DTOR"},
    {0x5480,"_RTOD"},{0x5484,"_TIMESPT5"},
    {0x5488,"_FPSQUARE"},{0x548C,"_FPMULT"},
    {0x5490,"_INVOP1SC"},{0x5494,"_INVOP1S"},
    {0x5498,"_INVOP2S"},{0x549C,"_FRAC"},
    {0x54A0,"_MLTDIV0"},{0x54A4,"_FPRECIP"},
    {0x54A8,"_FPDIV"},{0x54AC,"_SQROOT"},
    {0x54B0,"_SQROOTP"},{0x54B4,"_CRNDGUARD"},
    {0x54B8,"_CROUND"},{0x54BC,"_RNDGUARD"},
    {0x54C0,"_ROUND"},{0x54C4,"_LNX"},{0x54C8,"_LNXP"},
    {0x54CC,"_LOGXP"},{0x54D0,"_LOGX"},{0x54D4,"_ETOX"},
    {0x54D8,"_TENX"},{0x54DC,"_SINCOSRAD"},
    {0x54E0,"_SIN"},{0x54E4,"_COS"},{0x54E8,"_TAN"},
    {0x54EC,"_SINHCOSH"},{0x54F0,"_TANH"},{0x54F4,"_COSH"},
    {0x54F8,"_SINH"},{0x54FC,"_ACOSRAD"},
    {0x5500,"_ATAN2RAD"},{0x5504,"_ASINRAD"},
    {0x5508,"_ACOS"},{0x550C,"_ACOSP"},{0x5510,"_ATAN"},
    {0x5514,"_ASIN"},{0x5518,"_ATAN2"},{0x551C,"_ATANH"},
    {0x5520,"_OnePlusMinusX"},{0x5524,"_ASINH"},
    {0x5528,"_ACOSH"},{0x552C,"_PTOR"},{0x5530,"_RTOP"},
    {0x5534,"_XROOTY"},{0x5538,"_YTOX"},
    {0x553C,"_RANDNRM"},{0x5540,"_RANDBIN"},
    {0x5544,"_RANDINT"},{0x5548,"_gammalog"},
    {0x554C,"_gamma_fact"},{0x5550,"_cmdApp"},
    {0x5554,"_JcmdApp"},{0x5557,"_cmdInit"},
    {0x555B,"_cmdReset"},{0x555F,"_TOKREGEQ"},
    {0x5563,"_TOKX"},{0x5567,"_writeb_inc_ahl"},
    {0x556B,"_TOKNUM"},{0x556F,"_DREQUTOK"},
    {0x5573,"_CONVDIM"},{0x5577,"_CONVOP1"},
    {0x557B,"_EXMODE"},{0x557F,"_exgstyletok"},
    {0x5583,"_DOPLOTTOK"},{0x5587,"_DO_EXTEXT"},
    {0x558B,"_graph_explr"},{0x558F,"_Axes"},
    {0x5593,"_CEILING"},{0x5597,"_exstregtok"},
    {0x559B,"_STOLDIM"},{0x559F,"_STODIM"},
    {0x55A3,"_exasmcomp"},{0x55A7,"_JRamApp"},
    {0x55AA,"_getkey"},{0x55AE,"_DECTOBASE"},
    {0x55B2,"_BASETODEC"},{0x55B6,"_EXBINMATH1"},
    {0x55BA,"_FAND"},{0x55BE,"_FOR"},{0x55C2,"_FXOR"},
    {0x55C6,"_FNOT"},{0x55CA,"_ROTATER"},
    {0x55CE,"_SHIFTR"},{0x55D2,"_ROTATEL"},
    {0x55D6,"_SHIFTL"},{0x55DA,"_RANDOM"},
    {0x55DE,"_STORAND"},{0x55E2,"_RANDINIT"},
    {0x55E6,"_JstatEDITApp"},{0x55E9,"_dispLstCol"},
    {0x55ED,"_DELLSTNAME"},{0x55F1,"_LSTNAMEVWHANDLEKEY"},
    {0x55F5,"_hiCurLstField"},{0x55F9,"_DspLstTitle"},
    {0x55FD,"_dsplsts"},{0x5601,"_dspLstLabelM"},
    {0x5605,"_DspModDec"},{0x5609,"_clrcur"},
    {0x560C,"_DSP_LST_1_LINE"},{0x5610,"_SetUpDsp"},
    {0x5614,"_closeEditBuf"},{0x5618,"_DelAnyLst"},
    {0x561C,"_MatPutAway"},{0x5620,"_StatPutAway"},
    {0x5624,"_lstPutAway"},{0x5628,"_resetstuff"},
    {0x562C,"_resetfps"},{0x5630,"_parseEditBuf"},
    {0x5634,"_EDITLST"},{0x5638,"_JTableEditApp"},
    {0x563B,"_DSPCURTBL"},{0x563F,"_TBDECODA"},
    {0x5643,"_VPUTBLANK"},{0x5647,"_clearUserMenu"},
    {0x564B,"_setUserMenuItem"},{0x564F,"_promptUserMenu"},
    {0x5653,"_resetUserMenu"},{0x5657,"_StatSetApp"},
    {0x565A,"_StatItemName"},{0x565E,"_JTableSetApp"},
    {0x5661,"_UpSetMenu"},
    {0x5664,"_regraphstatifnecessary"},
    {0x5668,"_DownSPSetMenu"},{0x566B,"_StatPlotInfoApp"},
    {0x566E,"_statplottypedisp"},
    {0x5672,"_statplotsetputaway"},
    {0x5676,"_statplotinit"},{0x567A,"_Showfield02"},
    {0x567D,"_MatVwHandleKey"},{0x5681,"_dspmattableall"},
    {0x5685,"_dspmattable2"},{0x5689,"_dspmattable"},
    {0x568D,"_MatSizeWind"},{0x5691,"_setEditCurmatfield"},
    {0x5695,"_matredisp"},{0x5699,"_CONVERT"},
    {0x569D,"_copycharmapl"},{0x56A1,"_copycharmaps"},
    {0x56A5,"_JmatEditApp"},{0x56A8,"_EditMVName"},
    {0x56AC,"_matError"},{0x56B0,"_rowedhandlekey"},
    {0x56B4,"_coledhandlekey"},{0x56B8,"_JsolverApp"},
    {0x56BB,"_clearbal_asap"},{0x56BF,"_deleteLftRt"},
    {0x56C3,"_JvcEditApp"},{0x56C6,"_JequEdApp"},
    {0x56C9,"_dispequ"},{0x56CD,"_openEquField"},
    {0x56D1,"_closeEquField"},{0x56D5,"_AutoSelect"},
    {0x56D9,"_EQSELUNSEL"},{0x56DD,"_setStyleCode"},
    {0x56E1,"_asap_to_ram"},{0x56E4,"_load_asap_jump"},
    {0x56E7,"_newcontext_asap"},{0x56EA,"_on_script"},
    {0x56ED,"_off_script"},{0x56F0,"_asap_grphexpr"},
    {0x56F3,"_chk_alt_parser"},{0x56F6,"_chk_alt_execlop"},
    {0x56F9,"_chk_alt_formdisp"},
    {0x56FC,"_chk_alt_cmd_tok"},{0x56FF,"_chk_alt_home"},
    {0x5702,"_chk_alt_getkey"},{0x5705,"_chk_alt_regraph"},
    {0x5708,"_EOSTOK_ENTRY"},{0x570B,"_EXEC_ASAP_CMD"},
    {0x570E,"_asap_cmdtok"},{0x5711,"_ASAP_FUNC_EXEC"},
    {0x5714,"_exec_pg3"},{0x5718,"_exec_pg4"},
    {0x571C,"_exec_io"},{0x5720,"_unopexec1"},
    {0x5724,"_mode_exec"},{0x5728,"_slot_number"},
    {0x572C,"_menu_offset"},{0x5730,"_exec_assembly"},
    {0x5732,"_errsignchange"},{0x5735,"_aclosefield"},
    {0x5738,"_adispnumvars"},{0x573B,"_adispvarval"},
    {0x573E,"_adispval"},{0x5741,"_aseteditcurrent"},
    {0x5744,"_anumhandlekey"},{0x5747,"_aredispfield"},
    {0x574A,"_anumputaway"},{0x574D,"_anumsizewind"},
    {0x5750,"_ainitvars"},{0x5753,"_alt_parse_end"},
    {0x5756,"_newtoken"},{0x5759,"_cmdret"},
    {0x575C,"_load_asap"},{0x5760,"_amode_exec"},
    {0xC000,"_kbdScanCode"},{0xC001,"_kbdLGSC"},
    {0xC002,"_kbdPSC"},{0xC003,"_kbdWUR"},
    {0xC004,"_kbdDebncCnt"},{0xC005,"_kbdkey"},
    {0xC006,"_kbdGetKy"},{0xC007,"_keyextend"},
    {0xC008,"_contrast"},{0xC009,"_APDSubTimer"},
    {0xC00A,"_APDTimer"},{0xC00B,"_APDWarmUp"},
    {0xC00C,"_viet"},{0xC00E,"_curTime"},
    {0xC00F,"_curRow"},{0xC010,"_curCol"},
    {0xC011,"_curUnder"},{0xC012,"_undelBufLen"},
    {0xC013,"_undelBuf"},{0xC077,"_P_tokVarPtr"},
    {0xC07A,"_toklen"},{0xC07C,"_TOK_B3"},
    {0xC07D,"_DETOK_H3"},{0xC07E,"_MEMPRE_H3"},
    {0xC07F,"_indicMem"},{0xC087,"_indicCounter"},
    {0xC088,"_indicBusy"},{0xC089,"_OP1"},
    {0xC08A,"_OP1EXPM"},{0xC08B,"_OP1EXPL"},
    {0xC08C,"_OP1M"},{0xC093,"_OP1EXT"},{0xC094,"_OP2"},
    {0xC095,"_OP2EXPM"},{0xC096,"_OP2EXPL"},
    {0xC097,"_OP2M"},{0xC09E,"_OP2EXT"},{0xC09F,"_OP3"},
    {0xC0A0,"_OP3EXPM"},{0xC0A1,"_OP3EXPL"},
    {0xC0A2,"_OP3M"},{0xC0A9,"_OP3EXT"},{0xC0AA,"_OP4"},
    {0xC0AB,"_OP4EXPM"},{0xC0AC,"_OP4EXPL"},
    {0xC0AD,"_OP4M"},{0xC0B4,"_OP4EXT"},{0xC0B5,"_OP5"},
    {0xC0B6,"_OP5EXPM"},{0xC0B7,"_OP5EXPL"},
    {0xC0B8,"_OP5M"},{0xC0B9,"_EITS"},{0xC0BA,"_ENM2"},
    {0xC0BB,"_ENA"},{0xC0BC,"_EEN"},{0xC0BF,"_OP5EXT"},
    {0xC0C0,"_OP6"},{0xC0C1,"_OP6EXPM"},
    {0xC0C2,"_OP6EXPL"},{0xC0C3,"_OP6M"},{0xC0C5,"_ELOW"},
    {0xC0C6,"_EIGH"},{0xC0CA,"_OP6EXT"},{0xC0CC,"_OP7"},
    {0xC0D7,"_IOFLAG"},{0xC0D8,"_P_IMATHPTR1"},
    {0xC0DB,"_P_IMATHPTR2"},{0xC0DE,"_P_IMATHPTR3"},
    {0xC0E1,"_P_IMATHPTR4"},{0xC0E4,"_P_IMATHPTR5"},
    {0xC0E7,"_P_CHKDELPTR1"},{0xC0EA,"_P_CHKDELPTR2"},
    {0xC0ED,"_P_INSDELPTR"},{0xC0F0,"_P_UPDOWNPTR"},
    {0xC0F3,"_STDRNGSGN"},{0xC0F4,"_POLRNGSGN"},
    {0xC0F5,"_PARRNGSGN"},{0xC0F6,"_DIFRNDSGN"},
    {0xC0F7,"_USRRNGSGN"},{0xC0F8,"_STATSGN"},
    {0xC0F9,"_textShadow"},{0xC1A1,"_textShadCur"},
    {0xC1A3,"_textShadTop"},{0xC1A4,"_textShadAlph"},
    {0xC1A5,"_textShadIns"},{0xC1A6,"_textAccent"},
    {0xC1A7,"_cxMain"},{0xC1A9,"_cxPPutAway"},
    {0xC1AB,"_cxPutAway"},{0xC1AD,"_cxRedisp"},
    {0xC1AF,"_cxErrorEP"},{0xC1B1,"_cxSizeWind"},
    {0xC1B3,"_cxPage"},{0xC1B4,"_CXCURAPP"},
    {0xC1B5,"_cxPrev"},{0xC1C4,"_monQH"},{0xC1C5,"_monQT"},
    {0xC1C6,"_monQueue"},{0xC1D6,"_onSP"},
    {0xC1D8,"_onCheckSum"},{0xC1DA,"_promptRow"},
    {0xC1DB,"_promptCol"},{0xC1DC,"_promptIns"},
    {0xC1DD,"_promptShift"},{0xC1DE,"_promptRet"},
    {0xC1E0,"_promptValid"},{0xC1E2,"_P_promptTop"},
    {0xC1E5,"_P_promptCursor"},{0xC1E8,"_P_promptTail"},
    {0xC1EB,"_P_promptBtm"},{0xC1EE,"_varType"},
    {0xC1EF,"_varCurrent"},{0xC1F8,"_varFAFlags"},
    {0xC1FA,"_varClass"},{0xC1FB,"_catCurrent"},
    {0xC1FD,"_menuActive"},{0xC1FE,"_menu2Hilite"},
    {0xC1FF,"_menuSingle"},{0xC201,"_menuAppStack"},
    {0xC20D,"_menuAppPtr"},{0xC20F,"_menuAppDepth"},
    {0xC210,"_menuSysStack"},{0xC21C,"_menuSysPtr"},
    {0xC21E,"_menuSysDepth"},{0xC21F,"_menuPrvStack"},
    {0xC22B,"_menuPrvPtr"},{0xC22D,"_menuPrvDepth"},
    {0xC22E,"_m2i"},{0xC242,"_menuDyn1"},
    {0xC26A,"_menuDyn5"},{0xC274,"_userMenu1"},
    {0xC275,"_userMenuTitle"},{0xC27C,"_userMenu2"},
    {0xC284,"_userMenu3"},{0xC28C,"_userMenu4"},
    {0xC294,"_userMenu5"},{0xC29C,"_userMenuSA"},
    {0xC31C,"_XSTATSAV"},{0xC324,"_ioPrompt"},
    {0xC326,"_YSTATSAV"},{0xC330,"_FSTATSAV"},
    {0xC33A,"_IOSNDTYP"},{0xC33B,"_SNDRECSTATE"},
    {0xC33C,"_IOERRSTATE"},{0xC33D,"_HEADER"},
    {0xC346,"_IODATA"},{0xC352,"_BAKHEADER"},
    {0xC35B,"_TBLRNGSGN"},{0xC35C,"_calc_id"},
    {0xC37C,"_penCol"},{0xC37D,"_penRow"},
    {0xC37E,"_P_RCLQUEUE"},{0xC381,"_ERRNO"},
    {0xC382,"_ERRSP"},{0xC384,"_errOffset"},
    {0xC386,"_ram_to_use"},{0xC390,"_offerr_sav_bc"},
    {0xC392,"_ABS_SRC_ADDR"},{0xC395,"_ABS_DEST_ADDR"},
    {0xC398,"_MM_NUM_BYTES"},{0xC39B,"_mm_tmp1"},
    {0xC39D,"_mm_tmp2"},{0xC39F,"_mm_tmp3"},
    {0xC3A1,"_mm_tmp4"},{0xC3A3,"_mm_tmp5"},
    {0xC3A5,"_ram_cache"},{0xC3E5,"_Flags"},
    {0xC40A,"_ram_to_use1"},{0xC414,"_statReg"},
    {0xC415,"_STATVARS"},{0xC555,"_curgstyle"},
    {0xC556,"_curGY"},{0xC557,"_curGX"},
    {0xC558,"_curGY2"},{0xC559,"_curGX2"},
    {0xC55A,"_curgstyle_save"},{0xC55B,"_curgstylesave"},
    {0xC55C,"_plotflagsave"},{0xC55D,"_XMINPTR"},
    {0xC55F,"_XMAXPTR"},{0xC561,"_XSCLPTR"},
    {0xC563,"_YMINPTR"},{0xC565,"_YMAXPTR"},
    {0xC567,"_YSCLPTR"},{0xC569,"_DIF1STCURINC"},
    {0xC56B,"_TRACEPLOT"},{0xC56C,"_BOXPLOTINFO"},
    {0xC56D,"_SCURINC"},{0xC56F,"_CURINC"},
    {0xC571,"_YPIXEL"},{0xC572,"_ORGXMIN"},
    {0xC57C,"_PANSHIFT"},{0xC586,"_USRRNGSIZE"},
    {0xC588,"_UTHETMIN"},{0xC58D,"_STRAMStart"},
    {0xC592,"_UTHETMAX"},{0xC59C,"_UTHETSTEP"},
    {0xC5A6,"_UTPLOT"},{0xC5B0,"_UTMIN"},{0xC5BA,"_UTMAX"},
    {0xC5C4,"_UTSTEP"},{0xC5CE,"_UXMIN"},
    {0xC5D8,"_UXMAX"},{0xC5E2,"_UXSCL"},{0xC5EC,"_UYMIN"},
    {0xC5F6,"_UYMAX"},{0xC600,"_UYSCL"},{0xC60A,"_UXRES"},
    {0xC614,"_XRES_INT"},{0xC615,"_HDERIV"},
    {0xC61F,"_TOL"},{0xC629,"_XFACT"},{0xC633,"_YFACT"},
    {0xC63D,"_DELTAX"},{0xC647,"_DELTAY"},
    {0xC651,"_SHORTX"},{0xC65B,"_SHORTY"},
    {0xC665,"_FUNRNGSIZE"},{0xC667,"_FLAGSF"},
    {0xC668,"_XMINF"},{0xC672,"_XMAXF"},{0xC67C,"_XSCLF"},
    {0xC686,"_YMINF"},{0xC690,"_YMAXF"},{0xC69A,"_YSCLF"},
    {0xC6A4,"_LOWER"},{0xC6AE,"_UPPER"},{0xC6B8,"_XRES"},
    {0xC6C2,"_POLRNGSIZE"},{0xC6C4,"_FLAGSPOL"},
    {0xC6C5,"_THETAMIN"},{0xC6CF,"_THETAMAX"},
    {0xC6D9,"_THETASTEP"},{0xC6E3,"_XMINPOL"},
    {0xC6ED,"_XMAXPOL"},{0xC6F7,"_XSCLPOL"},
    {0xC701,"_YMINPOL"},{0xC70B,"_YMAXPOL"},
    {0xC715,"_YSCLPOL"},{0xC71F,"_PARRNGSIZE"},
    {0xC721,"_FLAGSPAR"},{0xC722,"_TMINPAR"},
    {0xC72C,"_TMAXPAR"},{0xC736,"_TSTEPPAR"},
    {0xC740,"_XMINPAR"},{0xC74A,"_XMAXPAR"},
    {0xC754,"_XSCLPAR"},{0xC75E,"_YMINPAR"},
    {0xC768,"_YMAXPAR"},{0xC772,"_YSCLPAR"},
    {0xC77C,"_DIFRNGSIZE"},{0xC77E,"_FLAGSDIF"},
    {0xC77F,"_TOLERDIF"},{0xC789,"_TPLOTDIF"},
    {0xC793,"_TMINDIF"},{0xC79D,"_TMAXDIF"},
    {0xC7A7,"_TSTEPDIF"},{0xC7B1,"_XMINDIF"},
    {0xC7BB,"_XMAXDIF"},{0xC7C5,"_XSCLDIF"},
    {0xC7CF,"_YMINDIF"},{0xC7D9,"_YMAXDIF"},
    {0xC7E3,"_YSCLDIF"},{0xC7ED,"_XAXISDIF"},
    {0xC7EE,"_YAXISDIF"},{0xC7EF,"_SLOPEF_EQU"},
    {0xC7F0,"_DIRF_X"},{0xC7F1,"_DIRF_Y"},
    {0xC7F2,"_DIRF_TIME"},{0xC7FC,"_FRES"},
    {0xC806,"_INTS"},{0xC810,"_DNEQ"},
    {0xC811,"_P_XOUTSYM"},{0xC814,"_P_XOUTDAT"},
    {0xC817,"_P_YOUTSYM"},{0xC81A,"_P_YOUTDAT"},
    {0xC81D,"_P_INPUTSYM"},{0xC820,"_P_INPUTDAT"},
    {0xC823,"_P_FOUTDAT"},{0xC826,"_PREVDATA"},
    {0xC862,"_PREVDATA_EXT"},{0xC86C,"_P1TYPE"},
    {0xC86D,"_SavX1List"},{0xC876,"_SavY1List"},
    {0xC87F,"_SavF1List"},{0xC888,"_P1FRQONOFF"},
    {0xC889,"_P2TYPE"},{0xC88A,"_SavX2List"},
    {0xC893,"_SavY2List"},{0xC89C,"_SavF2List"},
    {0xC8A5,"_P2FRQONOFF"},{0xC8A6,"_P3TYPE"},
    {0xC8A7,"_SavX3List"},{0xC8B0,"_SavY3List"},
    {0xC8B9,"_SavF3List"},{0xC8C2,"_P3FRQONOFF"},
    {0xC8C3,"_oldtype"},{0xC8C4,"_oldxlist"},
    {0xC8CD,"_oldylist"},{0xC8D6,"_oldflist"},
    {0xC8DF,"_oldonoff"},{0xC8E0,"_tblpsrow"},
    {0xC8E1,"_tblscroll"},{0xC8E3,"_INPUTDAT_PG0"},
    {0xC8ED,"_TblLine"},{0xC8F7,"_OldTblMin"},
    {0xC901,"_TBLRNGSIZE"},{0xC903,"_TblMin"},
    {0xC90D,"_TblStep"},{0xC917,"_TABLESGN"},
    {0xC918,"_TableYPtr"},{0xC919,"_curTblcol"},
    {0xC91A,"_curTblrow"},{0xC91B,"_dspTblcol"},
    {0xC91C,"_dspTblrow"},{0xC91D,"_higTblcol"},
    {0xC91E,"_higTblrow"},{0xC920,"_TABLEXDATA"},
    {0xC95C,"_TABLEYDATA"},{0xC9D4,"_TABLETEMPLATE"},
    {0xC9D5,"_SavedEqTok"},{0xC9D7,"_SavedEqNum1"},
    {0xC9D8,"_SavedEqTok1"},{0xC9DA,"_SaveAppFlags"},
    {0xC9DB,"_SaveCurFlags"},{0xC9DC,"_SaveCurGstyle"},
    {0xC9DD,"_SaveGraphFlags"},{0xC9DE,"_evalflevel"},
    {0xC9DF,"_TmpMatCols"},{0xC9E0,"_TmpMatRows"},
    {0xC9E1,"_P_DERIVPTR"},{0xC9E4,"_DTMPThresh"},
    {0xC9E6,"_ELCPLXLCNT"},{0xC9E8,"_DERIVLEVEL"},
    {0xC9E9,"_P_DIFFEQPTR"},{0xC9EB,"_P_DSOLVPTR"},
    {0xC9EE,"_SOLVAR"},{0xC9F7,"_P_QUADPTR"},
    {0xC9FA,"_plotSScreen"},{0xCDFA,"_SEED1"},
    {0xCE04,"_SEED2"},{0xCE0E,"_PARSEVAR"},
    {0xCE18,"_P_BEGPC"},{0xCE1B,"_P_CURPC"},
    {0xCE1E,"_P_ENDPC"},{0xCE21,"_ELCNT"},
    {0xCE23,"_COLCNT"},{0xCE24,"_ROWCNT"},
    {0xCE25,"_LCOUNT"},{0xCE27,"_EOS_ASAP_2ND"},
    {0xCE28,"_EXEC_CONV_SAVE"},{0xCE2A,"_LASTENTRYPTR"},
    {0xCE2C,"_LASTENTRYSTK"},{0xCEAC,"_numlastentries"},
    {0xCEAD,"_currlastentry"},{0xCEAE,"_FREESAVEY"},
    {0xCEAF,"_FREESAVEX"},{0xCEB0,"_STRACESAVE_TYPE"},
    {0xCEB1,"_STRACESAVE"},{0xCEB3,"_TRACESAVE"},
    {0xCEB5,"_DIF_T_SAVE"},{0xCEBF,"_A_B_SAVE"},
    {0xCEC0,"_A_B_TYPE"},{0xCEC1,"_GS_DELX"},
    {0xCEC2,"_GS_D1_YINC"},{0xCEC3,"_GS_D2_YINC"},
    {0xCEC4,"_GS_DELY"},{0xCEC5,"_GS_MAX_Y_PIX"},
    {0xCEC6,"_CURRENT_STYLE"},{0xCEC7,"_CL_X1"},
    {0xCEC8,"_CL_X2"},{0xCEC9,"_CL_Y_DAT"},
    {0xCECB,"_PREV_POINT"},{0xCECD,"_RESSAVE"},
    {0xCECE,"_DREQU_X"},{0xCECF,"_DREQU_XINIT"},
    {0xCED9,"_DREQU_Y"},{0xCEDA,"_DREQU_YINIT"},
    {0xCEE4,"_DREQU_XLIST"},{0xCEE7,"_DREQU_YLIST"},
    {0xCEEA,"_DREQU_tLIST"},{0xCEED,"_DREQU_COUNT"},
    {0xCEEF,"_GY1"},{0xCF21,"_GX1"},{0xCF53,"_GR1"},
    {0xCF85,"_GQ1"},{0xCF8A,"_EQU_EDIT_SAVE"},
    {0xCF8B,"_FORMULA_BITMAP"},{0xCFAB,"_MENUCMD_M2I"},
    {0xCFC9,"_MENUCMD_ITEMS"},{0xD041,"_MENUCMD_NUMROWS"},
    {0xD042,"_MENUCMD_CURROW"},{0xD053,"_cmdShadCur"},
    {0xD055,"_cmdShadAlph"},{0xD056,"_cmdShadIns"},
    {0xD057,"_cmdCursor"},{0xD059,"_P_editTop"},
    {0xD05C,"_P_EDITCURSOR"},{0xD05F,"_P_editTail"},
    {0xD062,"_P_editBtm"},{0xD065,"_curmatcol"},
    {0xD066,"_curmatrow"},{0xD067,"_curlstrow"},
    {0xD069,"_curlistel"},{0xD06A,"_curlstrowh"},
    {0xD06B,"_higmatcol"},{0xD06C,"_higmatrow"},
    {0xD06D,"_higlstrow"},{0xD06F,"_maxdsprow"},
    {0xD070,"_ForCurMat"},{0xD072,"_ForDspCol"},
    {0xD074,"_forerrornum"},{0xD075,"_P_editSym"},
    {0xD078,"_P_editDat"},{0xD07B,"_DspMatCol"},
    {0xD07C,"_DspMatRow"},{0xD07D,"_TmpMatCol"},
    {0xD07E,"_TmpMatRow"},{0xD07F,"_numoflist"},
    {0xD080,"_num1stlist"},{0xD081,"_NumCurList"},
    {0xD082,"_STATED_CUT_COL"},{0xD083,"_listnamebuffer"},
    {0xD12E,"_LastName"},{0xD137,"_modeRoot"},
    {0xD139,"_modeCount"},{0xD13A,"_modeItem"},
    {0xD13B,"_modePtr"},{0xD13D,"_winTop"},
    {0xD13E,"_winBtm"},{0xD13F,"_winLeftEdge"},
    {0xD140,"_winLeft"},{0xD142,"_winAbove"},
    {0xD144,"_winRow"},{0xD146,"_winCol"},
    {0xD148,"_fmtDigits"},{0xD149,"_fmtString"},
    {0xD18A,"_fmtConv"},{0xD19E,"_fmtLeft"},
    {0xD1A0,"_fmtIndex"},{0xD1A2,"_P_fmtMatSym"},
    {0xD1A5,"_P_fmtMatMem"},{0xD1A8,"_EQS"},
    {0xD1AA,"_LSTINDEX"},{0xD1AC,"_LSTSIZE"},
    {0xD1AE,"_EQUINDEX"},{0xD1B0,"_order"},
    {0xD1B1,"_xnamesav"},{0xD1BA,"_ynamesav"},
    {0xD1C3,"_CustMType"},{0xD1C4,"_CustMLen"},
    {0xD1C5,"_CustMSav"},{0xD1E3,"_custmnames"},
    {0xD279,"_VARSAVECNT"},{0xD27A,"_DELADJAMT"},
    {0xD27D,"_TEMPINPUT"},{0xD27E,"_TSYMPTR1"},
    {0xD280,"_TSYMPTR2"},{0xD282,"_P_CHKDELPTR3"},
    {0xD285,"_P_CHKDELPTR4"},{0xD288,"_P_TEMPMEM"},
    {0xD28B,"_FPBASE"},{0xD28D,"_FPS"},{0xD28F,"_OPBASE"},
    {0xD291,"_OPS"},{0xD293,"_PTempCnt"},
    {0xD295,"_CLEANTMP"},{0xD297,"_P_PTEMP"},
    {0xD29A,"_PTEMP_END"},{0xD29D,"_FREE_MEM"},
    {0xD2A0,"_newdataptr"},{0xD2A3,"_SavBotRow"},
    {0xD2B8,"_curstatplot"},{0xD2B9,"_curstatplotprompt"},
    {0xD2BA,"_difeqfieldmode"},{0xD2BB,"_matedoldtype"},
    {0xD2BC,"_modesave1"},{0xD2BD,"_statansfirst"},
    {0xD2BF,"_statanslast"},{0xD2C1,"_statanscur"},
    {0xD2C3,"_charmap"},{0xD2CB,"_altcharmap"},
    {0xD2D3,"_toktmp1"},{0xD2D4,"_toktmp2"},
    {0xD2D5,"_IOSAVOP1"},{0xD2DF,"_DELVAR_SAV_F"},
    {0xD2E0,"_DEL_SAV_OP1"},{0xD2EB,"_alt_asm_exec_btm"},
    {0xD2ED,"_altlfontptr"},{0xD2F0,"_altsfontptr"},
    {0xD2F3,"_altonptr"},{0xD2F6,"_altslinkptr"},
    {0xD2F9,"_alt_ret_status"},
    {0xD2FA,"_alt_ret_jmp_page"},
    {0xD2FB,"_alt_ret_jmp_addr"},
    {0xD2FD,"_alt_int_chksum"},
    {0xD2FE,"_alt_interrupt_exec"},
    {0xD3C6,"_alt_slink_chksum"},
    {0xD3C7,"_alt_slink_exec"},{0xD48F,"_alt_on_chksum"},
    {0xD490,"_alt_on_exec"},{0xD558,"_alt_off_chksum"},
    {0xD559,"_alt_off_exec"},{0xD621,"_asm_exec_btm"},
    {0xD623,"_ASAP_IND"},{0xD624,"_asm_reg_af"},
    {0xD625,"_asm_reg_a"},{0xD626,"_asm_reg_hl"},
    {0xD627,"_asm_reg_h"},{0xD628,"_asm_reg_bc"},
    {0xD629,"_asm_reg_b"},{0xD62A,"_asm_reg_de"},
    {0xD62B,"_asm_reg_d"},{0xD62C,"_mPrgmMATH"},
    {0xD64C,"_mMath"},{0xD65A,"_mMath_asap1"},
    {0xD65C,"_mMath_asap2"},{0xD65E,"_mMath_asap3"},
    {0xD66C,"_iASAP1"},{0xD678,"_iASAP2"},
    {0xD684,"_iASAP3"},{0xD690,"_iASAP4"},
    {0xD69C,"_iASAP5"},{0xD6A8,"_iASAP6"},
    {0xD6B4,"_iASAP7"},{0xD6C0,"_iASAP8"},
    {0xD6CC,"_iASAP9"},{0xD6D8,"_asap_nl1"},
    {0xD6E1,"_asap_nl2"},{0xD6EA,"_asap_nl3"},
    {0xD6FC,"_asapvar"},{0xD706,"_tokspell_asap1"},
    {0xD708,"_tokspell_asap2"},{0xD70A,"_tokspell_asap3"},
    {0xD70E,"_numtok_asap1"},{0xD70F,"_numtok_asap2"},
    {0xD710,"_numtok_asap3"},{0xD712,"_eostbl_asap1"},
    {0xD714,"_eostbl_asap2"},{0xD716,"_eostbl_asap3"},
    {0xD71A,"_Amenu_offset"},{0xD722,"_reinstall_asap1"},
    {0xD724,"_reinstall_asap2"},
    {0xD726,"_reinstall_asap3"},{0xD72A,"_asap1_ram"},
    {0xD734,"_asap2_ram"},{0xD73E,"_asap3_ram"},
    {0xD748,"_asm_exec_ram"},{0,NULL}};

TI86::TI86()
{
    romFuncs=romFuncs86;
    asm_readmem=asm_86_readmem;
    asm_readmem_word=asm_86_readmem_word;
    asm_readmem_dword=asm_86_readmem_dword;
    asm_writemem=asm_86_writemem;
    asm_writemem_word=asm_86_writemem_word;
    asm_writemem_dword=asm_86_writemem_dword;
}

void TI86::EnableDebug()
{
    asm_readmem=debug_readmem;
    asm_readmem_word=debug_readmem_word;
    asm_readmem_dword=debug_readmem_dword;
    asm_writemem=debug_writemem;
    asm_writemem_word=debug_writemem_word;
    asm_writemem_dword=debug_writemem_dword;
}

void TI86::DisableDebug()
{
    asm_readmem=asm_86_readmem;
    asm_readmem_word=asm_86_readmem_word;
    asm_readmem_dword=asm_86_readmem_dword;
    asm_writemem=asm_86_writemem;
    asm_writemem_word=asm_86_writemem_word;
    asm_writemem_dword=asm_86_writemem_dword;
}

int* TI86::GetKeyMap()
{
    return keymap86;
}

int TI86::GetKeyMat(int r,int c)
{
    return ti86kmat[r][c];
}

int TI86::GetKeyAlphaMat(int r,int c)
{
    return ti86alphakmat[r][c];
}

void TI86::Reset()
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
	Z80_Reset();
    R.PC.D=initialPC;
	contrast=16;
	kbmask=0xff;
    for (i=0;i<7;i++) kbmatrix[i]=0xff;
	onheld=1; lcdBase=0xfc00;
    for (i=0;i<6;i++) oldScreenBase[i]=lcdBase;
	lcdRefresh=0;
    cpuCompleteStop=0; unstop=0; lcdOff=0;
    pageA=rom; pageAType=0; pageANum=0;
    pageB=rom; pageBType=0; pageBNum=0;
    port3Left=100000;
    redWire=1; whiteWire=1;
    outRed=0; outWhite=0;
    intr=0; getError=0;
    for (int i=0;romFuncs[i].name;i++)
        romFuncAddr[i]=romFuncs[i].num;
}

int TI86::getmem(int addr)
{
	if (addr<0x4000)
		return (rom[addr&0x3fff])&0xff;
	else if (addr<0x8000)
		return (pageA[addr&0x3fff])&0xff;
	else if (addr<0xc000)
		return (pageB[addr&0x3fff])&0xff;
	else if (addr<0x10000)
		return (ram[addr&0x3fff])&0xff;
    else if (addr<0x2c000)
        return (ram[addr-0xc000])&0xff;
    else
        return 0;
}

int TI86::getmem_word(int addr)
{
	return ((getmem(addr+1)<<8)|getmem(addr))&0xffff;
}

int TI86::getmem_dword(int addr)
{
	return (getmem(addr+3)<<24)|
	       (getmem(addr+2)<<16)|
	       (getmem(addr+1)<<8)|
	       getmem(addr);
}

void TI86::setmem(int addr,int v)
{
    addr&=0xffffff;
	if ((addr>=0x4000)&&(addr<0x8000)&&(pageAType))
		pageA[addr&0x3fff]=v;
    else if ((addr>=0x8000)&&(addr<0xc000)&&(pageBType))
		pageB[addr&0x3fff]=v;
    else if ((addr>=0xc000)&&(addr<0x10000))
		ram[addr&0x3fff]=v;
    else if (addr<0x2c000)
        ram[addr-0xc000]=v;
}

void TI86::setmem_word(int addr,int v)
{
	setmem(addr+1,(v>>8)&0xff);
	setmem(addr,v&0xff);
}

void TI86::setmem_dword(int addr,int v)
{
	setmem(addr+3,(v>>24)&0xff);
	setmem(addr+2,(v>>16)&0xff);
	setmem(addr+1,(v>>8)&0xff);
	setmem(addr,v&0xff);
}

int TI86::readmem(int addr)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,1,DATABREAK_READ,0))
            DataBreak();
    }
	if ((word)addr<0x4000)
		return (rom[addr&0x3fff])&0xff;
	else if ((word)addr<0x8000)
		return (pageA[addr&0x3fff])&0xff;
	else if ((word)addr<0xc000)
		return (pageB[addr&0x3fff])&0xff;
	else
		return (ram[addr&0x3fff])&0xff;
}

int TI86::readmem_word(int addr)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,2,DATABREAK_READ,0))
            DataBreak();
    }
    addr&=0xffff;
	return ((getmem(addr+1)<<8)|getmem(addr))&0xffff;
}

int TI86::readmem_dword(int addr)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,4,DATABREAK_READ,0))
            DataBreak();
    }
    addr&=0xffff;
	return (getmem(addr+3)<<24)|
	       (getmem(addr+2)<<16)|
	       (getmem(addr+1)<<8)|
	       getmem(addr);
}

void TI86::writemem(int addr,int v)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,1,DATABREAK_WRITE,v))
            DataBreak();
    }
    addr&=0xffffff;
	if (((word)addr>=0x4000)&&((word)addr<0x8000)&&(pageAType))
		pageA[addr&0x3fff]=v;
    else if (((word)addr>=0x8000)&&((word)addr<0xc000)&&(pageBType))
		pageB[addr&0x3fff]=v;
    else if ((word)addr>=0xc000)
		ram[addr&0x3fff]=v;
}

void TI86::writemem_word(int addr,int v)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,2,DATABREAK_WRITE,v))
            DataBreak();
    }
    addr&=0xffff;
	setmem(addr+1,(v>>8)&0xff);
	setmem(addr,v&0xff);
}

void TI86::writemem_dword(int addr,int v)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,4,DATABREAK_WRITE,v))
            DataBreak();
    }
    addr&=0xffff;
	setmem(addr+3,(v>>24)&0xff);
	setmem(addr+2,(v>>16)&0xff);
	setmem(addr+1,(v>>8)&0xff);
	setmem(addr,v&0xff);
}

int TI86::readport(int port)
{
    int i,v;
    static int onpressed=0;
    static recvInProgress=0;
    if (CheckAddr((port&0xff)|0x600000,1,DATABREAK_READ,0)) DataBreak();
    switch(port&7)
    {
        case 1:
            for (v=0xff,i=0;i<7;i++)
			{
			    if (!(kbmask&(1<<i)))
			   	    v&=kbmatrix[i];
            }
			return v;
        case 3:
            if (port3Left)
            {
                port3Left--;
                return 1;
            }
            if (unstop)
                return (R.IFF1?0:2)|1;
            if ((!onpressed)&&(!onheld))
            {
                onpressed=1;
                return (onheld<<3)|(R.IFF1?0:2)|1;
            }
            if ((onpressed)&&(onheld))
                onpressed=0;
            return (onheld<<3)|(R.IFF1?0:2);
        case 4:
            return 1;
        case 5:
            return (pageAType<<6)|pageANum;
        case 6:
            return (pageBType<<6)|pageBNum;
        case 7:
            return (outWhite<<3)|(outRed<<2)|
                ((whiteWire&(1-outWhite))<<1)|
                (redWire&(1-outRed));
    }
    return 0;
}

void TI86::writeport(int port,int v)
{
    if (CheckAddr((port&0xff)|0x600000,1,DATABREAK_WRITE,v)) DataBreak();
    v&=0xff;
    switch(port&7)
    {
        case 0:
            lcdBase=0xc000+((v&0x3f)<<8);
            break;
        case 1:
            kbmask=v;
            break;
        case 2:
            contrast=v&31;
            break;
        case 3:
            if (port3Left) return;
            lcdOff=1-((v>>3)&1);
            cpuCompleteStop=lcdOff&(v&1);
            if (!lcdOff)
                unstop=0;
            break;
        case 4:
            break;
        case 5:
            pageAType=(v>>6)&1;
            pageANum=v&(pageAType?0x7:0xf);
            pageA=pageAType?(&ram[pageANum<<14]):
                (&rom[pageANum<<14]);
            break;
        case 6:
            pageBType=(v>>6)&1;
            pageBNum=v&(pageBType?0x7:0xf);
            pageB=pageBType?(&ram[pageBNum<<14]):
                (&rom[pageBNum<<14]);
            break;
        case 7:
            outRed=(v>>2)&1;
            outWhite=(v>>3)&1;
            if ((!sendInProgress)&&(!emuLink)&&
                ((outRed)||(outWhite))&&(!soundEnable))
                ReceiveFile();
            if ((!sendInProgress)&&(emuLink)&&
                ((outRed)||(outWhite))&&(!soundEnable))
            {
                sendInProgress=1;
                int v;
                if (GetByte(v))
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
                sendInProgress=0;
            }
            break;
    }
}

void TI86::Execute()
{
    if (!run) return;
    UpdateDebugCheckEnable();
    intr+=375;
    if (intr>=33333)//100000)
    {
        RefreshLCD();
        intr=0;
        if (cpuCompleteStop) return;
        Z80_Cause_Interrupt(0);
    }
    if (cpuCompleteStop) return;
    static int leftOver=0;
	leftOver=Z80_Execute(375-leftOver)-(375-leftOver);
}

void TI86::OneInstruction()
{
    UpdateDebugCheckEnable();
    Z80_Execute(1);
}

void TI86::OnUp()
{
    onheld=1;
}

void TI86::OnDown()
{
    if (!onheld) return;
    onheld=0;
    if (cpuCompleteStop)
    {
        cpuCompleteStop=0;
        unstop=1;
    }
}

void TI86::KeyUp(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]|=1<<c;
}

void TI86::KeyDown(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]&=~(1<<c);
}

void TI86::AlphaUp()
{
    KeyUp(5,7);
}

void TI86::AlphaDown()
{
    KeyDown(5,7);
}

void TI86::KBInt()
{
    for (int i=0;i<200;i++)
        Execute();
}

int TI86::GetKeyID(int i)
{
    return key[i];
}

void TI86::SaveState(char *fn)
{
    FILE *fp=fopen(fn,"wb");
    fprintf(fp,"VTIv2.0 ");
    fwrite(romImage[currentROM].name,56,1,fp);
    fwrite(&R,sizeof(Z80_Regs),1,fp);
    fwrite(ram,128,1024,fp);
    fwrite(&kbmask,4,1,fp);
    fwrite(&pageAType,4,1,fp); fwrite(&pageANum,4,1,fp);
    fwrite(&pageBType,4,1,fp); fwrite(&pageBNum,4,1,fp);
    fwrite(&port3Left,4,1,fp); fwrite(&intr,4,1,fp);
    fwrite(&redWire,4,1,fp); fwrite(&whiteWire,4,1,fp);
    fwrite(&outRed,4,1,fp); fwrite(&outWhite,4,1,fp);
    fwrite(&lcdBase,4,1,fp); fwrite(&lcdOff,4,1,fp);
    fwrite(&contrast,4,1,fp); fwrite(&unstop,4,1,fp);
    fwrite(&cpuCompleteStop,4,1,fp);
    fclose(fp);
}

void TI86::LoadState(char *fn)
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
    fread(&R,sizeof(Z80_Regs),1,fp);
    fread(ram,128,1024,fp);
    fread(&kbmask,4,1,fp);
    fread(&pageAType,4,1,fp); fread(&pageANum,4,1,fp);
    pageA=pageAType?(&ram[pageANum<<14]):(&rom[pageANum<<14]);
    fread(&pageBType,4,1,fp); fread(&pageBNum,4,1,fp);
    pageB=pageBType?(&ram[pageBNum<<14]):(&rom[pageBNum<<14]);
    fread(&port3Left,4,1,fp); fread(&intr,4,1,fp);
    fread(&redWire,4,1,fp); fread(&whiteWire,4,1,fp);
    fread(&outRed,4,1,fp); fread(&outWhite,4,1,fp);
    fread(&lcdBase,4,1,fp); fread(&lcdOff,4,1,fp);
    fread(&contrast,4,1,fp); fread(&unstop,4,1,fp);
    fread(&cpuCompleteStop,4,1,fp);
    fclose(fp);
}

int TI86::SendByte(int c)
{
    int bit=1;
    int left;
    for (int i=0;i<8;i++,bit<<=1)
    {
        if (c&bit)
        {
            redWire=1;
            whiteWire=0;
            left=200000;
            while (((redWire&(1-outRed))||(whiteWire&
                (1-outWhite)))&&(left--))
            {
                Z80_Execute(1);
                intr++;
                if (intr>=100000)
                {
                    RefreshLCD();
                    intr=0;
                    Z80_Cause_Interrupt(0);
                }
            }
            if (!left) return 0;
            redWire=1;
            whiteWire=1;
            while (!((redWire&(1-outRed))&&
                (whiteWire&(1-outWhite)))&&(left--))
            {
                Z80_Execute(1);
                intr++;
                if (intr>=100000)
                {
                    RefreshLCD();
                    intr=0;
                    Z80_Cause_Interrupt(0);
                }
            }
            if (!left) return 0;
        }
        else
        {
            redWire=0;
            whiteWire=1;
            left=200000;
            while (((redWire&(1-outRed))||(whiteWire&
                (1-outWhite)))&&(--left))
            {
                Z80_Execute(1);
                intr++;
                if (intr>=100000)
                {
                    RefreshLCD();
                    intr=0;
                    Z80_Cause_Interrupt(0);
                }
            }
            if (!left) return 0;
            redWire=1;
            whiteWire=1;
            left=200000;
            while ((!((redWire&(1-outRed))&&
                (whiteWire&(1-outWhite))))&&(--left))
            {
                Z80_Execute(1);
                intr++;
                if (intr>=100000)
                {
                    RefreshLCD();
                    intr=0;
                    Z80_Cause_Interrupt(0);
                }
            }
            if (!left) return 0;
        }
    }
    return 1;
}

int TI86::GetByte(int &c)
{
    int left=6000000;
    c=0;
    for (int i=0;i<8;i++)
    {
        while ((redWire&(1-outRed))&&(whiteWire&(1-outWhite))&&
            (--left))
        {
            Z80_Execute(1);
            intr++;
            if (intr>=100000)
            {
                RefreshLCD();
                intr=0;
                Z80_Cause_Interrupt(0);
            }
        }
        if (!left) { getError=1; return 0; }
        c|=outWhite<<i;
        whiteWire=outWhite; redWire=outRed;
        left=200000;
        while ((outWhite||outRed)&&(--left))
        {
            Z80_Execute(1);
            intr++;
            if (intr>=100000)
            {
                RefreshLCD();
                intr=0;
                Z80_Cause_Interrupt(0);
            }
        }
        if (!left) { getError=1; return 0; }
        whiteWire=1; redWire=1;
        left=200000;
    }
    return 1;
}

int TI86::SendReady()
{
    return 1;
}

int TI86::SendBuffer(int c)
{
    return SendByte(c);
}

int TI86::CheckBuffer()
{
    if ((redWire&(1-outRed))&&(whiteWire&(1-outWhite)))
        return 0;
    return 1;
}

int TI86::GetBuffer(int &c)
{
    return GetByte(c);
}

#define WaitForAck() \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; }

int TI86::SendFile(char *fn,int more)
{
    int i,hdr,csum,c,len,type,nameLen;
    char name[8];

    sendInProgress=1;
    FILE *fp=fopen(fn,"rb");
    fseek(fp,0x37,SEEK_SET);
    while (!feof(fp))
    {
        hdr=0; fread(&hdr,2,1,fp);
        if ((hdr==0)||(hdr==0xffff)) break;
        len=0; fread(&len,2,1,fp);
        if ((len==0)||(len==0xffff)) break;
        type=0; fread(&type,1,1,fp);
        nameLen=0; fread(&nameLen,1,1,fp);
        for (i=0;i<8;i++) name[i]=0;
        fread(name,hdr-4,1,fp);
        fseek(fp,2,SEEK_CUR);
        whiteWire=1; redWire=1; KBInt();
        SendByte(0x6); SendByte(0xc9);
        SendByte(12); SendByte(0);
        SendByte(len&0xff); SendByte(len>>8);
        SendByte(type); SendByte(nameLen);
        csum=(len&0xff)+(len>>8)+type+nameLen;
        for (i=0;i<8;i++)
        {
            c=(int)((unsigned char)name[i]);
            if (c==0) break;
            SendByte(c);
            csum+=c;
        }
        for (;i<8;i++)
        {
            SendByte(' ');
            csum+=' ';
        }
        SendByte(csum&0xff); SendByte((csum>>8)&0xff);
        WaitForAck(); WaitForAck();
        whiteWire=1; redWire=1; KBInt();
        SendByte(0x6); SendByte(0x56); SendByte(0); SendByte(0);
        SendByte(0x6); SendByte(0x15);
        SendByte(len&0xff); SendByte(len>>8);
        csum=0;
        for (i=0;i<len;i++)
        {
            c=fgetc(fp);
            if (!SendByte(c)) { fclose(fp); sendInProgress=0; return 0; }
            csum+=c;
        }
        SendByte(csum&0xff); SendByte((csum>>8)&0xff);
        WaitForAck();
        whiteWire=1; redWire=1;
        for (i=0;i<100000;i++)
            Execute();
    }
    fclose(fp);
    sendInProgress=0;
    return 1;
}

int TI86::ReceiveFile()
{
    int i,j,c,len,size,type,id,csum,bufLen,n;
    int curOfs,group;
    char name[9],*buf,*newBuf;

    for (i=0;i<7;i++) kbmatrix[i]=0xff;
    sendInProgress=1;
    getError=0;
    GetByte(c); id=c;
    GetByte(c); if (c!=6) { sendInProgress=0; return 0; }
    if (getError) { sendInProgress=0; getError=0; return 0; }
    Screen->Cursor=crHourGlass;
    buf=NULL; bufLen=0; n=0;
    while (1)
    {
        GetByte(c); len=c;
        GetByte(c); len|=c<<8;
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        GetByte(c); size=c;
        GetByte(c); size|=c<<8;
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        GetByte(c); type=c;
        GetByte(c); j=c;
        if (j>8) { Screen->Cursor=crDefault; sendInProgress=0; return 0; }
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        memset(name,0,8);
        for (i=0;i<j;i++)
        {
            GetByte(c);
            name[i]=c;
        }
        name[j]=0;
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        whiteWire=1; redWire=1; KBInt();
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
            return 0;
        }
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        newBuf=new char[bufLen+size+17];
        if (buf)
        {
            memcpy(newBuf,buf,bufLen);
            delete[] buf;
        }
        buf=newBuf;
        curOfs=bufLen+8+strlen(name);
        buf[bufLen]=4+strlen(name); buf[bufLen+1]=0;
        buf[bufLen+2]=size&0xff; buf[bufLen+3]=size>>8;
        buf[bufLen+4]=type; buf[bufLen+5]=strlen(name);
        for (i=0;i<strlen(name);i++)
            buf[bufLen+6+i]=name[i];
        buf[bufLen+6+i]=size&0xff; buf[bufLen+7+i]=size>>8;
        for (i=0;i<size;i++)
        {
            GetByte(c);
            buf[curOfs+i]=c;
            if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        }
        bufLen+=size+8+strlen(name);
        GetByte(c); GetByte(c);
        whiteWire=1; redWire=1; KBInt();
        SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        if (!transferAsGroup)
        {
            TSaveDialog *dlog=new TSaveDialog(EmuWnd);
            dlog->DefaultExt=defExt86[type&0x1f];
            dlog->FileName=AnsiString(name)+AnsiString(".")+
                dlog->DefaultExt;
            dlog->FilterIndex=4;
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
                csum=0;
                for (i=0;i<bufLen;i++)
                    csum+=buf[i]&0xff;
                buf[bufLen]=csum&0xff;
                buf[bufLen+1]=csum>>8;
                bufLen+=2;
                FILE *fp=fopen(dlog->FileName.c_str(),"wb");
                fprintf(fp,"**TI86**");
                fputc(0x1a,fp); fputc(0xa,fp);
                for (i=0;i<0x2b;i++)
                    fputc(0,fp);
                fputc((bufLen-2)&0xff,fp);
                fputc((bufLen-2)>>8,fp);
                for (i=0;i<bufLen;i++)
                    fputc(buf[i],fp);
                fclose(fp);
            }
            Screen->Cursor=crHourGlass;
            delete dlog;
            delete[] buf; buf=NULL; bufLen=0;
        }
        n++;
        if (c!=6) break;
    }
    GetByte(c); GetByte(c);
    whiteWire=1; redWire=1; KBInt();
    SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
    getError=0;
    Screen->Cursor=crDefault;
    if (transferAsGroup)
    {
        TSaveDialog *dlog=new TSaveDialog(EmuWnd);
        if (n==1)
        {
            dlog->DefaultExt=defExt86[type&0x1f];
            group=0;
        }
        else
        {
            dlog->DefaultExt="86g";
            strcpy(name,"group");
            group=1;
        }
        dlog->FileName=AnsiString(name)+AnsiString(".")+
            dlog->DefaultExt;
        dlog->FilterIndex=4;
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
            csum=0;
            for (i=0;i<bufLen;i++)
                csum+=buf[i]&0xff;
            buf[bufLen]=csum&0xff;
            buf[bufLen+1]=csum>>8;
            bufLen+=2;
            FILE *fp=fopen(dlog->FileName.c_str(),"wb");
            fprintf(fp,"**TI86**");
            fputc(0x1a,fp); fputc(0xa,fp);
            for (i=0;i<0x2b;i++)
                fputc(0,fp);
            fputc((bufLen-2)&0xff,fp);
            fputc((bufLen-2)>>8,fp);
            for (i=0;i<bufLen;i++)
                fputc(buf[i],fp);
            fclose(fp);
        }
        delete dlog;
    }
    sendInProgress=0;
    if (buf) delete[] buf;
    return 1;
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
