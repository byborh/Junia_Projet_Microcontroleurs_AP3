# Rapport individuel — Projet Vumètre à LED (AP3 BX Électronique)

**Étudiant :** bebe
**Bloc principal traité :** Étage de pré-amplification
**Volume horaire :** 7 séances × 3 h ≈ 24 h

---

## 1. Présentation du projet

Le projet consiste à concevoir le circuit de traitement et d'affichage du niveau
sonore d'un signal audio sous forme de vumètre lumineux. Le signal d'entrée
(jack stéréo) est :

1. **pré-amplifié** sur chaque canal (gauche / droite),
2. **mixé** en mono par un sommateur inverseur,
3. **séparé en 4 bandes de fréquences** par des filtres analogiques
   (basses < 250 Hz, bas-médiums 250 Hz – 1 kHz, hauts-médiums 1 kHz – 4 kHz,
   aigus > 4 kHz),
4. **redressé et lissé** par des détecteurs d'enveloppe,
5. **acquis** par l'ADC du microcontrôleur **PIC18F25K40**, qui pilote ensuite
   une matrice **8×8 LEDs RGBW SK6812**.

La carte est alimentée en **+5 V** uniquement (alimentation simple), ce qui impose
de polariser tous les signaux audio autour d'une tension de référence
`Vref = Vcc/2 = 2,5 V`.

Ma contribution porte principalement sur **l'étage de pré-amplification**, premier
maillon de la chaîne analogique.

---

## 2. Déroulé chronologique

### Séance 1 — 13/05/2025 (3 h)
- Lecture du sujet et du schéma électrique, **compréhension globale du projet**.
- Mise en place des outils de travail collaboratif : dépôt **GitHub**, **Drive**,
  distribution des droits à l'équipe.
- Premiers pas sur le dimensionnement des composants RC.
- Brasage d'une première pièce.

**Ce que j'ai compris :** la notion de signaux qui « passent » en bas ou en haut
des fréquences (filtrage passe-bas / passe-haut).
**Difficulté :** la méthode pour mener les calculs de dimensionnement.

### Séance 2 — 22/05/2025 (3 h)
- Attaque des **calculs du pré-amplificateur**. Première tentative non aboutie,
  mais la **méthode de calcul** est désormais acquise.

**Ce que j'ai compris :** comment calculer les résistances et condensateurs à
partir des lois du montage (gain d'un AOP, fréquence de coupure d'un couplage RC).
**Difficulté :** que faire des résultats une fois les calculs terminés
(choix des valeurs normalisées, validation en pratique).

### Séance 3 — 27/05/2025 (3 h)
- Finalisation des **calculs du pré-amplificateur**.
- Aide apportée à Hugo (huhu) sur la compréhension des amplificateurs.
- Démarrage de la réflexion sur la partie logicielle.

**Ce que j'ai compris :** le fonctionnement d'un amplificateur opérationnel en
montage non-inverseur.
**Difficulté :** ce qu'il faut concrètement réaliser pour la partie software.

### Séances 4 à 7
Poursuite du projet en équipe (assemblage final, tests, intégration logicielle).

---

## 3. Analyse théorique — Étage de pré-amplification

### 3.1 Rôle
Le signal délivré par une source audio (sortie casque/jack) est de faible amplitude
(de l'ordre de la centaine de mV crête-à-crête) et **centré sur 0 V** (alternatif).
Or l'ADC du PIC ne mesure qu'entre 0 et +5 V. L'étage de pré-amplification doit donc :

- **amplifier** le signal pour exploiter au mieux la dynamique de l'ADC ;
- **coupler en alternatif** (condensateur de liaison `Cpre`) pour bloquer toute
  composante continue de la source ;
- **re-centrer** le signal autour de `Vref = 2,5 V` (point milieu de l'alimentation).

Il y a un étage identique par canal : **gauche** (Rpre0, Rpre1, Cpre0) et
**droite** (Rpre2, Rpre3, Cpre1).

### 3.2 Montage et gain
Le montage est un **amplificateur non-inverseur couplé en alternatif** :

```
Audio_IN ──Cpre──┬──[+] AOP ───► OUT_PREAMP
                 │
               (vers Vref)
              [−]──Rpre1──┐
                          ├──► sortie (contre-réaction)
        Vref ──Rpre0──────┘
```

Le gain en tension d'un montage non-inverseur est :

$$G = 1 + \frac{R_{pre1}}{R_{pre0}}$$

Le condensateur de liaison `Cpre` forme avec la résistance d'entrée un **filtre
passe-haut** dont la fréquence de coupure doit rester **sous la bande audio utile**
(typiquement < 20 Hz) pour ne pas atténuer les basses :

$$f_c = \frac{1}{2\pi \cdot R \cdot C_{pre}}$$

---

## 4. Calculs et valeurs retenues

| Composant | Canal | Valeur retenue |
|---|---|---|
| `Rpre0`, `Rpre2` | résistance de masse (entrée −) | **8,2 kΩ** |
| `Rpre1`, `Rpre3` | contre-réaction | **47 kΩ** |
| `Cpre0`, `Cpre1` | couplage d'entrée | **1 µF** |

**Tension d'entrée de référence mesurée :** `Vccin ≈ 904 mV`.

### Gain obtenu
$$G = 1 + \frac{R_{pre1}}{R_{pre0}} = 1 + \frac{47\,\text{k}\Omega}{8,2\,\text{k}\Omega} \approx 1 + 5,73 \approx 6,7$$

Ce gain d'environ **6,7** permet de ramener un signal d'entrée de l'ordre de
~0,9 V (`Vccin`) à une amplitude exploitable centrée sur `Vref`, sans saturer
contre les rails 0 V / +5 V.

### Fréquence de coupure du couplage
Avec `Cpre = 1 µF` et une résistance vue de l'ordre de quelques kΩ, la fréquence de
coupure basse est de l'ordre de quelques Hz à quelques dizaines de Hz, donc
**en dessous de la bande audio** : les basses fréquences sont conservées. ✔

---

## 5. Synthèse personnelle

J'ai pris en main l'étage d'entrée de la chaîne analogique. La principale
difficulté a été de passer de la **formule théorique** au **choix des valeurs
normalisées réelles**, puis de comprendre l'enchaînement des étages
(préampli → mixeur → filtres). J'ai également contribué à la mise en place des
outils collaboratifs de l'équipe et accompagné un camarade sur la partie
amplification.
