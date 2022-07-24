//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#include <dir.h>
#include <stdio.h>
#include <string.h>
#pragma hdrstop

#include "calchw.h"
#include "rom.h"
#define NOTYPEREDEF
#include "m68000.h"
#include "Debug.h"
#include "Log.h"
#include "Emu.h"

static int keymap89[]={VK_ESCAPE,k_esc,'1',k_1,'2',k_2,
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
    k_f8,VK_F9,k_apps,VK_PRIOR,k_power,VK_SCROLL,
    k_on,VK_NUMPAD7,k_7,VK_NUMPAD8,k_8,VK_NUMPAD9,k_9,
    VK_SUBTRACT,k_minus,VK_NUMPAD4,k_4,VK_NUMPAD5,k_5,
    VK_NUMPAD6,k_6,VK_ADD,k_plus,VK_NUMPAD1,k_1,
    VK_NUMPAD2,k_2,VK_NUMPAD3,k_3,VK_NUMPAD0,k_0,
    VK_DECIMAL,k_dot,VK_SEPARATOR,k_enter2,VK_DIVIDE,
    k_div,VK_HOME,k_cos,VK_UP,k_up,
    VK_LEFT,k_left,VK_RIGHT,k_right,VK_DOWN,k_down,
    VK_INSERT,k_sin,VK_DELETE,k_clear,0xba,
    k_theta,-1,-1};

static int ti89kmat[][8]=
    {k_hand,k_dia,k_shift,k_2nd,k_right,k_down,k_left,k_up,
     k_f5,k_clear,k_power,k_div,k_mult,k_minus,k_plus,k_enter1,
     k_f4,k_bs,k_t,k_comma,k_9,k_6,k_3,k_neg,
     k_f3,k_f6,k_z,k_rparan,k_8,k_5,k_2,k_dot,
     k_f2,k_mode,k_y,k_lparan,k_7,k_4,k_1,k_0,
     k_f1,k_cos,k_x,k_equ,k_ln,k_sin,k_sto,k_apps,
     kn,kn,kn,kn,kn,kn,kn,k_esc,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn};

static int ti89alphakmat[][8]=
    {kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,k_e,k_j,k_o,k_u,kn,
     kn,kn,kn,k_d,k_i,k_n,k_s,k_space,
     kn,kn,kn,k_c,k_h,k_m,k_r,k_w,
     kn,kn,kn,k_b,k_g,k_l,k_q,k_v,
     kn,kn,kn,k_a,k_f,k_k,k_p,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn};

static int key[]=
    {k_f1,k_f2,k_f3,k_f4,k_f5,
     k_2nd,k_shift,k_esc,k_left,k_right,
     k_up,k_down,k_dia,k_hand,k_apps,
     k_cos,k_mode,k_f6,k_bs,k_clear,
     k_x,k_y,k_z,k_t,k_power,
     k_equ,k_lparan,k_rparan,k_comma,k_div,
     k_ln,k_7,k_8,k_9,k_mult,
     k_sin,k_4,k_5,k_6,k_minus,
     k_sto,k_1,k_2,k_3,k_plus,
     k_on,k_0,k_dot,k_neg,k_enter1};

char* defExt89[0x20]={"89e","89z","89z","89z","89l","89z",
    "89m","89z","89z","89z","89c","89t","89s","89d",
    "89z","89z","89i","89z","89p","89f","89z","89z",
    "89z","89z","89z","89z","89z","89z","89z","89z",
    "89z","89z"};

