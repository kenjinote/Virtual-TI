;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
;       ЫЫ   ЬЫ  ЬЫЫЫЫЫ  ЬЫЫЫЫЫЫ  ЬЫЫЫЫЫЬ  ЫЫЫЫЫЫЬ  ЫЫЫЫЫЫЬ
;       ЫЫ   ЫЫ  ЫЫ      ЫЫ       ЫЫ   ЫЫ  ЫЫ   ЫЫ  ЫЫ   ЫЫ
;       ЫЫ   ЫЫ  ЯЫЫЫЫЬ  ЫЫ  ЫЫЫ  ЫЫ ЫЫЫЫ  ЫЫЫЫЫЫ   ЫЫ   ЫЫ
;       ЫЫ   ЫЫ      ЫЫ  ЫЫ   ЫЫ  ЫЫ   ЫЫ  ЫЫ  ЫЫ   ЫЫ   ЫЫ
;       ЯЫЫЫЫЫЯ  ЫЫЫЫЫЯ  ЯЫЫЫЫЫЯ  ЫЫ   ЫЫ  ЫЫ   ЫЫ  ЫЫЫЫЫЫЯ
;               Usgard (C) 1997 by      Andreas Ess,
;                                       Sam Davies,
;                                       Jimmy M†rdell,
;                                       Austin Butler &
;                                       Mel Tsai
;
;
;
;Just recompile your program with #include "usgard.h" instead of "ti-85.h"!
;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД

;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
;CALL's, ROM_CALL's
;Note: in Usgard, you can use a normal call instead of ROM_CALL(
; Example: write call D_ZM_STR instead of ROM_CALL(D_ZM_STR)
;RCALL ensures a 'safe' return when program has moved
;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
#DEFINE CALL_Z(addr)            call $8C0C \ .dw addr
#DEFINE CALL_(addr)             call $8C0F \ .dw addr
#DEFINE CALL_NZ(addr)           call $8C12 \ .dw addr
#DEFINE CALL_C(addr)            call $8C18 \ .dw addr
#DEFINE CALL_NC(addr)           call $8C1E \ .dw addr
#DEFINE JUMP_Z(addr)            call $8C24 \ .dw addr
#DEFINE JUMP_(addr)             call $8C27 \ .dw addr
#DEFINE JUMP_NZ(addr)           call $8C2A \ .dw addr
#DEFINE JUMP_C(addr)            call $8C30 \ .dw addr
#DEFINE JUMP_NC(addr)           call $8C36 \ .dw addr
#DEFINE ROM_CALL(addr)          call addr
#DEFINE ROM_CALL_C(addr)        call c, addr
#DEFINE ROM_CALL_NC(addr)       call nc, addr
#DEFINE ROM_CALL_Z(addr)        call z, addr
#DEFINE ROM_CALL_NZ(addr)       call nz, addr
#DEFINE RCALL_(addr)            call $8CC8 \ .dw addr

;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
; Data Locations within Usgard
;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
PROGRAM_ADDR    =$8C3C  ; program address
ROM_VERS        =$8C3E  ; ROM version byte:
			;  2Ah: 2.0
			;  57h: 3.0A
			;  8Eh: 4.0
			;  5Eh: 5.0
			;  3Dh: 6.0
			;  97h: 8.0
			;  91h: 9.0
			;  35h: 10.0
USG_VER         =$8C3F  ; Usgard version number ($24=1.5 lite, $25=1.5 std)
USG_BITS        =$8C40  ; bit 0 set: program has been modified (not nec.)
			; bit 1 set: warp out (must be done by the shell)
ORGSP           =$8E8B  ; original SP(needed for OTH_EXIT)
USGSHELL        =$8EA2  ; name of shell, zero-terminated, 9 bytes
VATName         =$8EAB  ; name of the current VAT entry

DEST_ADDR       =$8EB4  ; destination of video routines (e.g. VIDEO_MEM)
PAGE1ADDR       =$8EB4  ; DEST_ADDR is also used as first grayscale layer
PAGE2ADDR       =$8EB6  ; second grayscale layer

PROG_BYTE	=$8C08	; this is always = PROG_HEADER
PROG_HEADER     =$F8    ; Usgard 1.5 program's header

;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
; The ROM_CALL parameters supported by ZShell
;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
LD_HL_MHL       =$0033  ; HL=(HL), A=L
CP_HL_DE        =$008E  ; CP HL,DE (modifies flags only)
UNPACK_HL       =$009A  ; unpacks one digit of HL into %A
READ_KEYPAD     =$01A2  ; read the keypad port(A=key group mask->A=B=key press data)
STORE_KEY       =$01B1  ; store immediate keystroke from %A
GET_KEY         =$01BE  ; get immediate keystroke into %A
UPDATE_APD      =$0115  ; set APD to 5 minutes again.
READ_KEY	=$0168	; get keystroke into %A, use ports directly

TX_CHARPUT      =$8C41  ; xlated display of %A on screen, normal text style
D_LT_STR        =$8C44  ; display length-byte string
M_CHARPUT       =$8C47  ; display %A on screen, menu style
D_ZM_STR        =$8C4A  ; display zero-terminated string,  menu style
D_LM_STR        =$8C4D  ; display length-byte string, menu style
SCROLL_UP       =$8C50  ; scroll text screen up
TR_CHARPUT      =$8C53  ; raw display of %A on screen, normal text style
CLEARLCD        =$8C56  ; clear LCD, but not text or graphics memory
D_HL_DECI       =$8C59  ; disp. HL as 5-byte, right just., blank-pad. decimal
CLEARTEXT       =$8C5C  ; clear LCD and text memory (affected by 1,(IY+13))
D_ZT_STR        =$8C5F  ; display zero-terminated normal text string
BUSY_OFF        =$8C62  ; turn off "busy" indicataor
BUSY_ON         =$8C65  ; turn on "busy" indicator
RANDOM          =$8C68  ; return random number in A
FIND_PIXEL      =$8C6B  ; Find location in RAM for a pixel on the GFX screen

FREEMEM         =$8C77  ; HL = free RAM on TI
VAR_LENGTH      =$8C7A  ; DE = length of variable data at HL
ASCIIZ_LEN      =$8C7D  ; get length of asciiz string
NEG_BC          =$8C80  ; negate BC: HL = BC = -BC
MUL_HL          =$8C83  ; HL = H * L
COPY_STRING     =$8C8C  ; copy string from HL to DE

;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
;The new Usgard functions(15!)
;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
;Interrupt functions
INT_INSTALL     =$8C9B  ; installs an interrupt. HL = interrupt routine
INT_REMOVE      =$8C9E  ; removes the last interrupt routine installed
INT_CLEAN       =$8CA1  ; remove all installed interrupts (except TSRs)

;Append functions
APPEND          =$8C95  ; Appends BC bytes from HL to end of Usgard string
UNAPPEND        =$8C98  ; Removes the last BC bytes from Usgard string
CHECK_APPEND    =$8CCB  ; Check if code at HL, size BC, has been appended

;Var & Mem functions
VAR_NEW         =$8CA4  ; generates a var.(HL = name, A = type, BC = size)
VAR_DELETE      =$8CA7  ; deletes a var(name = HL)
VAR_EXEC        =$8CAA  ; execute a variable(name = HL)
VAR_GET         =$8CAD  ; gets a var(name = HL; DE = VAT, HL = data)
VAR_RESIZE      =$8CB0  ; resizes a var, HL = name, BC = new size.

;Relocate functions
RELOC           =$8CCE  ; relocate var (HL = name).
DERELOC         =$8CD1  ; derelocate var (HL = name)
RELOC_TAB	=$8CD7	; relocate memory loc. HL = loc., DE = end of rel. table, A = mode

;Other functions
SEARCH_VAT      =$8CB3  ; search for file %A at VAT addr %HL, returns:
			; DE = data area, HL updated, carry if end of vat
			; check VATName for the name of the VAT entry
OTH_SHUTDOWN    =$8CB6  ; shutdown the calc.
DM_HL_DECI      =$8CB9  ; display HL in menu style, right justified.
OTH_PAUSE       =$8CBC  ; wait until Enter pressed
OTH_CLEAR       =$8CBF  ; zero BC-1 bytes from HL
OTH_EXIT        =$8CC2  ; exit to Usgard, no need for pops!
OTH_ARROW       =$8CC5  ; get arrow keys, 2nd, EXIT and MORE(multi-key)
OTH_FILL	=$8CD4	; fills BC-1 bytes from HL with (HL)

;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
; Data Locations
;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
KEY_0           =$8000  ; translated scancode of last key, but 0 if gotten
KEY_1           =$8001  ; translated scancode of key down now
KEY_2           =$8002  ; same as 8001, but $FF if more than one key is down
KEY_STAT        =$8004  ; bit 2 set = key down now
LAST_KEY        =$8006  ; last key pressed
CONTRAST        =$8007  ; contrast
CURSOR_ROW      =$800C  ; text cursor row
CURSOR_COL      =$800D  ; text cursor column
CURSOR_LET      =$800E  ; letter under text cursor
BUSY_COUNTER    =$8080  ; counter for busy indicator
BUSY_BITMAP     =$8081  ; bitmap for busy indicator
OP1             =$8082  ; OP1 'register'(var type, length, name, 11 bytes)
OP2             =$808D  ; OP2 'register'(var type, length, name, 11 bytes)
OP3             =$8098  ; OP3 'register'(var type, length, name, 11 bytes)
OP4             =$80A3  ; OP4 'register'(var type, length, name, 11 bytes)
OP5             =$80AE  ; OP5 'register'(var type, length, name, 11 bytes)
OP6             =$80B9  ; OP6 'register'(var type, length, name, 11 bytes)
CURR_INPUT      =$80C6  ; -> current home-screen input
BYTES_USED      =$80CC  ; # of used user memory (+$80C8 = first free byte)
TEXT_MEM        =$80DF  ; text memory
CURSOR_ROW2     =$800C  ; text cursor row
CURSOR_COL2     =$800D  ; text cursor column
DELC_LEN        =$800F  ; DelChar buffer length (max. 100)
DELC            =$8010  ; DelChar buffer(100 bytes)
CHECKSUM        =$81BE  ; memory checksum from 8BF7 to FA6F
TEMP_STORAGE    =$827C  ; 128 bytes temporary storage
CURSOR_X        =$8333  ; x value of cursor
CURSOR_Y        =$8334  ; y value of cursor
_IY_TABLE       =$8346  ; where IY usually points
GRAPH_MEM       =$8641  ; graphics memory
TEXT_MEM2       =$8A6B  ; secondary text memory
USER_MEM        =$8B1B  ; -> start of user memory
FIXED_POINT     =$8B3A  ; fixed-point decimal place (FFh for floating point)
FIRST_FREE      =$8BE1  ; first free byte in memory
VAT_END         =$8BEB  ; -> one byte before end of VAT (backwards)
VAT_START       =$FA6F  ; start of VAT
VIDEO_MEM       =$FC00  ; video memory


;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
; Key Scancode definitions
;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
;                     Function                Character
K_NOKEY       =$00    ;No key
K_DOWN        =$01    ;Down
K_LEFT        =$02    ;Left
K_RIGHT       =$03    ;Right
K_UP          =$04    ;Up
K_ENTER       =$09    ;Enter
K_PLUS        =$0A    ;+                      X
K_MINUS       =$0B    ;-                      T
K_STAR        =$0C    ;*                      O
K_SLASH       =$0D    ;/                      J
K_RAISE       =$0E    ;^                      E
K_CLEAR       =$0F    ;Clear
K_SIGN        =$11    ;(-)                    Space
K_3           =$12    ;3                      W
K_6           =$13    ;6                      S
K_9           =$14    ;9                      N
K_RIGHTPAR    =$15    ;)                      I
K_TAN         =$16    ;Tan                    D
K_CUSTOM      =$17    ;Custom
K_DOT         =$19    ;.                      Z
K_2           =$1A    ;2                      V
K_5           =$1B    ;5                      R
K_8           =$1C    ;8                      M
K_LEFTPAR     =$1D    ;(                      H
K_COS         =$1E    ;Cos                    C
K_PRGM        =$1F    ;Prgm
K_DEL         =$20    ;Del
K_0           =$21    ;0                      Y
K_1           =$22    ;1                      U
K_4           =$23    ;4                      Q
K_7           =$24    ;7                      L
K_EE          =$25    ;EE                     G
K_SIN         =$26    ;Sin                    B
K_STAT        =$27    ;Stat
K_XVAR        =$28    ;x-Var                  x
K_ON          =$29    ;On
K_STO         =$2A    ;Sto                    =
K_COMMA       =$2B    ;,                      P
K_SQUARE      =$2C    ;x^2                    K
K_LN          =$2D    ;Ln                     F
K_LOG         =$2E    ;Log                    A
K_GRAPH       =$2F    ;Graph
K_ALPHA       =$30    ;Alpha
K_F5          =$31    ;F5
K_F4          =$32    ;F4
K_F3          =$33    ;F3
K_F2          =$34    ;F2
K_F1          =$35    ;F1
K_SECOND      =$36    ;2nd
K_EXIT        =$37    ;EXIT
K_MORE        =$38    ;MORE


;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
;Bits for OTH_ARROW:
;ДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДДД
DOWN  = 0
LEFT  = 1
RIGHT = 2
UP  = 3
F1  = 4
SECOND  = 5
EXIT  = 6
MORE  = 7
