/* --------------------------------------------------------------
 * Fichier     :   main.c
 * Auteur(s)   :
 * Description :
 * -------------------------------------------------------------- */

#include <xc.h>
#include <stdint.h>

// Configuration materielle du PIC :
#pragma config FEXTOSC = OFF           // Pas de source d'horloge externe
#pragma config RSTOSC = HFINTOSC_64MHZ // Horloge interne de 64 MHz
#pragma config WDTE = OFF              // Désactiver le watchdog

#pragma config MCLRE = EXTMCLR         // Broche MCLR active (reset externe)
#pragma config LVP = OFF               // Programmation basse tension desactivee

#define _XTAL_FREQ 64000000 // Frequence d'horloge - necessaire aux macros de delay (_delay(N) ; __delay_us(N) ; __delay_ms(N)))

//  Detecteurs d'enveloppe (entrees ADC) :
//     basses        = OUT_ENV_1 = RA5 = AN5
//     bas-mediums   = OUT_ENV_2 = RA4 = AN4
//     hauts-mediums = OUT_ENV_3 = RA3 = AN3
//     aigus         = OUT_ENV_4 = RA2 = AN2
#define CH_BASSES     5   // AN5 | OUT_ENv_1
#define CH_BAS_MED    4   // AN4 | OUT_ENv_2
#define CH_HAUT_MED   3   // AN3 | OUT_ENv_3
#define CH_AIGUS      2   // AN2 | OUT_ENv_4


// ============================================================================
//  Definition des broches (voir schematic.pdf)
// ============================================================================
//  CMD_MATRIX = RB5  (data matrice, pilotee en ASM)
//  LED_M      = RB4  (LED "master" / power)
//  LED_0..7   = RC0..RC7 (8 LEDs de test = PORTC)
//  BP0        = RA7  (bouton 0)
//  BP1        = RA6  (bouton 1)


#define LED_M_ON()    (LATB |= 0x10)   // RB4
#define LED_M_OFF()   (LATB &= ~0x10)

#define BP0           (PORTAbits.RA7)  // 0 = appuye (pull-up)
#define BP1           (PORTAbits.RA6)

// ============================================================================
//  Parametres d'affichage
// ============================================================================
#define NB_LEDS       64
#define BAR_HEIGHT    8        // 8 LEDs de haut
#define INTENSITY     24       // intensite des couleurs : JAMAIS 255 ! (~16-32)

// Index des 4 octets dans LED_MATRIX pour chaque LED (ordre d'envoi G,R,B,W)
#define OFF_G  0
#define OFF_R  1
#define OFF_B  2
#define OFF_W  3

// Déclaration de fonctions et variables globales permettant au code C et à l'asm de les partager
// Une même fonction ou variable côté asm est préfixée par un underscore, et ne l'est pas côté C
// Avec ce formalisme, elles sont utilisables de façon intercangeable et transparente :
// | ---- asm ----- | ------------- C ----------------- |
// | _TX_64LEDS  <--|--> void TX_64LEDS(void)           |
// | _pC         <--|--> volatile const char * pC       |
// | _LED_MATRIX <--|--> volatile char LED_MATRIX [256] |

// Définition des fonctions relatives à la matrice de LEDs:
extern void TX_64LEDS(void); // Fonction définie dans tx.asm ; Fonction permettant d'envoyer la commande pour piloter les 64 LEDs, telle que décrite dans LED_MATRIX

// Définition des constantes / variables relatives à la matrice de LEDs :
volatile char LED_MATRIX [256] ; // Definition d'une matrice de 64 x 4 octets contenant les composantes R/G/B/W de chaque LED (1 octet/couleur/LED)
volatile const char * pC = LED_MATRIX; // Pointeur vers LED_MATRIX


// - Fonction main ----------------------------------------------------------------------
void main(void) {
    /* Configuration des entrées / sorties */
    // TODO

    /* Corps du programme */
    // TODO

    /* Code pour vérification du bon fonctionnement de la partir uC (à retirer par la suite) : === DEMO CODE */

    // Initialisation des LEDs =================================================================== DEMO CODE
    TRISB &= 0xEF; // LED_MASTER : OUTPUT -------------------------------------------------------- DEMO CODE
    TRISC &= 0x00; // LED0-7     : OUTPUT -------------------------------------------------------- DEMO CODE

    LATB &= 0xEF; // Eteindre LEDM   ------------------------------------------------------------- DEMO CODE
    LATC  = 0x00; // Eteindre LED0-7 ------------------------------------------------------------- DEMO CODE

    // Blink sur LEDM : ========================================================================== DEMO CODE
    LATB |= 0x10;    // Allumer LEDM   ----------------------------------------------------------- DEMO CODE
    __delay_ms(500); // Macro de délai ----------------------------------------------------------- DEMO CODE
    LATB &= 0xEF;    // Eteindre LEDM  ----------------------------------------------------------- DEMO CODE
    __delay_ms(500); // Macro de délai ----------------------------------------------------------- DEMO CODE
    LATB |= 0x10;    // Allumer LEDM   ----------------------------------------------------------- DEMO CODE

    // Chenillard : ============================================================================== DEMO CODE
    while(1){ //---------------------------------------------------------------------------------- DEMO CODE
        for (int i=0; i<8; i++){ // -------------------------------------------------------------- DEMO CODE
            LATC = 0x01 << i;    // Commander les LEDs de test sur le PORTC ---------------------- DEMO CODE
            __delay_ms(125);     // Macro de délai ----------------------------------------------- DEMO CODE
        } // ------------------------------------------------------------------------------------- DEMO CODE
    } // ----------------------------------------------------------------------------------------- DEMO CODE

    return;
}