ROMFunc romFuncs89[]=
   {{0,"FirstWindow"},{1,"WinActivate"},{2,"WinAttr"},
    {3,"WinBackupToScr"},{4,"WinBackground"},
    {5,"WinBegin"},{6,"WinBitmapGet"},{7,"WinBitmapPut"},
    {8,"WinBitmapSize"},{9,"WinCharXY"},{0xa,"WinChar"},
    {0xb,"WinClose"},{0xc,"WinClr"},{0xd,"WinDeactivate"},
    {0xe,"WinDupStat"},{0xf,"WinEllipse"},{0x10,"WinFill"},
    {0x11,"WinFillLines2"},{0x12,"WinFillTriangle"},
    {0x13,"WinFont"},{0x14,"WinGetCursor"},
    {0x15,"WinHide"},{0x16,"WinHome"},{0x17,"WinLine"},
    {0x18,"WinLineNC"},{0x19,"WinLineTo"},
    {0x1a,"WinLineRel"},{0x1b,"WinMoveCursor"},
    {0x1c,"WinMoveTo"},{0x1d,"WinMoveRel"},
    {0x1e,"WinOpen"},{0x1f,"WinPixGet"},{0x20,"WinPixSet"},
    {0x21,"WinRect"},{0x22,"WinReOpen"},
    {0x23,"WinScrollH"},{0x24,"WinScrollV"},
    {0x25,"WinStr"},{0x26,"WinStrXY"},
    {0x27,"DrawWinBorder"},{0x28,"ScrRectDivide"},
    {0x29,"RectWinToWin"},{0x2a,"RectWinToScr"},
    {0x2b,"UpdateWindows"},{0x2c,"MakeWinRect"},
    {0x2d,"ScrToWin"},{0x2e,"ScrToHome"},{0x2f,"ScrRect"},
    {0x30,"Dialog"},{0x31,"NoCallBack"},{0x32,"DialogDo"},
    {0x33,"DialogAdd"},{0x34,"DialogNew"},
    {0x35,"DrawStaticButton"},{0x36,"MenuBegin"},
    {0x37,"MenuCheck"},{0x38,"MenuEnd"},{0x39,"MenuKey"},
    {0x3a,"MenuOn"},{0x3b,"MenuPopup"},
    {0x3c,"MenuSubStat"},{0x3d,"MenuTopStat"},
    {0x3e,"MenuTopSelect"},{0x3f,"MenuTopRedef"},
    {0x40,"MenuGetTopRedef"},{0x41,"MenuAddText"},
    {0x42,"MenuAddIcon"},{0x43,"MenuNew"},
    {0x44,"PopupAddText"},{0x45,"PopupNew"},
    {0x46,"PopupClear"},{0x47,"PopupDo"},
    {0x48,"PopupText"},{0x49,"MenuUpdate"},
    {0x4a,"Parse2DExpr"},{0x4b,"Parse2DMultiExpr"},
    {0x4c,"Print2DExpr"},{0x4d,"Parms2D"},
    {0x4e,"display_statements"},{0x4f,"Parse1DExpr"},
    {0x50,"pushkey"},{0x51,"ngetchx"},{0x52,"kbhit"},
    {0x53,"sprintf"},{0x54,"getcalc"},{0x55,"sendcalc"},
    {0x56,"LIO_Send"},{0x57,"LIO_Get"},
    {0x58,"LIO_Receive"},{0x59,"LIO_GetMultiple"},
    {0x5a,"LIO_SendData"},{0x5b,"LIO_RecvData"},
    {0x5c,"SymAdd"},{0x5d,"SymAddMain"},{0x5e,"SymDel"},
    {0x5f,"HSymDel"},{0x60,"SymFind"},{0x61,"SymFindMain"},
    {0x62,"SymFindHome"},{0x63,"SymMove"},
    {0x64,"FolderAdd"},{0x65,"FolderCur"},
    {0x66,"FolderDel"},{0x67,"FolderFind"},
    {0x68,"FolderGetCur"},{0x69,"FolderOp"},
    {0x6a,"FolderRename"},{0x6b,"FolderCount"},
    {0x6c,"SymFindFirst"},{0x6d,"SymFindNext"},
    {0x6e,"SymFindPrev"},{0x6f,"SymFindFoldername"},
    {0x70,"AddSymToFolder"},{0x71,"FindSymInFolder"},
    {0x72,"FolderCurTemp"},{0x73,"FolderAddTemp"},
    {0x74,"FolderDelTemp"},{0x75,"FolderDelAllTemp"},
    {0x76,"TempFolderName"},{0x77,"IsMainFolderStr"},
    {0x78,"ParseSymName"},{0x79,"DerefSym"},
    {0x7a,"HSYMtoName"},{0x7b,"StrToTokN"},
    {0x7c,"TokToStrN"},{0x7d,"CheckGraphRef"},
    {0x7e,"ClearUserDef"},{0x7f,"CheckLinkLockFlag"},
    {0x80,"TokenizeSymName"},{0x81,"SymCmp"},
    {0x82,"SymCpy"},{0x83,"SymCpy0"},
    {0x84,"ValidateSymName"},{0x85,"VarRecall"},
    {0x86,"VarStore"},{0x87,"VarStoreLink"},
    {0x88,"QSysProtected"},{0x89,"CheckSysFunc"},
    {0x8a,"GetSysGraphRef"},{0x8b,"CheckReservedName"},
    {0x8c,"SymSysVar"},{0x8d,"ValidateStore"},
    {0x8e,"ResetSymFlags"},{0x8f,"HeapAvail"},
    {0x90,"HeapAlloc"},{0x91,"HeapAllocESTACK"},
    {0x92,"HeapAllocHigh"},{0x93,"HeapAllocThrow"},
    {0x94,"HeapAllocHighThrow"},{0x95,"HeapCompress"},
    {0x96,"HeapDeref"},{0x97,"HeapFree"},
    {0x98,"HeapFreeIndir"},{0x99,"HLock"},
    {0x9a,"HeapLock"},{0x9b,"HeapGetLock"},
    {0x9c,"HeapMax"},{0x9d,"HeapRealloc"},
    {0x9e,"HeapSize"},{0x9f,"HeapUnlock"},
    {0xa0,"HeapMoveHigh"},{0xa1,"HeapEnd"},
    {0xa2,"HeapAllocPtr"},{0xa3,"HeapFreePtr"},
    {0xa4,"NeedStack"},{0xa5,"TE_close"},
    {0xa6,"TE_checkSlack"},{0xa7,"TE_empty"},
    {0xa8,"TE_focus"},{0xa9,"TE_handleEvent"},
    {0xaa,"TE_indicateReadOnly"},{0xab,"TE_isBlank"},
    {0xac,"TE_open"},{0xad,"TE_openFixed"},
    {0xae,"TE_pasteText"},{0xaf,"TE_reopen"},
    {0xb0,"TE_reopenPlain"},{0xb1,"TE_select"},
    {0xb2,"TE_shrinkWrap"},{0xb3,"TE_unfocus"},
    {0xb4,"TE_updateCommand"},{0xb5,"_bcd_math"},
    {0xb6,"bcdadd"},{0xb7,"bcdsub"},{0xb8,"bcdmul"},
    {0xb9,"bcddiv"},{0xba,"bcdneg"},{0xbb,"bcdcmp"},
    {0xbc,"bcdlong"},{0xbd,"bcdbcd"},{0xbe,"EX_getArg"},
    {0xbf,"EX_getBCD"},{0xc0,"EX_stoBCD"},
    {0xc1,"CB_replaceTEXT"},{0xc2,"CB_fetchTEXT"},
    {0xc3,"CU_restore"},{0xc4,"CU_start"},
    {0xc5,"CU_stop"},{0xc6,"EV_captureEvents"},
    {0xc7,"EV_clearPasteString"},{0xc8,"EV_getc"},
    {0xc9,"EV_getSplitRect"},{0xca,"EV_notifySwitchGraph"},
    {0xcb,"EV_paintOneWindow"},{0xcc,"EV_paintWindows"},
    {0xcd,"EV_restorePainting"},{0xce,"EV_sendEvent"},
    {0xcf,"EV_sendEventSide"},{0xd0,"EV_sendString"},
    {0xd1,"EV_setCmdCheck"},{0xd2,"EV_setCmdState"},
    {0xd3,"EV_setFKeyState"},{0xd4,"EV_startApp"},
    {0xd5,"EV_startSide"},{0xd6,"EV_startTask"},
    {0xd7,"EV_suspendPainting"},{0xd8,"EV_switch"},
    {0xd9,"MO_currentOptions"},{0xda,"MO_defaults"},
    {0xdb,"MO_digestOptions"},{0xdc,"MO_isMultigraphTask"},
    {0xdd,"MO_modeDialog"},{0xde,"MO_notifyModeChange"},
    {0xdf,"MO_sendQuit"},{0xe0,"ST_angle"},
    {0xe1,"ST_batt"},{0xe2,"ST_busy"},
    {0xe3,"ST_eraseHelp"},{0xe4,"ST_folder"},
    {0xe5,"ST_graph"},{0xe6,"ST_helpMsg"},
    {0xe7,"ST_modKey"},{0xe8,"ST_precision"},
    {0xe9,"ST_readOnly"},{0xea,"ST_stack"},
    {0xeb,"ST_refDsp"},{0xec,"OSCheckBreak"},
    {0xed,"OSClearBreak"},{0xee,"OSEnableBreak"},
    {0xef,"OSDisableBreak"},{0xf0,"OSRegisterTimer"},
    {0xf1,"OSFreeTimer"},{0xf2,"OSTimerCurVal"},
    {0xf3,"OSTimerExpired"},{0xf4,"OSTimerRestart"},
    {0xf5,"acos"},{0xf6,"asin"},{0xf7,"atan"},
    {0xf8,"atan2"},{0xf9,"cos"},{0xfa,"sin"},
    {0xfb,"tan"},{0xfc,"cosh"},{0xfd,"sinh"},
    {0xfe,"tanh"},{0xff,"exp"},{0x100,"log"},
    {0x101,"log10"},{0x102,"modf"},{0x103,"pow"},
    {0x104,"sqrt"},{0x105,"ceil"},{0x106,"fabs"},
    {0x107,"floor"},{0x108,"fmod"},{0x109,"top_estack"},
    {0x10a,"next_expression_index"},{0x10b,"gr_active"},
    {0x10c,"gr_other"},{0x10d,"ABT_dialog"},
    {0x10e,"HomeExecute"},{0x10f,"HomePushEStack"},
    {0x110,"SP_Define"},{0x111,"store_data_var"},
    {0x112,"recall_data_var"},{0x113,"CharNumber"},
    {0x114,"spike_optionD"},{0x115,"spike_geo_titles"},
    {0x116,"spike_in_editor"},
    {0x117,"dv_create_graph_titles"},
    {0x118,"spike_titles_in_editor"},
    {0x119,"dv_findColumn"},
    {0x11a,"spike_chk_gr_dirty"},{0x11b,"GetStatValue"},
    {0x11c,"partial_len"},{0x11d,"paint_all_except"},
    {0x11e,"EQU_select"},{0x11f,"EQU_setStyle"},
    {0x120,"EQU_getNameInfo"},{0x121,"checkCurrent"},
    {0x122,"BN_power17Mod"},{0x123,"BN_powerMod"},
    {0x124,"BN_prodMod"},{0x125,"CAT_dialog"},
    {0x126,"caddcert"},{0x127,"cdecrypt"},
    {0x128,"ceof"},{0x129,"cfindcertfield"},
    {0x12a,"cfindfield"},{0x12b,"cgetc"},
    {0x12c,"cgetcert"},{0x12d,"cgetflen"},
    {0x12e,"cgetfnl"},{0x12f,"cgetnl"},{0x130,"cgetns"},
    {0x131,"cgetvernum"},{0x132,"copen"},
    {0x133,"copensub"},{0x134,"cputhdr"},
    {0x135,"cputnl"},{0x136,"cputns"},{0x137,"cread"},
    {0x138,"ctell"},{0x139,"cwrite"},{0x13a,"cacos"},
    {0x13b,"casin"},{0x13c,"catan"},{0x13d,"cacosh"},
    {0x13e,"casinh"},{0x13f,"catanh"},{0x140,"ccos"},
    {0x141,"csin"},{0x142,"ctan"},{0x143,"ccosh"},
    {0x144,"csinh"},{0x145,"ctanh"},{0x146,"csqrt"},
    {0x147,"cln"},{0x148,"clog10"},{0x149,"cexp"},
    {0x14a,"CustomBegin"},{0x14b,"CustomMenuItem"},
    {0x14c,"CustomEnd"},{0x14d,"ReallocExprStruct"},
    {0x14e,"SearchExprStruct"},{0x14f,"handleRclKey"},
    {0x150,"CustomFree"},{0x151,"ERD_dialog"},
    {0x152,"ERD_process"},{0x153,"ER_throwVar"},
    {0x154,"ER_catch"},{0x155,"ER_success"},
    {0x156,"EV_centralDispatcher"},
    {0x157,"EV_defaultHandler"},
    {0x158,"EV_eventLoop"},{0x159,"EV_registerMenu"},
    {0x15a,"EX_patch"},{0x15b,"EM_abandon"},
    {0x15c,"EM_blockErase"},{0x15d,"EM_blockVerifyErase"},
    {0x15e,"EM_delete"},{0x15f,"EM_findEmptySlot"},
    {0x160,"EM_GC"},{0x161,"EM_moveSymFromExtMem"},
    {0x162,"EM_moveSymToExtMem"},{0x163,"EM_open"},
    {0x164,"EM_put"},{0x165,"EM_survey"},
    {0x166,"EM_twinSymFromExtMem"},{0x167,"EM_write"},
    {0x168,"EM_writeToExtMem"},{0x169,"FL_addCert"},
    {0x16a,"FL_download"},
    {0x16b,"FL_getHardwareParmBlock"},
    {0x16c,"FL_getCert"},{0x16d,"FL_getVerNum"},
    {0x16e,"EQU_deStatus"},{0x16f,"cmpstri"},
    {0x170,"fix_loop_displacements"},{0x171,"FL_write"},
    {0x172,"fpisanint"},{0x173,"fpisodd"},
    {0x174,"round12"},{0x175,"round14"},
    {0x176,"GD_Circle"},{0x177,"GD_Line"},
    {0x178,"GD_HVLine"},{0x179,"GD_Pen"},
    {0x17a,"GD_Eraser"},{0x17b,"GD_Text"},
    {0x17c,"GD_Select"},{0x17d,"GD_Contour"},
    {0x17e,"GKeyIn"},{0x17f,"GKeyDown"},
    {0x180,"GKeyFlush"},{0x181,"HelpKeys"},
    {0x182,"QModeKey"},{0x183,"QSysKey"},
    {0x184,"WordInList"},{0x185,"BitmapGet"},
    {0x186,"BitmapInit"},{0x187,"BitmapPut"},
    {0x188,"BitmapSize"},{0x189,"ScrRectFill"},
    {0x18a,"ScrRectOverlap"},{0x18b,"ScrRectScroll"},
    {0x18c,"ScrRectShift"},{0x18d,"QScrRectOverlap"},
    {0x18e,"FontGetSys"},{0x18f,"FontSetSys"},
    {0x190,"FontCharWidth"},{0x191,"DrawClipChar"},
    {0x192,"DrawClipEllipse"},{0x193,"DrawClipLine"},
    {0x194,"DrawClipPix"},{0x195,"DrawClipRect"},
    {0x196,"DrawMultiLines"},{0x197,"DrawStrWidth"},
    {0x198,"FillTriangle"},{0x199,"FillLines2"},
    {0x19a,"SetCurAttr"},{0x19b,"SetCurClip"},
    {0x19c,"LineTo"},{0x19d,"MoveTo"},
    {0x19e,"ScreenClear"},{0x19f,"GetPix"},
    {0x1a0,"SaveScrState"},{0x1a1,"RestoreScrState"},
    {0x1a2,"PortSet"},{0x1a3,"PortRestore"},
    {0x1a4,"DrawChar"},{0x1a5,"DrawFkey"},
    {0x1a6,"DrawIcon"},{0x1a7,"DrawLine"},
    {0x1a8,"DrawPix"},{0x1a9,"DrawStr"},
    {0x1aa,"GM_Value"},{0x1ab,"GM_Intersect"},
    {0x1ac,"GM_Integrate"},{0x1ad,"GM_Inflection"},
    {0x1ae,"GM_TanLine"},{0x1af,"GM_Math1"},
    {0x1b0,"GM_Derivative"},{0x1b1,"GM_DistArc"},
    {0x1b2,"GM_Shade"},{0x1b3,"YCvtFtoWin"},
    {0x1b4,"DlgMessage"},{0x1b5,"SetGraphMode"},
    {0x1b6,"Regraph"},{0x1b7,"GrAxes"},
    {0x1b8,"gr_xres_pixel"},{0x1b9,"CptFuncX"},
    {0x1ba,"XCvtPtoF"},{0x1bb,"YCvtPtoF"},
    {0x1bc,"YCvtFtoP"},{0x1bd,"XCvtFtoP"},
    {0x1be,"GrLineFlt"},{0x1bf,"FuncLineFlt"},
    {0x1c0,"GrClipLine"},{0x1c1,"CptDeltax"},
    {0x1c2,"CptDeltay"},{0x1c3,"CkValidDelta"},
    {0x1c4,"GR_Pan"},{0x1c5,"FindFunc"},
    {0x1c6,"FindGrFunc"},{0x1c7,"grFuncName"},
    {0x1c8,"gr_initCondName"},{0x1c9,"CptIndep"},
    {0x1ca,"gr_CptIndepInc"},{0x1cb,"gr_del_locals"},
    {0x1cc,"gr_DelFolder"},{0x1cd,"gr_openFolder"},
    {0x1ce,"setup_more_graph_fun"},
    {0x1cf,"unlock_more_graph_fun"},
    {0x1d0,"execute_graph_func"},
    {0x1d1,"cpt_gr_fun"},{0x1d2,"cpt_gr_param"},
    {0x1d3,"cpt_gr_polar"},{0x1d4,"gr_execute_seq"},
    {0x1d5,"CountGrFunc"},{0x1d6,"FirstSeqPlot"},
    {0x1d7,"cleanup_seq_mem"},{0x1d8,"time_loop"},
    {0x1d9,"InitTimeSeq"},{0x1da,"seqWebInit"},
    {0x1db,"run_one_seq"},{0x1dc,"gr_seq_value"},
    {0x1dd,"StepCk"},{0x1de,"seqStepCk"},
    {0x1df,"rngLen"},{0x1e0,"gdb_len"},
    {0x1e1,"gdb_store"},{0x1e2,"gdb_recall"},
    {0x1e3,"gr_DispLabels"},{0x1e4,"GraphOrTableCmd"},
    {0x1e5,"ck_valid_float"},{0x1e6,"CreateEmptyList"},
    {0x1e7,"QSkipGraphErr"},{0x1e8,"gr_find_de_result"},
    {0x1e9,"InitDEAxesRng"},{0x1ea,"InitDEMem"},
    {0x1eb,"de_loop"},{0x1ec,"cleanup_de_mem"},
    {0x1ed,"gr_de_value"},{0x1ee,"gr_find_func_index"},
    {0x1ef,"CptLastIndepDE"},{0x1f0,"de_initRes"},
    {0x1f1,"gr_del_vars_in_folder"},
    {0x1f2,"gr_de_axes_lbl"},{0x1f3,"gr_execute_de"},
    {0x1f4,"gr_delete_fldpic"},{0x1f5,"gr_remove_fldpic"},
    {0x1f6,"gr_add_fldpic"},{0x1f7,"gr_stopic"},
    {0x1f8,"gr_find_el"},{0x1f9,"deStepCk"},
    {0x1fa,"gr_ck_solvergraph"},{0x1fb,"GR3_addContours"},
    {0x1fc,"GraphActivate"},{0x1fd,"GR3_freeDB"},
    {0x1fe,"GR3_handleEvent"},{0x1ff,"GR3_paint3d"},
    {0x200,"GR3_xyToWindow"},{0x201,"GS_PlotTrace"},
    {0x202,"GS_PlotAll"},{0x203,"PlotDel"},
    {0x204,"PlotPut"},{0x205,"PlotGet"},{0x206,"PlotInit"},
    {0x207,"PlotDup"},{0x208,"PlotSize"},
    {0x209,"PlotLookup"},{0x20a,"QActivePlots"},
    {0x20b,"QPlotActive"},{0x20c,"GT_BackupToScr"},
    {0x20d,"GT_CalcDepVals"},
    {0x20e,"GT_CenterGraphCursor"},
    {0x20f,"GT_CursorKey"},{0x210,"GT_DspFreeTraceCoords"},
    {0x211,"GT_DispTraceCoords"},{0x212,"GT_DspMsg"},
    {0x213,"GT_Error"},{0x214,"GT_Format"},
    {0x215,"GT_FreeTrace"},{0x216,"GT_IncXY"},
    {0x217,"GT_KeyIn"},{0x218,"GT_QFloatCursorsInRange"},
    {0x219,"GT_Regraph"},{0x21a,"GT_Regraph_if_neccy"},
    {0x21b,"GT_Open"},{0x21c,"GT_SaveAs"},
    {0x21d,"GT_SelFunc"},{0x21e,"GT_SetGraphRange"},
    {0x21f,"GT_SetCursorXY"},{0x220,"GT_ShowMarkers"},
    {0x221,"GT_Trace"},{0x222,"GT_ValidGraphRanges"},
    {0x223,"GT_WinBound"},{0x224,"GT_WinCursor"},
    {0x225,"GYcoord"},{0x226,"GXcoord"},
    {0x227,"round12_err"},{0x228,"GT_Set_Graph_Format"},
    {0x229,"GT_PrintCursor"},{0x22a,"GT_DE_Init_Conds"},
    {0x22b,"GZ_Box"},{0x22c,"GZ_Center"},
    {0x22d,"GZ_Decimal"},{0x22e,"GZ_Fit"},
    {0x22f,"GZ_InOut"},{0x230,"Integer"},
    {0x231,"GZ_Previous"},{0x232,"GZ_Recall"},
    {0x233,"GZ_SetFactors"},{0x234,"GZ_Square"},
    {0x235,"GZ_Standard"},{0x236,"GZ_Stat"},
    {0x237,"GZ_Store"},{0x238,"GZ_Trig"},
    {0x239,"HeapGetHandle"},{0x23a,"HeapPtrToHandle"},
    {0x23b,"FreeHandles"},{0x23c,"HS_chopFIFO"},
    {0x23d,"HS_countFIFO"},{0x23e,"HS_deleteFIFONode"},
    {0x23f,"HS_freeAll"},{0x240,"HS_freeFIFONode"},
    {0x241,"HS_getAns"},{0x242,"HS_getEntry"},
    {0x243,"HS_getFIFONode"},{0x244,"HS_popEStack"},
    {0x245,"HS_newFIFONode"},{0x246,"HS_pushFIFONode"},
    {0x247,"HtoESI"},{0x248,"OSInitKeyInitDelay"},
    {0x249,"OSInitBetweenKeyDelay"},
    {0x24a,"OSCheckSilentLink"},{0x24b,"OSLinkCmd"},
    {0x24c,"OSLinkReset"},{0x24d,"OSLinkOpen"},
    {0x24e,"OSLinkClose"},{0x24f,"OSReadLinkBlock"},
    {0x250,"OSWriteLinkBlock"},
    {0x251,"OSLinkTxQueueInquire"},
    {0x252,"OSLinkTxQueueActive"},
    {0x253,"LIO_SendProduct"},{0x254,"MD5Init"},
    {0x255,"MD5Update"},{0x256,"MD5Final"},
    {0x257,"MD5Done"},{0x258,"convert_to_TI_92"},
    {0x259,"gen_version"},{0x25a,"is_executable"},
    {0x25b,"NG_RPNToText"},{0x25c,"NG_approxESI"},
    {0x25d,"NG_execute"},{0x25e,"NG_graphESI"},
    {0x25f,"NG_rationalESI"},{0x260,"NG_tokenize"},
    {0x261,"NG_setup_graph_fun"},
    {0x262,"NG_cleanup_graph_fun"},
    {0x263,"push_END_TAG"},{0x264,"push_LIST_TAG"},
    {0x265,"tokenize_if_TI_92_or_text"},{0x266,"setjmp"},
    {0x267,"longjmp"},{0x268,"VarGraphRefBitsClear"},
    {0x269,"VarInit"},{0x26a,"memcpy"},{0x26b,"memmove"},
    {0x26c,"strcpy"},{0x26d,"strncpy"},{0x26e,"strcat"},
    {0x26f,"strncat"},{0x270,"memcmp"},{0x271,"strcmp"},
    {0x272,"strncmp"},{0x273,"memchr"},{0x274,"strchr"},
    {0x275,"strcspn"},{0x276,"strpbrk"},{0x277,"strrchr"},
    {0x278,"strspn"},{0x279,"strstr"},{0x27a,"strtok"},
    {0x27b,"_memset"},{0x27c,"memset"},{0x27d,"strerror"},
    {0x27e,"strlen"},{0x27f,"SymAddTwin"},
    {0x280,"SymDelTwin"},{0x281,"LoadSymFromFindHandle"},
    {0x282,"MakeHsym"},{0x283,"SymFindPtr"},
    {0x284,"OSVRegisterTimer"},{0x285,"OSVFreeTimer"},
    {0x286,"sincos"},{0x287,"asinh"},{0x288,"acosh"},
    {0x289,"atanh"},{0x28a,"itrig"},{0x28b,"trig"},
    {0x28c,"VarOpen"},{0x28d,"VarSaveAs"},
    {0x28e,"VarNew"},{0x28f,"VarCreateFolderPopup"},
    {0x290,"VarSaveTitle"},{0x291,"WinWidth"},
    {0x292,"WinHeight"},{0x293,"XR_stringPtr"},
    {0x294,"OSReset"},{0x295,"SumStoChkMem"},
    {0x296,"OSContrastUp"},{0x297,"OSContrastDn"},
    {0x298,"OSKeyScan"},{0x299,"OSGetStatKeys"},
    {0x29a,"off"},{0x29b,"idle"},{0x29c,"OSSetSR"},
    {0x29d,"AB_prodid"},{0x29e,"AB_prodname"},
    {0x29f,"AB_serno"},{0x2a0,"cgetcertrevno"},
    {0x2a1,"cgetsn"},{0x2a2,"de_rng_no_graph"},
    {0x2a3,"EV_hook"},{0x2a4,"_ds16u16"},
    {0x2a5,"_ms16u16"},{0x2a6,"_du16u16"},
    {0x2a7,"_mu16u16"},{0x2a8,"_ds32s32"},
    {0x2a9,"_ms32s32"},{0x2aa,"_du32u32"},
    {0x2ab,"_mu32u32"},{0x2ac,"assign_between"},
    {0x2ad,"did_push_var_val"},{0x2ae,"does_push_fetch"},
    {0x2af,"delete_list_element"},{0x2b0,"push_ans_entry"},
    {0x2b1,"index_after_match_endtag"},
    {0x2b2,"push_indir_name"},{0x2b3,"push_user_func"},
    {0x2b4,"store_func_def"},
    {0x2b5,"store_to_subscripted_element"},
    {0x2b6,"index_below_display_expression"},
    {0x2b7,"get_key_ptr"},{0x2b8,"get_list_indicies"},
    {0x2b9,"get_matrix_indicies"},
    {0x2ba,"init_list_indicies"},
    {0x2bb,"init_matrix_indicies"},
    {0x2bc,"push_float_qr_fact"},{0x2bd,"push_lu_fact"},
    {0x2be,"push_symbolic_qr_fact"},
    {0x2bf,"are_expressions_identical"},
    {0x2c0,"compare_expressions"},
    {0x2c1,"find_error_message"},
    {0x2c2,"check_estack_size"},{0x2c3,"delete_between"},
    {0x2c4,"deleted_between"},{0x2c5,"delete_expression"},
    {0x2c6,"deleted_expression"},{0x2c7,"estack_to_short"},
    {0x2c8,"estack_to_ushort"},{0x2c9,"factor_base_index"},
    {0x2ca,"factor_exponent_index"},{0x2cb,"GetValue"},
    {0x2cc,"im_index"},{0x2cd,"index_numeric_term"},
    {0x2ce,"index_of_lead_base_of_lead_term"},
    {0x2cf,"index_main_var"},{0x2d0,"is_advanced_tag"},
    {0x2d1,"is_antisymmetric"},{0x2d2,"is_complex_number"},
    {0x2d3,"is_complex0"},{0x2d4,"is_free_of_tag"},
    {0x2d5,"is_independent_of"},
    {0x2d6,"is_independent_of_de_seq_vars"},
    {0x2d7,"is_independent_of_tail"},
    {0x2d8,"is_independent_of_elements"},
    {0x2d9,"is_monomial"},{0x2da,"is_monomial_in_kernel"},
    {0x2db,"is_narrowly_independent_of"},
    {0x2dc,"is_symmetric"},
    {0x2dd,"is_tail_independent_of"},
    {0x2de,"lead_base_index"},
    {0x2df,"lead_exponent_index"},
    {0x2e0,"lead_factor_index"},{0x2e1,"lead_term_index"},
    {0x2e2,"main_gen_var_index"},
    {0x2e3,"map_unary_over_comparison"},
    {0x2e4,"min_quantum"},{0x2e5,"move_between_to_top"},
    {0x2e6,"moved_between_to_top"},
    {0x2e7,"numeric_factor_index"},{0x2e8,"push_between"},
    {0x2e9,"push_expr_quantum"},
    {0x2ea,"push_expr2_quantum"},
    {0x2eb,"push_next_arb_int"},
    {0x2ec,"push_next_arb_real"},
    {0x2ed,"push_next_internal_var"},
    {0x2ee,"push_quantum"},{0x2ef,"push_quantum_pair"},
    {0x2f0,"reductum_index"},
    {0x2f1,"remaining_factors_index"},
    {0x2f2,"re_index"},{0x2f3,"reset_estack_size"},
    {0x2f4,"reset_control_flags"},
    {0x2f5,"can_be_approxed"},
    {0x2f6,"compare_complex_magnitudes"},
    {0x2f7,"compare_Floats"},
    {0x2f8,"did_push_cnvrt_Float_to_integer"},
    {0x2f9,"estack_number_to_Float"},
    {0x2fa,"float_class"},{0x2fb,"frexp10"},
    {0x2fc,"gcd_exact_whole_Floats"},{0x2fd,"init_float"},
    {0x2fe,"is_Float_exact_whole_number"},
    {0x2ff,"is_float_infinity"},
    {0x300,"is_float_negative_zero"},
    {0x301,"is_float_positive_zero"},
    {0x302,"is_float_signed_infintiy"},
    {0x303,"is_float_transfinite"},
    {0x304,"is_float_unsigned_inf_or_nan"},
    {0x305,"is_float_unsigned_zero"},
    {0x306,"is_nan"},
    {0x307,"likely_approx_to_complex_number"},
    {0x308,"likely_approx_to_number"},
    {0x309,"norm1_complex_Float"},{0x30a,"push_Float"},
    {0x30b,"push_Float_to_nonneg_int"},
    {0x30c,"push_Float_to_rat"},
    {0x30d,"push_cvrt_integer_if_whole_nmb"},
    {0x30e,"push_overflow_to_infinity"},
    {0x30f,"push_pow"},{0x310,"push_round_Float"},
    {0x311,"should_and_did_push_approx_arg2"},
    {0x312,"signum_Float"},{0x313,"did_push_to_polar"},
    {0x314,"push_degrees"},{0x315,"push_format"},
    {0x316,"push_getkey"},{0x317,"push_getfold"},
    {0x318,"push_getmode"},{0x319,"push_gettype"},
    {0x31a,"push_instring"},{0x31b,"push_mrow_aux"},
    {0x31c,"push_part"},{0x31d,"push_pttest"},
    {0x31e,"push_pxltest"},{0x31f,"push_rand"},
    {0x320,"push_randpoly"},{0x321,"push_setfold"},
    {0x322,"push_setgraph"},{0x323,"push_setmode"},
    {0x324,"push_settable"},{0x325,"push_str_to_expr"},
    {0x326,"push_string"},{0x327,"push_switch"},
    {0x328,"psuh_to_cylin"},{0x329,"push_to_sphere"},
    {0x32a,"cmd_andpic"},{0x32b,"cmd_blddata"},
    {0x32c,"cmd_circle"},{0x32d,"cmd_clrdraw"},
    {0x32e,"cmd_clrerr"},{0x32f,"cmd_clrgraph"},
    {0x330,"cmd_clrhome"},{0x331,"cmd_clrio"},
    {0x332,"cmd_clrtable"},{0x333,"cmd_copyvar"},
    {0x334,"cmd_cubicreg"},{0x335,"cmd_custmoff"},
    {0x336,"cmd_custmon"},{0x337,"cmd_custom"},
    {0x338,"cmd_cycle"},{0x339,"cmd_cyclepic"},
    {0x33a,"cmd_delfold"},{0x33b,"cmd_delvar"},
    {0x33c,"cmd_dialog"},{0x33d,"cmd_disp"},
    {0x33e,"cmd_dispg"},{0x33f,"cmd_disphome"},
    {0x340,"cmd_disptbl"},{0x341,"cmd_drawfunc"},
    {0x342,"cmd_drawinv"},{0x343,"cmd_drawparm"},
    {0x344,"cmd_drawpol"},{0x345,"cmd_else"},
    {0x346,"cmd_endfor"},{0x347,"cmd_endloop"},
    {0x348,"cmd_endtry"},{0x349,"cmd_endwhile"},
    {0x34a,"cmd_exit"},{0x34b,"cmd_expreg"},
    {0x34c,"cmd_fill"},{0x34d,"cmd_fnoff"},
    {0x34e,"cmd_fnon"},{0x34f,"cmd_for"},
    {0x350,"cmd_get"},{0x351,"cmd_getcalc"},
    {0x352,"cmd_goto"},{0x353,"cmd_graph"},
    {0x354,"cmd_if"},{0x355,"cmd_ifthen"},
    {0x356,"cmd_input"},{0x357,"cmd_inputstr"},
    {0x358,"cmd_line"},{0x359,"cmd_linehorz"},
    {0x35a,"cmd_linetan"},{0x35b,"cmd_linevert"},
    {0x35c,"cmd_linreg"},{0x35d,"cm_lnreg"},
    {0x35e,"cmd_local"},{0x35f,"cmd_lock"},
    {0x360,"cmd_logistic"},{0x361,"cmd_medmed"},
    {0x362,"cmd_movevar"},{0x363,"cmd_newdata"},
    {0x364,"cmd_newfold"},{0x365,"cmd_newpic"},
    {0x366,"cmd_newplot"},{0x367,"cmd_newprob"},
    {0x368,"cmd_onevar"},{0x369,"cmd_output"},
    {0x36a,"cmd_passerr"},{0x36b,"cmd_pause"},
    {0x36c,"cmd_plotsoff"},{0x36d,"cmd_plotson"},
    {0x36e,"cmd_popup"},{0x36f,"cmd_powerreg"},
    {0x370,"cmd_printobj"},{0x371,"cmd_prompt"},
    {0x372,"cmd_ptchg"},{0x373,"cmd_ptoff"},
    {0x374,"cmd_pton"},{0x375,"cmd_pttext"},
    {0x376,"cmd_pxlchg"},{0x377,"cmd_cmd_pxlcircle"},
    {0x378,"cmd_pxlhorz"},{0x379,"cmd_pxlline"},
    {0x37a,"cmd_pxloff"},{0x37b,"cmd_pxlon"},
    {0x37c,"cmd_pxltext"},{0x37d,"cmd_pxlvert"},
    {0x37e,"cmd_quadreg"},{0x37f,"cmd_quartreg"},
    {0x380,"cmd_randseed"},{0x381,"cmd_rclgdb"},
    {0x382,"cmd_rclpic"},{0x383,"cmd_rename"},
    {0x384,"cmd_request"},{0x385,"cmd_return"},
    {0x386,"cmd_rplcpic"},{0x387,"cmd_send"},
    {0x388,"cmd_sendcalc"},{0x389,"cmd_sendchat"},
    {0x38a,"cmd_shade"},{0x38b,"cmd_showstat"},
    {0x38c,"cmd_sinreg"},{0x38d,"cmd_slpline"},
    {0x38e,"cmd_sorta"},{0x38f,"cmd_sortd"},
    {0x390,"cmd_stogdb"},{0x391,"cmd_stopic"},
    {0x392,"cmd_style"},{0x393,"cmd_table"},
    {0x394,"cmd_text"},{0x395,"cmd_toolbar"},
    {0x396,"cmd_trace"},{0x397,"cmd_try"},
    {0x398,"cmd_twovar"},{0x399,"cmd_unlock"},
    {0x39a,"cmd_while"},{0x39b,"cmd_xorpic"},
    {0x39c,"cmd_zoombox"},{0x39d,"cmd_zoomdata"},
    {0x39e,"cmd_zoomdec"},{0x39f,"cmd_zoomfit"},
    {0x3a0,"cmd_zoomin"},{0x3a1,"cmd_zoomint"},
    {0x3a2,"cmd_zoomout"},{0x3a3,"cmd_zoomprev"},
    {0x3a4,"cmd_zoomrcl"},{0x3a5,"cmd_zoomsqr"},
    {0x3a6,"cmd_zoomstd"},{0x3a7,"cmd_zoomsto"},
    {0x3a8,"cmd_zoomtrig"},{0x3a9,"OSenqueue"},
    {0x3aa,"OSdequeue"},{0x3ab,"OSqinquire"},
    {0x3ac,"OSqhead"},{0x3ad,"OSqclear"},
    {0x3ae,"did_push_divide_units"},
    {0x3af,"has_unit_base"},{0x3b0,"init_unit_system"},
    {0x3b1,"is_units_term"},
    {0x3b2,"push_auto_units_conversion"},
    {0x3b3,"push_unit_system_list"},
    {0x3b4,"setup_unit_system"},{0x3b5,"all_tail"},
    {0x3b6,"any_tail"},{0x3b7,"is_matrix"},
    {0x3b8,"is_square_matrix"},
    {0x3b9,"is_valid_smap_aggregate"},
    {0x3ba,"last_element_index"},{0x3bb,"map_tail"},
    {0x3bc,"map_tail_Int"},{0x3bd,"push_list_plus"},
    {0x3be,"push_list_times"},{0x3bf,"push_reversed_tail"},
    {0x3c0,"push_sq_matrix_to_whole_number"},
    {0x3c1,"push_transpose_aux"},
    {0x3c2,"push_zero_partial_column"},
    {0x3c3,"remaining_element_count"},
    {0x3c4,"push_offset_array"},
    {0x3c5,"push_matrix_product"},
    {0x3c6,"is_pathname"},{0x3c7,"next_token"},
    {0x3c8,"nonblank"},
    {0x3c9,"push_parse_prgm_or_func_text"},
    {0x3ca,"push_parse_text"},{0,NULL}};

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

