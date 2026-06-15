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
#define INTENSITY     8        // BAISSE pour tester l'hypothese courant/alim (etait 24)

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

// ============================================================================
//  Prototypes
//  On ANNONCE ici toutes les fonctions. Le compilateur connait alors leur
//  signature (type de retour + arguments) avant de rencontrer le moindre
//  appel : l'ordre des definitions plus bas n'a donc plus aucune importance.
// ============================================================================
static uint16_t adc_read(uint8_t channel);
static uint8_t  adc_to_level(uint16_t value);
static void     build_frame(const uint8_t level[4]);
static uint8_t  led_index(uint8_t col, uint8_t row);
static void     set_led(uint8_t idx, uint8_t g, uint8_t r, uint8_t b, uint8_t w);
static void     clear_matrix(void);
static void     set_led_by_height(uint8_t idx, uint8_t row);

// Lit un canal ADC, retourne 0..1023
static uint16_t adc_read(uint8_t channel) {
    ADPCH = channel;
    __delay_us(5);
    ADCON0bits.ADGO = 1;          // démarre la conversion (bit 1 !)
    NOP();                        // voir note ci-dessous
    while (ADCON0bits.ADGO) { ; } // ADGO repasse à 0 en fin de conversion
    return (uint16_t)(((uint16_t)ADRESH << 8) | ADRESL);
}

// Convertit une valeur ADC (0..1023) en hauteur de barre (0..8)
static uint8_t adc_to_level(uint16_t value) {
    uint8_t lvl = (uint8_t)((value * (BAR_HEIGHT + 1u)) / 1024u); // 0..8
    if (lvl > BAR_HEIGHT) lvl = BAR_HEIGHT;
    return lvl;
}

// Construit la trame : chaque bande occupe 2 colonnes, hauteur = niveau
static void build_frame(const uint8_t level[4]) {
    clear_matrix();
    for (uint8_t band = 0; band < 4; band++) {
        uint8_t lvl = level[band];
        for (uint8_t col = (uint8_t)(band * 2); col < (uint8_t)(band * 2 + 2); col++) {
            for (uint8_t row = 0; row < lvl; row++) {
                set_led_by_height(led_index(col, row), row);
            }
        }
    }
}

// ============================================================================
//  Outils matrice : conversion (colonne, ligne) -> index de LED
// ============================================================================
// La matrice est cablee en colonnes de 8 (cf. sujet, fig. ordre des LEDs) :
//   LED n°1..8   = colonne 0 (ligne 0 en bas -> ligne 7 en haut)
//   LED n°9..16  = colonne 1, etc.
// Index 0-based de la LED en colonne c (0..7), ligne r (0..7) :
static uint8_t led_index(uint8_t col, uint8_t row) {
    return (uint8_t)(col * 8u + row);
}

// Ecrit les 4 octets G/R/B/W d'une LED dans la trame
static void set_led(uint8_t idx, uint8_t g, uint8_t r, uint8_t b, uint8_t w) {
    uint16_t base = (uint16_t)idx * 4u;
    LED_MATRIX[base + OFF_G] = (char)g;
    LED_MATRIX[base + OFF_R] = (char)r;
    LED_MATRIX[base + OFF_B] = (char)b;
    LED_MATRIX[base + OFF_W] = (char)w;
}

static void clear_matrix(void) {
    for (uint16_t i = 0; i < 256; i++) {
        LED_MATRIX[i] = 0;
    }
}

// Couleur d'une LED selon sa hauteur (degrade vert -> jaune -> rouge)
static void set_led_by_height(uint8_t idx, uint8_t row) {
    if (row <= 3) {
        set_led(idx, INTENSITY, 0, 0, 0);            // vert
    } else if (row <= 5) {
        set_led(idx, INTENSITY, INTENSITY, 0, 0);    // jaune (vert + rouge)
    } else {
        set_led(idx, 0, INTENSITY, 0, 0);            // rouge
    }
}


