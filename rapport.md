# RAPPORT PROJET ÉLECTRONIQUE

**ISEN 3 AP — JUNIA BX**
**Projet : Vumètre à LED (matrice 8×8 RGBW)**
**Équipe : 4 étudiants** — 7 séances × 3 h ≈ 24 h

---

## Sommaire

1. [Objectifs](#1-objectifs)
2. [Présentation du sujet](#2-présentation-du-sujet)
3. [Analyse générale du circuit](#3-analyse-générale-du-circuit)
4. [Préamplificateur](#4-préamplificateur)
5. [Mixeur stéréo](#5-mixeur-stéréo)
6. [Filtres analogiques](#6-filtres-analogiques)
7. [Détecteur d'enveloppe](#7-détecteur-denveloppe)
8. [Polarisation : pont diviseur Vref et pull-up](#8-polarisation--pont-diviseur-vref-et-pull-up)
9. [Partie numérique — logiciel (C + assembleur)](#9-partie-numérique--logiciel-c--assembleur)
10. [Assemblage et brasage de la carte](#10-assemblage-et-brasage-de-la-carte)
11. [Conclusion](#11-conclusion)
12. [Annexe — tableau des composants](#12-annexe--tableau-des-composants)

---

## 1. Objectifs

- Comprendre, mettre au point, assembler et tester un montage électronique dont la fonction
  peut aisément être mise en évidence.
- Connaître les principes de fonctionnement et les montages de base des principaux composants
  élémentaires (diodes et amplificateurs opérationnels).
- Programmer un microcontrôleur.
- Implémenter un système électronique dont l'architecture est représentative des systèmes
  électroniques mixtes actuels :
  - Conformation d'un signal analogique en préparation à un traitement numérique ;
  - Traitement numérique du signal ;
  - Restitution de l'information numérique.

---

## 2. Présentation du sujet

Le projet consiste à concevoir le circuit de traitement et d'affichage du niveau sonore d'un
signal audio sous la forme de vu-mètre lumineux :

- Le signal audio d'entrée est prélevé et distribué vers
  1. l'étage de filtrage de la carte, et
  2. une sortie audio permettant de relier des enceintes actives.
- Le signal est ensuite séparé à l'aide de filtres analogiques en **4 bandes de fréquences** :
  basses, bas-médiums, hauts-médiums et aigus.
- Enfin, un **microcontrôleur PIC18F25K40** prend en charge l'acquisition des signaux
  correspondant aux différentes bandes de fréquences et commande une matrice de **64 LEDs RGBW**
  (Red / Green / Blue / White) adressables individuellement, afin d'afficher en temps réel le
  niveau sonore dans chaque bande de fréquence.

> ⚠️ La carte est alimentée en **+5 V uniquement** (alimentation simple). Cette valeur ne doit
> jamais être dépassée : les composants sont fragiles. Tous les signaux audio (alternatifs) sont
> polarisés autour d'une référence `Vref = Vcc/2 = 2,5 V`.

---

## 3. Analyse générale du circuit

Le circuit est composé des éléments suivants (cf. *Figure 1 : Schéma de principe du projet*) :

- **Noir** : la partie analogique du circuit.
- **Blanc** : le pré-amplificateur.
- **Vert** : les filtres analogiques.
- **Bleu** : le microcontrôleur, qui réalise la conversion analogique-numérique, traite les
  données via le code en C, puis forme le message de commande des LEDs.

Tout à gauche se trouve l'entrée audio ; tout à droite, la sortie audio, les LEDs de test, la
matrice de LEDs et le pull-up.

### Trajet du signal

```
 Jack L/R ─► Pré-ampli ─► Mixeur mono ─► ┬─► Filtre BASSES        ─► Env.1 ─► AN5
                                         ├─► Filtre BAS-MÉDIUMS    ─► Env.2 ─► AN4
                                         ├─► Filtre HAUTS-MÉDIUMS  ─► Env.3 ─► AN3
                                         └─► Filtre AIGUS          ─► Env.4 ─► AN2
                                                                          │
                                                       PIC18F25K40 (ADC)  ◄┘
                                                              │
                                            Traitement C  ─►  Trame ASM  ─►  Matrice 8×8 RGBW
```

Le signal entre par l'entrée audio, passe par les amplificateurs droit et gauche (lignes vertes),
puis par le mixeur (lignes jaunes). En sortie du mixeur, le signal alimente les 4 blocs de filtres
(lignes rouges), polarisés par `Vref` issu du pré-amplificateur (ligne magenta). À la sortie des
filtres, le signal passe dans les détecteurs d'enveloppe (lignes bleu foncé), qui lissent le signal
pour limiter le bruit perçu. Le signal entre ensuite dans le microcontrôleur (ligne cyan) ; après
conversion, traitement et formation du message, il est envoyé aux LEDs et à la matrice (lignes grises).

| Bloc | Domaine | Langage |
|------|---------|---------|
| Pré-ampli, mixeur, filtres, détecteurs d'enveloppe | Analogique | — |
| Conversion A/N, traitement, LEDs de test | Numérique | **C** |
| Commande de la matrice SK6812 RGBW | Numérique critique (temps réel) | **Assembleur** |

---

## 4. Préamplificateur

Le préamplificateur a pour tâche de mettre en forme le signal reçu pour respecter deux conditions :

1. La **fréquence de coupure** du filtre de couplage réalisé est d'environ **20 Hz** (fréquence
   limite basse de l'audition humaine), afin de bloquer la composante continue de la source sans
   atténuer les basses utiles.
2. Le **gain statique** de la fonction amplifie la tension d'entrée crête-à-crête (`Vccin`, mesurée)
   vers une sortie `Vccout` d'environ **5 V** (tension d'alimentation du circuit), centrée sur `Vref`.

### 4.1 Montage

Le montage est un **amplificateur non-inverseur couplé en alternatif** (un étage par canal :
gauche et droite) :

```
Audio_IN ──Cpre──┬──[+] AOP ───► OUT_PREAMP
                 │
              (vers Vref = 2,5 V)
              [−]──Rpre1──┐
                          ├──► sortie (contre-réaction)
        Vref ──Rpre0──────┘
```

- Le condensateur `Cpre` bloque la composante continue de la source et forme, avec la résistance
  d'entrée, un **filtre passe-haut** dont la coupure reste sous la bande audio utile.
- Le signal est re-centré autour de `Vref = 2,5 V` pour exploiter toute la dynamique 0 – 5 V de l'ADC.

### 4.2 Gain et coupure

Gain d'un montage non-inverseur :

$$G = 1 + \frac{R_{pre1}}{R_{pre0}}$$

Fréquence de coupure du couplage d'entrée :

$$f_c = \frac{1}{2\pi \cdot R_{pre0} \cdot C_{pre}}$$

### 4.3 Valeurs retenues

| Composant | Canal | Rôle | Valeur |
|---|---|---|---|
| `Rpre0`, `Rpre2` | gauche / droite | résistance de masse (entrée −) | **8,2 kΩ** |
| `Rpre1`, `Rpre3` | gauche / droite | contre-réaction | **47 kΩ** |
| `Cpre0`, `Cpre1` | gauche / droite | couplage d'entrée | **1 µF** |

**Tension d'entrée mesurée à l'oscilloscope :** `Vccin ≈ 904 mV`.

**Gain obtenu :**

$$G = 1 + \frac{47\,\text{k}\Omega}{8,2\,\text{k}\Omega} \approx 1 + 5,73 \approx \mathbf{6,7}$$

Ce gain d'environ **6,7** ramène un signal de l'ordre de 0,9 V (`Vccin`) à une amplitude
exploitable centrée sur `Vref`, sans saturer contre les rails 0 V / +5 V.

**Coupure du couplage :** avec `Cpre = 1 µF` et `Rpre0 = 8,2 kΩ` :

$$f_c = \frac{1}{2\pi \cdot 8\,200 \cdot 1\times10^{-6}} \approx 19,4\ \text{Hz}$$

soit ≈ 20 Hz, sous la bande audio : les basses fréquences sont conservées. ✔

---

## 5. Mixeur stéréo

### 5.1 Rôle

La musique arrive en stéréo (canaux **gauche** et **droite**), mais le vumètre ne traite qu'une
seule voie. Il faut donc fusionner les deux signaux en un signal **mono**, sans risque pour la
source audio. On utilise pour cela un **amplificateur opérationnel monté en sommateur inverseur**,
qui additionne les tensions de manière isolée et sécurisée. Un potentiomètre `Pmix0` (10 kΩ, imposé)
agit comme réglage de **balance** (pan) entre les deux canaux.

### 5.2 Montage et formule

Le signal gauche pré-amplifié (`OUT_PREAMP_L`) traverse `Rmix0 + Rmix1`, le signal droit
(`OUT_PREAMP_R`) traverse `Rmix2 + Rmix3`. Les deux se rejoignent sur l'entrée inverseuse (broche 6)
de l'AOP, qui se comporte comme une **masse virtuelle** (théorème de Millman). La tension de sortie
théorique est :

$$V_{out\_mix} = -\left( \frac{R_{mix4}}{R_{mix0} + R_{mix1}} \cdot V_L + \frac{R_{mix4}}{R_{mix2} + R_{mix3}} \cdot V_R \right)$$

### 5.3 Démarche de dimensionnement

Le cahier des charges (vérificateur automatique) impose deux contraintes simultanées :

- **Potentiomètre au centre** : chaque canal est divisé par deux pour ne pas saturer l'AOP à la
  somme → cible **gain ≈ 0,5** (tolérance 0,375 – 0,625).
- **Potentiomètre à fond d'un côté** : le canal sélectionné doit exploiter toute la dynamique →
  cible **gain ≈ 1,0** (tolérance 0,80 – 1,20).

Un premier essai (`Rmix0…3 = 4,7 kΩ`, `Rmix4 = 10 kΩ`) **échouait** : il négligeait la fuite vers
la masse virtuelle introduite par le potentiomètre `Pmix0` de 10 kΩ. La vraie équation, au centre,
comporte un terme au carré :

$$Gain_{centre} = \frac{R_{mix4}}{2 \cdot R_{mix0} + \dfrac{R_{mix0}^2}{5000}}$$

Avec 4,7 kΩ et 10 kΩ, on obtenait **0,72** au lieu de 0,5 (signal saturé → ❌).

**Solution :** atténuer l'influence du potentiomètre en augmentant les résistances fixes
(`Rmix0…3 = 22 kΩ`), puis compenser le freinage du signal en augmentant la contre-réaction
(`Rmix4 = 82 kΩ`, valeur normalisée E12). On obtient alors :

- au centre : gain ≈ **0,58** → dans la cible 0,5 ± 25 % ✔
- à fond d'un côté : gain ≈ **0,88** → dans la cible 1,0 ± 20 % ✔

### 5.4 Valeurs retenues

| Composant | Rôle | Valeur |
|---|---|---|
| `Rmix0`, `Rmix1` | branche d'entrée canal gauche | **22 kΩ** chacune |
| `Rmix2`, `Rmix3` | branche d'entrée canal droit | **22 kΩ** chacune |
| `Rmix4` | contre-réaction (fixe le gain) | **82 kΩ** |
| `Pmix0` | potentiomètre (balance) | 10 kΩ *(imposé)* |

---

## 6. Filtres analogiques

Le montage possède 4 filtres ayant des rôles différents : bloquer certaines fréquences et en
laisser passer d'autres. Ils découpent le spectre en 4 catégories :

- Bande **basses** : `< 250 Hz`
- Bande **bas-médiums** : `250 Hz – 1 kHz`
- Bande **hauts-médiums** : `1 kHz – 4 kHz`
- Bande **aigus** : `> 4 kHz`

### 6.1 Loi de dimensionnement

La fréquence de coupure d'une cellule RC du premier ordre est :

$$f_c = \frac{1}{2\pi \cdot R \cdot C}$$

**Méthode pratique :** on **fixe le condensateur** (valeurs moins variées que les résistances) et
on **calcule la résistance**, puis on retient la valeur normalisée la plus proche (série E12). Les
bandes intermédiaires (bas-médiums, hauts-médiums) sont des **passe-bande** obtenus en associant
une coupure passe-haut et une coupure passe-bas.

### 6.2 Valeurs retenues et fréquences de coupure calculées

| Filtre | Type | Bande | R | C | `fc` calculée |
|---|---|---|---|---|---|
| **Filtre 1** — `R0`,`C0` | passe-bas | basses | 680 Ω | 1 µF | **≈ 234 Hz** |
| **Filtre 3** — `R2`,`C2` | passe-haut (cellule basse) | bas-médiums | 680 Ω | 1 µF | **≈ 234 Hz** |
| **Filtre 3** — `R3`,`C3` | passe-bas (cellule haute) | bas-médiums | 680 Ω | 0,22 µF | **≈ 1,06 kHz** |
| **Filtre 4** — `R4`,`C4` | passe-haut (cellule basse) | hauts-médiums | 750 Ω | 0,22 µF | **≈ 965 Hz** |
| **Filtre 4** — `R5`,`C5` | passe-bas (cellule haute) | hauts-médiums | 750 Ω | 47 nF | **≈ 4,52 kHz** |
| **Filtre 2** — `R1`,`C1` | passe-haut | aigus | 820 Ω | 47 nF | **≈ 4,13 kHz** |

**Lecture :**

- Le **Filtre 1** (≈ 234 Hz, passe-bas) fixe la limite haute des **basses** (cahier des charges : 250 Hz).
- Le **Filtre 3** combine 234 Hz (passe-haut) et 1,06 kHz (passe-bas) → passe-bande **bas-médiums**
  (≈ 250 Hz – 1 kHz).
- Le **Filtre 4** combine 965 Hz (passe-haut) et 4,52 kHz (passe-bas) → passe-bande **hauts-médiums**
  (≈ 1 – 4 kHz).
- Le **Filtre 2** (≈ 4,13 kHz, passe-haut) marque le début des **aigus** (> 4 kHz).

> *Exemple de calcul (Filtre 1)* :
> $f_c = \dfrac{1}{2\pi \times 680 \times 1\times10^{-6}} \approx 234\ \text{Hz}$ ✔ (< 250 Hz).

> 📈 **Livrable associé :** le **diagramme de Bode expérimental** de chaque filtre est à relever à
> l'oscilloscope (balayage en fréquence au générateur) pour valider les fréquences de coupure calculées.

---

## 7. Détecteur d'enveloppe

### 7.1 Rôle

Un détecteur d'enveloppe est un « traducteur » qui transforme un signal électrique variant très
vite (le son) en une **tension continue** beaucoup plus lente, représentant le **volume**
(l'amplitude) du son. Il permet de retranscrire fidèlement le niveau sonore au microcontrôleur,
sans faux positifs. Il est constitué d'**une diode** + **un circuit RC** :

- la **diode** (1N4148, `D1…D4`) ne laisse passer que les alternances positives : elle **redresse**
  le signal ;
- le **condensateur** se charge quand le son monte et se décharge quand il descend ; la **résistance**
  contrôle la vitesse de décharge.

Une décharge trop rapide rend le signal bruité/saccadé ; trop lente, le signal devient « mou » et
peu réactif à la musique.

### 7.2 Dimensionnement

La constante de temps `τ = R × C` doit être :

- **assez grande** pour lisser les oscillations de la plus basse fréquence de la bande considérée ;
- **assez petite** pour suivre les variations de volume (attaque / relâchement).

Règle pratique appliquée : `τ ≥ 20 ms` ou la plus petite période des fréquences concernées, en
retenant la plus grande des deux valeurs.

### 7.3 Valeurs retenues *(BOM final, annexe)*

| Bande | Diode | Re | Ce | `τ = R·C` |
|---|---|---|---|---|
| Enveloppe 1 — basses | 1N4148 | 10 kΩ | 22 µF | **≈ 220 ms** |
| Enveloppe 2 — bas-médiums | 1N4148 | 1 kΩ | 33 µF | **≈ 33 ms** |
| Enveloppe 3 — hauts-médiums | 1N4148 | 3,3 kΩ | 22 µF | **≈ 73 ms** |
| Enveloppe 4 — aigus | 1N4148 | 3,3 kΩ | 33 µF | **≈ 109 ms** |

> *Réglage final affiné à l'oscilloscope : si le vumètre « saute » trop → augmenter τ ; s'il est
> trop mou → diminuer τ.* Les sorties (`OUT_ENV_1…4`) sont les tensions continues lues par l'ADC
> sur `AN5`, `AN4`, `AN3`, `AN2`.

---

## 8. Polarisation : pont diviseur Vref et pull-up

### 8.1 Pont diviseur de référence (`Rref1`, `Rref2`)

Le pont fournit la tension de référence `Vref = +2,5 V` (Vcc/2) qui polarise le signal audio
alternatif au milieu de l'alimentation simple +5 V :

$$V_{ref} = V_{cc} \cdot \frac{R_{ref2}}{R_{ref1} + R_{ref2}}$$

Pour `Vref = Vcc/2`, il faut `Rref1 = Rref2`. Le cahier des charges impose en outre un courant de
fuite `I_DC < 100 µA`. Avec deux résistances de 10 kΩ, on aurait `I = 5/20\,000 = 250\ µA` → **trop
fort**. On augmente donc la valeur :

$$R_{total} > \frac{5\,\text{V}}{100\,µA} = 50\ \text{k}\Omega \implies R_{ref} > 25\ \text{k}\Omega$$

**Valeur retenue : `Rref1 = Rref2 = 47 kΩ`** (E12, marge confortable).

- Tension obtenue : $5 \times \dfrac{47\text{k}}{47\text{k}+47\text{k}} = \mathbf{2,5\ V}$ ✔
- Courant consommé : $\dfrac{5}{94\,000} = \mathbf{53,2\ µA} < 100\ µA$ ✔

Un condensateur de découplage (`CdecOPA*`, 100 nF) filtre le bruit de l'alimentation sur cette référence.

### 8.2 Pull-up des boutons (`RB0`, `RB1`)

Les boutons de test sont câblés entre le GPIO et la masse, avec une résistance de pull-up vers +5 V :
au repos le GPIO lit `1`, bouton appuyé il est tiré à `0`. La valeur doit être assez faible pour
limiter la sensibilité au bruit, mais assez grande pour limiter la consommation.

**Valeur retenue : `RB0 = RB1 = 20 kΩ`** → courant bouton appuyé `= 5/20\,000 = 0,25 mA` (négligeable).

> *Remarque :* le PIC18F25K40 dispose de pull-up internes ; ils sont d'ailleurs activés en parallèle
> dans le code (`WPUA`, voir §9.2).

---

## 9. Partie numérique — logiciel (C + assembleur)

### 9.1 Architecture du code

Conformément au sujet, le travail est partagé entre deux langages :

- **C (`main.c`)** : configuration du PIC, acquisition ADC des 4 bandes, traitement, construction de
  la trame d'affichage, gestion des LEDs de test et des boutons ;
- **Assembleur (`tx.asm`)** : commande temps-réel de la matrice par **bitbang**, car le protocole
  SK6812 impose des contraintes temporelles trop fines pour le C.

Les deux fichiers partagent, par convention de nommage (préfixe `_` côté ASM) :

| ASM | C |
|---|---|
| `_TX_64LEDS` | `void TX_64LEDS(void)` |
| `_LED_MATRIX` | `volatile char LED_MATRIX[256]` |
| `_pC` | `volatile const char * pC` |

`TX_64LEDS` est définie en ASM et appelée en C ; `LED_MATRIX[256]` (64 LEDs × 4 octets G/R/B/W)
est définie en C et lue en ASM.

### 9.2 Configuration matérielle (`main.c`)

- Horloge **interne 64 MHz** (`RSTOSC = HFINTOSC_64MHZ`), oscillateur externe coupé, watchdog désactivé.
- **Brochage :**
  - `CMD_MATRIX = RB5` (data matrice, pilotée en ASM)
  - `LED_M = RB4` (LED « power » / témoin sous tension)
  - `LED_0…7 = RC0…RC7` (8 LEDs de test sur PORTC)
  - `BP0 = RA7`, `BP1 = RA6` (boutons, pull-up internes activés via `WPUA`)
  - Entrées ADC : basses = `AN5` (RA5), bas-médiums = `AN4`, hauts-médiums = `AN3`, aigus = `AN2`.
- **ADC :** `ADON = 1`, horloge FRC dédiée (`ADCS = 1`), résultat justifié à droite (`ADFM = 1`),
  référence `Vref+ = VDD`, `Vref- = VSS`. Résolution 10 bits → valeurs 0…1023.

### 9.3 Boucle principale

1. **Acquisition** des 4 canaux ADC (0–1023) :
   `adc_read()` sélectionne le canal, attend l'acquisition (`__delay_us(5)`), lance la conversion
   (`ADGO = 1`) et attend sa fin.
2. **Conversion** de chaque valeur en hauteur de barre (0–8 LEDs) :
   `adc_to_level()` → `lvl = value × 9 / 1024`, borné à 8.
3. **Construction de la trame** (`build_frame`) : chaque bande occupe **2 colonnes** ; la hauteur de
   la barre correspond au niveau, avec un dégradé de couleur :
   - lignes 0–3 → **vert**, lignes 4–5 → **jaune**, lignes 6–7 → **rouge**.
4. **Envoi** à la matrice via `TX_64LEDS()`.
5. **Rafraîchissement** à ~50 Hz (`__delay_ms(20)`), ce qui laisse aussi la ligne data au repos
   plus de 50 µs (signal « reset » de la matrice).

La matrice est câblée en colonnes de 8 LEDs ; l'index d'une LED est `led_index(col, row) = col×8 + row`.
La LED « master » (`LED_M`) clignote au démarrage pour signaler la mise sous tension, et les 8 LEDs
de PORTC affichent en barre le niveau de la bande « basses » (aide au débogage).

> ⚠️ **Intensité des LEDs :** ne jamais envoyer la valeur `255` (courant excessif, risque
> d'endommagement de la matrice). On utilise `INTENSITY = 24` (quelques dizaines), comme recommandé
> dans le sujet.

### 9.4 Protocole de commande de la matrice (`tx.asm`)

La matrice SK6812 RGBW utilise un protocole **« 1 fil »** : chaque bit est encodé par la durée de
l'état haut d'une impulsion de période fixe. À `FOSC = 64 MHz`, un cycle d'instruction vaut
`Tcy = 62,5 ns`. La période d'un bit visée est de **1,25 µs ≈ 20 cycles** :

| Bit | État haut visé | Cycles haut (réalisés) | Durée réalisée |
|---|---|---|---|
| **0** | ≈ 0,32 µs | 5 cycles | ≈ 0,31 µs |
| **1** | ≈ 0,82 µs | 13 cycles | ≈ 0,81 µs |

- Données envoyées **MSB en premier**, ordre des octets **G, R, B, W** (et non R-G-B-W comme la
  datasheet le suggérerait).
- 64 LEDs × 32 bits = **2048 bits** par trame.

La routine est **entièrement déroulée bit par bit** (bits 7 à 0) pour garantir un timing constant.
Le bit de poids fort est extrait par `RLCF TABLAT` (rotation à gauche, le bit sort dans le *Carry*),
puis `BTFSS STATUS,C` aiguille vers le chemin « 1 » (13 cycles hauts) ou « 0 » (5 cycles hauts), le
reste étant comblé par des `NOP` pour atteindre exactement 20 cycles. L'octet suivant est rechargé
**pendant l'état bas du dernier bit** (`MOVF POSTINC0`) pour ne pas perturber le chronogramme.

> 🔬 **Calibrage / livrable :** les délais (nombre de `NOP`) sont **nominaux** et doivent être
> **calibrés à l'oscilloscope** sur la broche `RB5` : on mesure la largeur d'un bit 0 et d'un bit 1
> (tolérance ≈ ±150 ns). Ce sont précisément les **oscillogrammes** demandés dans le livrable.

---

## 10. Assemblage et brasage de la carte

### 10.1 Bonnes pratiques appliquées

- Braser **en priorité** la partie microcontrôleur (alimentation, support DIP28, connecteur de
  programmation, LEDs de test, boutons) pour disposer d'une plateforme de programmation stable.
- Ne **jamais** braser directement les circuits intégrés (AOP `MCP6274`, PIC) mais leurs **supports**
  (sockets DIP14 / DIP28), puis insérer les CI.
- Respecter l'**orientation** des composants polarisés (diodes, condensateurs électrolytiques).
- Utiliser les **points de test `TP___`** pour mesurer à la sonde (multimètre / oscilloscope), et
  tester chaque étage au **générateur de fonctions** (signal contrôlé) plutôt qu'avec un téléphone.
- Vérifier au multimètre l'absence de court-circuit `+5 V ↔ GND` avant toute mise sous tension.

La carte expose des points de test permettant de valider la chaîne **étage par étage** :

| Point de test | Mesure |
|---|---|
| `TPIL1`, `TPIR1` | entrées audio gauche / droite (avant pré-ampli) |
| `TPOL1`, `TPOR1` | sorties pré-amplificateur gauche / droite |
| `TPM1` | sortie du mixeur (signal mono) |
| `TPREF1` | tension de référence `Vref = 2,5 V` |
| `TPE1…TPE4` | sorties des 4 filtres |
| `TPF1…TPF4` | sorties des 4 détecteurs d'enveloppe (tensions lues par l'ADC) |
| `JGND0…JGND3` | masses (continuité) |

### 10.2 Déroulé du projet (synthèse des séances)

| Séance | Date | Travaux réalisés |
|---|---|---|
| S1 | 13/05/2025 | Lecture du sujet et du schéma, compréhension du circuit, mise en place GitHub/Drive, premiers calculs RC, brasage d'une première pièce. |
| S2 | 22/05/2025 | Calculs (pré-ampli, mixage, filtres) ; brasage des composants ; **dessoudage** de broches mal positionnées (coûteux en temps). |
| S3 | 27/05/2025 | Finalisation de calculs (pré-ampli, mixage stéréo) ; poursuite des soudures ; entraide sur le choix des composants ; début de la réflexion logicielle. |
| S4 | 29/05/2025 | Reprise de soudures défectueuses (avec le professeur) ; **test de la carte** (microcontrôleur, LEDs, résistances) ; prise en main des instruments (générateur, oscilloscope, multimètre) ; démarrage de la partie logicielle. |
| S5 | 03/06/2025 | Mesure de `Vccin` à l'oscilloscope ; brasage des boutons et des RC ; cours sur les concepts logiciels ; **code en C** ; brasage du mixeur stéréo et ajustement de ses valeurs. |
| S6–S7 | 05–10/06/2025 | Finalisation de l'assemblage, calibrage du timing de la matrice à l'oscilloscope, **intégration logicielle complète (C + assembleur)** et brasage final des composants. |

Une leçon pratique majeure : une **erreur de placement de broches** coûte très cher en temps de
dessoudage — d'où l'intérêt de valider chaque étage avant le brasage définitif.

---

## 11. Conclusion

Ce projet nous a permis de concevoir, dimensionner, assembler et programmer un système électronique
mixte complet : une chaîne **analogique** (pré-amplification → mixage mono → filtrage en 4 bandes →
détection d'enveloppe) qui conforme le signal audio, suivie d'une chaîne **numérique** (acquisition
ADC → traitement en C → commande temps-réel de la matrice en assembleur) qui restitue l'information
sous forme de vumètre lumineux.

Sur le plan **théorique**, nous avons appliqué les lois fondamentales des montages à amplificateur
opérationnel (gain non-inverseur, sommateur inverseur, théorème de Millman) et des filtres du premier
ordre (`fc = 1/2πRC`), en passant systématiquement de la formule au **choix des valeurs normalisées E12**.
Plusieurs dimensionnements ont dû être **corrigés à l'épreuve de la pratique** : le mixeur (prise en
compte de la fuite du potentiomètre, passage de 4,7 kΩ/10 kΩ à 22 kΩ/82 kΩ) et le pont de référence
(passage de 10 kΩ à 47 kΩ pour respecter `I < 100 µA`).

Sur le plan **pratique**, nous avons progressé en technique de soudure/dessoudage, en diagnostic de
carte et en usage des instruments de mesure (générateur, oscilloscope, multimètre, points de test).
La partie logicielle nous a fait manipuler la configuration d'un PIC18, son ADC, et surtout
l'**interfaçage C ↔ assembleur** avec un protocole temps-réel exigeant (SK6812, ~20 cycles par bit).

**Livrables restant à finaliser pour la validation :** les **diagrammes de Bode expérimentaux** de
chaque filtre et les **oscillogrammes** de calibrage du timing de la matrice sur `RB5`.

La principale difficulté commune, levée par le travail d'équipe et l'accompagnement du professeur,
aura été la **méthode de dimensionnement** : comprendre quoi faire des résultats de calcul une fois
ceux-ci posés, et les confronter à la réalité de la carte.

---

## 12. Annexe — tableau des composants

| Désignation | Qté | Montage | Valeur / Réf |
|---|---|---|---|
| B0, B1 | 2 | Traversant | 1825910-6 Tact. Switch |
| C0 | 1 | Traversant | 1 µF |
| C1 | 1 | Traversant | 47 nF |
| C2 | 1 | Traversant | 1 µF |
| C3 | 1 | Traversant | 0,22 µF |
| C4 | 1 | Traversant | 0,22 µF |
| C5 | 1 | Traversant | 47 nF |
| CdecMATRIX1 | 1 | Traversant | 10 µF |
| CdecOPA2, CdecMCU1, CdecOPA1 | 3 | Traversant | 100 nF |
| Ce1, Ce2, Ce3, Ce4 | 4 | Traversant | 22 µF, 33 µF, 22 µF, 33 µF |
| Cpre0, Cpre1 | 2 | Traversant | 1 µF, 1 µF |
| D0 | 1 | CMS | B340LA-13-F |
| D1, D2, D3, D4 | 4 | Traversant | 1N4148 |
| IC0 | 1 | Traversant | PIC18F25K40 |
| IC0-SOCKET | 1 | Traversant | Socket DIP28 0.3" |
| J_ALIM1 | 1 | Traversant | N/R |
| J_JACK_IN1, J_JACK_OUT1 | 2 | Traversant | JYO-39-5P switched |
| J3, J4 | 2 | Traversant | N/R |
| LD0…LD7, LDM1 | 9 | CMS | LG R971 |
| OPA1, OPA2 | 2 | Traversant | MCP6274 |
| OPA1-SOCKET, OPA2-SOCKET | 2 | Traversant | Socket DIP14 0.3" |
| Pmix0 | 1 | Traversant | 10 kΩ |
| R_LED0…R_LED7, R_LEDM1 | 9 | CMS | 680 Ω |
| R0 | 1 | Traversant | 680 Ω |
| R1 | 1 | Traversant | 820 Ω |
| R2 | 1 | Traversant | 680 Ω |
| R3 | 1 | Traversant | 680 Ω |
| R4 | 1 | Traversant | 750 Ω |
| R5 | 1 | Traversant | 750 Ω |
| RB0, RB1 | 2 | Traversant | 20 kΩ, 20 kΩ |
| Re1, Re2, Re3, Re4 | 4 | Traversant | 10 kΩ, 1 kΩ, 3,3 kΩ, 3,3 kΩ |
| RMCLR1 | 1 | Traversant | 10 kΩ |
| Rmix0, Rmix1 | 2 | Traversant | 22 kΩ, 22 kΩ |
| Rmix2, Rmix3 | 2 | Traversant | 22 kΩ, 22 kΩ |
| Rmix4 | 1 | Traversant | 82 kΩ |
| Rpre0, Rpre2 | 2 | Traversant | 8,2 kΩ, 8,2 kΩ |
| Rpre1, Rpre3 | 2 | Traversant | 47 kΩ, 47 kΩ |
| Rref1, Rref2 | 2 | Traversant | 47 kΩ, 47 kΩ |
| JGND0-3, TPE1-4, TPF1-4, TPIL1, TPIR1, TPM1, TPOL1, TPOR1, TPREF1 | 18 | Traversant | N/R (points de test / masse) |
| Visserie — Entretoise | 4 | N/A | N/R |
| Visserie — Vis | 4 | N/A | N/R |
| **TOTAL** | **102** | | |

> Les valeurs en **gras** dans le corps du rapport (pré-ampli, mixeur, filtres, enveloppes, Vref,
> pull-up) correspondent aux composants dimensionnés par l'équipe ; les autres (CI, sockets,
> connecteurs, LEDs, diode Schottky d'alimentation) étaient imposés par le sujet.
