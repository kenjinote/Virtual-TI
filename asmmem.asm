	.486p
	model flat

_DATA segment dword public use32 'DATA'
exceptRetIP dd 0
exceptSP dd 0
exceptBP dd 0
exceptSet dd 0
exceptNest dd 0
exceptReady db 0
_DATA ends

_TEXT segment dword public use32 'CODE'

    extern @debug_readmem
    extern @debug_readmem_word
    extern @debug_readmem_dword
    extern @debug_writemem
    extern @debug_writemem_word
    extern @debug_writemem_dword
    extern _mem
    extern _ram
    extern _rom
    extern _memAnd
    extern _pageA
    extern _pageB
    extern _pageAType
    extern _pageBType
    extern _romRetOr

@SetupExceptionHandler proc near
    add dword ptr [exceptNest],1
    cmp dword ptr [exceptSet],0
    jne ExceptionSetUp
    mov eax,dword ptr [esp]
    mov dword ptr [exceptRetIP],eax
    mov dword ptr [exceptSP],esp
    mov dword ptr [exceptBP],ebp
    mov dword ptr [exceptSet],1
    mov dword ptr [exceptNest],0
    mov byte ptr [exceptReady],1
ExceptionSetUp:
    xor eax,eax
    ret
ExceptionExit:
    mov dword ptr [exceptSet],0
    mov eax,1
    ret
@SetupExceptionHandler endp

@HandleException proc near
    cmp byte ptr [exceptReady],0
    je HandleExceptionReturn
    cmp dword ptr [exceptNest],0
    jne HandleExceptionExit
    mov esp,dword ptr [exceptSP]
    mov eax,dword ptr [exceptRetIP]
    mov dword ptr [esp],eax
    mov ebp,dword ptr [exceptBP]
    mov byte ptr [exceptReady],0
    push offset ExceptionExit
    ret
HandleExceptionExit:
    sub dword ptr [exceptNest],1
HandleExceptionReturn:
    ret
@HandleException endp

@asm_86_readmem proc near
    cmp ax,4000h
    jb a86rm_rom0
    cmp ax,8000h
    jb a86rm_pageA
    cmp ax,0c000h
    jb a86rm_pageB
    and eax,03fffh
    add eax,_ram
    mov al,byte ptr [eax]
    and eax,0ffh
    ret
a86rm_rom0:
    and eax,03fffh
    add eax,_rom
    mov al,byte ptr [eax]
    and eax,0ffh
    ret
a86rm_pageA:
    and eax,03fffh
    add eax,_pageA
    mov al,byte ptr [eax]
    and eax,0ffh
    ret
a86rm_pageB:
    and eax,03fffh
    add eax,_pageB
    mov al,byte ptr [eax]
    and eax,0ffh
    ret
@asm_86_readmem endp

@asm_86_readmem_word proc near
    cmp ax,4000h
    jb a86rmw_rom0
    cmp ax,8000h
    jb a86rmw_pageA
    cmp ax,0c000h
    jb a86rmw_pageB
    and eax,03fffh
    add eax,_ram
    mov ax,word ptr [eax]
    bswap eax
    shr eax,16
    ret
a86rmw_rom0:
    and eax,03fffh
    add eax,_rom
    mov ax,word ptr [eax]
    bswap eax
    shr eax,16
    ret
a86rmw_pageA:
    and eax,03fffh
    add eax,_pageA
    mov ax,word ptr [eax]
    bswap eax
    shr eax,16
    ret
a86rmw_pageB:
    and eax,03fffh
    add eax,_pageB
    mov ax,word ptr [eax]
    bswap eax
    shr eax,16
    ret
@asm_86_readmem_word endp

@asm_86_readmem_dword proc near
    cmp ax,4000h
    jb a86rmdw_rom0
    cmp ax,8000h
    jb a86rmdw_pageA
    cmp ax,0c000h
    jb a86rmdw_pageB
    and eax,03fffh
    add eax,_ram
    mov eax,dword ptr [eax]
    bswap eax
    ret
a86rmdw_rom0:
    and eax,03fffh
    add eax,_rom
    mov eax,dword ptr [eax]
    bswap eax
    ret
a86rmdw_pageA:
    and eax,03fffh
    add eax,_pageA
    mov eax,dword ptr [eax]
    bswap eax
    ret
a86rmdw_pageB:
    and eax,03fffh
    add eax,_pageB
    mov eax,dword ptr [eax]
    bswap eax
    ret
@asm_86_readmem_dword endp

@asm_86_writemem proc near
    cmp ax,4000h
    jb a86wm_end
    cmp ax,8000h
    jb a86wm_pageA
    cmp ax,0c000h
    jb a86wm_pageB
    and eax,03fffh
    add eax,_ram
    mov byte ptr [eax],dl
    ret
a86wm_pageA:
    cmp _pageAType,0
    je a86wm_end
    and eax,03fffh
    add eax,_pageA
    mov byte ptr [eax],dl
    ret
a86wm_pageB:
    cmp _pageBType,0
    je a86wm_end
    and eax,03fffh
    add eax,_pageB
    mov byte ptr [eax],dl
a86wm_end:
    ret
@asm_86_writemem endp

@asm_86_writemem_word proc near
    cmp ax,4000h
    jb a86wmw_end
    cmp ax,8000h
    jb a86wmw_pageA
    cmp ax,0c000h
    jb a86wmw_pageB
    and eax,03fffh
    add eax,_ram
    bswap edx
    shr edx,16
    mov word ptr [eax],dx
    ret
a86wmw_pageA:
    cmp _pageAType,0
    je a86wmw_end
    and eax,03fffh
    add eax,_pageA
    bswap edx
    shr edx,16
    mov word ptr [eax],dx
    ret
