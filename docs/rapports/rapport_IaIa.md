# Rapport individuel — Projet Vumètre à LED (AP3 BX Électronique)

**Étudiant :** IaIa
**Blocs principaux traités :** Assemblage / brasage + Tests carte + Partie logicielle (C + assembleur)
**Volume horaire :** 7 séances × 3 h ≈ 24 h

---

## 1. Présentation du projet

Le projet réalise un **vumètre à LED** affichant le niveau sonore d'un signal audio
sur une matrice **8×8 RGBW (SK6812)**. La chaîne analogique (pré-ampli → mixeur →
4 filtres → détecteurs d'enveloppe) prépare 4 signaux continus, un par bande de
fréquences, qui sont ensuite acquis par l'ADC du **PIC18F25K40**. Le microcontrôleur
construit une image (hauteur de barre par bande) et l'envoie à la matrice.

La carte est alimentée en **+5 V** uniquement (valeur à ne jamais dépasser : les
composants sont fragiles).

Ma contribution porte sur la **réalisation matérielle** (brasage, dessoudage,
réparations, tests de la carte numérique) et sur la **partie logicielle** (code C
d'acquisition/affichage et code assembleur de commande de la matrice).

---

## 2. Déroulé chronologique

### Séance 1 — 13/05/2025 (3 h)
- Lecture du sujet et du schéma électrique ; **compréhension du circuit électrique**.
- Mise en place de la plateforme d'échange (GitHub, Drive) et distribution des
  droits.
- Premiers calculs RC et **brasage d'une première pièce**.

**Ce que j'ai compris :** le fonctionnement général du circuit électrique.
**Difficulté :** les calculs de **puissance des résistances**.

### Séance 2 — 22/05/2025 (3 h)
- **Brasage des composants** ; **dessoudage de broches mal positionnées**.

**Ce que j'ai compris :** comment **souder** et **dessouder** proprement un composant.
**Difficulté :** les calculs.

### Séance 3 — 27/05/2025 (3 h)
- Poursuite des **soudures**.
- Aide aux camarades sur les **calculs** et le **choix des composants**.

### Séance 4 — 29/05/2025 (3 h)
- **Reprise de certaines soudures** mal faites (vues avec le professeur).
- **Test de la carte** : microcontrôleur, LEDs, résistances — pour localiser l'origine
  d'un problème.
- **Démarrage de la partie logicielle.**

**Ce que j'ai compris :** la bonne technique de soudure (vue avec le professeur), le
fonctionnement du **générateur** et des différents **éléments de test** (oscilloscope,
multimètre, points de test TP___).

### Séances 5 à 7
Finalisation de l'assemblage, calibrage du timing de la matrice à l'oscilloscope et
intégration logicielle.

---

## 3. Réalisation matérielle

### 3.1 Bonnes pratiques d'assemblage appliquées
- Braser **en priorité** les composants du microcontrôleur pour disposer d'une
  plateforme de programmation stable.
- Ne **pas** braser directement les circuits intégrés (AOP `MCP6274`, PIC) mais
  leurs **supports** (sockets DIP14 / DIP28), puis insérer les CI.
- Respecter l'**orientation** des composants polarisés (diodes, condensateurs
  électrolytiques).
- Utiliser les **points de test `TP___`** pour mesurer à la sonde.

### 3.2 Composants à valeur imposée / numérique
| Composant | Rôle | Valeur |
|---|---|---|
| `IC0` | microcontrôleur | PIC18F25K40 |
| `OPA1`, `OPA2` | amplificateurs opérationnels | MCP6274 (quad) |
| `RMCLR1` | pull-up du reset `MCLR` | 10 kΩ |
| `RB0`, `RB1` | pull-up des boutons de test | 10 kΩ |
| `Rref1`, `Rref2` | pont de référence `Vref = 2,5 V` | 10 kΩ chacune |
| `R_LED0…7`, `R_LEDM1` | limitation des 9 LEDs de test | 680 Ω |
| `CdecMATRIX1` | découplage matrice | 10 µF |
| `CdecOPA*`, `CdecMCU1` | découplage alimentations | 100 nF |

---

## 4. Partie logicielle

### 4.1 Architecture du code
Deux langages se partagent le travail, conformément au sujet :
- **C (`main.c`)** : configuration du PIC, acquisition ADC des 4 bandes, traitement,
  construction de la trame d'affichage, gestion des LEDs de test et des boutons ;
- **Assembleur (`tx.asm`)** : commande temps-réel de la matrice par **bitbang**, car
  le protocole SK6812 impose des contraintes temporelles trop fines pour le C.

Les deux fichiers partagent, par convention de nommage (préfixe `_` côté ASM) :
- la fonction `TX_64LEDS` (définie en ASM, appelée en C) ;
- le tableau `LED_MATRIX[256]` (64 LEDs × 4 octets G/R/B/W) et son pointeur `pC`.

### 4.2 Configuration matérielle (extraits de `main.c`)
- Horloge interne **64 MHz** (`RSTOSC = HFINTOSC_64MHZ`), watchdog désactivé.
- **Brochage** :
  - `CMD_MATRIX = RB5` (data matrice, pilotée en ASM)
  - `LED_M = RB4` (LED « power » / témoin sous tension)
  - `LED_0…7 = RC0…RC7` (8 LEDs de test sur PORTC)
  - `BP0 = RA7`, `BP1 = RA6` (boutons, pull-up internes activés)
  - Entrées ADC : basses = `AN5` (RA5), bas-médiums = `AN4`, hauts-médiums = `AN3`,
    aigus = `AN2`.

### 4.3 Boucle principale
1. **Acquisition** des 4 canaux ADC (0–1023).
2. **Conversion** de chaque valeur en hauteur de barre (0–8 LEDs).
3. **Construction de la trame** : chaque bande occupe 2 colonnes ; dégradé de
   couleur selon la hauteur (vert → jaune → rouge).
4. **Envoi** à la matrice via `TX_64LEDS()`.
5. Rafraîchissement à **~50 Hz** (`__delay_ms(20)`), qui laisse aussi la ligne au
   repos > 50 µs (signal « reset » de la matrice).

> ⚠️ **Intensité des LEDs** : ne jamais envoyer la valeur `255` (risque d'endommager
> la matrice et d'impacter tous les groupes). On utilise `INTENSITY = 24`
> (de l'ordre de quelques dizaines), comme recommandé dans le sujet.

### 4.4 Protocole de commande de la matrice (`tx.asm`)
Bitbang « 1 fil » sur `RB5`. À `FOSC = 64 MHz`, `Tcy = 62,5 ns`, on vise une
période de bit de **1,25 µs ≈ 20 cycles** :
- **bit 0** : niveau haut ≈ 0,32 µs (~5 cycles) puis bas ;
- **bit 1** : niveau haut ≈ 0,82 µs (~13 cycles) puis bas ;
- **MSB en premier**, ordre des octets **G, R, B, W**.

> Les délais (nombre de `NOP`) sont **nominaux** et doivent être **calibrés à
> l'oscilloscope** sur RB5 (mesure de la largeur d'un bit 0 et d'un bit 1, tolérance
> ≈ ±150 ns). Ce sont précisément les **oscillogrammes** demandés dans le livrable.

---

## 5. Synthèse personnelle

J'ai été le moteur de la **réalisation matérielle** : brasage, reprise des soudures
défectueuses (avec le professeur), et **diagnostic de la carte numérique**
(microcontrôleur, LEDs, résistances) à l'aide du générateur et des instruments de
test. Sur le plan logiciel, j'ai démarré la mise en route du code (C + assembleur).
Ma principale difficulté est restée le **calcul de la puissance des résistances** ;
en revanche j'ai gagné en autonomie sur la **technique de soudure** et l'usage des
**appareils de mesure**, compétences essentielles pour valider la carte étage par
étage.