TI89::TI89()
{
    romFuncs=romFuncs89;
    asm_readmem=asm_89_readmem;
    asm_readmem_word=asm_89_readmem_word;
    asm_readmem_dword=asm_89_readmem_dword;
    asm_writemem=asm_89_writemem;
    asm_writemem_word=asm_89_writemem_word;
    asm_writemem_dword=asm_89_writemem_dword;
}

void TI89::EnableDebug()
{
    asm_readmem=debug_readmem;
    asm_readmem_word=debug_readmem_word;
    asm_readmem_dword=debug_readmem_dword;
    asm_writemem=debug_writemem;
    asm_writemem_word=debug_writemem_word;
    asm_writemem_dword=debug_writemem_dword;
}

void TI89::DisableDebug()
{
    asm_readmem=asm_89_readmem;
    asm_readmem_word=asm_89_readmem_word;
    asm_readmem_dword=asm_89_readmem_dword;
    asm_writemem=asm_89_writemem;
    asm_writemem_word=asm_89_writemem_word;
    asm_writemem_dword=asm_89_writemem_dword;
}

int* TI89::GetKeyMap()
{
    return keymap89;
}

int TI89::GetKeyMat(int r,int c)
{
    return ti89kmat[r][c];
}

int TI89::GetKeyAlphaMat(int r,int c)
{
    return ti89alphakmat[r][c];
}