a86wmw_pageB:
    cmp _pageBType,0
    je a86wmw_end
    and eax,03fffh
    add eax,_pageB
    bswap edx
    shr edx,16
    mov word ptr [eax],dx
a86wmw_end:
    ret
@asm_86_writemem_word endp

@asm_86_writemem_dword proc near
    cmp ax,4000h
    jb a86wmdw_end
    cmp ax,8000h
    jb a86wmdw_pageA
    cmp ax,0c000h
    jb a86wmdw_pageB
    and eax,03fffh
    add eax,_ram
    bswap edx
    mov dword ptr [eax],edx
    ret
a86wmdw_pageA:
    cmp _pageAType,0
    je a86wmdw_end
    and eax,03fffh
    add eax,_pageA
    bswap edx
    mov dword ptr [eax],edx
    ret
a86wmdw_pageB:
    cmp _pageBType,0
    je a86wmdw_end
    and eax,03fffh
    add eax,_pageB
    bswap edx
    mov dword ptr [eax],edx
a86wmdw_end:
    ret
@asm_86_writemem_dword endp

@asm_89_readmem proc near
    and eax,0ffffffh
    cmp eax,600000h
    jge a89rm_calldebug
    cmp eax,1c0000h
    jb a89rm_go
    cmp eax,200000h
    jb a89rm_calldebug
    test eax,1e0000h
    jz a89rm_calldebug
a89rm_go:
    push ebx
    mov ebx,eax
    shr ebx,16
    and eax,0ffffh
    cmp ebx,020h
    mov ebx,dword ptr [_mem+ebx*4]
    mov al,byte ptr [ebx+eax]
    jb a89rm_notrom
    or eax,_romRetOr
a89rm_notrom:
    and eax,0ffh
    pop ebx
    ret
a89rm_calldebug:
    push offset @debug_readmem
    ret
@asm_89_readmem endp

@asm_89_readmem_word proc near
    and eax,0ffffffh
    test eax,1
    jnz a89rmw_calldebug
    cmp eax,600000h
    jge a89rmw_calldebug
    cmp eax,1c0000h
    jb a89rmw_go
    cmp eax,200000h
    jb a89rmw_calldebug
    test eax,1e0000h
    jz a89rmw_calldebug
a89rmw_go:
    push ebx
    mov ebx,eax
    shr ebx,16
    and eax,0ffffh
    cmp ebx,020h
    mov ebx,dword ptr [_mem+ebx*4]
    mov ax,word ptr [ebx+eax]
    jb a89rmw_notrom
    or eax,_romRetOr
a89rmw_notrom:
    bswap eax
    shr eax,16
    pop ebx
    ret
a89rmw_calldebug:
    push offset @debug_readmem_word
    ret
@asm_89_readmem_word endp

@asm_89_readmem_dword proc near
    and eax,0ffffffh
    test eax,1
    jnz a89rmdw_calldebug
    cmp eax,600000h
    jge a89rmdw_calldebug
    cmp eax,1c0000h
    jb a89rmdw_go
    cmp eax,200000h
    jb a89rmdw_calldebug
    test eax,1e0000h
    jz a89rmdw_calldebug
a89rmdw_go:
    push ebx
    mov ebx,eax
    shr ebx,16
    and eax,0ffffh
    cmp ebx,020h
    mov ebx,dword ptr [_mem+ebx*4]
    mov eax,dword ptr [ebx+eax]
    jb a89rmdw_notrom
    or eax,_romRetOr
a89rmdw_notrom:
    bswap eax
    pop ebx
    ret
a89rmdw_calldebug:
    push offset @debug_readmem_dword
    ret
@asm_89_readmem_dword endp

@asm_89_writemem proc near
    and eax,0ffffffh
    cmp eax,1c0000h
    jge a89wm_calldebug
    cmp eax,100h
    jb a89wm_calldebug
    and eax,_memAnd
    add eax,_ram
    mov byte ptr [eax],dl
    ret
a89wm_calldebug:
    push offset @debug_writemem
    ret
@asm_89_writemem endp

@asm_89_writemem_word proc near
    and eax,0ffffffh
    test eax,1
    jnz a89wmw_calldebug
    cmp eax,1c0000h
    jge a89wmw_calldebug
    cmp eax,100h
    jb a89wmw_calldebug
    push edx
    and eax,_memAnd
    bswap edx
    shr edx,16
    add eax,_ram
    mov word ptr [eax],dx
    pop edx
    ret
a89wmw_calldebug:
    push offset @debug_writemem_word
    ret
@asm_89_writemem_word endp

@asm_89_writemem_dword proc near
    and eax,0ffffffh
    test eax,1
    jnz a89wmdw_calldebug
    cmp eax,1c0000h
    jge a89wmdw_calldebug
    cmp eax,100h
    jb a89wmdw_calldebug
    push edx
    and eax,_memAnd
    bswap edx
    add eax,_ram
    mov dword ptr [eax],edx
    pop edx
    ret
a89wmdw_calldebug:
    push offset @debug_writemem_dword
    ret
@asm_89_writemem_dword endp

_TEXT ends
    public @SetupExceptionHandler
    public @HandleException
    public @asm_86_readmem
    public @asm_86_readmem_word
    public @asm_86_readmem_dword
    public @asm_86_writemem
    public @asm_86_writemem_word
    public @asm_86_writemem_dword
    public @asm_89_readmem
    public @asm_89_readmem_word
    public @asm_89_readmem_dword
    public @asm_89_writemem
    public @asm_89_writemem_word
    public @asm_89_writemem_dword
    end
