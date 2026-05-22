# Calculs des résistances — Vumètre AP3

Fiche méthodologique pour dimensionner toutes les résistances non fixées de la carte.
Référence : [schéma électrique](schematic.pdf), [liste composants](2023-2024_AP3_vumetre.pdf).

> **Principe général** : pour chaque résistance, on (1) écrit la loi physique applicable (Ohm, pont diviseur, gain AOP, fc d'un filtre…), (2) on isole R, (3) on choisit la valeur **normalisée la plus proche** dans la série E12 ou E24.

**Rappel série E12** (les plus courantes) : `1 / 1,2 / 1,5 / 1,8 / 2,2 / 2,7 / 3,3 / 3,9 / 4,7 / 5,6 / 6,8 / 8,2` × 10ⁿ Ω

---

## 1. Résistances de limitation des LEDs de test (R_LED0…R_LED7, R_LEDM1)

**Composant** : LED rouge SMD [LG R971](Datasheet%20-%20LEDS%20LG%20R971.pdf)
**Montage** : `+5V → R_LED → LED → GPIO (drain bas pour allumer)` ou l'inverse.

### Loi : Ohm sur la résistance série

```
V_R = V_alim − V_F(LED)
R = V_R / I_LED
```

### Données datasheet LG R971 (à vérifier)
- `V_F` (tension directe LED rouge) ≈ **2,0 V** typique @ 20 mA
- `I_F` (courant nominal) = 20 mA max ; pour signalisation **5 mA suffisent largement**

### Calcul
Avec `V_alim = 5V`, `V_F = 2V`, `I = 5 mA` :

```
R = (5 − 2) / 0,005 = 600 Ω
```

→ **Valeur normalisée : 560 Ω ou 680 Ω** (E12). Prendre **680 Ω** pour rester sous 5 mA.

Si tu veux plus de luminosité à 10 mA :
```
R = 3 / 0,010 = 300 Ω  →  330 Ω (E12)
```

### Vérification puissance
```
P = V_R × I = 3 V × 5 mA = 15 mW  →  résistance 1/4 W (250 mW) largement OK
```

**Choix recommandé : R_LED = 680 Ω** (5 mA, sobre, suffisant).

---

## 2. Résistances pull-up des boutons (RB0, RB1)

**Montage typique** : bouton entre GPIO et GND, résistance entre GPIO et +5V (**pull-up**).
Au repos GPIO lit `1` ; bouton appuyé → GPIO tiré à `0`.

### Critères
- **Pas trop faible** : sinon courant inutile quand bouton appuyé (`I = 5V / R`)
- **Pas trop forte** : sinon le GPIO est sensible au bruit (impédance haute = antenne)

### Plage usuelle : 4,7 kΩ à 100 kΩ
- 10 kΩ est le **standard** (bon compromis bruit/conso)
- Avec 10 kΩ → courant bouton appuyé = `5/10000 = 0,5 mA` → négligeable

**Choix recommandé : RB0 = RB1 = 10 kΩ** (cohérent avec `RMCLR1 = 10 kΩ` déjà imposé).

> Astuce : le PIC18F25K40 a des **pull-up internes** activables par registre `WPUx`. Si tu les utilises, RB0/RB1 deviennent inutiles — mais le sujet semble vouloir des pull-up externes (sinon les emplacements n'existeraient pas).

---

## 3. Pont diviseur de référence Vref (Rref1, Rref2)

**Rôle** : créer une tension `Vref = +2,5 V` (Vcc/2) pour polariser le signal AC audio au milieu de l'alim simple +5V. C'est l'entrée `+` de chaque AOP.

### Loi : pont diviseur
```
Vref = Vcc × Rref2 / (Rref1 + Rref2)
```

Pour `Vref = Vcc/2` → **Rref1 = Rref2**.

### Choix de la valeur
- **Trop faible** (1 kΩ) : conso permanente importante `5V / 2kΩ = 2,5 mA`
- **Trop forte** (1 MΩ) : impédance de source trop élevée, bruyante, et le condo de découplage met trop de temps à se charger

**Choix recommandé : Rref1 = Rref2 = 10 kΩ** (impédance vue = 5 kΩ, conso = 0,25 mA).

> **Important** : prévoir `CdecOPA1`/`CdecOPA2` (100 nF) en parallèle sur Rref2 pour filtrer le bruit de l'alim. C'est déjà sur le schéma.

---

## 4. Pré-amplification (Rpre0, Rpre1, Rpre2, Rpre3 + Cpre0, Cpre1)

**Montage** : AOP en **non-inverseur AC-couplé** (un par canal L et R), suivi d'un mixage.

### Topologie typique
```
Audio_IN → Cpre → [+] AOP → OUT_PREAMP
                  [−]──Rpre1──┐
                              ├── Vref
                  GND─Rpre0───┘
```
- `Rpre0` : résistance de masse (entre `−` et Vref ou GND virtuelle)
- `Rpre1` : résistance de contre-réaction (entre sortie et `−`)
- `Cpre` : couplage AC d'entrée

### Gain (non-inverseur)
```
G = 1 + Rpre1 / Rpre0
```

### Cible du gain
- Signal jack audio classique : amplitude ≈ **0,1 à 1 Vpp** (variable selon source)
- Plage ADC du PIC : 0 à 5 V → on veut un signal de sortie autour de **2 à 3 Vpp** centré sur Vref
- Donc gain visé : **G ≈ 3 à 5**

### Exemple de calcul (gain = 4)
```
G = 4  →  Rpre1/Rpre0 = 3
Choix : Rpre0 = 10 kΩ  →  Rpre1 = 30 kΩ  (E12 le plus proche : 33 kΩ → G = 4,3)
```

**Choix recommandé** :
- `Rpre0 = Rpre2 = 10 kΩ`
- `Rpre1 = Rpre3 = 33 kΩ` (gain ≈ 4,3)

### Capa de couplage Cpre
```
fc_coupling = 1 / (2π × Rpre0 × Cpre)
```
On veut `fc_coupling < 20 Hz` (sous la bande audio basse) :
```
Cpre > 1 / (2π × 10000 × 20) ≈ 800 nF  →  Cpre = 1 µF
```

---

## 5. Filtres analogiques (R0…R5, C1…C5)

**Topologie probable** : 4 filtres actifs Sallen-Key d'ordre 1 ou 2, un par bande (basses, bas-médiums, hauts-médiums, aigus).

### Formule générique d'un Sallen-Key passe-bas ordre 2 avec R1=R2=R, C1=C2=C
```
fc = 1 / (2π × R × C)
```

### Cibles (cahier des charges)
| Bande | Type | fc (Hz) |
|---|---|---|
| Basses | Passe-bas | 250 |
| Bas-médiums | Passe-bande | 250 – 1000 |
| Hauts-médiums | Passe-bande | 1000 – 4000 |
| Aigus | Passe-haut | 4000 |

### Méthode pratique : **on fixe C, on calcule R**

(les valeurs de condos sont moins variées que celles des résistances)

**Exemple — passe-bas 250 Hz** :
Choix C = 100 nF
```
R = 1 / (2π × fc × C) = 1 / (2π × 250 × 100e-9) = 6,37 kΩ
```
→ **R = 6,8 kΩ** (E12) → fc effective = 234 Hz ✓

**Exemple — passe-haut 4 kHz** :
Choix C = 10 nF
```
R = 1 / (2π × 4000 × 10e-9) = 3,98 kΩ
```
→ **R = 3,9 kΩ** (E12) → fc effective = 4080 Hz ✓

### Tableau récapitulatif suggéré
| Filtre | C choisi | R calculé | R normalisé (E12) | fc réelle |
|---|---|---|---|---|
| LP 250 Hz | 100 nF | 6,37 kΩ | 6,8 kΩ | 234 Hz |
| HP 250 Hz (bas-med) | 100 nF | 6,37 kΩ | 6,8 kΩ | 234 Hz |
| LP 1 kHz (bas-med) | 22 nF | 7,23 kΩ | 6,8 kΩ | 1063 Hz |
| HP 1 kHz (haut-med) | 22 nF | 7,23 kΩ | 6,8 kΩ | 1063 Hz |
| LP 4 kHz (haut-med) | 10 nF | 3,98 kΩ | 3,9 kΩ | 4080 Hz |
| HP 4 kHz (aigus) | 10 nF | 3,98 kΩ | 3,9 kΩ | 4080 Hz |

> **À vérifier sur ton schéma** : la topologie exacte (Sallen-Key, MFB, RC passif…) impacte la formule. Si tu vois 2R + 2C par filtre, c'est probablement Sallen-Key et la formule ci-dessus tient.

---

## 6. Détecteurs d'enveloppe (Re1…Re4, Ce1…Ce4 + diodes D1…D4)

**Montage** : diode 1N4148 en série + RC parallèle vers GND. Sortie = enveloppe AC redressée et lissée → tension DC quasi-continue lue par l'ADC.

### Loi : constante de temps RC
```
τ = Re × Ce
```

### Critère de dimensionnement
La constante τ doit être :
- **Assez grande** pour lisser les oscillations de la plus basse fréquence de la bande : `τ >> 1/fmin`
- **Assez petite** pour suivre les variations d'amplitude du signal audio (attaque/release) : `τ < 50 ms` typiquement

### Calcul par bande
Pour la bande basses (fmin ≈ 50 Hz pour être prudent) :
```
τ ≥ 10 × 1/50 = 200 ms ?  →  trop lent, on prend τ ≈ 20 ms (compromis)
Avec Ce = 1 µF  →  Re = τ/Ce = 20e-3 / 1e-6 = 20 kΩ
```

→ **Re1 = 22 kΩ, Ce1 = 1 µF** pour les basses.

Pour les aigus (fmin = 4 kHz, signal qui varie vite) :
```
τ ≈ 5 ms suffit
Avec Ce = 100 nF  →  Re = 5e-3 / 100e-9 = 50 kΩ
```

→ **Re4 = 47 kΩ, Ce4 = 100 nF** pour les aigus.

**Choix recommandé** (point de départ) :
| Bande | Re | Ce | τ |
|---|---|---|---|
| Basses | 22 kΩ | 1 µF | 22 ms |
| Bas-médiums | 22 kΩ | 470 nF | 10 ms |
| Hauts-médiums | 22 kΩ | 220 nF | 5 ms |
| Aigus | 22 kΩ | 100 nF | 2 ms |

> À affiner à l'oscilloscope : si le vumètre "saute" trop → augmenter τ ; s'il est mou → diminuer.

---

## 7. Sommateur de mixage (Rmix0…Rmix4, Pmix0)

**Rôle** : remettre ensemble L et R en mono (audio mixé renvoyé vers la sortie enceintes et/ou vers les filtres).

### Loi : AOP sommateur inverseur
```
Vout = − (Rmix4/Rmix0 × VL + Rmix4/Rmix1 × VR)
```

Pour un mix L+R équilibré et gain unitaire :
```
Rmix0 = Rmix1 = Rmix4
```

**Choix recommandé : Rmix0 = Rmix1 = Rmix4 = 10 kΩ**.

Pour `Rmix2` et `Rmix3` : ce sont probablement les résistances équivalentes côté entrée filtres — même logique, **10 kΩ** est un bon défaut tant que tu n'as pas identifié leur rôle exact sur le schéma.

`Pmix0 = 10 kΩ` est imposé (potentiomètre, sans doute le réglage de volume global).

---

## 8. Méthode de travail recommandée

1. **Avant de souder** : remplir un tableau Excel/papier avec ces 3 colonnes :
   `Référence | Valeur calculée | Valeur E12 choisie | fc/G réel`
2. **Tester sur platine d'essai** chaque étage avec un GBF sinus :
   - Pré-ampli : injecter 100 mVpp à 1 kHz → mesurer la sortie → vérifier le gain
   - Chaque filtre : balayer en fréquence → tracer le **Bode** (gain dB vs log f)
   - Détecteur d'enveloppe : signal modulé → vérifier que la sortie suit l'enveloppe
3. **Ajuster les valeurs** si nécessaire (souvent on remplace une R par une voisine pour cadrer le résultat)
4. **Seulement ensuite** souder sur la carte définitive

---

## 9. Outils de calcul rapide

### Calculateur en ligne
- Sallen-Key : https://www.ti.com/tool/FILTERPRO ou https://tools.analog.com/en/filterwizard/
- Diviseur de tension : https://ohmslawcalculator.com/voltage-divider-calculator

### Tableur (recommandé pour le rapport)
Une feuille avec :

| Étage | Formule | Variable | C (fixé) | R calculé | E12 retenu | Erreur % |
|---|---|---|---|---|---|---|
| LP 250Hz | `R = 1/(2π·fc·C)` | fc=250 | 100n | 6366 Ω | 6,8 kΩ | +6,8% |

→ tu reportes tout dans le rapport, ça donne le **tableau des composants** exigé dans le livrable.

---

## 10. Récap des choix par défaut (point de départ)

| Composant | Valeur de départ |
|---|---|
| `R_LED0..7, R_LEDM1` | 680 Ω |
| `RB0, RB1` | 10 kΩ |
| `Rref1, Rref2` | 10 kΩ |
| `Rpre0, Rpre2` | 10 kΩ |
| `Rpre1, Rpre3` | 33 kΩ |
| `Rmix0..4` | 10 kΩ |
| `R0..R5` (filtres) | 6,8 kΩ ou 3,9 kΩ selon fc — voir tableau §5 |
| `Re1..Re4` | 22 kΩ |
| `Cpre0, Cpre1` | 1 µF |
| `Ce1..Ce4` | 1µF / 470n / 220n / 100n |
| `C1..C5` | 100 nF / 22 nF / 10 nF selon fc |

À affiner en fonction du **schéma exact** (topologie précise des filtres) et des **mesures** en TP.