void TI89::Reset()
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
	ram128=0;
	memprotect=0;
	kbmask=0xff;
    for (i=0;i<10;i++) kbmatrix[i]=0xff;
	ramWrap=0; memAnd=0x3ffff;
	onheld=1; lcdBase=0x4c00;
    for (i=0;i<6;i++) oldScreenBase[i]=lcdBase;
	timer=0; timerInit=0xb2; int0Count=0;
	transflag=0; recvflag=0;
	transnotready=0;
	lcdRefresh=0;
    io0Bit2=1; io0Bit7=1;
    romWriteReady=0; romWritePhase=0; romRetOr=0;
    cpuCompleteStop=0; lcdOff=0;
    romErase=0; flashProtect=0;
    directAccess=0; outRed=1; outWhite=1; getError=0;
    for (i=0;i<32;i++) romChanged[i]=0;
    memset(romFuncAddr,0,sizeof(int)*0x800);
    for (int i=0;(romFuncs[i].name)&&(i<(getmem_dword(getmem_dword(0xc8)-4))-1);i++)
        romFuncAddr[i]=getmem_dword(getmem_dword(0xc8)+(i<<2));
}

int TI89::ioReadByte(int p)
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

void TI89::ioWriteByte(int p,int v)
{
	switch(p)
	{
		case 0:
            contrast=(contrast&(~1))|((v>>5)&1);
            io0Bit7=(v>>7)&1;
            io0Bit2=(v>>2)&1;
            break;
		case 1:
            ram128=v&1; memprotect=(v>>2)&1;
			memAnd=(ramWrap)?0x1ffff:(ram128?0x1ffff:0x3ffff);
            for (int i=0;i<32;i++)
                mem[i]=&ram[(i&(memAnd>>16))<<16];
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

int TI89::getmem(int addr)
{
    addr&=0xffffff;
    if (addr<0x200000)
		return (mem[addr>>16][addr&0xffff]);
	else if (addr<0x600000)
		return (mem[addr>>16][addr&0xffff])|romRetOr;
	else if (addr<0x700000)
		return (ioReadByte(addr&0x1f))&0xff;
}

int TI89::getmem_word(int addr)
{
	return ((getmem(addr)<<8)|getmem(addr+1))&0xffff;
}

int TI89::getmem_dword(int addr)
{
	return (getmem(addr)<<24)|
	       (getmem(addr+1)<<16)|
	       (getmem(addr+2)<<8)|
	       getmem(addr+3);
}

void TI89::setmem(int addr,int v)
{
    addr&=0xffffff;
	if (addr<0x200000)
		ram[addr&memAnd]=v;
    else if (addr<0x400000)
        intRomWriteByte(addr&0x1fffff,v);
    else if (addr<0x600000)
        extRomWriteByte(addr&0x1fffff,v);
	else if (addr<0x700000)
		ioWriteByte(addr&0x1f,v);
}

void TI89::setmem_word(int addr,int v)
{
	setmem(addr,(v>>8)&0xff);
	setmem(addr+1,v&0xff);
}

void TI89::setmem_dword(int addr,int v)
{
	setmem(addr,(v>>24)&0xff);
	setmem(addr+1,(v>>16)&0xff);
    setmem(addr+2,(v>>8)&0xff);
	setmem(addr+3,v&0xff);
}

int TI89::readmem(int addr)
{
    addr&=0xffffff;
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,1,DATABREAK_READ,0))
            DataBreak();
    }
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=1;
    if ((flashProtect)&&(addr>=0x210000)&&(addr<=0x211fff))
        return 0x14;
    if (addr<0x200000)
		return (mem[addr>>16][addr&0xffff]);
	else if (addr<0x600000)
		return (mem[addr>>16][addr&0xffff])|romRetOr;
	else if (addr<0x700000)
		return (ioReadByte(addr&0x1f))&0xff;
}

