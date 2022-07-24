        include "tios.h"
        include "flib.h"
        include "gray4lib.h"
        include "gray7lib.h"

__muls:
        move.l 8(a7),d0
        muls.w 6(a7),d0
        ext.l d0
        rts

__mulu:
        move.l 8(a7),d0
        mulu.w 6(a7),d0
        ext.l d0
        rts

__mods:
        move.l 4(a7),d0
        move.l 8(a7),d1
        jsr tios::_rs32s32
        move.l d1,d0
        rts

__modu:
        move.l 4(a7),d0
        move.l 8(a7),d1
        jsr tios::_ru32u32
        move.l d1,d0
        rts

__divs:
        move.l 4(a7),d0
        move.l 8(a7),d1
        jsr tios::_ds32s32
        move.l d1,d0
        rts

__divu:
        move.l 4(a7),d0
        move.l 8(a7),d1
        jsr tios::_du32u32
        move.l d1,d0
        rts

_pixel_on equ flib::pixel_on
_pixel_off equ flib::pixel_off
_pixel_chg equ flib::pixel_chg
_frame_rect equ flib::frame_rect
_erase_rect equ flib::erase_rect
_clr_scr equ flib::clr_scr
_zap_screen equ flib::zap_screen
_idle_loop equ flib::idle_loop

; void DrawCharXY(char c,short x,short y,short color)
_DrawCharXY equ tios::DrawCharXY

; void exec(short handle)
_exec equ kernel::exec

; void FindSymEntry(WORD symList,char *name)
_FindSymEntry equ flib::FindSymEntry

_Gray4On equ gray4lib::on
_Gray4Off equ gray4lib::off
_Gray4Plane0 equ gray4lib::plane0
_Gray4Plane1 equ gray4lib::plane1

_Gray7On equ gray7lib::on
_Gray7Off equ gray7lib::off
_Gray7Plane0 equ gray7lib::plane0
_Gray7Plane1 equ gray7lib::plane1
_Gray7Plane2 equ gray7lib::plane2

;----------------------------------------------------------------------------
; fline()
;
; Function: Draws a line, _FAST_
;
; Input:    D0.W = X1
;           D1.W = Y1
;           D2.W = X2
;           D3.W = Y2
;           A0   = Address to top left corner (usually $4440)
;              
;----------------------------------------------------------------------------
;linelib::fline          equ     linelib@0000

_ST_eraseHelp equ tios::ST_eraseHelp
_ST_showHelp equ tios::ST_showHelp
_HeapFree equ tios::HeapFree
_HeapAlloc equ tios::HeapAlloc
_ER_catch equ tios::ER_catch
_ER_success equ tios::ER_success
_reset_link equ tios::reset_link
_flush_link equ tios::flush_link
_tx_free equ tios::tx_free
_transmit equ tios::transmit
_receive equ tios::receive
_HeapFreeIndir equ tios::HeapFreeIndir
_ST_busy equ tios::ST_busy
_ER_throwVar equ tios::ER_throwVar
_HeapRealloc equ tios::HeapRealloc
_sprintf equ tios::sprintf
_DrawStrXY equ tios::DrawStrXY
_FontSetSys equ tios::FontSetSys
_DrawTo equ tios::DrawTo
_MoveTo equ tios::MoveTo
_PortSet equ tios::PortSet
_PortRestore equ tios::PortRestore
_WinActivate equ tios::WinActivate
_WinClose equ tios::WinClose
_WinOpen equ tios::WinOpen
_WinStrXY equ tios::WinStrXY
_HeapAllowThrow equ tios::HeapAllocThrow
_strcmp equ tios::strcmp
_FontGetSys equ tios::FontGetSys
_strlen equ tios::strlen
_strncmp equ tios::strncmp
_strncpy equ tios::strncpy
_strcat equ tios::strcat
_strchr equ tios::strchr
_memset equ tios::memset
_memcmp equ tios::memcmp
_memcpy equ tios::memcpy
_memmove equ tios::memmove
_DerefSym equ tios::DerefSym
_MenuPopup equ tios::MenuPopup
_MenuBegin equ tios::MenuBegin
_MenuOn equ tios::MenuOn
_ERD_dialog equ tios::ERD_dialog

_keyPressedFlag equ tios::kb_globals+$1c
_getkeyCode equ tios::kb_globals+$1e
