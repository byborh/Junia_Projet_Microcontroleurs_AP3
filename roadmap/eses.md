S1	13/05/2025	3h
##  What i did:
Analyse du sujet , synthese, to do list. Lecture du schéma electronique et sa comprehension + début soudage. 

## What i understand: 
-Ce qu’est un détecteur d’enveloppe et à quoi il va nous servir : 
Pour faire simple, un détecteur d’enveloppe est un traducteur qui transforme un signal électrique qui bouge très vite (le son) en une tension continue beaucoup plus lente qui représente le volume (l'amplitude) de ce son. 

Cela va nous servir afin de bien retranscrire le volume sonore au microcontroleur et qu’il n’y aie pas de faux positif. 
Le détecteur d’enveloppe est constitué d’une diode et d’un circuit RC. 

La Diode : Elle ne laisse passer que les alternances positives du signal (elle redresse le signal).  

Les valeurs du R et C sont cruciales : le condensateur se charge quand la musique monte et se décharge quand elle descend, la résistance empêche que le condensateur se décharge trop rapidement puisque dans ce cas la le signal sera trop bruyant, et dans le cas inverse (cad d’une décharge pas assez rapide) alors le signal sera trop mou et pas assez reactif a la musique. 


## What i didn't understand:

S2	22/05/2025	3h
##  What i did:
On a commencé les calculs de chaque compartiment , le mix stereo pour ma part, mais je les ai pas fini. 
Suite a une erreur de mise en place des pins on a du dessouder pas mal de pins de composants ce qui nouos a pris beaucouop de trmps 

## What i understand:
Le signal gauche pré-amplifié (OUT_PREAMP_L) passe par les résistances $R_{mix0}$ et $R_{mix1}$, et le signal droit (OUT_PREAMP_R) passe par $R_{mix2}$ et $R_{mix3}$.
Ils se rejoignent sur l'entrée inverseuse (borne -, broche 6) de l'AOP.
La formule de la tension de sortie fournie par ta fiche est :
  $$V_{out\_mix} = -\left( \frac{R_{mix4}}{R_{mix0} + R_{mix1}} \cdot V_{out\_preamp\_L} + \frac{R_{mix4}}{R_{mix2} + R_{mix3}} \cdot V_{out\_preamp\_R} \right)$$


  Étape 1 : Le calcul des composants (Le dimensionnement)Le but ici est d'avoir un mixage "équitable" (50% Gauche, 50% Droite) sans atténuer ni trop amplifier le signal (on veut un gain global de 1 pour chaque canal).Par symétrie, pose l'égalité de tes branches d'entrée : $R_{mix0} + R_{mix1} = R_{mix2} + R_{mix3}$.Pour ne pas ré-amplifier le signal (on a déjà le pré-ampli pour ça), choisis un gain de $1$. Cela impose que la résistance de rétroaction $R_{mix4}$ soit égale à la résistance totale de chaque branche d'entrée :$$R_{mix4} = R_{mix0} + R_{mix1}$$Regarde dans ta liste de composants (Page 8) : le potentiomètre $P_{mix0}$ fait $10\text{ k}\Omega$. Sers-toi des valeurs de composants standard disponibles au labo pour fixer tes valeurs de résistances (par exemple, viser une valeur totale autour de $10\text{ k}\Omega$ ou $22\text{ k}\Omega$ pour chaque branche).  Étape 2 : L'analyse théorique pour le rapportTu dois rédiger la section "Mixeur" de votre rapport écrit.  Recopie le schéma structurel centré sur OPA1B.  Redémontre ou explique la formule de la fiche en expliquant le théorème de Millman au point de masse virtuelle (la broche 6).Justifie le choix de tes valeurs de résistances grâce à tes calculs d'égalités.Étape 3 : Le prototypage sur paillasseNe soude rien sur la carte finale pour l'instant.  Prends une platine d'essai (breadboard).Installe un AOP MCP6274 (ou utilise celui du groupe s'il est sur breadboard).  Câble tes résistances calculées.Le test : Injecte un signal sinusoïdal (ex: $1\text{ kHz}$, $1\text{ V}$ crête-à-crête) avec le générateur de fonctions sur l'entrée Gauche uniquement, puis regarde à l'oscilloscope sur le point de test TPM1 (juste après le mixeur). Tu dois obtenir ton signal (inversé). Fais de même pour l'entrée Droite. Si tu injectes le même signal sur les deux entrées en même temps, ton signal de sortie doit doubler d'amplitude.  

## What i didn't understand:

S3	27/05/2025	3h
##  What i did: Calcul des résitances du stéréo. Pour trouver un resultat de 4.7 pour les 4 resistances et de 10Kohm pour la Rmix4

## What i understand:e calcul mathématique en 3 étapes pour ton rapportÉtape 1 : Poser la formule généraleD'après la théorie du montage sommateur inverseur, la formule de la tension de sortie est :$$V_{out\_mix} = -\left( \frac{R_{mix4}}{R_{mix0} + R_{mix1}} \cdot V_{L} + \frac{R_{mix4}}{R_{mix2} + R_{mix3}} \cdot V_{R} \right)$$Étape 2 : Définir les conditions de symétrie et de gainPour que le mixeur traite les deux canaux de la même manière (symétrie) et qu'il ne modifie pas le volume (gain de 1), on pose mathématiquement les conditions suivantes :Condition de symétrie : $R_{mix0} + R_{mix1} = R_{mix2} + R_{mix3}$Condition de gain unitaire : $\frac{R_{mix4}}{R_{mix0} + R_{mix1}} = 1 \implies R_{mix4} = R_{mix0} + R_{mix1}$Étape 3 : L'application numérique (Le choix des composants)Au laboratoire, on choisit une valeur standard de $10\text{ k}\Omega$ pour la résistance de rétroaction ($R_{mix4}$).$$R_{mix4} = 10\text{ k}\Omega$$Pour obtenir cette même valeur sur les branches d'entrée en associant deux résistances identiques, on calcule :$$R_{mix0} + R_{mix1} = 10\text{ k}\Omega$$Si on pose $R_{mix0} = R_{mix1}$, alors :$$2 \times R_{mix0} = 10\text{ k}\Omega \implies R_{mix0} = \frac{10\text{ k}\Omega}{2} = 5\text{ k}\Omega$$La valeur normalisée la plus proche disponible dans la série standard E12 (la boîte de résistances du labo) est $4,7\text{ k}\Omega$.On ré-injecte ces valeurs réelles pour vérifier le calcul final :Branche d'entrée réelle = $4,7\text{ k}\Omega + 4,7\text{ k}\Omega = 9,4\text{ k}\Omega$Gain réel pour chaque canal = $\frac{10\text{ k}\Omega}{9,4\text{ k}\Omega} \approx 1,06$

## What i didn't understand:

S4	29/05/2025	3h
##  What i did:

## What i understand:

## What i didn't understand:

S5	03/06/2025	3h
##  What i did:

## What i understand:

## What i didn't understand:

S6	05/06/2025	3h
##  What i did:

## What i understand:

## What i didn't understand:

S7	10/06/2025	3h
##  What i did:

## What i understand:

## What i didn't understand:

S8	12/05/2025	3h
##  What i did:

## What i understand:

## What i didn't understand:

TOTAL	 	24h