int TI89::readmem_word(int addr)
{
    addr&=0xffffff;
    if (addr&1) { ::Exception(3,0); HandleException(); }
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,2,DATABREAK_READ,0))
            DataBreak();
    }
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=1;
    if ((flashProtect)&&(addr>=0x210000)&&(addr<=0x211fff))
        return 0x1414;
	return ((getmem(addr)<<8)|getmem(addr+1))&0xffff;
}

int TI89::readmem_dword(int addr)
{
    addr&=0xffffff;
    if (addr&1) { ::Exception(3,0); HandleException(); }
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,4,DATABREAK_READ,0))
            DataBreak();
    }
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=1;
    if ((flashProtect)&&(addr>=0x210000)&&(addr<=0x211fff))
        return 0x14141414;
	return (getmem(addr)<<24)|
	       (getmem(addr+1)<<16)|
	       (getmem(addr+2)<<8)|
	       getmem(addr+3);
}

void TI89::intRomWriteByte(int addr,int v)
{
    if (flashProtect) return;
    if (calc!=89) return;
    if (romWriteReady)
    {
//        if ((intRom[addr]==0xff)||(romWriteReady==1))
        {
            intRom[addr]&=v;
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

void TI89::extRomWriteByte(int addr,int v)
{
    if (flashProtect) return;
    if ((calc!=92)||(!plusMod)) return;
    if (romWriteReady)
    {
        if ((extRom[addr]==0xff)||(romWriteReady==1))
        {
            extRom[addr]&=v;
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

void TI89::writemem(int addr,int v)
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
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=0;
	if (addr<0x200000)
		ram[addr&memAnd]=v;
    else if (addr<0x400000)
        intRomWriteByte(addr&0x1fffff,v);
    else if (addr<0x600000)
        extRomWriteByte(addr&0x1fffff,v);
	else if (addr<0x700000)
		ioWriteByte(addr&0x1f,v);
}

void TI89::writemem_word(int addr,int v)
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
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=0;
	setmem(addr,(v>>8)&0xff);
	setmem(addr+1,v&0xff);
}

void TI89::writemem_dword(int addr,int v)
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
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=0;
	setmem(addr,(v>>24)&0xff);
	setmem(addr+1,(v>>16)&0xff);
	setmem(addr+2,(v>>8)&0xff);
	setmem(addr+3,v&0xff);
}

int TI89::readport(int port)
{
    return 0;
}

void TI89::writeport(int port,int v)
{
}

void TI89::Execute()
{
	static i=0;
	int j,f;

    if (!run) return;
    UpdateDebugCheckEnable();
   	i++;
    if (i>=10)
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

void TI89::OneInstruction()
{
    UpdateDebugCheckEnable();
    MC68000_Execute(1);
}

void TI89::OnUp()
{
    onheld=1;
}

void TI89::OnDown()
{
    onheld=0;
    IRQ(6);
    if (cpuCompleteStop)
        cpuCompleteStop=0;
}

void TI89::KeyUp(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]|=1<<c;
}

void TI89::KeyDown(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]&=~(1<<c);
}

void TI89::AlphaUp()
{
    KeyUp(0,7);
}

void TI89::AlphaDown()
{
    KeyDown(0,7);
}

void TI89::KBInt()
{
    IRQ(1);
}

int TI89::GetKeyID(int i)
{
    return key[i];
}

void TI89::SaveState(char *fn)
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
    fwrite(&romWriteReady,4,1,fp); fwrite(&romWritePhase,4,1,fp);
    fwrite(&romRetOr,4,1,fp);
    fwrite(&lcdBase,4,1,fp); fwrite(&lcdOff,4,1,fp);
    fwrite(&contrast,4,1,fp);
    fwrite(&romErase,4,1,fp); fwrite(&romErasePhase,4,1,fp);
    fwrite(&cpuCompleteStop,4,1,fp);
    for (int i=0;i<32;i++)
    {
        fwrite(&romChanged[i],4,1,fp);
        if (romChanged[i])
            fwrite(&rom[i<<16],65536,1,fp);
    }
    fclose(fp);
}

void TI89::LoadState(char *fn)
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
    fread(&romWriteReady,4,1,fp); fread(&romWritePhase,4,1,fp);
    fread(&romRetOr,4,1,fp);
    fread(&lcdBase,4,1,fp); fread(&lcdOff,4,1,fp);
    fread(&contrast,4,1,fp);
    fread(&romErase,4,1,fp); fread(&romErasePhase,4,1,fp);
    fread(&cpuCompleteStop,4,1,fp);
    for (int i=0;i<32;i++)
    {
        fread(&romChanged[i],4,1,fp);
        if (romChanged[i])
            fread(&rom[i<<16],65536,1,fp);
    }
    for (int i=0;i<32;i++)
        mem[i]=&ram[(i&(memAnd>>16))<<16];
    fclose(fp);
}

