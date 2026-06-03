# Rapport individuel — Projet Vumètre à LED (AP3 BX Électronique)

**Étudiant :** eses
**Blocs principaux traités :** Mixeur stéréo (sommateur inverseur) + Détecteurs d'enveloppe
**Volume horaire :** 7 séances × 3 h ≈ 24 h

---

## 1. Présentation du projet

Le projet vise à afficher en temps réel le niveau sonore d'un signal audio sur une
matrice de **64 LEDs RGBW (SK6812)**, répartie en 4 bandes de fréquences. La chaîne
de traitement est :

**Entrée jack (L/R) → pré-amplification → mixage mono → 4 filtres (basses,
bas-médiums, hauts-médiums, aigus) → détecteurs d'enveloppe → ADC du PIC18F25K40 →
matrice LED.**

Carte alimentée en **+5 V** simple ; les signaux audio sont polarisés autour de
`Vref = 2,5 V`.

Ma contribution porte sur deux blocs : le **mixeur stéréo** (qui recombine les
canaux gauche et droit) et les **détecteurs d'enveloppe** (qui transforment chaque
bande filtrée en une tension continue représentant le volume).

---

## 2. Déroulé chronologique

### Séance 1 — 13/05/2025 (3 h)
- Analyse du sujet, **synthèse** et rédaction d'une **to-do list** d'équipe.
- Lecture du schéma électronique et compréhension de son fonctionnement.
- Début du soudage.

**Ce que j'ai compris — le détecteur d'enveloppe :**
> Un détecteur d'enveloppe est un « traducteur » qui transforme un signal
> électrique qui varie très vite (le son) en une **tension continue** beaucoup plus
> lente représentant le **volume (l'amplitude)** du son.

Il sert à retranscrire fidèlement le volume sonore au microcontrôleur, sans faux
positifs. Il est constitué d'**une diode** + **un circuit RC** :
- la **diode** ne laisse passer que les alternances positives (elle **redresse** le
  signal) ;
- le **condensateur** se charge quand le son monte et se décharge quand il descend ;
  la **résistance** contrôle la vitesse de décharge. Une décharge trop rapide rend
  le signal bruité/saccadé ; trop lente, le signal devient « mou » et peu réactif.

### Séance 2 — 22/05/2025 (3 h)
- Début des calculs de chaque compartiment, en particulier le **mixage stéréo**
  (ma partie), non terminé ce jour-là.
- Une erreur de mise en place des broches a obligé l'équipe à **dessouder de
  nombreux composants**, ce qui a coûté beaucoup de temps.

**Ce que j'ai compris — le mixeur (sommateur inverseur) :**
Le signal gauche pré-amplifié (`OUT_PREAMP_L`) passe par `Rmix0` + `Rmix1`, le
signal droit (`OUT_PREAMP_R`) par `Rmix2` + `Rmix3`. Ils se rejoignent sur l'entrée
inverseuse (broche 6) de l'AOP, qui se comporte comme une **masse virtuelle**
(théorème de Millman). La tension de sortie est :

$$V_{out\_mix} = -\left( \frac{R_{mix4}}{R_{mix0} + R_{mix1}} \cdot V_L + \frac{R_{mix4}}{R_{mix2} + R_{mix3}} \cdot V_R \right)$$

### Séance 3 — 27/05/2025 (3 h)
- **Calcul des résistances du mixage stéréo** : valeur de **4,7 kΩ** pour les
  4 résistances d'entrée et **10 kΩ** pour `Rmix4`.

**Ce que j'ai compris — la démarche de dimensionnement en 3 étapes :**
1. **Poser la formule** du sommateur inverseur (ci-dessus).
2. **Conditions de symétrie et de gain unitaire** : pour traiter les deux canaux à
   l'identique et ne pas modifier le volume :
   - symétrie : `Rmix0 + Rmix1 = Rmix2 + Rmix3` ;
   - gain unitaire : `Rmix4 = Rmix0 + Rmix1`.
3. **Application numérique** : on choisit `Rmix4 = 10 kΩ`. Pour obtenir 10 kΩ par
   branche avec deux résistances égales, il faudrait 5 kΩ ; la valeur normalisée
   **E12 la plus proche est 4,7 kΩ**. On vérifie :
   - branche d'entrée réelle = 4,7 + 4,7 = 9,4 kΩ ;
   - gain réel par canal = 10 / 9,4 ≈ **1,06** (proche de 1, conforme à l'objectif).

### Séances 4 à 7
Poursuite en équipe : assemblage final, dimensionnement et calibrage des
détecteurs d'enveloppe, tests et intégration.

---

## 3. Analyse théorique et valeurs retenues

### 3.1 Mixeur stéréo (AOP sommateur inverseur)

| Composant | Rôle | Valeur retenue |
|---|---|---|
| `Rmix0`, `Rmix1` | branche d'entrée canal gauche | **4,7 kΩ** chacune |
| `Rmix2`, `Rmix3` | branche d'entrée canal droit | **4,7 kΩ** chacune |
| `Rmix4` | contre-réaction (fixe le gain) | **10 kΩ** |
| `Pmix0` | potentiomètre (réglage de niveau) | 10 kΩ *(imposé)* |

**Gain par canal :** `10 / 9,4 ≈ 1,06` → mixage L+R équilibré, quasi unitaire.

### 3.2 Détecteurs d'enveloppe (diode 1N4148 + RC)

La constante de temps `τ = R × C` doit être :
- **assez grande** pour lisser les oscillations de la plus basse fréquence de la
  bande considérée ;
- **assez petite** pour suivre les variations de volume (attaque/relâchement).

Valeurs retenues par bande :

| Bande | Résistance | Condensateur | Constante de temps `τ = R·C` |
|---|---|---|---|
| Enveloppe 1 | 1 000 Ω | 22 µF | **22 ms** |
| Enveloppe 2 | 8 200 Ω | 33 µF | **≈ 271 ms** |
| Enveloppe 3 | 1 000 Ω + 8 200 Ω | 22 µF + 33 µF | combinaison (cf. schéma) |
| Enveloppe 4 | 8 200 Ω | 33 µF | **≈ 271 ms** |

Les diodes de redressement sont des **1N4148** (`D1…D4`). Les constantes de temps
plus longues (enveloppes 2 et 4) lissent davantage les bandes où l'on souhaite un
affichage plus stable ; les plus courtes (enveloppe 1) restent réactives.

> *Réglage final à affiner à l'oscilloscope : si le vumètre « saute » trop →
> augmenter τ ; s'il est trop mou → diminuer τ.*

---

## 4. Synthèse personnelle

J'ai compris en profondeur le **rôle du détecteur d'enveloppe** (redressement +
lissage) dès la première séance, ce qui m'a aidé à dimensionner les couples RC. Sur
le **mixeur**, j'ai mené la démarche complète : formule du sommateur inverseur,
conditions de symétrie/gain, puis choix des valeurs normalisées (4,7 kΩ et 10 kΩ).
La principale leçon pratique de ces séances : une **erreur de placement de broches**
coûte très cher en temps de dessoudage — d'où l'intérêt de valider sur platine
d'essai avant de braser définitivement.