// - Fonction main ----------------------------------------------------------------------
void main(void) {
    /* Configuration des entrées / sorties */
    // init adc / gpio
    // --- Sorties LEDs de test GPIO ---
    TRISC  = 0x00;          // RC0..RC7 = LED_0..7 en sortie
    ANSELC = 0x00;          // numerique
    LATC   = 0x00;          // eteintes

    // --- RB4 (LED_M) et RB5 (CMD_MATRIX) en sortie numerique ---
    TRISB  &= ~0x30;        // RB4, RB5 = sortie
    ANSELB &= ~0x30;        // numerique
    ODCONB &= ~0x30;        // push-pull (PAS open-drain) -> drive franc jusqu'a VDD
    LATB   &= ~0x30;        // niveau bas

    // --- Boutons BP0 = RA7, BP1 = RA6 en entree numerique ---
    TRISA  |= 0xC0;         // RA6, RA7 = entree
    ANSELA &= ~0xC0;        // numerique
    WPUA   |= 0xC0;         // pull-up internes (bouton vers GND -> 0 si appuye)


    // --- Broches analogiques AN2..AN5 (RA2..RA5) en entree analogique ---
    TRISA  |= 0x3C;         // RA2..RA5 = entree
    ANSELA |= 0x3C;         // analogique

    // --- Module ADC ---
    // ADCON0 : ADON=1 (b7) | ADCS=1 (b4, horloge FRC dediee) | ADFM=1 (b2, justifie a droite)
    // ADCON0 = 0x80 | 0x10 | 0x04;   // = 0x94
    ADCON0bits.ADON = 1;   // ON
    ADCON0bits.ADCS = 1;   // horloge FRC dédiée
    ADCON0bits.ADFM = 1;   // justifié à droite (bit 0)
    // ADREF par defaut : Vref+ = VDD, Vref- = VSS
    ADREF  = 0x00;
    // Pas d'acquisition automatique : on attend manuellement avant chaque conversion
    ADACQ  = 0x00;
    ADCON1 = 0x00;
    ADCON2 = 0x00;          // mode basique (pas de calcul/moyenne materielle)
    ADCON3 = 0x00;

    /* Petit test au demarrage : la LED master clignote (montre que c'est sous tension) */
    LED_M_ON();  __delay_ms(200);
    LED_M_OFF(); __delay_ms(200);
    LED_M_ON();

    /* Affichage matrice eteinte au depart */
    clear_matrix();
    TX_64LEDS();


    /* ========================================================================
     *  === TEST MATRICE (mettre TEST_MATRIX a 0 pour repasser au vumetre) ===
     *  Allume TEST_NLEDS LEDs avec la couleur (G,R,B,W) ci-dessous pendant 1 s,
     *  puis tout eteint pendant 1 s, en boucle.
     *
     *  La LED master (RB4) suit EXACTEMENT le meme rythme -> sert a VERIFIER
     *  L'HORLOGE au chronometre : elle doit faire 1 s ON / 1 s OFF (periode 2 s).
     *  Si la periode mesuree n'est pas 2 s, l'horloge n'est pas a 64 MHz et tout
     *  le timing du protocole est faux (cause possible du "blanc").
     *
     *  Reglages rapides (a changer "a la volee") :
     *    TEST_NLEDS   : nombre de LEDs allumees (1 = ne teste que la 1ere LED)
     *    TEST_G/R/B/W : composantes couleur (jamais 255 ; ~16-32)
     * ====================================================================== */
#define TEST_MATRIX 1           // 1 = test rouge<->noir (deterministe) ; 0 = vumetre sonore
#define TEST_NLEDS  1           // T2 : on n'allume QUE la 1ere LED (rouge)
#define TEST_G      0           // vert  (0 = test SANS vert)
#define TEST_R      INTENSITY   // rouge
#define TEST_B      0           // bleu
#define TEST_W      0           // blanc

    /* --- TEST BROCHE RB5 : verifie au MULTIMETRE que la data bascule ---
     * RB5 (pin 26) alterne ~5V / ~0V toutes les 2 s, EN PHASE avec PORTC.
     * Mesurer RB5 au multimetre (mode DC). Mettre TEST_PIN a 0 ensuite. */
#define TEST_PIN 0
#if TEST_PIN
    while (1) {
        LATB |=  0x20;   // RB5 = HAUT (~5V)
        LATC  = 0xFF;    // temoin visible PORTC
        __delay_ms(2000);
        LATB &= ~0x20;   // RB5 = BAS (~0V)
        LATC  = 0x00;
        __delay_ms(2000);
    }
#endif

    /* --- MODE OSCILLO : envoie la trame en boucle rapide pour scoper RB5 ---
     * Mettre TEST_SCOPE a 0 pour revenir au test visuel TEST_MATRIX. */
#define TEST_SCOPE 0
#if TEST_SCOPE
    for (uint8_t i = 0; i < TEST_NLEDS; i++) {
        set_led(i, TEST_G, TEST_R, TEST_B, TEST_W);
    }
    while (1) {
        TX_64LEDS();
        __delay_us(80);          // petit reset (>50us) puis on renvoie -> facile a declencher
    }
#endif

#if TEST_MATRIX
    // Test decisif : la matrice REAGIT-ELLE a nos donnees ?
    // On alterne TOUT ETEINT <-> TOUT ROUGE chaque seconde.
    while (1) {
        // Phase A : TOUT ETEINT (on envoie des zeros)
        clear_matrix();
        TX_64LEDS();
        __delay_ms(1000);

        // Phase B : TOUT ROUGE
        for (uint8_t i = 0; i < NB_LEDS; i++) {
            set_led(i, 0, INTENSITY, 0, 0);
        }
        TX_64LEDS();
        __delay_ms(1000);
    }
#endif


    /* Corps du programme : boucle vumetre ------------------------------------ */
    uint8_t level[4];
    while (1) {
        // 1) Acquisition des 4 bandes
        uint16_t v_basses  = adc_read(CH_BASSES);
        uint16_t v_bas_med = adc_read(CH_BAS_MED);
        uint16_t v_haut_med= adc_read(CH_HAUT_MED);
        uint16_t v_aigus   = adc_read(CH_AIGUS);

        // 2) Traitement : conversion en hauteur de barre
        level[0] = adc_to_level(v_basses);
        level[1] = adc_to_level(v_bas_med);
        level[2] = adc_to_level(v_haut_med);
        level[3] = adc_to_level(v_aigus);

        // --- DEBUG : niveau "basses" en barre sur les 8 LEDs PORTC ---
        uint8_t n = level[0];                       // 0..8
        LATC = (n >= 8) ? 0xFF : (uint8_t)((1u << n) - 1u);

        // 3) Construction de la trame puis envoi a la matrice (ASM)
        build_frame(level);
        TX_64LEDS();

        // 4) Cadence de rafraichissement (~50 Hz) ; laisse aussi la ligne au repos (>50us)
        __delay_ms(20);
    }

    return;
}