int TI89::SendByte(int c)
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

int TI89::GetByte(int &c)
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

int TI89::SendReady()
{
    return !recvflag;
}

int TI89::SendBuffer(int c)
{
    if (recvflag) return 0;
    recvflag=1;
    recvbyte=c;
    IRQ(4);
    return 1;
}

int TI89::CheckBuffer()
{
    return transflag;
}

int TI89::GetBuffer(int &c)
{
    if (!transflag) return 0;
    transflag=0;
    c=transbyte&0xff;
    IRQ(4);
    return 1;
}

#define WaitForAck() \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; }

int TI89::SendFile(char *fn,int more)
{
	FILE *fp;
	int csum,type,size,id=0x89;
	int i,j,c,n,pos;
	char calcName[32],folderName[9],buf[16];
    unsigned char hdr[16];

	for (i=0;i<100;i++)
        Execute();
	fp=fopen(fn,"rb");
	if (!fp)
		return 1;
    sendInProgress=1;
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
    		 if (!SendByte(i))
             {
                fclose(fp);
                sendInProgress=0;
                return 0;
             }
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

int TI89::ReceiveFile()
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
            dlog->DefaultExt=defExt89[type&0x1f];
            dlog->FileName=AnsiString(name)+AnsiString(".")+
                dlog->DefaultExt;
            dlog->FilterIndex=5;
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
                fprintf(fp,"**TI89**");
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
            dlog->DefaultExt=defExt89[type&0x1f];
            group=0;
            if (n==2)
            {
                memcpy(list,&list[16],16);
                n=1;
            }
        }
        else
        {
            dlog->DefaultExt="89g";
            strcpy(name,"group");
            group=1;
        }
        dlog->FileName=AnsiString(name)+AnsiString(".")+
            dlog->DefaultExt;
        dlog->FilterIndex=5;
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
            fprintf(fp,"**TI89**");
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
