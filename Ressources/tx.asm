#include <xc.inc>

; Envoi de la matrice SK6812 (64 LEDs x 4 octets = 256 octets) sur RB5.
; Bit-banging, 64 MHz -> 1 cycle = 62,5 ns. Periode bit = 20 cycles = 1,25 us.
;   "1" : HAUT 13 cycles (0,81 us)
;   "0" : HAUT  5 cycles (0,31 us)
; ATTENTION : en asm, ';' = COMMENTAIRE. Donc UN SEUL NOP par ligne.

psect   txfunc,local,class=CODE,reloc=2

global _TX_64LEDS
global _pC
global _LED_MATRIX

_TX_64LEDS:
    ; FSR0 = adresse de LED_MATRIX (fournie par le C via pC)
    MOVFF _pC + 0, FSR0L
    MOVFF _pC + 1, FSR0H

    CLRF PRODL, 0       ; compteur d'octets : 0 -> wrap -> 256 iterations
    BANKSEL LATB        ; banque de LATB (une fois, hors boucle)

_next_byte:
    MOVF POSTINC0, 0, 0 ; W = octet courant, FSR0++
    MOVWF TABLAT, 0     ; TABLAT = octet de travail
    MOVLW 8
    MOVWF PRODH, 0      ; PRODH = compteur de bits (8)

_bit_loop:
    BSF LATB, 5, 1      ; 1  : HAUT
    RLCF TABLAT, 1, 0   ; 2  : MSB -> Carry
    BTFSS STATUS, 0, 0  ; 3  : saute si C=1
    BRA _bit0           ; 4-5: si C=0

_bit1:                  ; "1" : HAUT 13 cycles
    NOP                 ; 5
    NOP                 ; 6
    NOP                 ; 7
    NOP                 ; 8
    NOP                 ; 9
    NOP                 ; 10
    NOP                 ; 11
    NOP                 ; 12
    NOP                 ; 13
    BCF LATB, 5, 1      ; 14 : BAS
    NOP                 ; 15
    NOP                 ; 16
    NOP                 ; 17
    DECFSZ PRODH, 1, 0  ; 18 : bit suivant ?
    BRA _bit_loop       ; 19-20
    BRA _byte_done      ; dernier bit -> octet suivant

_bit0:                  ; "0" : HAUT 5 cycles
    BCF LATB, 5, 1      ; 6  : BAS
    NOP                 ; 7
    NOP                 ; 8
    NOP                 ; 9
    NOP                 ; 10
    NOP                 ; 11
    NOP                 ; 12
    NOP                 ; 13
    NOP                 ; 14
    NOP                 ; 15
    NOP                 ; 16
    NOP                 ; 17
    DECFSZ PRODH, 1, 0  ; 18
    BRA _bit_loop       ; 19-20
    BRA _byte_done

_byte_done:
    DECFSZ PRODL, 1, 0  ; octet suivant ? (256 au total)
    BRA _next_byte
    RETURN
