/* --------------------------------------------------------------
 * Fichier     : main.c
 * Description : Vumetre 4 bandes sur matrice SK6812 (64 LEDs)
 * -------------------------------------------------------------- */

#include <xc.h>
#include <stdint.h>

#pragma config FEXTOSC = OFF
#pragma config RSTOSC  = HFINTOSC_64MHZ   // horloge interne 64 MHz
#pragma config WDTE    = OFF
#pragma config MCLRE   = EXTMCLR
#pragma config LVP     = OFF

#define _XTAL_FREQ 64000000

// Canaux ADC (detecteurs d'enveloppe)
#define CH_BASSES   5   // AN5
#define CH_BAS_MED  4   // AN4
#define CH_HAUT_MED 3   // AN3
#define CH_AIGUS    2   // AN2

#define NB_LEDS    64
#define BAR_HEIGHT 8
#define INTENSITY  24   // jamais 255 ! (~16-32)

// Plage utile de l'ADC mappee sur les 8 niveaux :
//   NOISE_FLOOR = valeur au REPOS (silence) -> barre a 0
//   SIGNAL_MAX  = valeur au son FORT         -> barre au max (rouge)
// A AJUSTER : si ca ne monte jamais au rouge -> BAISSE SIGNAL_MAX (+ de gain).
//             si ca sature trop vite          -> AUGMENTE SIGNAL_MAX.
//             si ca reste allume au silence    -> AUGMENTE NOISE_FLOOR.
#define NOISE_FLOOR 350
#define SIGNAL_MAX  600

// Ordre des octets en memoire = ordre d'envoi : G, R, B, W
#define OFF_G 0
#define OFF_R 1
#define OFF_B 2
#define OFF_W 3

// Interface C <-> ASM
extern void TX_64LEDS(void);
volatile char LED_MATRIX[256];
volatile const char *pC = LED_MATRIX;   // adresse de LED_MATRIX pour l'asm

// 1 = petit test couleur de la matrice au demarrage ; 0 = direct vumetre
#define STARTUP_TEST 1

static uint16_t adc_read(uint8_t ch);
static uint8_t  adc_to_level(uint16_t v);
static void     build_frame(const uint8_t level[4]);
static uint8_t  led_index(uint8_t col, uint8_t row);
static void     set_led(uint8_t idx, uint8_t g, uint8_t r, uint8_t b, uint8_t w);
static void     clear_matrix(void);
static void     fill_matrix(uint8_t g, uint8_t r, uint8_t b, uint8_t w);
static void     set_led_by_height(uint8_t idx, uint8_t row);

static uint16_t adc_read(uint8_t ch) {
    ADPCH = ch;
    __delay_us(5);
    ADCON0bits.ADGO = 1;
    while (ADCON0bits.ADGO) { ; }
    return (uint16_t)(((uint16_t)ADRESH << 8) | ADRESL);
}

static uint8_t adc_to_level(uint16_t v) {
    if (v <= NOISE_FLOOR) return 0;                  // silence -> barre a 0
    uint16_t span  = (uint16_t)(v - NOISE_FLOOR);    // partie utile du signal
    uint16_t range = (uint16_t)(SIGNAL_MAX - NOISE_FLOOR);
    uint8_t lvl = (uint8_t)(((uint32_t)span * (BAR_HEIGHT + 1u)) / range);
    if (lvl > BAR_HEIGHT) lvl = BAR_HEIGHT;          // sature au max (rouge)
    return lvl;
}

static uint8_t led_index(uint8_t col, uint8_t row) {
    return (uint8_t)(col * 8u + row);
}

static void set_led(uint8_t idx, uint8_t g, uint8_t r, uint8_t b, uint8_t w) {
    uint16_t base = (uint16_t)idx * 4u;
    LED_MATRIX[base + OFF_G] = (char)g;
    LED_MATRIX[base + OFF_R] = (char)r;
    LED_MATRIX[base + OFF_B] = (char)b;
    LED_MATRIX[base + OFF_W] = (char)w;
}

static void clear_matrix(void) {
    for (uint16_t i = 0; i < 256; i++) LED_MATRIX[i] = 0;
}

static void fill_matrix(uint8_t g, uint8_t r, uint8_t b, uint8_t w) {
    for (uint8_t i = 0; i < NB_LEDS; i++) set_led(i, g, r, b, w);
}

// Couleur selon la hauteur : vert (bas) -> jaune -> rouge (haut)
static void set_led_by_height(uint8_t idx, uint8_t row) {
    if (row <= 3)      set_led(idx, INTENSITY, 0, 0, 0);          // vert
    else if (row <= 5) set_led(idx, INTENSITY, INTENSITY, 0, 0);  // jaune
    else               set_led(idx, 0, INTENSITY, 0, 0);          // rouge
}

// Chaque bande occupe 2 colonnes ; hauteur = niveau
static void build_frame(const uint8_t level[4]) {
    clear_matrix();
    for (uint8_t band = 0; band < 4; band++) {
        uint8_t lvl = level[band];
        for (uint8_t col = (uint8_t)(band * 2); col < (uint8_t)(band * 2 + 2); col++)
            for (uint8_t row = 0; row < lvl; row++)
                set_led_by_height(led_index(col, row), row);
    }
}

void main(void) {
    // --- GPIO ---
    TRISC = 0x00; ANSELC = 0x00; LATC = 0x00;                          // PORTC = LEDs de test (sorties)
    TRISB &= ~0x30; ANSELB &= ~0x30; ODCONB &= ~0x30; LATB &= ~0x30;   // RB4 (LED_M), RB5 (data) push-pull
    TRISA |= 0xC0; ANSELA &= ~0xC0; WPUA |= 0xC0;                      // RA6/RA7 = boutons (pull-up)
    TRISA |= 0x3C; ANSELA |= 0x3C;                                     // RA2..RA5 = analogiques

    // --- ADC ---
    ADCON0bits.ADON = 1;   // ON
    ADCON0bits.ADCS = 1;   // horloge FRC dediee
    ADCON0bits.ADFM = 1;   // justifie a droite
    ADREF = 0x00; ADACQ = 0x00; ADCON1 = 0x00; ADCON2 = 0x00; ADCON3 = 0x00;

    // matrice eteinte au depart
    clear_matrix();
    TX_64LEDS();

#if STARTUP_TEST
    // Test visuel : si la matrice decode bien, tu verras tout vert, puis rouge, puis bleu
    fill_matrix(INTENSITY, 0, 0, 0); TX_64LEDS(); __delay_ms(600);   // tout VERT
    fill_matrix(0, INTENSITY, 0, 0); TX_64LEDS(); __delay_ms(600);   // tout ROUGE
    fill_matrix(0, 0, INTENSITY, 0); TX_64LEDS(); __delay_ms(600);   // tout BLEU
    clear_matrix();                  TX_64LEDS(); __delay_ms(300);
#endif

    // --- Boucle vumetre ---
    uint8_t level[4];
    while (1) {
        level[0] = adc_to_level(adc_read(CH_BASSES));
        level[1] = adc_to_level(adc_read(CH_BAS_MED));
        level[2] = adc_to_level(adc_read(CH_HAUT_MED));
        level[3] = adc_to_level(adc_read(CH_AIGUS));

        // debug : niveau des basses sur les 8 LEDs PORTC
        uint8_t n = level[0];
        LATC = (n >= 8) ? 0xFF : (uint8_t)((1u << n) - 1u);

        build_frame(level);
        TX_64LEDS();
        __delay_ms(20);
    }
}
