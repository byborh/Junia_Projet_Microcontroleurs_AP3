#include <xc.inc>
; =====================================================================
;  tx.asm  -  Communication avec la matrice de LEDs SK6812RGBW
; ---------------------------------------------------------------------
;  Bitbanging du protocole "1-fil" sur la broche CMD_MATRIX = RB5.
;
;  Horloge : FOSC = 64 MHz  ->  FCY = FOSC/4 = 16 MHz  ->  Tcy = 62,5 ns
;
;  Protocole (cf. annexe du sujet) :
;     periode d'un bit = 1,25 us = 20 cycles
;     bit 0 : niveau HAUT pendant 0,32 us (~5 cy) puis BAS
;     bit 1 : niveau HAUT pendant 0,82 us (~13 cy) puis BAS
;     MSB en premier, ordre des octets : G, R, B, W (cf. main.c)
;
;  >>> Les valeurs de delai ci-dessous sont NOMINALES. Le timing reel
;  >>> depend du nombre exact de NOP : a AJUSTER A L'OSCILLOSCOPE sur RB5
;  >>> (mesurer la largeur d'un bit 0 et d'un bit 1). Tolerance ~+/-150 ns.
; =====================================================================

DATA_PIN EQU 5          ; RB5 = CMD_MATRIX

; --- Variables de travail en RAM (banque d'acces -> operande a = 0) ---
PSECT   txbss,class=COMMON,space=1,noexec
TXBYTE:  DS 1           ; octet en cours d'envoi
BITCNT:  DS 1           ; compteur de bits (8 -> 0)
BYTECNT: DS 1           ; compteur d'octets (256)

; When assembly code is placed in a psect, it can be manipulated as a
; whole by the linker and placed in memory.
PSECT   txfunc,local,class=CODE,reloc=2 ; PIC18's should have a reloc (alignment) flag of 2 for any psect which contains executable code.

; -----------------------------------------------------------------
; GLOBALS
;
; Déclaration de fonctions et variables globales permettant au code C et à l'asm de les partager
; Une même fonction ou variable côté asm est préfixée par un underscore, et ne l'est pas côté C
; Avec ce formalisme, elles sont utilisables de façon intercangeable et transparente :
; | ---- asm ----- | ------------- C ----------------- |
; | _TX_64LEDS  <--|--> void TX_64LEDS(void)           |
; | _pC         <--|--> volatile const char * pC       |
; | _LED_MATRIX <--|--> volatile char LED_MATRIX [256] |

; Fonction globales
global _TX_64LEDS ; Fonction définie dans tx.asm ; Fonction permettant d'envoyer la commande pour piloter les 64 LEDs, telle que décrite dans LED_MATRIX

; Constantes/variables globales
global _pC         ; Constante définie dans main.c ; Pointeur vers LED_MATRIX
global _LED_MATRIX ; Variable  définie dans main.c ; Tableau (256 octets = 64 x 4) des composantes RGBW de la matrice LED (1 octet/couleur/LED)

_TX_64LEDS:
    ; Cette fonction envoie sur CMD_MATRIX l'intégralité de la matrice LED_MATRIX,
    ; Chaque bit de chaque octet encodé en largeur d'impulsion

    ; --- Place un pointeur (FSR0) au début de la matrice LED_MATRIX ---
    ; Voir section 10.8.12 (p. 150) de la datasheet PIC18F25K40
    MOVFF _pC + 0, FSR0L    ; LSB de l'adresse de LED_MATRIX -> FSR0L
    MOVFF _pC + 1, FSR0H    ; MSB de l'adresse de LED_MATRIX -> FSR0H

    ; --- Compteur d'octets : 256 (un MOVLW 0 + DECFSZ boucle 256 fois) ---
    MOVLW   0
    MOVWF   BYTECNT, 0

NEXT_BYTE:
    MOVF    POSTINC0, W, 0  ; W <- *FSR0 ; puis FSR0++ (octet suivant)
    MOVWF   TXBYTE, 0       ; octet a transmettre
    MOVLW   8
    MOVWF   BITCNT, 0       ; 8 bits a envoyer, MSB d'abord

; --------- Envoi d'un bit (objectif : 20 cycles = 1,25 us) ----------
SEND_BIT:
    BSF     LATB, DATA_PIN, 0   ; [cy 1 ] front montant : RB5 = 1
    RLCF    TXBYTE, F, 0        ; [cy 2 ] rotation gauche : bit7 (MSB) -> Carry
    NOP                         ; [cy 3 ]
    NOP                         ; [cy 4 ]
    BTFSS   STATUS, C, 0        ; [cy 5 ] bit = 1 ? -> saute le BCF (reste HAUT)
    BCF     LATB, DATA_PIN, 0   ; [cy 6 ] bit = 0 -> RB5 = 0  (fin T0H ~0,32 us)
    ;   Les deux chemins consomment 6 cycles (saute = 2 cy == BTFSS+BCF) : timing aligne
    NOP                         ; [cy 7 ]
    NOP                         ; [cy 8 ]
    NOP                         ; [cy 9 ]
    NOP                         ; [cy 10]
    NOP                         ; [cy 11]
    NOP                         ; [cy 12]
    BCF     LATB, DATA_PIN, 0   ; [cy 13] RB5 = 0 pour les bits 1 (fin T1H ~0,82 us)
    NOP                         ; [cy 14]
    NOP                         ; [cy 15]
    NOP                         ; [cy 16]
    DECFSZ  BITCNT, F, 0        ; [cy 17] bit suivant de l'octet ?
    BRA     SEND_BIT            ; [cy 18-19] -> reboucle (periode ~20 cy avec le BSF suivant)

    ; Octet termine : passe a l'octet suivant
    DECFSZ  BYTECNT, F, 0       ; reste-t-il des octets ?
    BRA     NEXT_BYTE

    ; --- Trame complete : laisser la ligne au repos (BAS) > 50 us = "reset" ---
    ; (le retour vers le main, puis le temps de traitement, assurent ce repos ;
    ;  on peut ajouter une temporisation explicite si necessaire.)
    RETURN
