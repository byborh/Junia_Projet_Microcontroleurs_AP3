# Rapport individuel — Projet Vumètre à LED (AP3 BX Électronique)

**Étudiant :** huhu (Hugo)
**Blocs principaux traités :** Architecture / trajet du signal + Filtres analogiques (4 bandes)
**Volume horaire :** 7 séances × 3 h ≈ 24 h

---

## 1. Présentation du projet

L'objectif est de visualiser le niveau sonore d'un signal audio sur une matrice de
**64 LEDs RGBW (SK6812)**, à travers une chaîne **électronique mixte** combinant
traitement analogique et traitement numérique.

Le système comporte deux grands volets, que j'ai cherché à bien distinguer dès le
début :

- **Côté analogique :** conformer le signal audio (amplification, mixage, filtrage,
  détection d'enveloppe) pour le préparer à la numérisation.
- **Côté numérique :** acquisition par l'ADC du **PIC18F25K40**, traitement, puis
  commande de la matrice LED (en C et en assembleur).

Ma contribution porte sur la **compréhension de l'architecture globale et du trajet
du signal**, ainsi que sur les **filtres analogiques** qui séparent le signal en
4 bandes de fréquences.

---

## 2. Déroulé chronologique

### Séance 1 — 13/05/2025 (3 h)
- **Compréhension du sujet** et des objectifs des premières séances.

**Ce que j'ai compris :**
- la distinction entre **objectif côté numérique** (microcontrôleur, acquisition,
  affichage) et **objectif côté analogique** (mise en forme du signal) ;
- le **trajet du signal** à travers les différents éléments de la carte :
  entrée jack → pré-amplification → mixage → **filtrage en 4 bandes** →
  détection d'enveloppe → ADC → matrice LED.

**Difficulté :** les calculs de dimensionnement à réaliser.

### Séances 2 à 7
Travail d'équipe sur le dimensionnement et l'assemblage. J'ai été accompagné par un
camarade (bebe) sur la compréhension des étages d'amplification, puis j'ai
contribué au **dimensionnement des filtres** des 4 bandes de fréquences.

---

## 3. Analyse théorique — Architecture et filtrage

### 3.1 Trajet du signal (vue d'ensemble)

```
 Jack L/R ─► Pré-ampli ─► Mixeur mono ─► ┬─► Filtre BASSES        ─► Env. ─► AN5
                                         ├─► Filtre BAS-MÉDIUMS    ─► Env. ─► AN4
                                         ├─► Filtre HAUTS-MÉDIUMS  ─► Env. ─► AN3
                                         └─► Filtre AIGUS          ─► Env. ─► AN2
                                                                          │
                                                       PIC18F25K40 (ADC)  ◄┘
                                                              │
                                                       Matrice 8×8 RGBW
```

### 3.2 Cahier des charges des filtres

| Bande | Type | Bande passante |
|---|---|---|
| Basses | Passe-bas | < 250 Hz |
| Bas-médiums | Passe-bande | 250 Hz – 1 kHz |
| Hauts-médiums | Passe-bande | 1 kHz – 4 kHz |
| Aigus | Passe-haut | > 4 kHz |

### 3.3 Loi de dimensionnement

La fréquence de coupure d'une cellule RC du premier ordre est :

$$f_c = \frac{1}{2\pi \cdot R \cdot C}$$

**Méthode pratique :** on **fixe le condensateur** (valeurs moins variées que les
résistances) et on **calcule la résistance**, puis on retient la valeur normalisée
la plus proche.

---

## 4. Valeurs retenues pour les filtres

D'après les valeurs dimensionnées (`Ressources/FiltreValue`), avec
`f_c = 1 / (2π·R·C)` :

| Filtre | R | C | Fréquence de coupure calculée |
|---|---|---|---|
| Filtre 1 — `R0`, `C0` | 680 Ω | 1 µF | `f_c ≈ 234 Hz` |
| Filtre 2 — `R1`, `C1` | 820 Ω | 47 nF | `f_c ≈ 4,13 kHz` |
| Filtre 3 — `R2`, `C2` | 680 Ω | 1 µF | `f_c ≈ 234 Hz` |
| Filtre 3 — `R3`, `C3` | 680 Ω | 0,22 µF | `f_c ≈ 1,06 kHz` |
| Filtre 4 — `R4`, `C4` | 750 Ω | 0,45 µF | `f_c ≈ 471 Hz` |
| Filtre 4 — `R5`, `C5` | 750 Ω | 47 nF | `f_c ≈ 4,52 kHz` |

**Lecture :**
- La cellule à **234 Hz** fixe la limite haute des **basses** (≈ 250 Hz) et la
  limite basse des **bas-médiums**.
- La cellule à **≈ 1,06 kHz** marque la frontière **bas-médiums / hauts-médiums**.
- Les cellules à **≈ 4,1 – 4,5 kHz** marquent la frontière **hauts-médiums /
  aigus** (≈ 4 kHz).

Les bandes passe-bande (bas-médiums, hauts-médiums) sont obtenues en **associant**
une coupure passe-haut et une coupure passe-bas (combinaison des cellules RC
ci-dessus), conformément au cahier des charges.

> *Détail du calcul (exemple filtre 1) :*
> `f_c = 1 / (2π × 680 × 1×10⁻⁶) ≈ 234 Hz` ✔ (< 250 Hz, bande basses).

---

## 5. Synthèse personnelle

Ma contribution a d'abord été de **clarifier l'architecture du système** et le
trajet du signal — étape indispensable pour que toute l'équipe comprenne le rôle de
chaque étage avant de dimensionner. J'ai ensuite participé au dimensionnement des
**filtres des 4 bandes**, en appliquant la loi `f_c = 1/(2πRC)`. La principale
difficulté initiale (les calculs à mener) a été levée grâce au travail collaboratif
et à l'accompagnement de mes camarades. Les **diagrammes de Bode expérimentaux** de
chaque filtre restent à relever à l'oscilloscope pour valider les fréquences de
coupure calculées.
