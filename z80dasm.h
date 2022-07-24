/*** Z80Em: Portable Z80 emulator *******************************************/
/***                                                                      ***/
/***                               Z80Dasm.h                              ***/
/***                                                                      ***/
/*** This file contains a routine to disassemble a buffer containing Z80  ***/
/*** opcodes. It is included from Z80Debug.c and z80dasm.c                ***/
/***                                                                      ***/
/*** Copyright (C) Marcel de Kogel 1996,1997                              ***/
/***     You are not allowed to distribute this software commercially     ***/
/***     Please, notify me, if you make any changes to this file          ***/
/****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "calchw.h"

static char *mnemonic_xx_cb[256]=
{
 "rlc b=Y","rlc c=Y","rlc d=Y","rlc e=Y","rlc h=Y","rlc l=Y","rlc Y",  "rlc a=Y",
 "rrc b=Y","rrc c=Y","rrc d=Y","rrc e=Y","rrc h=Y","rrc l=Y","rrc Y",  "rrc a=Y",
 "rl b=Y", "rl c=Y", "rl d=Y", "rl e=Y", "rl h=Y", "rl l=Y", "rl Y",   "rl a=Y",
 "rr b=Y" ,"rr c=Y" ,"rr d=Y" ,"rr e=Y" ,"rr h=Y" ,"rr l=Y", "rr Y",   "rr a=Y",
 "sla b=Y","sla c=Y","sla d=Y","sla e=Y","sla h=Y","sla l=Y","sla Y",  "sla a=Y",
 "sra b=Y","sra c=Y","sra d=Y","sra e=Y","sra h=Y","sra l=Y","sra Y",  "sra a=Y",
 "sll b=Y","sll c=Y","sll d=Y","sll e=Y","sll h=Y","sll l=Y","sll Y",  "sll a=Y",
 "srl b=Y","srl c=Y","srl d=Y","srl e=Y","srl h=Y","srl l=Y","srl Y",  "srl a=Y",
 "bit b=0,Y","bit c=0,Y","bit d=0,Y","bit e=0,Y","bit h=0,Y","bit l=0,Y","bit 0,Y","bit a=0,Y",
 "bit b=1,Y","bit c=1,Y","bit d=1,Y","bit e=1,Y","bit h=1,Y","bit l=1,Y","bit 1,Y","bit a=1,Y",
 "bit b=2,Y","bit c=2,Y","bit d=2,Y","bit e=2,Y","bit h=2,Y","bit l=2,Y","bit 2,Y","bit a=2,Y",
 "bit b=3,Y","bit c=3,Y","bit d=3,Y","bit e=3,Y","bit h=3,Y","bit l=3,Y","bit 3,Y","bit a=3,Y",
 "bit b=4,Y","bit c=4,Y","bit d=4,Y","bit e=4,Y","bit h=4,Y","bit l=4,Y","bit 4,Y","bit a=4,Y",
 "bit b=5,Y","bit c=5,Y","bit d=5,Y","bit e=5,Y","bit h=5,Y","bit l=5,Y","bit 5,Y","bit a=5,Y",
 "bit b=6,Y","bit c=6,Y","bit d=6,Y","bit e=6,Y","bit h=6,Y","bit l=6,Y","bit 6,Y","bit a=6,Y",
 "bit b=7,Y","bit c=7,Y","bit d=7,Y","bit e=7,Y","bit h=7,Y","bit l=7,Y","bit 7,Y","bit a=7,Y",
 "res b=0,Y","res c=0,Y","res d=0,Y","res e=0,Y","res h=0,Y","res l=0,Y","res 0,Y","res a=0,Y",
 "res b=1,Y","res c=1,Y","res d=1,Y","res e=1,Y","res h=1,Y","res l=1,Y","res 1,Y","res a=1,Y",
 "res b=2,Y","res c=2,Y","res d=2,Y","res e=2,Y","res h=2,Y","res l=2,Y","res 2,Y","res a=2,Y",
 "res b=3,Y","res c=3,Y","res d=3,Y","res e=3,Y","res h=3,Y","res l=3,Y","res 3,Y","res a=3,Y",
 "res b=4,Y","res c=4,Y","res d=4,Y","res e=4,Y","res h=4,Y","res l=4,Y","res 4,Y","res a=4,Y",
 "res b=5,Y","res c=5,Y","res d=5,Y","res e=5,Y","res h=5,Y","res l=5,Y","res 5,Y","res a=5,Y",
 "res b=6,Y","res c=6,Y","res d=6,Y","res e=6,Y","res h=6,Y","res l=6,Y","res 6,Y","res a=6,Y",
 "res b=7,Y","res c=7,Y","res d=7,Y","res e=7,Y","res h=7,Y","res l=7,Y","res 7,Y","res a=7,Y",
 "set b=0,Y","set c=0,Y","set d=0,Y","set e=0,Y","set h=0,Y","set l=0,Y","set 0,Y","set a=0,Y",
 "set b=1,Y","set c=1,Y","set d=1,Y","set e=1,Y","set h=1,Y","set l=1,Y","set 1,Y","set a=1,Y",
 "set b=2,Y","set c=2,Y","set d=2,Y","set e=2,Y","set h=2,Y","set l=2,Y","set 2,Y","set a=2,Y",
 "set b=3,Y","set c=3,Y","set d=3,Y","set e=3,Y","set h=3,Y","set l=3,Y","set 3,Y","set a=3,Y",
 "set b=4,Y","set c=4,Y","set d=4,Y","set e=4,Y","set h=4,Y","set l=4,Y","set 4,Y","set a=4,Y",
 "set b=5,Y","set c=5,Y","set d=5,Y","set e=5,Y","set h=5,Y","set l=5,Y","set 5,Y","set a=5,Y",
 "set b=6,Y","set c=6,Y","set d=6,Y","set e=6,Y","set h=6,Y","set l=6,Y","set 6,Y","set a=6,Y",
 "set b=7,Y","set c=7,Y","set d=7,Y","set e=7,Y","set h=7,Y","set l=7,Y","set 7,Y","set =a7Y,"
};

static char *mnemonic_cb[256]=
{
 "rlc b"  ,"rlc c"  ,"rlc d"  ,"rlc e"  ,"rlc h"  ,"rlc l"  ,"rlc (hl)"  ,"rlc a"  ,
 "rrc b"  ,"rrc c"  ,"rrc d"  ,"rrc e"  ,"rrc h"  ,"rrc l"  ,"rrc (hl)"  ,"rrc a"  ,
 "rl b"   ,"rl c"   ,"rl d"   ,"rl e"   ,"rl h"   ,"rl l"   ,"rl (hl)"   ,"rl a"   ,
 "rr b"   ,"rr c"   ,"rr d"   ,"rr e"   ,"rr h"   ,"rr l"   ,"rr (hl)"   ,"rr a"   ,
 "sla b"  ,"sla c"  ,"sla d"  ,"sla e"  ,"sla h"  ,"sla l"  ,"sla (hl)"  ,"sla a"  ,
 "sra b"  ,"sra c"  ,"sra d"  ,"sra e"  ,"sra h"  ,"sra l"  ,"sra (hl)"  ,"sra a"  ,
 "sll b"  ,"sll c"  ,"sll d"  ,"sll e"  ,"sll h"  ,"sll l"  ,"sll (hl)"  ,"sll a"  ,
 "srl b"  ,"srl c"  ,"srl d"  ,"srl e"  ,"srl h"  ,"srl l"  ,"srl (hl)"  ,"srl a"  ,
 "bit 0,b","bit 0,c","bit 0,d","bit 0,e","bit 0,h","bit 0,l","bit 0,(hl)","bit 0,a",
 "bit 1,b","bit 1,c","bit 1,d","bit 1,e","bit 1,h","bit 1,l","bit 1,(hl)","bit 1,a",
 "bit 2,b","bit 2,c","bit 2,d","bit 2,e","bit 2,h","bit 2,l","bit 2,(hl)","bit 2,a",
 "bit 3,b","bit 3,c","bit 3,d","bit 3,e","bit 3,h","bit 3,l","bit 3,(hl)","bit 3,a",
 "bit 4,b","bit 4,c","bit 4,d","bit 4,e","bit 4,h","bit 4,l","bit 4,(hl)","bit 4,a",
 "bit 5,b","bit 5,c","bit 5,d","bit 5,e","bit 5,h","bit 5,l","bit 5,(hl)","bit 5,a",
 "bit 6,b","bit 6,c","bit 6,d","bit 6,e","bit 6,h","bit 6,l","bit 6,(hl)","bit 6,a",
 "bit 7,b","bit 7,c","bit 7,d","bit 7,e","bit 7,h","bit 7,l","bit 7,(hl)","bit 7,a",
 "res 0,b","res 0,c","res 0,d","res 0,e","res 0,h","res 0,l","res 0,(hl)","res 0,a",
 "res 1,b","res 1,c","res 1,d","res 1,e","res 1,h","res 1,l","res 1,(hl)","res 1,a",
 "res 2,b","res 2,c","res 2,d","res 2,e","res 2,h","res 2,l","res 2,(hl)","res 2,a",
 "res 3,b","res 3,c","res 3,d","res 3,e","res 3,h","res 3,l","res 3,(hl)","res 3,a",
 "res 4,b","res 4,c","res 4,d","res 4,e","res 4,h","res 4,l","res 4,(hl)","res 4,a",
 "res 5,b","res 5,c","res 5,d","res 5,e","res 5,h","res 5,l","res 5,(hl)","res 5,a",
 "res 6,b","res 6,c","res 6,d","res 6,e","res 6,h","res 6,l","res 6,(hl)","res 6,a",
 "res 7,b","res 7,c","res 7,d","res 7,e","res 7,h","res 7,l","res 7,(hl)","res 7,a",
 "set 0,b","set 0,c","set 0,d","set 0,e","set 0,h","set 0,l","set 0,(hl)","set 0,a",
 "set 1,b","set 1,c","set 1,d","set 1,e","set 1,h","set 1,l","set 1,(hl)","set 1,a",
 "set 2,b","set 2,c","set 2,d","set 2,e","set 2,h","set 2,l","set 2,(hl)","set 2,a",
 "set 3,b","set 3,c","set 3,d","set 3,e","set 3,h","set 3,l","set 3,(hl)","set 3,a",
 "set 4,b","set 4,c","set 4,d","set 4,e","set 4,h","set 4,l","set 4,(hl)","set 4,a",
 "set 5,b","set 5,c","set 5,d","set 5,e","set 5,h","set 5,l","set 5,(hl)","set 5,a",
 "set 6,b","set 6,c","set 6,d","set 6,e","set 6,h","set 6,l","set 6,(hl)","set 6,a",
 "set 7,b","set 7,c","set 7,d","set 7,e","set 7,h","set 7,l","set 7,(hl)","set 7,a"
};

static char *mnemonic_ed[256]=
{
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "in b,(c)","out (c),b","sbc hl,bc","ld (W),bc","neg","retn","im 0","ld i,a",
 "in c,(c)","out (c),c","adc hl,bc","ld bc,(W)","!"  ,"reti","!"   ,"ld r,a",
 "in d,(c)","out (c),d","sbc hl,de","ld (W),de","!"  ,"!"   ,"im 1","ld a,i",
 "in e,(c)","out (c),e","adc hl,de","ld de,(W)","!"  ,"!"   ,"im 2","ld a,r",
 "in h,(c)","out (c),h","sbc hl,hl","ld (W),hl","!"  ,"!"   ,"!"   ,"rrd"   ,
 "in l,(c)","out (c),l","adc hl,hl","ld hl,(W)","!"  ,"!"   ,"!"   ,"rld"   ,
 "in 0,(c)","out (c),0","sbc hl,sp","ld (W),sp","!"  ,"!"   ,"!"   ,"!"     ,
 "in a,(c)","out (c),a","adc hl,sp","ld sp,(W)","!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "ldi"     ,"cpi"      ,"ini"      ,"outi"     ,"!"  ,"!"   ,"!"   ,"!"     ,
 "ldd"     ,"cpd"      ,"ind"      ,"outd"     ,"!"  ,"!"   ,"!"   ,"!"     ,
 "ldir"    ,"cpir"     ,"inir"     ,"otir"     ,"!"  ,"!"   ,"!"   ,"!"     ,
 "lddr"    ,"cpdr"     ,"indr"     ,"otdr"     ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"     ,
 "!"       ,"!"        ,"!"        ,"!"        ,"!"  ,"!"   ,"!"   ,"!"
};

static char *mnemonic_xx[256]=
{
  "@"      ,"@"       ,"@"       ,"@"        ,"@"       ,"@"       ,"@"      ,"@"      ,
  "@"      ,"add I,bc","@"       ,"@"        ,"@"       ,"@"       ,"@"      ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"@"       ,"@"       ,"@"      ,"@"      ,
  "@"      ,"add I,de","@"       ,"@"        ,"@"       ,"@"       ,"@"      ,"@"      ,
  "@"      ,"ld I,W"  ,"ld (W),I","inc I"    ,"inc Ih"  ,"dec Ih"  ,"ld Ih,B","@"      ,
  "@"      ,"add I,I" ,"ld I,(W)","dec I"    ,"inc Il"  ,"dec Il"  ,"ld Il,B","@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"inc X"   ,"dec X"   ,"ld X,B" ,"@"      ,
  "@"      ,"add I,sp","@"       ,"@"        ,"@"       ,"@"       ,"@"      ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"ld b,Ih" ,"ld b,Il" ,"ld b,X" ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"ld c,Ih" ,"ld c,Il" ,"ld c,X" ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"ld d,Ih" ,"ld d,Il" ,"ld d,X" ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"ld e,Ih" ,"ld e,Il" ,"ld e,X" ,"@"      ,
  "ld Ih,b","ld Ih,c" ,"ld Ih,d" ,"ld Ih,e"  ,"ld Ih,h" ,"ld Ih,l" ,"ld h,X" ,"ld Ih,a",
  "ld Il,b","ld Il,c" ,"ld Il,d" ,"ld Il,e"  ,"ld Il,h" ,"ld Il,l" ,"ld l,X" ,"ld Il,a",
  "ld X,b" ,"ld X,c"  ,"ld X,d"  ,"ld X,e"   ,"ld X,h"  ,"ld X,l"  ,"@"      ,"ld X,a" ,
  "@"      ,"@"       ,"@"       ,"@"        ,"ld a,Ih" ,"ld a,Il" ,"ld a,X" ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"add a,Ih","add a,Il","add a,X","@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"adc a,Ih","adc a,Il","adc a,X","@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"sub Ih"  ,"sub Il"  ,"sub X"  ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"sbc a,Ih","sbc a,Il","sbc a,X","@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"and Ih"  ,"and Il"  ,"and X"  ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"xor Ih"  ,"xor Il"  ,"xor X"  ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"or Ih"   ,"or Il"   ,"or X"   ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"cp Ih"   ,"cp Il"   ,"cp X"   ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"@"       ,"@"       ,"@"      ,"@"      ,
  "@"      ,"@"       ,"@"       ,"fd cb"    ,"@"       ,"@"       ,"@"      ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"@"       ,"@"       ,"@"      ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"@"       ,"@"       ,"@"      ,"@"      ,
  "@"      ,"pop I"   ,"@"       ,"ex (sp),I","@"       ,"push I"  ,"@"      ,"@"      ,
  "@"      ,"jp (I)"  ,"@"       ,"@"        ,"@"       ,"@"       ,"@"      ,"@"      ,
  "@"      ,"@"       ,"@"       ,"@"        ,"@"       ,"@"       ,"@"      ,"@"      ,
  "@"      ,"ld sp,I" ,"@"       ,"@"        ,"@"       ,"@"       ,"@"      ,"@"
};

static char *mnemonic_main[256]=
{
 "nop"      ,"ld bc,W"  ,"ld (bc),a","inc bc"    ,"inc b"    ,"dec b"    ,"ld b,B"    ,"rlca"     ,
 "ex af,af'","add hl,bc","ld a,(bc)","dec bc"    ,"inc c"    ,"dec c"    ,"ld c,B"    ,"rrca"     ,
 "djnz R"   ,"ld de,W"  ,"ld (de),a","inc de"    ,"inc d"    ,"dec d"    ,"ld d,B"    ,"rla"      ,
 "jr R"     ,"add hl,de","ld a,(de)","dec de"    ,"inc e"    ,"dec e"    ,"ld e,B"    ,"rra"      ,
 "jr nz,R"  ,"ld hl,W"  ,"ld (W),hl","inc hl"    ,"inc h"    ,"dec h"    ,"ld h,B"    ,"daa"      ,
 "jr z,R"   ,"add hl,hl","ld hl,(W)","dec hl"    ,"inc l"    ,"dec l"    ,"ld l,B"    ,"cpl"      ,
 "jr nc,R"  ,"ld sp,W"  ,"ld (W),a" ,"inc sp"    ,"inc (hl)" ,"dec (hl)" ,"ld (hl),B" ,"scf"      ,
 "jr c,R"   ,"add hl,sp","ld a,(W)" ,"dec sp"    ,"inc a"    ,"dec a"    ,"ld a,B"    ,"ccf"      ,
 "ld b,b"   ,"ld b,c"   ,"ld b,d"   ,"ld b,e"    ,"ld b,h"   ,"ld b,l"   ,"ld b,(hl)" ,"ld b,a"   ,
 "ld c,b"   ,"ld c,c"   ,"ld c,d"   ,"ld c,e"    ,"ld c,h"   ,"ld c,l"   ,"ld c,(hl)" ,"ld c,a"   ,
 "ld d,b"   ,"ld d,c"   ,"ld d,d"   ,"ld d,e"    ,"ld d,h"   ,"ld d,l"   ,"ld d,(hl)" ,"ld d,a"   ,
 "ld e,b"   ,"ld e,c"   ,"ld e,d"   ,"ld e,e"    ,"ld e,h"   ,"ld e,l"   ,"ld e,(hl)" ,"ld e,a"   ,
 "ld h,b"   ,"ld h,c"   ,"ld h,d"   ,"ld h,e"    ,"ld h,h"   ,"ld h,l"   ,"ld h,(hl)" ,"ld h,a"   ,
 "ld l,b"   ,"ld l,c"   ,"ld l,d"   ,"ld l,e"    ,"ld l,h"   ,"ld l,l"   ,"ld l,(hl)" ,"ld l,a"   ,
 "ld (hl),b","ld (hl),c","ld (hl),d","ld (hl),e" ,"ld (hl),h","ld (hl),l","halt"      ,"ld (hl),a",
 "ld a,b"   ,"ld a,c"   ,"ld a,d"   ,"ld a,e"    ,"ld a,h"   ,"ld a,l"   ,"ld a,(hl)" ,"ld a,a"   ,
 "add a,b"  ,"add a,c"  ,"add a,d"  ,"add a,e"   ,"add a,h"  ,"add a,l"  ,"add a,(hl)","add a,a"  ,
 "adc a,b"  ,"adc a,c"  ,"adc a,d"  ,"adc a,e"   ,"adc a,h"  ,"adc a,l"  ,"adc a,(hl)","adc a,a"  ,
 "sub b"    ,"sub c"    ,"sub d"    ,"sub e"     ,"sub h"    ,"sub l"    ,"sub (hl)"  ,"sub a"    ,
 "sbc a,b"  ,"sbc a,c"  ,"sbc a,d"  ,"sbc a,e"   ,"sbc a,h"  ,"sbc a,l"  ,"sbc a,(hl)","sbc a,a"  ,
 "and b"    ,"and c"    ,"and d"    ,"and e"     ,"and h"    ,"and l"    ,"and (hl)"  ,"and a"    ,
 "xor b"    ,"xor c"    ,"xor d"    ,"xor e"     ,"xor h"    ,"xor l"    ,"xor (hl)"  ,"xor a"    ,
 "or b"     ,"or c"     ,"or d"     ,"or e"      ,"or h"     ,"or l"     ,"or (hl)"   ,"or a"     ,
 "cp b"     ,"cp c"     ,"cp d"     ,"cp e"      ,"cp h"     ,"cp l"     ,"cp (hl)"   ,"cp a"     ,
 "ret nz"   ,"pop bc"   ,"jp nz,W"  ,"jp W"      ,"call nz,W","push bc"  ,"add a,B"   ,"rst 00h"  ,
 "ret z"    ,"ret"      ,"jp z,W"   ,"cb"        ,"call z,W" ,"call W"   ,"adc a,B"   ,"rst 08h"  ,
 "ret nc"   ,"pop de"   ,"jp nc,W"  ,"out (B),a" ,"call nc,W","push de"  ,"sub B"     ,"rst 10h"  ,
 "ret c"    ,"exx"      ,"jp c,W"   ,"in a,(B)"  ,"call c,W" ,"dd"       ,"sbc a,B"   ,"rst 18h"  ,
 "ret po"   ,"pop hl"   ,"jp po,W"  ,"ex (sp),hl","call po,W","push hl"  ,"and B"     ,"rst 20h"  ,
 "ret pe"   ,"jp (hl)"  ,"jp pe,W"  ,"ex de,hl"  ,"call pe,W","ed"       ,"xor B"     ,"rst 28h"  ,
 "ret p"    ,"pop af"   ,"jp p,W"   ,"di"        ,"call p,W" ,"push af"  ,"or B"      ,"rst 30h"  ,
 "ret m"    ,"ld sp,hl" ,"jp m,W"   ,"ei"        ,"call m,W" ,"fd"       ,"cp B"      ,"rst 38h"
};

static char Sign (unsigned char a)
{
 return (a&128)? '-':'+';
}

static int Abs (unsigned char a)
{
 if (a&128) return 256-a;
 else return a;
}

static char tmpStrZ80[64];

static int inline IsROMAddrZ80(int addr)
{
    if (addr==0) return 0;
    if (!romFuncs) return 0;
    for (int i=0;romFuncs[i].name;i++)
    {
        if (addr==romFuncAddr[i])
        {
            if ((addr>=0x4000)&&(addr<0x8000)&&(calc==86)&&
                (pageANum!=0xd))
                return 0;
            if ((addr>=0x4000)&&(addr<0x8000)&&(calc==83)&&
                (pageANum!=0xc))
                return 0;
            return 1;
        }
    }
    return 0;
}

static char inline *ROMAddrNameZ80(int addr)
{
    for (int i=0;romFuncs[i].name;i++)
    {
        if (addr==romFuncAddr[i])
        {
            sprintf(tmpStrZ80,"%s",romFuncs[i].name);
            return tmpStrZ80;
        }
    }
    return "Internal Error";
}
/****************************************************************************/
/* Disassemble first opcode in buffer and return number of bytes it takes   */
/****************************************************************************/
static int DasmZ80 (char *dest,int PC)
{
 char *S;
 char *r;
 int i,j,k,addr;
 char buf[64];
 char Offset = 0;
 i=PC;
 r="INTERNAL PROGRAM ERROR";
 dest[0]='\0';
 switch (hw->getmem(i))
 {
  case 0xCB:
   i++;
   S=mnemonic_cb[hw->getmem(i++)];
   break;
  case 0xED:
   i++;
   S=mnemonic_ed[hw->getmem(i++)];
   break;
  case 0xDD:
   i++;
   r="ix";
   switch (hw->getmem(i))
   {
    case 0xcb:
     i++;
     Offset=hw->getmem(i++);
     S=mnemonic_xx_cb[hw->getmem(i++)];
     break;
    default:
     S=mnemonic_xx[hw->getmem(i++)];
     break;
   }
   break;
  case 0xFD:
   i++;
   r="iy";
   switch (hw->getmem(i))
   {
    case 0xcb:
     i++;
     Offset=hw->getmem(i++);
     S=mnemonic_xx_cb[hw->getmem(i++)];
     break;
    default:
     S=mnemonic_xx[hw->getmem(i++)];
     break;
   }
   break;
  default:
   S=mnemonic_main[hw->getmem(i++)];
   break;
 }
 for (j=0;S[j];++j)
 {
  switch (S[j])
  {
   case 'B':
    sprintf (buf,"$%02x",hw->getmem(i++));
    strcat (dest,buf);
    break;
   case 'R':
    sprintf (buf,"$%04x",(PC+2+(signed char)hw->getmem(i))&0xFFFF);
    i++;
    strcat (dest,buf);
    break;
   case 'W':
    addr=hw->getmem(i)+hw->getmem(i+1)*256;
    if (IsROMAddrZ80(addr))
        sprintf (buf,"%s",ROMAddrNameZ80(addr));
    else
        sprintf (buf,"$%04x",addr);
    i+=2;
    strcat (dest,buf);
    break;
   case 'X':
    sprintf (buf,"(%s%c$%02x)",r,Sign(hw->getmem(i)),Abs(hw->getmem(i)));
    i++;
    strcat (dest,buf);
    break;
   case 'Y':
    sprintf (buf,"(%s%c$%02x)",r,Sign(Offset),Abs(Offset));
    strcat (dest,buf);
    break;
   case 'I':
    strcat (dest,r);
    break;
   case '!':
    sprintf (dest,"db     $ed,$%02x",hw->getmem(PC+1));
    return 2;
   case '@':
    sprintf (dest,"db     $%02x",hw->getmem(PC));
    return 1;
#if 0
   case '#':
    sprintf (dest,"db     $%02x,$cb,$%02x",hw->getmem(PC),hw->getmem(PC+2));
    return 2;
#endif
   case ' ':
    k=strlen(dest);
    if (k<4) k=5-k;
    else k=1;
    memset (buf,' ',k);
    buf[k]='\0';
    strcat (dest,buf);
    break;
   default:
    buf[0]=S[j];
    buf[1]='\0';
    strcat (dest,buf);
    break;
  }
 }
 if ((calc==82)&&(!strcmp(dest,"call $8d74")))
 {
    int addr=hw->getmem_word(i);
    if (IsROMAddrZ80(addr|0x10000))
        sprintf(dest,"ROM_CALL(%s)",ROMAddrNameZ80(addr|0x10000));
    else
        sprintf(dest,"ROM_CALL($%04x)",addr);
    i+=2;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8c09")))
 {
    int addr=hw->getmem(i);
    if (IsROMAddrZ80(addr|0x10000))
        sprintf(dest,"ROM_CALL(%s)",ROMAddrNameZ80(addr|0x10000));
    else
        sprintf(dest,"ROM_CALL($%04x)",addr);
    i++;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8cc8")))
 {
    int addr=hw->getmem_word(i);
    if (IsROMAddrZ80(addr))
        sprintf(dest,"RCALL_(%s)",ROMAddrNameZ80(addr));
    else
        sprintf(dest,"RCALL_($%04x)",addr);
    i+=2;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8c0c")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"CALL_Z($%04x)",addr+hw->getmem_word(0x8c3c));
    i+=2;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8c0f")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"CALL_($%04x)",addr+hw->getmem_word(0x8c3c));
    i+=2;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8c12")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"CALL_NZ($%04x)",addr+hw->getmem_word(0x8c3c));
    i+=2;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8c18")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"CALL_C($%04x)",addr+hw->getmem_word(0x8c3c));
    i+=2;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8c1e")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"CALL_NC($%04x)",addr+hw->getmem_word(0x8c3c));
    i+=2;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8c24")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"JUMP_Z($%04x)",addr+hw->getmem_word(0x8c3c));
    i+=2;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8c27")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"JUMP_($%04x)",addr+hw->getmem_word(0x8c3c));
    i+=2;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8c2a")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"JUMP_NZ($%04x)",addr+hw->getmem_word(0x8c3c));
    i+=2;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8c30")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"JUMP_C($%04x)",addr+hw->getmem_word(0x8c3c));
    i+=2;
 }
 else if ((calc==85)&&(!strcmp(dest,"call $8c36")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"JUMP_NC($%04x)",addr+hw->getmem_word(0x8c3c));
    i+=2;
 }
 else if ((calc==84)&&(!strcmp(dest,"rst  28h")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"B_CALL($%04x)",addr);
    i+=2;
 }
 else if ((calc==84)&&(!strcmp(dest,"call $0050")))
 {
    int addr=hw->getmem_word(i);
    sprintf(dest,"B_JUMP($%04x)",addr);
    i+=2;
 }
 return (i - PC);
}

