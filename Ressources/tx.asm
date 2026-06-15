#include <xc.inc>

; When assembly code is placed in a psect, it can be manipulated as a
; whole by the linker and placed in memory.
psect   txfunc,local,class=CODE,reloc=2 ; PIC18's should have a reloc (alignment) flag of 2 for any psect which contains executable code.

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
global _LED_MATRIX ; Variable  définie dans main.c ; Tableau (256 octets = 64 x 4) des composantes RGBW de la matrice LED (1 octet/couleur/LED)

_TX_64LEDS:
    LFSR 0, _LED_MATRIX   ; Charge directement l'adresse 16-bits du tableau dans FSR0

    ; Initialisation
    CLRF PRODL, 0       ; Compteur d'octets (débordera à 255 -> 256 itérations)

    ; Chargement du tout premier octet avant de lancer le chronomètre
    MOVF POSTINC0, 0, 0
    MOVWF TABLAT, 0

_byte_loop:

    ; =========================================================
    ; BIT 7
    ; =========================================================
    BSF LATB, 5, 0      ; [1] Cycle 1
    RLCF TABLAT, 1, 0   ; [1] Cycle 2
    BTFSS STATUS, 0, 0  ; [1/2] Cycle 3
    BRA _b7_0           ; [2]
_b7_1:
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP ; [9] Cycles 5-13
    BCF LATB, 5, 0      ; [1] Cycle 14
    NOP; NOP; NOP; NOP  ; [4] Cycles 15-18
    BRA _b6_start       ; [2] Cycles 19-20
_b7_0:
    BCF LATB, 5, 0      ; [1] Cycle 6
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP ; [14] Cycles 7-20
    ; Le code "tombe" dans _b6_start au cycle 21 (= cycle 1 du bit suivant)

_b6_start:
    ; =========================================================
    ; BIT 6
    ; =========================================================
    BSF LATB, 5, 0      ; [1] Cycle 1
    RLCF TABLAT, 1, 0   ; [1] Cycle 2
    BTFSS STATUS, 0, 0  ; [1/2] Cycle 3
    BRA _b6_0           ; [2]
_b6_1:
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP ; [9] Cycles 5-13
    BCF LATB, 5, 0      ; [1] Cycle 14
    NOP; NOP; NOP; NOP  ; [4] Cycles 15-18
    BRA _b5_start       ; [2] Cycles 19-20
_b6_0:
    BCF LATB, 5, 0      ; [1] Cycle 6
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP ; [14] Cycles 7-20

_b5_start:
    ; =========================================================
    ; BIT 5
    ; =========================================================
    BSF LATB, 5, 0
    RLCF TABLAT, 1, 0
    BTFSS STATUS, 0, 0
    BRA _b5_0
_b5_1:
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP
    BCF LATB, 5, 0
    NOP; NOP; NOP; NOP
    BRA _b4_start
_b5_0:
    BCF LATB, 5, 0
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP

_b4_start:
    ; =========================================================
    ; BIT 4
    ; =========================================================
    BSF LATB, 5, 0
    RLCF TABLAT, 1, 0
    BTFSS STATUS, 0, 0
    BRA _b4_0
_b4_1:
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP
    BCF LATB, 5, 0
    NOP; NOP; NOP; NOP
    BRA _b3_start
_b4_0:
    BCF LATB, 5, 0
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP

_b3_start:
    ; =========================================================
    ; BIT 3
    ; =========================================================
    BSF LATB, 5, 0
    RLCF TABLAT, 1, 0
    BTFSS STATUS, 0, 0
    BRA _b3_0
_b3_1:
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP
    BCF LATB, 5, 0
    NOP; NOP; NOP; NOP
    BRA _b2_start
_b3_0:
    BCF LATB, 5, 0
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP

_b2_start:
    ; =========================================================
    ; BIT 2
    ; =========================================================
    BSF LATB, 5, 0
    RLCF TABLAT, 1, 0
    BTFSS STATUS, 0, 0
    BRA _b2_0
_b2_1:
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP
    BCF LATB, 5, 0
    NOP; NOP; NOP; NOP
    BRA _b1_start
_b2_0:
    BCF LATB, 5, 0
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP

_b1_start:
    ; =========================================================
    ; BIT 1
    ; =========================================================
    BSF LATB, 5, 0
    RLCF TABLAT, 1, 0
    BTFSS STATUS, 0, 0
    BRA _b1_0
_b1_1:
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP
    BCF LATB, 5, 0
    NOP; NOP; NOP; NOP
    BRA _b0_start
_b1_0:
    BCF LATB, 5, 0
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP

_b0_start:
    ; =========================================================
    ; BIT 0 - LE BIT INTELLIGENT (CHARGEMENT DE L'OCTET SUIVANT)
    ; =========================================================
    BSF LATB, 5, 0      ; [1] Cycle 1
    RLCF TABLAT, 1, 0   ; [1] Cycle 2
    BTFSS STATUS, 0, 0  ; [1/2] Cycle 3
    BRA _b0_0           ; [2]
_b0_1:
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP ; [9] Cycles 5-13
    BCF LATB, 5, 0      ; [1] Cycle 14
    
    ; -- On profite de l'état BAS pour recharger WREG et TABLAT ! --
    MOVF POSTINC0, 0, 0 ; [1] Cycle 15
    MOVWF TABLAT, 0     ; [1] Cycle 16
    NOP                 ; [1] Cycle 17
    DECF PRODL, 1, 0    ; [1] Cycle 18 (Décrémente le compteur d'octets)
    BNZ _byte_loop      ; [2] Cycles 19-20 (Si != 0, remonte en haut !)
    RETURN              ; Si c'était le dernier octet (Z=1), on quitte la fonction.

_b0_0:
    BCF LATB, 5, 0      ; [1] Cycle 6
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP ; [8] Cycles 7-14
    
    ; -- Même gymnastique de chargement pour le chemin "0" --
    MOVF POSTINC0, 0, 0 ; [1] Cycle 15
    MOVWF TABLAT, 0     ; [1] Cycle 16
    NOP                 ; [1] Cycle 17
    DECF PRODL, 1, 0    ; [1] Cycle 18 (Décrémente le compteur d'octets)
    BNZ _byte_loop      ; [2] Cycles 19-20
    
    RETURN              ; Fin de la matrice !

    
_next_byte:
    MOVF POSTINC0, 0, 0 ; WREG = charge l'octet actuel et incrémente le pointeur (1 cycle)
    MOVWF TABLAT, 0     ; TABLAT = copie l'octet à envoyer ici (notre zone de travail)
    MOVLW 8             ; WREG = 8
    MOVWF PRODH, 0      ; PRODH = notre compteur de bits (8 bits par octet)

_bit_loop:
    ; --- DEBUT DU BIT (Total : 20 cycles = 1,25 µs) ---
    BSF LATB, 5, 0      ; [1 cycle]  BROCHE A L'ETAT HAUT (RB5 = CMD_MATRIX)
    RLCF TABLAT, 1, 0   ; [1 cycle]  Décale l'octet vers la gauche. Le bit fort tombe dans le Carry (C)
    BTFSS STATUS, 0, 0  ; [1 ou 2]   Teste le Carry. Si C=1, on saute la ligne suivante.
    BRA _bit_is_zero    ; [2 cycles] Si C=0, on part vers le code du bit "0"

_bit_is_one:
    ; Si on est ici, on est au cycle 4. Pour un "1", la broche doit rester HAUTE 13 cycles.
    ; Il nous manque 9 cycles d'attente.
    NOP                 ; [5]
    NOP                 ; [6]
    NOP                 ; [7]
    NOP                 ; [8]
    NOP                 ; [9]
    NOP                 ; [10]
    NOP                 ; [11]
    NOP                 ; [12]
    NOP                 ; [13]
    BCF LATB, 5, 0      ; [1 cycle] (Cycle 14) BROCHE A L'ETAT BAS
    
    ; La broche doit rester BASSE pendant 7 cycles (pour atteindre 20)
    NOP                 ; [15]
    NOP                 ; [16]
    NOP                 ; [17]
    DECFSZ PRODH, 1, 0  ; [1 ou 2] Décrémente le compteur de bits. Saute si = 0.
    BRA _bit_loop       ; [2 cycles] (Cycle 19-20) Retour au début pour le bit suivant !
    BRA _byte_done      ; [2 cycles] Si c'était le dernier bit, on sort.

_bit_is_zero:
    ; Si on est ici, on est au cycle 6 (car le saut BRA a pris 2 cycles). 
    ; Pour un "0", la broche doit être HAUTE pendant 5 cycles. C'est le moment de la baisser !
    BCF LATB, 5, 0      ; [1 cycle] (Cycle 6) BROCHE A L'ETAT BAS
    
    ; La broche doit rester BASSE pendant 15 cycles (pour atteindre 20)
    NOP                 ; [7]
    NOP                 ; [8]
    NOP                 ; [9]
    NOP                 ; [10]
    NOP                 ; [11]
    NOP                 ; [12]
    NOP                 ; [13]
    NOP                 ; [14]
    NOP                 ; [15]
    NOP                 ; [16]
    NOP                 ; [17]
    DECFSZ PRODH, 1, 0  ; [1 ou 2] Décrémente le compteur de bits. Saute si = 0.
    BRA _bit_loop       ; [2 cycles] (Cycle 19-20) Retour au début pour le bit suivant !
    BRA _byte_done      ; [2 cycles] Si c'était le dernier bit, on sort.

_byte_done:
    DECFSZ PRODL, 1, 0  ; Décrémente le compteur d'octets. (256 itérations)
    BRA _next_byte      ; S'il reste des octets, on charge le suivant
    
    RETURN              ; Fin de la fonction !
