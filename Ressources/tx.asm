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
global _pC         ; Constante définie dans main.c ; Pointeur vers LED_MATRIX
global _LED_MATRIX ; Variable  définie dans main.c ; Tableau (256 octets = 64 x 4) des composantes RGBW de la matrice LED (1 octet/couleur/LED)

_TX_64LEDS:
    ; Cette fonction envoie sur CMD_MATRIX l'intégralité de la matrice LED_MATRIX,
    ; Chaque bit de chaque octet encodé en largeur d'impulsion

    ; Place un pointeur au début de la matrice LED_MATRIX
    ; Voir section 10.8.12 (p. 150) de la datasheet PIC18F25K40
    MOVFF _pC + 0, WREG ; Charge le LSB du pointeur de LED_MATRIX dans WREG
    MOVWF FSR0L, 0      ; Définit le LSB du registre d'adressage indirect
    MOVFF _pC + 1, WREG ; Charge le MSB du pointeur de LED_MATRIX dans WREG
    MOVWF FSR0H, 0      ; Définit le MSB du registre d'adressage indirect

    ; Désormais, dès l'exécution de l'instruction suivante, la valeur pointée par <FSR0H-FSR0L> est chargée dans WREG, et <FSR0H-FSR0L> est incrémenté :
    ; MOVF POSTINC0, 0, 0

    ; Envoie la commande pour piloter chacune des 64 LEDs
    ; TODO
    ; Envoie la commande pour piloter chacune des 64 LEDs (256 octets)
    
    ; 1. Initialisation des compteurs
    ; On utilise des registres systèmes libres (PRODL, PRODH, TABLAT) pour ne pas gêner le compilateur C
    CLRF PRODL, 0       ; PRODL sera notre compteur d'octets. Init à 0 (bouclera 256 fois)

_next_byte:
    MOVF POSTINC0, 0, 0 ; WREG = charge l'octet actuel et incrémente le pointeur (1 cycle)
    MOVWF TABLAT, 0     ; TABLAT = copie l'octet à envoyer ici (notre zone de travail)
    MOVLW 8             ; WREG = 8
    MOVWF PRODH, 0      ; PRODH = notre compteur de bits (8 bits par octet)

_bit_loop:
    ; --- DEBUT DU BIT (Total : 20 cycles = 1,25 µs) ---
    BSF LATC, 0, 0      ; [1 cycle]  BROCHE A L'ETAT HAUT (Ajuste "LATC, 0" selon ton câblage !)
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
    BCF LATC, 0, 0      ; [1 cycle] (Cycle 14) BROCHE A L'ETAT BAS
    
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
    BCF LATC, 0, 0      ; [1 cycle] (Cycle 6) BROCHE A L'ETAT BAS
    
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

    RETURN
