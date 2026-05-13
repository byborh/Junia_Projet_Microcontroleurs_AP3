# Guide du Projet — Vumètre à LED (AP3 BX 2023-2024)

Document de travail synthétisant le sujet [2023-2024_AP3_vumetre.pdf](2023-2024_AP3_vumetre.pdf).
Objectif : concevoir un vumètre lumineux affichant le niveau sonore d'un signal audio sur 4 bandes de fréquences, via une matrice 8x8 de LEDs RGBW pilotée par un PIC18F25K40.

---

## 1. Vue d'ensemble du système

Chaîne de traitement :

```
Entrée audio → Pré-amplification → Filtres analogiques (4 bandes)
            → Détecteurs d'enveloppe → ADC du PIC18F25K40
            → Traitement (C) → Formation trame (ASM) → Matrice 8x8 RGBW
                                                    → Sortie audio (enceintes)
```

| Bloc | Domaine | Langage |
|------|---------|---------|
| Pré-ampli, filtres, détecteurs d'enveloppe | Analogique | — |
| Conversion A/N, traitement, GPIO test | Numérique | **C** |
| Communication matrice SK6812RGBW | Numérique critique | **Assembleur** |

Bandes de fréquences imposées :
- Basses : `< 250 Hz`
- Bas-médiums : `250 Hz – 1 kHz`
- Hauts-médiums : `1 kHz – 4 kHz`
- Aigus : `> 4 kHz`

Alimentation unique **+5 V** — à ne JAMAIS dépasser (composants fragiles).

---

## 2. TODOLIST — Partie HARDWARE

### 2.1 Préparation & étude
- [ ] Lire le [schématique](schematic.pdf) et annoter chaque bloc fonctionnel
- [ ] Identifier chaque étage : pré-ampli, filtres (LP/BP/HP), détecteurs d'enveloppe, mixage
- [ ] Repérer les Test Points (`TP___`) sur la carte
- [ ] Lister tous les composants à dimensionner (cases vides dans [annexe V](2023-2024_AP3_vumetre.pdf) — `R0..R5`, `Re1..Re4`, `Rmix*`, `Rpre*`, `Rref*`, `Ce1..Ce4`, `Cpre0/1`, `C0..C5`, `RB0/RB1`, résistances de LEDs)

### 2.2 Dimensionnement (théorique + simulation)
- [ ] Pré-amplification : calculer gain à partir de `Rpre0..Rpre3` et `Cpre0/Cpre1`
- [ ] Filtre passe-bas (basses) — fc ≈ 250 Hz
- [ ] Filtre passe-bande (bas-médiums) — 250 Hz / 1 kHz
- [ ] Filtre passe-bande (hauts-médiums) — 1 kHz / 4 kHz
- [ ] Filtre passe-haut (aigus) — fc ≈ 4 kHz
- [ ] Détecteurs d'enveloppe (diodes `D1..D4` 1N4148 + `Ce*` + `Re*`) : choisir constante de temps RC
- [ ] Tension de référence `Vref` (pont diviseur `Rref1/Rref2`) — centrer le signal autour de 2,5 V
- [ ] Résistances de limitation des LEDs de test (`R_LED*`) à partir de la datasheet [LG R971](Datasheet%20-%20LEDS%20LG%20R971.pdf)
- [ ] Résistances de pull-up/pull-down boutons (`RB0/RB1`)

### 2.3 Validation sur platine d'essai
- [ ] Tester chaque filtre individuellement au générateur de fonctions
- [ ] Tracer le **diagramme de Bode expérimental** de chaque filtre (à inclure au rapport)
- [ ] Vérifier la sortie des détecteurs d'enveloppe (signal DC proportionnel à l'amplitude)

### 2.4 Assemblage final (dans l'ordre)
- [ ] **Braser EN PREMIER** : alimentation, socket microcontrôleur (DIP28), connecteur PicKIT, LEDs de test, boutons → plateforme de prog stable
- [ ] **EN DERNIER** : composants analogiques (après dimensionnement validé)
- [ ] Braser uniquement les **supports** (sockets) pour OPA1, OPA2, IC0 — jamais les CI directement
- [ ] Respecter l'orientation des **diodes** (`D0..D4`) et **condensateurs polarisés**
- [ ] Vérifier au multimètre l'absence de court-circuit `+5V ↔ GND` avant mise sous tension

### 2.5 Tests post-assemblage
- [ ] Test alimentation à vide (multimètre)
- [ ] Continuité GND sur tous les `JGND*`
- [ ] Programmer un chenillard simple → valider la chaîne PicKIT + PIC + LEDs
- [ ] Mesurer signal à chaque TP (TPE1..TPE4 = sortie filtres, TPF1..TPF4 = sortie enveloppe, etc.)

