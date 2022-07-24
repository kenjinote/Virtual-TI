#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "calchw.h"

#define cpu_readmem(a) asm_readmem(a)
#define cpu_readmem_word(a) asm_readmem_word(a)
#define cpu_readmem_dword(a) asm_readmem_dword(a)
#define cpu_writemem(a,v) asm_writemem(a,v)
#define cpu_writemem_word(a,v) asm_writemem_word(a,v)
#define cpu_writemem_dword(a,v) asm_writemem_dword(a,v)
#define cpu_readport(a) hw->readport(a)
#define cpu_writeport(a,v) hw->writeport(a,v)
/*extern "C" int cpu_readmem(int addr);
extern "C" int cpu_readmem_word(int addr);
extern "C" int cpu_readmem_dword(int addr);
extern "C" void cpu_writemem(int addr,int v);
extern "C" void cpu_writemem_word(int addr,int v);
extern "C" void cpu_writemem_dword(int addr,int v);
extern "C" int cpu_readport(int port);
extern "C" void cpu_writeport(int port,int v);*/
#define cpu_readmem24 cpu_readmem
#define cpu_readmem24_word cpu_readmem_word
#define cpu_readmem24_dword cpu_readmem_dword
#define cpu_writemem24 cpu_writemem
#define cpu_writemem24_word cpu_writemem_word
#define cpu_writemem24_dword cpu_writemem_dword

#endif