---

## 3. TODOLIST — Partie SOFTWARE

### 3.1 Configuration du PIC18F25K40
- [ ] Récupérer les templates `main.c` et `tx.asm` fournis
- [ ] Configurer les `#pragma config` (oscillateur, WDT, MCLR, LVP…)
- [ ] Configurer l'horloge interne (FOSC) — choix lié au timing du protocole LED
- [ ] Configurer les ports GPIO (LEDs de test en sortie, boutons en entrée)

### 3.2 Acquisition analogique (C)
- [ ] Initialiser l'**ADC** du PIC (résolution, Vref, horloge de conversion)
- [ ] Sélectionner les 4 canaux analogiques (sortie détecteurs d'enveloppe)
- [ ] Lire séquentiellement les 4 bandes — stocker dans un tableau
- [ ] (Optionnel) Lissage / moyenne glissante sur N échantillons

### 3.3 Traitement (C)
- [ ] Mapper la valeur ADC (0–1023 ou 0–255) vers un niveau 0–8 (hauteur de barre)
- [ ] Construire le tableau de 64 LEDs × 4 octets (G/R/B/W) — partagé avec l'asm
- [ ] Choisir un dégradé de couleurs par niveau (ex: vert → jaune → rouge)
- [ ] **Limiter les intensités à ~16-32, JAMAIS 255** (risque endommagement matrice)

### 3.4 Communication matrice (ASM — bitbanging)
- [ ] Implémenter `TX_64LEDS` dans `tx.asm`
- [ ] Respecter le timing **SK6812RGBW** :
  - Bit `0` : T_high = 0,32 µs / T_total = 1,25 µs
  - Bit `1` : T_high = 0,82 µs / T_total = 1,25 µs
- [ ] Envoyer 2048 bits par trame (64 LEDs × 32 bits)
- [ ] **Ordre des couleurs : G → R → B → W** (cf. remarque du sujet, contre-intuitif vs datasheet)
- [ ] Calibrer les délais (`nop`) à l'**oscilloscope** sur la broche `CMD_MATRIX`

### 3.5 Interface C ↔ ASM
- [ ] Déclarer la fonction `TX_64LEDS` comme `extern` côté C
- [ ] Déclarer le tableau partagé suivant les conventions XC8 (linker / section)
- [ ] Tester un appel minimal (allumer LED 1 en rouge)

### 3.6 Fonctionnalités bonus (si temps)
- [ ] LED "power on"
- [ ] Bouton B0 : changer de mode d'affichage (bande seule / max / moyen)
- [ ] Bouton B1 : pause de l'affichage
- [ ] Affichage du niveau **max** persistant (peak-hold) en plus du niveau instantané

---

## 4. Connaissances nécessaires

### 4.1 Analogique
- **Amplificateur opérationnel** : montages non-inverseur, inverseur, suiveur, sommateur — datasheet [MCP6274](Datasheet%20-%20MCP6274.pdf) (rail-to-rail, single-supply)
- **Filtres actifs** : Sallen-Key, structure multiple-feedback (MFB), ordre 2 — calcul de `fc`, `Q`, gain
- **Diagramme de Bode** : tracé théorique vs expérimental
- **Détecteur d'enveloppe (peak detector)** : diode + RC, choix de τ vs fréquence min du signal
- **Polarisation alim simple** : décalage du signal AC autour de Vref = Vcc/2
- **Diodes** : Schottky (B340LA, faible Vf) vs silicium (1N4148, signal)

### 4.2 Numérique / microcontrôleur
- Architecture PIC18, jeu d'instructions, **datasheet** [PIC18F25K40](Datasheet%20-%20PIC18\(L\)F24_25K40.pdf)
- **ADC** : Sample & Hold, Tacq, Tad, Vref interne/externe
- **GPIO** : TRIS, LAT, ANSEL
- **MPLAB X + XC8** : projet, build, programmation via PicKIT — [doc XC8](Documentation%20-%20MPLAB%20XC8.pdf)
- **Assembleur PIC18** : `movlw`, `movwf`, `bsf`/`bcf`, `nop`, boucles — [doc PIC ASM](Documentation%20-%20PIC%20Assembly.pdf)
- **Interop C/ASM** : conventions de nommage, passage d'arguments via WREG, sections mémoire

### 4.3 Protocole NeoPixel / SK6812RGBW
- Encodage **PWM-like** sur un seul fil
- Sensibilité au timing (~±150 ns toléré)
- Pas de horloge — la fin de trame est marquée par un état bas prolongé (>50 µs typique, "reset")

---

## 5. Bonnes pratiques

### 5.1 Méthodologie
- **Approche par blocs** : valider un étage avant de passer au suivant. Ne pas tout brancher d'un coup.
- **Documenter au fil de l'eau** : prendre photos, scope captures, calculs — pas la veille du rendu.
- **Versionner le code** : git, commits courts, message clair (déjà initialisé dans ce repo).
- **Tester avec un générateur de fonctions** (sinus pur), pas un téléphone — signal contrôlable.

### 5.2 Dimensionnement
- Toujours partir des **contraintes** (fc, gain) → calculer → choisir parmi les valeurs E12/E24 disponibles
- Pour un filtre actif Sallen-Key : fixer C, calculer R (les capas existent en moins de valeurs)
- Tableau récapitulatif comportement vs composants (utile au rapport)

### 5.3 Assemblage / brasage
- Souder bas → haut (composants les plus plats en premier)
- Vérifier polarité (diodes : bande = cathode ; condos électrochim : barre = `-`)
- Pas de CI directement soudé — toujours via socket DIP
- Nettoyer le flux après brasage

### 5.4 Programmation
- Code en blocs : fonction `init_*()` séparées (`init_adc`, `init_gpio`, `init_uart`…)
- Constantes en `#define` (broches, seuils, gain) — pas de magic numbers
- **Tester l'ASM à l'oscilloscope** : capturer un bit 0 et un bit 1, mesurer les largeurs
- Commenter le code ASM ligne à ligne (les `nop` notamment : indiquer le délai cumulé)

### 5.5 Sécurité matérielle
- **JAMAIS** `255` sur une couleur de LED matrice → courant excessif, risque d'endommagement
- Mettre hors tension avant tout (dé)branchement de connecteur
- Vérifier alim à vide avant insertion du PIC dans son socket

---

## 6. Risques & pièges courants

| Risque | Prévention |
|---|---|
| Saturation du pré-ampli sur signal audio fort | Gain modeste, prévoir réglage `Pmix0` |
| Filtres qui se chevauchent / bandes mal séparées | Choisir Q raisonnable (~0,7 pour Butterworth), tracer Bode |
| Détecteur d'enveloppe trop lent → suit pas le signal | Réduire τ = Re·Ce ; trop rapide → ondulation |
| ADC qui lit du bruit | Câblage propre, condo de découplage proche du PIC, Vref stable |
| Trame LED instable / scintillement | Timing ASM hors tolérance → ajuster `nop` à l'oscillo |
| Inversion G/R | Respecter ordre **G-R-B-W** (pas R-G-B-W de la datasheet !) |
| Matrice qui chauffe / casse | Limiter chaque couleur à ≤ 32 |
| Diode/condo dans le mauvais sens | Vérification visuelle systématique avant mise sous tension |

---

## 7. Livrables (rappel du sujet)

À remettre en fin de projet :
- [ ] **Prototype physique** de la carte fonctionnelle
- [ ] **Archive ZIP** du code, propre et commenté (C + ASM)
- [ ] **Rapport écrit** contenant impérativement :
  - Analyse du circuit (rôle de chaque étage)
  - Analyse théorique de chaque bloc
  - Calculs de dimensionnement
  - Diagramme de Bode expérimental de chaque filtre
  - Oscillogrammes validant la commande de la matrice LED
  - Tableau récap des composants utilisés + leurs valeurs

---

## 8. Planning indicatif (7 séances)

| Séance | Objectif |
|---|---|
| 1 | Analyse du schéma, identification des blocs, début dimensionnement |
| 2 | Fin dimensionnement filtres + simulation, brasage partie numérique |
| 3 | Validation filtres sur platine, prog PIC (LEDs test + boutons) |
| 4 | ADC + lecture 4 bandes, début ASM `TX_64LEDS` |
| 5 | Calibrage timing ASM à l'oscilloscope, intégration C/ASM |
| 6 | Brasage analogique, intégration complète, mesures Bode |
| 7 | Finitions, oscillogrammes pour rapport, bonus, finalisation rapport |

---

## 9. Références documentaires (dans `docs/`)

- [Sujet complet](2023-2024_AP3_vumetre.pdf)
- [Schéma électrique](schematic.pdf) — [PCB couleur](pcb_color.pdf)
- [Datasheet PIC18F25K40](Datasheet%20-%20PIC18\(L\)F24_25K40.pdf)
- [Datasheet MCP6274 (quad AOP)](Datasheet%20-%20MCP6274.pdf)
- [Datasheet LEDs LG R971 (LEDs de test)](Datasheet%20-%20LEDS%20LG%20R971.pdf)
- [Doc MPLAB XC8](Documentation%20-%20MPLAB%20XC8.pdf)
- [Doc PIC Assembly](Documentation%20-%20PIC%20Assembly.pdf)
- Datasheet SK6812RGBW : à récupérer en ligne (non fournie)
