# 3W -- Worldwide Weather Watcher

## README technique et guide utilisateur

**Carte cible :** STM32 NUCLEO-L476RG\
**Langage :** C\
**Environnement :** PlatformIO / STM32Cube HAL\

------------------------------------------------------------------------

# 1. Présentation du projet

Le projet **3W -- Worldwide Weather Watcher** est une station météo
embarquée construite autour d'une carte STM32 NUCLEO-L476RG. Le
programme collecte des informations environnementales, les associe à une
date et une heure, les présente sur la console série et prévoit leur
classement dans des fichiers journaliers.

Les grandeurs traitées sont :

-   la température de l'air, en °C ;
-   l'humidité relative, en % ;
-   la pression atmosphérique, en hPa ;
-   la luminosité, sous forme d'une valeur numérique de 0 à 1023 et
    d'une catégorie (`faible`, `moyenne`, `forte`) ;
-   la date et l'heure fournies par une horloge RTC.

Le programme propose quatre modes : **STANDARD**, **CONFIGURATION**,
**MAINTENANCE** et **ECONOMIQUE**.

## 1.1 Objectifs 

1.  Acquérir périodiquement les mesures des capteurs.
2.  Horodater les mesures à l'aide du module RTC DS1307.
3.  Filtrer les mesures selon les paramètres activés et les plages
    configurées.
4.  Afficher les données et les messages d'état sur la console série.
5.  Indiquer le mode actif et les défauts à l'aide d'une LED RGB.
6.  Permettre à l'utilisateur de modifier les paramètres par la console.
7.  Conserver les paramètres dans la mémoire Flash interne du STM32.
8.  Prévoir une organisation des journaux par jour et par révision.

> **Limite importante de cette version :** le code de `stockage.c`
> simule la carte SD. Le commentaire du fichier indique que la carte
> n'est pas câblée (les trois fils dupont sont manquants). Il n'y a pas
> d'initialisation de contrôleur SD, de montage de système de fichiers,
> ni d'ouverture/écriture réelle de fichier. Les noms, tailles et
> changements de révision sont calculés et affichés sur la console
> uniquement.

------------------------------------------------------------------------

# 2. Architecture matérielle

## 2.1 Éléments matériels identifiés dans le code

  -----------------------------------------------------------------------
  Élément                 Rôle                    Interface / broches
                                                  indiquées
  ----------------------- ----------------------- -----------------------
  STM32 NUCLEO-L476RG     Microcontrôleur         Carte cible
                          principal               

  BME280                  Température, humidité,  I2C1, adresse `0x76`
                          pression                

  RTC DS1307              Date et heure           I2C1, adresse `0x68`

  Capteur de luminosité   Mesure analogique de    PA0 / ADC1, canal 5
                          luminosité              

  Bouton rouge            Sélection de mode /     PB4, entrée avec
                          retour standard         pull-up

  Bouton vert             Passage en mode         PB10, entrée avec
                          économique              pull-up

  LED RGB Grove P9813     Indication des modes et PA8 = CI (horloge), PA9
                          erreurs                 = DI (données)

  Console PC              Commandes et affichage  USART2, PA2 = TX, PA3 =
                                                  RX, via ST-Link/USB

  Carte SD                Stockage prévu des      Non opérationnelle dans
                          journaux                cette version
  -----------------------------------------------------------------------

## 2.2 Bus et interfaces

-   **I2C1** : les modules BME280 et DS1307 partagent les lignes PB8
    (SCL) et PB9 (SDA). Ils sont différenciés par leurs adresses I2C.
-   **USART2 / UART** : la console série utilise PA2 et PA3, avec un
    débit de 115200 bauds.
-   **ADC1** : le capteur de luminosité est lu sur PA0 avec une
    résolution de 10 bits, soit des valeurs théoriques de 0 à 1023.
-   **GPIO** : les boutons sont lus en entrée et les lignes de la LED
    RGB sont commandées en sortie.

## 2.3 Schéma des connexions logiques

``` text
                       ORDINATEUR
                    Moniteur série 115200
                            |
                       USB / ST-Link
                            |
                         USART2
                      PA2 TX / PA3 RX
                            |
                            v
+---------------------------------------------------+
|               STM32 NUCLEO-L476RG                 |
|                                                   |
|  I2C1 PB8/SCL + PB9/SDA                            |
|       |                       |                   |
|       v                       v                   |
|   BME280 (0x76)           RTC DS1307 (0x68)        |
|                                                   |
|  ADC1 PA0 <--------- Capteur de luminosité        |
|                                                   |
|  GPIO PB4 <--------- Bouton rouge                 |
|  GPIO PB10 <-------- Bouton vert                  |
|                                                   |
|  GPIO PA8/PA9 ------> LED RGB Grove P9813          |
|                                                   |
|  stockage.c -------> Simulation de journal SD     |
|                      (pas d'écriture réelle)       |
|                                                   |
|  Flash interne ----> Paramètres persistants       |
+---------------------------------------------------+
```

------------------------------------------------------------------------

# 3. Architecture logicielle

Le programme est organisé par modules. Chaque module possède
généralement un fichier `.c` qui contient l'implémentation et un fichier
`.h` qui expose les fonctions et définitions utilisées par les autres
modules.

## 3.1 Organisation des fichiers

``` text
build/
├── include/
│   ├── bme280.h
│   ├── boutons.h
│   ├── capteurs.h
│   ├── config.h
│   ├── console.h
│   ├── i2c_bus.h
│   ├── led.h
│   ├── luminosite.h
│   ├── parametres.h
│   ├── rtc_ds1307.h
│   ├── stockage.h
│   └── systeme.h
├── src/
│   ├── bme280.c
│   ├── boutons.c
│   ├── capteurs.c
│   ├── console.c
│   ├── i2c_bus.c
│   ├── led.c
│   ├── luminosite.c
│   ├── main.c
│   ├── parametres.c
│   ├── rtc_ds1307.c
│   ├── stockage.c
│   └── systeme.c
└── platformio.ini
```

## 3.2 Rôle des modules

  -----------------------------------------------------------------------
  Module                              Responsabilité
  ----------------------------------- -----------------------------------
  `main.c`                            Initialisation générale, boucle
                                      principale, modes, périodicité et
                                      orchestration

  `capteurs.c`                        Acquisition globale, vérification
                                      des mesures et création de la ligne
                                      texte

  `bme280.c`                          Initialisation, lecture et
                                      conversion des valeurs brutes du
                                      BME280

  `rtc_ds1307.c`                      Lecture et réglage de l'horloge
                                      DS1307

  `i2c_bus.c`                         Configuration et fonctions communes
                                      du bus I2C1

  `luminosite.c`                      Configuration ADC1 et lecture du
                                      capteur analogique

  `boutons.c`                         Lecture des boutons, filtrage
                                      anti-rebond et détection d'appui
                                      long

  `led.c`                             Pilotage de la LED RGB et
                                      signalement des erreurs

  `console.c`                         Communication série USART2 avec le
                                      PC

  `parametres.c`                      Paramètres configurables,
                                      validation des commandes et
                                      sauvegarde en Flash

  `stockage.c`                        Simulation de la gestion des
                                      journaux journaliers et de leur
                                      taille

  `systeme.c`                         Configuration de l'horloge CPU,
                                      mesure de pile et gestionnaires
                                      d'exceptions
  -----------------------------------------------------------------------

## 3.3 Schéma du flux d'information

``` text
              +--------------------------+
              |        main.c            |
              | boucle principale        |
              +------------+-------------+
                           |
                    déclenche mesure
                           |
                           v
              +--------------------------+
              |      capteurs.c          |
              |    capteurs_mesurer()    |
              +------------+-------------+
                           |
            +--------------+---------------+
            |              |               |
            v              v               v
       +---------+    +-----------+   +-----------+
       | BME280  |    | RTC       |   | Luminosité|
       | bme280.c|    | rtc_ds... |   | luminosite|
       +----+----+    +-----+-----+   +-----+-----+
            |               |               |
            +---------------+---------------+
                            |
                            v
                 +----------------------+
                 | struct Mesure        |
                 | température, humidité|
                 | pression, lumière,   |
                 | date et heure        |
                 +----------+-----------+
                            |
                            v
                 +----------------------+
                 | capteurs_ligne()     |
                 | formatage texte CSV  |
                 +----------+-----------+
                            |
                 +----------+-----------+
                 |                      |
                 v                      v
          +-------------+        +----------------+
          | console.c   |        | stockage.c     |
          | printf UART |        | simulation SD  |
          +-------------+        +----------------+
```

Le bus I2C est partagé par le BME280 et le DS1307. La luminosité utilise
l'ADC et ne passe donc pas par I2C.

## 3.4 Séquence de démarrage

Dans `main()` (`src/main.c`), l'ordre d'initialisation est :

1.  `HAL_Init()` : initialise la HAL et la base de temps.
2.  `SystemClock_Config()` : configure l'horloge système à 80 MHz.
3.  `boutons_init()` : configure PB4 et PB10.
4.  `bouton_rouge_au_demarrage()` : détecte une demande de mode
    configuration.
5.  `led_init()` : configure la LED RGB.
6.  `console_init()` : configure USART2.
7.  Attente de 500 ms pour laisser le temps au terminal série de se
    connecter.
8.  `i2c_init()` : configure I2C1.
9.  `VerificationEEPROM()` : récupère les paramètres en Flash ou écrit
    les valeurs par défaut.
10. `capteurs_init()` : initialise RTC, BME280 et capteur de luminosité.
11. `stockage_init()` : affiche que le stockage SD est simulé.
12. Affichage de l'utilisation de la pile et de l'en-tête des données.
13. Sélection du mode initial, puis entrée dans `while (1)`.

------------------------------------------------------------------------

# 4. Fonctionnement général et acquisition

## 4.1 Déroulement d'une mesure

La fonction `mesurer(int enregistrer)` de `main.c` orchestre une mesure
:

1.  `capteurs_mesurer()` actualise la structure globale `mesure`.
2.  `capteurs_ligne()` convertit les valeurs en une chaîne de caractères
    dans `ligne`.
3.  `printf()` envoie cette ligne sur la console série.
4.  Si `enregistrer == 1`, `stockage_ajouter()` met à jour la simulation
    du journal.

La ligne utilise le séparateur point-virgule. L'en-tête défini dans
`capteurs.h` est :

``` text
date;heure;temperature_C;humidite_pct;pression_hPa;luminosite;niveau_lumiere
```

Exemple de ligne :

``` text
2026-10-23;14:20:00;27.1;38.5;983.2;224;faible
```

Si une valeur n'est pas disponible, le code écrit `NA`. La constante
`NA` vaut `-9999` dans `config.h`.

## 4.2 Acquisition du BME280

Le module `bme280.c` :

-   lit les coefficients de calibration du capteur ;
-   configure l'humidité, le filtre et le mode de mesure ;
-   lit les registres de données ;
-   calcule la température, la pression et l'humidité à partir des
    valeurs brutes ;
-   renvoie `1` en cas de lecture correcte et `0` en cas d'échec.

La température est calculée en premier, car le calcul de pression et
d'humidité utilise la variable intermédiaire `t_fine`.

La pression calculée en pascals est divisée par 100 pour être exprimée
en hPa. L'humidité est bornée entre 0 et 100 %.

Dans `capteurs.c`, les mesures BME280 sont initialisées à `NA`. Le
programme réessaie pendant la durée `TIMEOUT` (en secondes). Entre les
tentatives, il réinitialise le capteur, actualise la LED et attend 100
ms. Après deux cycles de mesure consécutifs sans réponse, l'erreur
capteur est signalée par la LED.

Les paramètres `TEMP_AIR`, `HYGR` et `PRESSURE` activent ou désactivent
l'utilisation de chaque grandeur. Les plages sont également contrôlées.
L'humidité n'est conservée que si la température est comprise entre
`HYGR_MINT` et `HYGR_MAXT`.

## 4.3 Acquisition de la date et de l'heure

Le DS1307 est lu par I2C à l'adresse `0x68`. Les données de l'horloge
sont codées en BCD et converties en nombres décimaux par `rtc_ds1307.c`.

Si le RTC ne répond pas, `dateOK` vaut 0 et la date/heure de la ligne
est remplacée par `NA;NA`. Le défaut RTC est signalé par la LED.

Au premier démarrage, si le bit d'arrêt de l'horloge est détecté, le
programme règle une date initiale au 1er janvier 2026 à 00:00:00.
L'utilisateur doit ensuite régler l'heure et la date correctes par les
commandes `CLOCK`, `DATE` et `DAY`.

## 4.4 Acquisition de la luminosité

Le capteur est lu par ADC1 sur PA0, en résolution 10 bits. La valeur
attendue est comprise entre 0 et 1023. La catégorie est déterminée avec
:

-   `LUMIN_LOW` : en dessous, luminosité `faible` ;
-   `LUMIN_HIGH` : au-dessus, luminosité `forte` ;
-   entre les deux : luminosité `moyenne`.

Si le paramètre `LUMIN` vaut 0, la luminosité est désactivée et la ligne
contient `NA`.

------------------------------------------------------------------------

# 5. Modes de fonctionnement

Les modes sont déclarés dans `src/main.c` :

``` c
enum { STANDARD, CONFIGURATION, MAINTENANCE, ECONOMIQUE };
```

Les couleurs associées sont définies dans la logique de `changer_mode()`
de `main.c`.

  -----------------------------------------------------------------------
  Mode              LED en            Fonctionnement    Enregistrement
                    fonctionnement                      demandé ?
                    normal                              
  ----------------- ----------------- ----------------- -----------------
  STANDARD          Verte             Mesures à         Oui
                                      l'intervalle      (`mesurer(1)`)
                                      configuré         

  CONFIGURATION     Jaune             Saisie des        Non
                                      commandes sur la  
                                      console ; pas de  
                                      mesure périodique 

  MAINTENANCE       Orange            Mesures toutes    Non
                                      les 5 secondes et (`mesurer(0)`)
                                      affichage ; carte 
                                      SD retirable      
                                      selon le message  
                                      du programme      

  ECONOMIQUE        Bleue             Mesures deux fois Oui
                                      moins fréquentes  (`mesurer(1)`)
                                      qu'en STANDARD    
  -----------------------------------------------------------------------

## 5.1 Passage d'un mode à l'autre

``` text
                         DÉMARRAGE
                             |
                 bouton rouge détecté ?
                     /             \
                   OUI              NON
                    |                |
                    v                v
             CONFIGURATION       STANDARD
                    |                |
             QUITTER ou 30 min       | rouge maintenu 5 s
             sans activité           +------> MAINTENANCE
                    |                |
                    v                | vert maintenu 5 s
                STANDARD             +------> ECONOMIQUE

             MAINTENANCE -- rouge 5 s --> STANDARD
             ECONOMIQUE  -- rouge 5 s --> STANDARD
```

Dans le code : - le bouton rouge maintenu 5 secondes depuis STANDARD
provoque le passage en MAINTENANCE ; - le bouton vert maintenu 5
secondes depuis STANDARD provoque le passage en ECONOMIQUE ; - le bouton
rouge maintenu 5 secondes depuis MAINTENANCE ou ECONOMIQUE ramène en
STANDARD ; - le bouton vert n'a pas d'action dans MAINTENANCE ou
ECONOMIQUE ; - CONFIGURATION est sélectionné au démarrage si le bouton
rouge est détecté pendant la fenêtre de démarrage ; - `QUITTER` ou 30
minutes sans saisie ramène en STANDARD.

À chaque changement de mode, `mesureImmediate` est positionné à 1 : une
mesure est donc demandée immédiatement dans le nouveau mode (sauf
CONFIGURATION, où la boucle ne lance pas de mesure).

## 5.2 Périodicité des mesures

La constante `MODE_DEMO` se trouve dans `include/config.h` et vaut
actuellement `1`.

-   Si `MODE_DEMO == 1`, `LOG_INTERVALL` est interprété en secondes.
-   Si `MODE_DEMO == 0`, `LOG_INTERVALL` est interprété en minutes,
    conformément au commentaire du projet.

La valeur par défaut de `LOG_INTERVALL` est 10.

  Mode              Intervalle avec les valeurs actuelles
  --------------- ---------------------------------------
  STANDARD                                    10 secondes
  ECONOMIQUE                                  20 secondes
  MAINTENANCE                                  5 secondes
  CONFIGURATION                  Pas de mesure périodique

Ces durées décrivent la configuration du code, pas une mesure de
performance chronométrée. Les opérations de lecture et les délais de
réponse des capteurs peuvent allonger le temps réel d'un cycle.

------------------------------------------------------------------------

# 6. Boutons et anti-rebond

Le traitement est défini dans `src/boutons.c`, avec les broches et la
durée d'appui long déclarées dans `include/config.h`.

  -----------------------------------------------------------------------
  Bouton                  Broche                  Fonction
  ----------------------- ----------------------- -----------------------
  Rouge                   PB4                     Entrée en maintenance
                                                  depuis STANDARD ;
                                                  retour en STANDARD
                                                  depuis les modes
                                                  MAINTENANCE et
                                                  ECONOMIQUE

  Vert                    PB10                    Entrée en mode
                                                  ECONOMIQUE depuis
                                                  STANDARD
  -----------------------------------------------------------------------

Les broches sont configurées en entrée avec résistance `GPIO_PULLUP` : -
au repos, la broche vaut 1 ; - lorsqu'on appuie, la broche passe à 0 ; -
`bouton_enfonce()` inverse cette lecture logique et renvoie 1 lorsque le
bouton est appuyé.

## 6.1 Anti-rebond logiciel

Les durées sont définies au début de `boutons.c` :

``` c
#define DELAI_APPUI   40
#define DELAI_RELACHE 300
```

Le programme applique deux filtres :

-   **Validation de l'appui : 40 ms.** Un appui doit rester détecté
    pendant au moins 40 ms avant d'être accepté. Cela filtre les rebonds
    électriques au moment de l'enfoncement.
-   **Validation du relâchement : 300 ms.** Un relâchement doit être
    stable pendant 300 ms avant d'être accepté. Cela évite qu'une brève
    coupure du contact soit interprétée comme un relâchement.

Un appui long est reconnu après `DUREE_APPUI_LONG`, définie à 5000 ms (5
secondes) dans `config.h`. Il n'est signalé qu'une fois par appui.
`bouton_appui_long()` consomme l'événement : elle renvoie 1 une fois,
puis 0.

Le traitement est réalisé par interrogation répétée dans la boucle
principale via `boutons_mise_a_jour()`. **Les boutons ne sont pas
traités par interruption GPIO dans cette version.**

Au démarrage, `bouton_rouge_au_demarrage()` observe le bouton rouge
pendant 500 ms, avec des lectures espacées de 5 ms. Le bouton est
considéré comme appuyé si au moins 30 % des lectures l'ont détecté
enfoncé.

------------------------------------------------------------------------

# 7. LED RGB : indications utilisateur

Le pilotage est dans `src/led.c`. La LED Grove utilise une puce P9813 et
deux lignes : - PA8 : CI, signal d'horloge ; - PA9 : DI, données.

## 7.1 Couleur du mode

  -----------------------------------------------------------------------
  Couleur                             Signification
  ----------------------------------- -----------------------------------
  Verte                               Mode STANDARD

  Jaune                               Mode CONFIGURATION

  Orange                              Mode MAINTENANCE

  Bleue                               Mode ECONOMIQUE

  Éteinte                             Initialisation / absence de couleur
                                      de mode avant sélection
  -----------------------------------------------------------------------

## 7.2 Signalisation des erreurs

En présence d'une erreur, la LED clignote à une période d'une seconde
entre le rouge et une seconde couleur. Si plusieurs erreurs sont
actives, le code donne priorité à la première dans l'ordre ci-dessous.

  ------------------------------------------------------------------------
                      Priorité Défaut                Alternance
  ---------------------------- --------------------- ---------------------
                             1 RTC non détecté /     Rouge 500 ms / bleu
                               date indisponible     500 ms

                             2 BME280 indisponible   Rouge 500 ms / vert
                               (après deux échecs de 500 ms
                               mesure consécutifs)   

                             3 Mesure incohérente    Rouge 333 ms / vert
                               selon les seuils      667 ms

                             4 Stockage simulé       Rouge 500 ms / blanc
                               déclaré plein         500 ms

                             5 Erreur d'accès SD     Rouge 333 ms / blanc
                                                     667 ms
  ------------------------------------------------------------------------

Remarques : - L'état `ERREUR_SD_ACCES` est défini dans `led.h`, mais
aucun appel d'activation correspondant n'est présent dans le code
actuel. - La gestion de la LED est rafraîchie à chaque tour de boucle
par `led_mise_a_jour()`. Elle est également appelée pendant les attentes
de nouvelle tentative BME280. - Une erreur prend visuellement la
priorité sur la couleur du mode. Lorsque l'erreur disparaît, la couleur
du mode réapparaît.

------------------------------------------------------------------------

# 8. Console utilisateur et commandes

La console utilise USART2 à **115200 bauds**, 8 bits de données, sans
parité et 1 bit d'arrêt (configuration 8N1). Le moniteur série doit être
réglé à 115200 bauds.

Le code de `console.c` redirige `printf()` vers l'UART2. La lecture d'un
caractère est non bloquante : le programme ne reste pas en attente d'une
touche.

Le mode CONFIGURATION permet de saisir les commandes suivantes. Les
commandes sont converties en majuscules avant traitement.

## 8.1 Commandes générales

  -----------------------------------------------------------------------
  Commande                            Action
  ----------------------------------- -----------------------------------
  `HELP`                              Affiche les paramètres et la liste
                                      des commandes

  `RESET`                             Remet les paramètres à leurs
                                      valeurs par défaut et les
                                      sauvegarde en Flash

  `VERSION`                           Affiche la version et le numéro de
                                      lot

  `PILE`                              Affiche l'estimation de la pile
                                      utilisée

  `QUITTER`                           Quitte le mode CONFIGURATION et
                                      revient en STANDARD
  -----------------------------------------------------------------------

## 8.2 Réglage de l'horloge

  ------------------------------------------------------------------------
  Commande                 Exemple                 Action
  ------------------------ ----------------------- -----------------------
  `CLOCK=HH:MM:SS`         `CLOCK=14:30:00`        Règle l'heure

  `DATE=MOIS,JOUR,ANNEE`   `DATE=10,23,2026`       Règle le mois, le jour
                                                   et l'année

  `DAY=JOUR`               `DAY=MON`               Règle le jour de la
                                                   semaine
  ------------------------------------------------------------------------

Valeurs acceptées : - heure : 0 à 23 ; - minute et seconde : 0 à 59 ; -
mois : 1 à 12 ; - jour : 1 à 31 ; - année : 2000 à 2099 ; - jour de
semaine : `MON`, `TUE`, `WED`, `THU`, `FRI`, `SAT`, `SUN`.

La vérification de la date ne contrôle pas le nombre réel de jours de
chaque mois (par exemple, le 31 février peut passer le contrôle
élémentaire du code).

## 8.3 Paramètres configurables

Les noms ci-dessous sont ceux utilisés par `TraiterCommande()` dans
`parametres.c`. Exemple : `LOG_INTERVALL=20`.

  -------------------------------------------------------------------------------
  Paramètre         Rôle                   Défaut         Minimum         Maximum
  ----------------- ------------- --------------- --------------- ---------------
  `LOG_INTERVALL`   Intervalle                 10               1            1440
                    entre deux                                    
                    mesures                                       
                    (secondes en                                  
                    mode démo,                                    
                    minutes                                       
                    sinon)                                        

  `FILE_MAX_SIZE`   Taille                   2048             512           32768
                    maximale                                      
                    théorique                                     
                    d'un journal,                                 
                    en octets                                     

  `TIMEOUT`         Durée                      30               1             300
                    maximale                                      
                    d'attente du                                  
                    BME280, en                                    
                    secondes                                      

  `LUMIN`           Activation de               1               0               1
                    la luminosité                                 
                    (0 =                                          
                    désactivée, 1                                 
                    = activée)                                    

  `LUMIN_LOW`       Seuil                     255               0            1023
                    inférieur de                                  
                    luminosité                                    

  `LUMIN_HIGH`      Seuil                     768               0            1023
                    supérieur de                                  
                    luminosité                                    

  `TEMP_AIR`        Activation de               1               0               1
                    la                                            
                    température                                   

  `MIN_TEMP_AIR`    Température               -10             -40              85
                    minimale                                      
                    acceptée                                      

  `MAX_TEMP_AIR`    Température                60             -40              85
                    maximale                                      
                    acceptée                                      

  `HYGR`            Activation de               1               0               1
                    l'humidité                                    

  `HYGR_MINT`       Température                 0             -40              85
                    minimale pour                                 
                    retenir                                       
                    l'humidité                                    

  `HYGR_MAXT`       Température                50             -40              85
                    maximale pour                                 
                    retenir                                       
                    l'humidité                                    

  `PRESSURE`        Activation de               1               0               1
                    la pression                                   

  `PRESSURE_MIN`    Pression                  850             300            1100
                    minimale                                      
                    acceptée, en                                  
                    hPa                                           

  `PRESSURE_MAX`    Pression                 1080             300            1100
                    maximale                                      
                    acceptée, en                                  
                    hPa                                           
  -------------------------------------------------------------------------------

Le code vérifie que la valeur saisie est un entier et qu'elle se trouve
dans les bornes autorisées. Les paramètres sont ensuite sauvegardés en
Flash.

**Attention :** le code valide chaque borne individuellement, mais ne
vérifie pas toutes les relations entre paramètres. Il est donc possible
de configurer un seuil inférieur supérieur au seuil supérieur.

------------------------------------------------------------------------

# 9. Gestion du stockage et des fichiers journaux

## 9.1 Convention de nommage prévue

Le module `stockage.c` prépare un nom basé sur la date :

``` text
AAMMJJ_R.LOG
```

-   `AA` : deux derniers chiffres de l'année ;
-   `MM` : mois ;
-   `JJ` : jour ;
-   `R` : numéro de révision de 0 à 9 ;
-   `.LOG` : extension du journal.

Exemple : `261102_0.LOG` désignerait le journal du 2 novembre 2026,
révision 0.

Si la date RTC n'est pas disponible, le code utilise `000000` comme
identifiant de jour.

## 9.2 Principe d'organisation prévu

Une mesure correspond à une ligne du journal du jour. Le code initialise
le compteur de taille avec la longueur de l'en-tête, puis ajoute la
longueur de chaque ligne, avec deux octets supplémentaires comptés pour
`\r\n`.

La limite par défaut est de **2048 octets (2 Kio au sens binaire)**.
Elle est configurable avec `FILE_MAX_SIZE`.

Lorsque la taille théorique dépasse la limite, le code incrémente le
numéro de révision, affiche un message d'archivage et remet le compteur
à la taille de l'en-tête. Le nombre maximal de révisions est défini par
`REVISION_MAX`, égal à 9.

## 9.3 Limites réelles de l'implémentation

Le stockage n'est pas encore un système de fichiers fonctionnel sur
carte SD :

-   `stockage_init()` affiche un message de simulation ;
-   aucune bibliothèque de système de fichiers (par exemple FatFs) n'est
    appelée ;
-   aucun fichier n'est créé, ouvert, écrit, fermé ou relu sur la carte
    ;
-   le contenu des mesures n'est pas conservé sur la SD ;
-   le compteur `tailleFichier` est uniquement une estimation en RAM ;
-   le message d'archivage est informatif : aucun fichier existant n'est
    réellement renommé ou déplacé.

**Point d'attention dans le code actuel :** après incrément de
`revision`, le message de mesure continue d'afficher `%s_0.LOG`. Le
programme ne gère donc pas réellement plusieurs fichiers de révision.
Cette partie doit être complétée avant de considérer l'archivage comme
opérationnel.

Le mode MAINTENANCE affiche « La carte SD peut etre retiree » et appelle
`mesurer(0)`, donc aucune mise à jour du stockage n'est demandée dans ce
mode.

------------------------------------------------------------------------

# 10. Paramètres persistants en mémoire Flash

Malgré le nom historique `EEPROM` employé dans les fonctions, le
programme utilise la **mémoire Flash interne du STM32**, et non une
EEPROM externe.

Dans `parametres.c` : - l'adresse de départ utilisée est `0x080FF800`
; - la page indiquée est la page 255 de la banque 2 ; - le numéro de
version des paramètres est `EEPROM_VERSION`, égal à 4 ; - les paramètres
sont écrits sous forme de double-mots de 8 octets.

Au démarrage, `VerificationEEPROM()` compare la version stockée à la
version attendue : - si elle ne correspond pas, les valeurs par défaut
sont écrites ; - sinon, les paramètres sont relus dans le tableau
`parametre`.

Lorsqu'une commande modifie un paramètre, `SauvegardeEEPROM()`
déverrouille la Flash, efface la page ciblée, écrit la version et les
paramètres, puis reverrouille la Flash.

> La page Flash réservée aux paramètres doit rester exclue de toute
> autre utilisation du programme ou du processus de programmation.
> L'effacement d'une page Flash efface toutes les données qu'elle
> contient.

------------------------------------------------------------------------

# 11. Interruptions, horloge et temps système

## 11.1 Où sont définies les interruptions ?

Les gestionnaires d'exceptions sont dans `src/systeme.c` :

  -----------------------------------------------------------------------
  Fonction                            Rôle
  ----------------------------------- -----------------------------------
  `SysTick_Handler()`                 Appel périodique qui exécute
                                      `HAL_IncTick()`

  `NMI_Handler()`                     Gestionnaire d'interruption non
                                      masquable ; corps vide dans ce code

  `HardFault_Handler()`               Gestionnaire d'erreur processeur
                                      grave ; boucle infinie dans ce code
  -----------------------------------------------------------------------

`SysTick_Handler()` fait avancer le compteur de temps HAL. Le
commentaire du code indique un appel toutes les 1 ms. Cette base de
temps sert notamment à `HAL_GetTick()` et `HAL_Delay()`.

`HardFault_Handler()` bloque volontairement le programme dans une boucle
infinie. Il n'y a pas de récupération logicielle dans cette fonction.

## 11.2 Les boutons utilisent-ils des interruptions ?

Non. Les boutons sont lus par scrutation (*polling*) dans la boucle
principale : - `boutons_mise_a_jour()` lit les entrées ; - les durées
sont calculées avec `HAL_GetTick()` ; - aucun gestionnaire EXTI de
bouton n'est défini dans les fichiers du projet.

Le SysTick est une exception périodique du cœur, tandis que les boutons
ne déclenchent pas d'interruption dans cette version.

------------------------------------------------------------------------

# 12. Pointeurs et passages d'adresses

Le code utilise des pointeurs pour permettre à une fonction de remplir
des variables appartenant à son appelant, ou pour transmettre un tableau
de données.

  ----------------------------------------------------------------------------
  Fonction / élément           Pointeur                Utilité
  ---------------------------- ----------------------- -----------------------
  `bme280_lire()` dans         `float *temperature`,   Écrit les trois
  `bme280.h/.c`                `float *humidite`,      résultats directement
                               `float *pression`       dans les variables
                                                       fournies par l'appelant

  `rtc_lire()` dans            six paramètres `int *`  Remplit année, mois,
  `rtc_ds1307.h/.c`                                    jour, heure, minute et
                                                       seconde

  `i2c_lire()` dans            `uint8_t *donnees`      Reçoit les octets lus
  `i2c_bus.h/.c`                                       sur le bus

  `i2c_ecrire()` dans          `uint8_t *donnees`      Pointe vers les octets
  `i2c_bus.h/.c`                                       à transmettre

  `console_lire_caractere()`   `char *c`               Reçoit le caractère lu
  dans `console.h/.c`                                  sur l'UART

  `lire_flash()` dans          `int *case_memoire`     Interprète une adresse
  `parametres.c`                                       Flash comme l'adresse
                                                       d'un entier à lire

  `TraiterCommande()`          `char commande[]`       Reçoit le texte de
                                                       commande à analyser ;
                                                       en paramètre de
                                                       fonction, le tableau
                                                       est transmis comme
                                                       adresse

  `HAL_UART_Transmit()`        pointeur vers le tampon Transmet le texte
                               d'octets                fourni sur USART2

  `HAL_I2C_Mem_Read/Write()`   pointeurs vers les      Échangent des blocs
                               tampons                 d'octets avec les
                                                       périphériques I2C
  ----------------------------------------------------------------------------

Il n'y a pas d'allocation dynamique visible (`malloc`/`free`) dans les
sources examinées. Les tableaux sont statiques, globaux ou locaux.

## 12.1 Variables et tableaux globaux importants

  ----------------------------------------------------------------------------
  Variable                     Fichier                 Utilité
  ---------------------------- ----------------------- -----------------------
  `mode`                       `main.c`                Mode de fonctionnement
                                                       courant

  `derniereMesure`             `main.c`                Instant du dernier
                                                       déclenchement de
                                                       mesure, en ms

  `mesureImmediate`            `main.c`                Demande une mesure
                                                       immédiate après un
                                                       changement de mode

  `derniereTouche`             `main.c`                Instant de la dernière
                                                       touche en mode
                                                       configuration

  `commande[64]`               `main.c`                Tampon de commande
                                                       série

  `longueurCommande`           `main.c`                Nombre de caractères
                                                       déjà reçus

  `mesure`                     `capteurs.c`            Structure contenant la
                                                       dernière acquisition

  `ligne[100]`                 `capteurs.c`            Dernière mesure mise en
                                                       forme en texte

  `parametre[NB_PARAMETRES]`   `parametres.c`          Valeurs actuelles des
                                                       paramètres
                                                       configurables

  `huart2`                     `console.c`             Handle de configuration
                                                       de l'USART2

  `hi2c1`                      `i2c_bus.c`             Handle de configuration
                                                       de l'I2C1

  `hadc1`                      `luminosite.c`          Handle de configuration
                                                       de l'ADC1
  ----------------------------------------------------------------------------

## 12.2 Variables statiques de module

`static` limite la visibilité d'une variable ou d'une fonction au
fichier `.c` concerné.

  -----------------------------------------------------------------------
  Variable                Fichier                 Utilité
  ----------------------- ----------------------- -----------------------
  `echecsBME`             `capteurs.c`            Compte les échecs
                                                  consécutifs de lecture
                                                  BME280

  `dig_T1...dig_T3`       `bme280.c`              Coefficients de
                                                  calibration température

  `dig_P1...dig_P9`       `bme280.c`              Coefficients de
                                                  calibration pression

  `dig_H1...dig_H6`       `bme280.c`              Coefficients de
                                                  calibration humidité

  `t_fine`                `bme280.c`              Valeur intermédiaire de
                                                  compensation BME280

  `etat[]`                `boutons.c`             État filtré de chaque
                                                  bouton

  `debutChangement[]`     `boutons.c`             Instant de début d'un
                                                  changement d'état
                                                  détecté

  `debutAppui[]`          `boutons.c`             Instant où l'appui a
                                                  été validé

  `dejaTraite[]`          `boutons.c`             Empêche de signaler
                                                  plusieurs fois le même
                                                  appui long

  `appuiLong[]`           `boutons.c`             Événement d'appui long
                                                  en attente de lecture

  `couleurMode`           `led.c`                 Couleur associée au
                                                  mode courant

  `erreurs[]`             `led.c`                 États actifs/inactifs
                                                  des erreurs

  `couleurAffichee`       `led.c`                 Évite de retransmettre
                                                  une couleur déjà
                                                  affichée

  `jourActuel`            `stockage.c`            Date associée au
                                                  journal simulé courant

  `tailleFichier`         `stockage.c`            Taille théorique
                                                  cumulée du journal

  `revision`              `stockage.c`            Compteur de révision
                                                  simulé

  `nom[]`                 `parametres.c`          Noms textuels des
                                                  paramètres

  `valeurParDefaut[]`     `parametres.c`          Valeurs par défaut

  `minimum[]`,            `parametres.c`          Bornes de validation
  `maximum[]`                                     des paramètres
  -----------------------------------------------------------------------

## 12.3 Structure `Mesure`

Définie dans `include/capteurs.h` et instanciée sous le nom global
`mesure` dans `capteurs.c`.

  Champ                          Type      Signification
  ------------------------------ --------- ----------------------------------
  `dateOK`                       `int`     1 si le RTC a répondu, sinon 0
  `annee`, `mois`, `jour`        `int`     Date
  `heure`, `minute`, `seconde`   `int`     Heure
  `temperature`                  `float`   Température en °C
  `humidite`                     `float`   Humidité relative en %
  `pression`                     `float`   Pression en hPa
  `lumiere`                      `int`     Valeur ADC de luminosité ou `NA`

------------------------------------------------------------------------

# 13. Liste des fonctions par module

Les fonctions ci-dessous sont celles présentes dans les sources du
projet. Les fonctions marquées `static` sont internes à leur fichier et
ne sont pas appelables directement depuis les autres modules.

## `main.c`

  -----------------------------------------------------------------------------
  Fonction                                  Utilité
  ----------------------------------------- -----------------------------------
  `changer_mode(int nouveauMode)`           Change le mode, met à jour la
                                            couleur LED, affiche un message et
                                            réinitialise les appuis

  `intervalle_mesure(void)`                 Calcule l'intervalle en ms selon
                                            les paramètres et le mode

  `temps_de_mesurer(uint32_t intervalle)`   Détermine si une mesure doit être
                                            déclenchée

  `mesurer(int enregistrer)`                Lance l'acquisition, formate,
                                            affiche et éventuellement demande
                                            l'enregistrement

  `gerer_configuration(void)`               Lit et traite les commandes saisies
                                            dans le terminal

  `main(void)`                              Initialise le système puis exécute
                                            la boucle principale
  -----------------------------------------------------------------------------

## `boutons.c`

  -----------------------------------------------------------------------
  Fonction                            Utilité
  ----------------------------------- -----------------------------------
  `bouton_enfonce(int bouton)`        Lit physiquement l'entrée et
                                      renvoie 1 si le bouton est appuyé

  `boutons_init(void)`                Configure PB4/PB10 en entrée avec
                                      pull-up

  `bouton_rouge_au_demarrage(void)`   Détecte un appui rouge pendant la
                                      fenêtre de démarrage

  `boutons_mise_a_jour(void)`         Applique le filtrage anti-rebond et
                                      détecte les appuis longs

  `bouton_appui_long(int bouton)`     Lit et consomme un événement
                                      d'appui long

  `boutons_reinitialiser(void)`       Efface les événements d'appui en
                                      cours lors d'un changement de mode
  -----------------------------------------------------------------------

## `capteurs.c`

  -----------------------------------------------------------------------
  Fonction                            Utilité
  ----------------------------------- -----------------------------------
  `capteurs_init(void)`               Initialise RTC, BME280 et
                                      luminosité

  `mesurer_date()` (`static`)         Lit et valide la date/heure

  `mesurer_bme()` (`static`)          Lit le BME280, réessaie en cas
                                      d'échec et applique les seuils

  `mesurer_lumiere()` (`static`)      Lit la luminosité si elle est
                                      activée

  `capteurs_mesurer(void)`            Lance toutes les acquisitions

  `ajouter_valeur(float valeur)`      Ajoute une valeur formatée à la
  (`static`)                          ligne de données

  `capteurs_ligne(void)`              Construit la ligne CSV/texte de la
                                      mesure
  -----------------------------------------------------------------------

## `bme280.c`

  ------------------------------------------------------------------------------
  Fonction                                   Utilité
  ------------------------------------------ -----------------------------------
  `bme280_init(void)`                        Lit les coefficients de calibration
                                             et configure le capteur

  `calculer_temperature(int adc_T)`          Convertit la valeur brute en °C et
  (`static`)                                 calcule `t_fine`

  `calculer_pression(int adc_P)` (`static`)  Convertit la valeur brute en hPa

  `calculer_humidite(int adc_H)` (`static`)  Convertit la valeur brute en %

  `bme280_lire(float *, float *, float *)`   Lit les registres et remplit
                                             température, humidité et pression
  ------------------------------------------------------------------------------

## `rtc_ds1307.c`

  ------------------------------------------------------------------------------------------
  Fonction                                               Utilité
  ------------------------------------------------------ -----------------------------------
  `bcd_vers_decimal(uint8_t)` (`static`)                 Convertit une valeur BCD en entier

  `decimal_vers_bcd(int)` (`static`)                     Convertit un entier en BCD

  `rtc_lire(int *, int *, int *, int *, int *, int *)`   Lit la date et l'heure

  `rtc_regler_heure(int, int, int)`                      Règle heures, minutes et secondes

  `rtc_regler_date(int, int, int)`                       Règle mois, jour et année

  `rtc_regler_jour(int)`                                 Règle le jour de la semaine

  `rtc_init(void)`                                       Vérifie la présence et l'état de
                                                         l'horloge
  ------------------------------------------------------------------------------------------

## `i2c_bus.c`

  ------------------------------------------------------------------------------------
  Fonction                                         Utilité
  ------------------------------------------------ -----------------------------------
  `i2c_init(void)`                                 Configure I2C1 sur PB8/PB9

  `i2c_lire(uint8_t, uint8_t, uint8_t *, int)`     Lit plusieurs octets à partir d'un
                                                   registre

  `i2c_ecrire(uint8_t, uint8_t, uint8_t *, int)`   Écrit plusieurs octets dans un
                                                   périphérique
  ------------------------------------------------------------------------------------

## `luminosite.c`

  -----------------------------------------------------------------------
  Fonction                            Utilité
  ----------------------------------- -----------------------------------
  `luminosite_init(void)`             Configure PA0 et ADC1 en 10 bits

  `luminosite_lire(void)`             Lance une conversion et renvoie la
                                      valeur ADC
  -----------------------------------------------------------------------

## `led.c`

  ----------------------------------------------------------------------------------
  Fonction                                       Utilité
  ---------------------------------------------- -----------------------------------
  `petite_pause(void)` (`static`)                Introduit une courte attente
                                                 pendant l'envoi des bits

  `envoyer_octet(uint8_t)` (`static`)            Envoie un octet à la puce P9813

  `envoyer_couleur(uint8_t, uint8_t, uint8_t)`   Envoie les composantes RGB
  (`static`)                                     

  `afficher(int)` (`static`)                     Affiche une couleur si elle est
                                                 différente de la précédente

  `led_init(void)`                               Configure PA8/PA9 et éteint la LED

  `led_couleur_mode(int)`                        Définit la couleur du mode

  `led_erreur(int, int)`                         Active ou désactive un indicateur
                                                 d'erreur

  `led_mise_a_jour(void)`                        Choisit la couleur ou le
                                                 clignotement à afficher
  ----------------------------------------------------------------------------------

## `console.c`

  -----------------------------------------------------------------------
  Fonction                            Utilité
  ----------------------------------- -----------------------------------
  `console_init(void)`                Configure USART2 en 115200 bauds

  `_write(int, char *, int)`          Redirige les sorties `printf()`
                                      vers l'UART

  `console_lire_caractere(char *)`    Essaie de lire un caractère sans
                                      attendre
  -----------------------------------------------------------------------

## `parametres.c`

  -----------------------------------------------------------------------
  Fonction                            Utilité
  ----------------------------------- -----------------------------------
  `lire_flash(uint32_t)` (`static`)   Lit un entier à une adresse Flash

  `SauvegardeEEPROM(void)` (`static`) Efface la page réservée et réécrit
                                      version + paramètres

  `RecuperationValeur(void)`          Recharge les paramètres depuis la
                                      Flash

  `InitialisationEEPROM(void)`        Charge les valeurs par défaut puis
                                      les sauvegarde

  `VerificationEEPROM(void)`          Vérifie la version et choisit
                                      récupération ou réinitialisation

  `AffichageData(void)`               Affiche les paramètres

  `est_un_nombre(char[])` (`static`)  Vérifie qu'un texte représente un
                                      entier

  `commande_clock(char[])` (`static`) Traite le réglage de l'heure

  `commande_date(char[])` (`static`)  Traite le réglage de la date

  `commande_day(char[])` (`static`)   Traite le réglage du jour de
                                      semaine

  `TraiterCommande(char[])`           Analyse et exécute une commande de
                                      configuration
  -----------------------------------------------------------------------

## `stockage.c`

  -----------------------------------------------------------------------
  Fonction                            Utilité
  ----------------------------------- -----------------------------------
  `stockage_init(void)`               Affiche que la carte SD est simulée

  `stockage_ajouter(void)`            Met à jour la date, la taille et la
                                      révision théoriques du journal
  -----------------------------------------------------------------------

## `systeme.c`

  Fonction                           Utilité
  ---------------------------------- ------------------------------------------
  `SystemClock_Config(void)`         Configure l'horloge du STM32 à 80 MHz
  `get_sp(void)` (`static inline`)   Lit le registre processeur SP
  `pile_utilisee(void)`              Estime l'utilisation de la pile
  `SysTick_Handler(void)`            Incrémente le compteur HAL à chaque tick
  `NMI_Handler(void)`                Gestionnaire NMI vide
  `HardFault_Handler(void)`          Bloque le programme en cas de HardFault

------------------------------------------------------------------------

# 14. Performances et limites connues

Les valeurs ci-dessous sont des réglages visibles dans le code, et non
des résultats issus d'une campagne de mesures de performance.

  Caractéristique                     Valeur / comportement
  ----------------------------------- --------------------------------------------------
  Fréquence système configurée        80 MHz
  Bus I2C1                            Configuration prévue à 100 kHz
  Console USART2                      115200 bauds, 8N1
  ADC luminosité                      Résolution 10 bits, valeurs 0 à 1023
  Tick HAL                            1 ms selon le commentaire de `SysTick_Handler()`
  Intervalle STANDARD par défaut      10 s en mode démo
  Intervalle ECONOMIQUE par défaut    20 s en mode démo
  Intervalle MAINTENANCE              5 s
  Timeout BME280 par défaut           30 s
  Appui long                          5 s
  Anti-rebond appui                   40 ms
  Validation relâchement              300 ms
  Inactivité CONFIGURATION            30 min
  Taille maximale de journal prévue   2048 octets par défaut
  Révisions prévues                   0 à 9
  Tampon de ligne de mesure           100 caractères
  Tampon de commande                  64 caractères

## 14.1 Limites et points à surveiller

1.  **Carte SD non opérationnelle :** aucune donnée n'est physiquement
    sauvegardée sur SD.
2.  **Révisions de journaux :** le compteur de révision est incrémenté,
    mais le nom utilisé dans le message d'ajout reste suffixé par
    `_0.LOG`. La rotation réelle est à finaliser.
3.  **Longueur des données :** `ligne` a une taille fixe de 100
    caractères. Il faut conserver une marge suffisante si le format est
    modifié.
4.  **Horodatage :** sans RTC fonctionnel, la ligne utilise `NA` pour
    date et heure et le nom de journal simulé devient `000000`.
5.  **Temps de mesure :** un défaut BME280 peut provoquer des tentatives
    jusqu'au timeout configuré et ralentir la boucle principale.
6.  **Traitement bloquant :** les lectures I2C, l'ADC, les `HAL_Delay()`
    et les transmissions UART sont effectués de manière bloquante.
7.  **Boutons :** la détection dépend de la fréquence d'exécution de la
    boucle principale ; ce n'est pas une interruption GPIO.
8.  **Mesure de pile :** `pile_utilisee()` fournit une estimation
    instantanée basée sur SP et `SRAM_END`, pas une mesure de pic
    maximal historique.
9.  **Flash :** les paramètres sont réécrits en effaçant la page
    réservée à chaque modification ; le nombre d'écritures Flash doit
    être pris en compte lors d'un usage intensif des commandes.
10. **Contrôle de date :** les bornes mois/jour sont contrôlées
    simplement, sans validation complète du calendrier.

------------------------------------------------------------------------

# 15. Guide utilisateur -- superviseur final

## 15.1 Mise en route

1.  Connecter la carte NUCLEO-L476RG au PC par USB.
2.  Ouvrir un moniteur série sur le port associé à la carte.
3.  Régler le terminal à 115200 bauds, 8 bits, sans parité, 1 bit
    d'arrêt.
4.  Mettre sous tension ou réinitialiser la carte.
5.  Lire les messages de démarrage : version, détection RTC/BME280,
    initialisation de la luminosité et état simulé du stockage.
6.  Si nécessaire, régler l'horloge en entrant au démarrage en mode
    CONFIGURATION.

## 15.2 Utilisation des modes

-   **STANDARD (vert)** : fonctionnement normal, acquisition et demande
    d'enregistrement simulé.
-   **ECONOMIQUE (bleu)** : acquisition deux fois moins fréquente que
    STANDARD.
-   **MAINTENANCE (orange)** : acquisition toutes les 5 secondes,
    affichage sans demande d'enregistrement. Le message console indique
    que la carte SD peut être retirée.
-   **CONFIGURATION (jaune)** : saisie de commandes dans le terminal,
    sans acquisition périodique.

## 15.3 Actions avec les boutons

  -----------------------------------------------------------------------
  Action                              Résultat
  ----------------------------------- -----------------------------------
  Maintenir rouge au démarrage        Démarre en CONFIGURATION
  (détection sur 500 ms)              

  Depuis STANDARD, maintenir rouge 5  Passe en MAINTENANCE
  s                                   

  Depuis STANDARD, maintenir vert 5 s Passe en ECONOMIQUE

  Depuis MAINTENANCE, maintenir rouge Retour STANDARD
  5 s                                 

  Depuis ECONOMIQUE, maintenir rouge  Retour STANDARD
  5 s                                 

  En CONFIGURATION, saisir `QUITTER`  Retour STANDARD
  -----------------------------------------------------------------------

Pour un appui long, maintenir le bouton pendant au moins 5 secondes sans
relâcher. Les filtres anti-rebond appliquent leurs propres délais.

## 15.4 Régler la date et l'heure

Au démarrage, maintenir le bouton rouge pour entrer en CONFIGURATION.
Dans le terminal, saisir les commandes puis appuyer sur Entrée :

``` text
CLOCK=14:30:00
DATE=10,23,2026
DAY=FRI
```

Les exemples sont illustratifs : utiliser la date, l'heure et le jour
réels. Le format de `DATE` est bien `MOIS,JOUR,ANNEE`.

## 15.5 Modifier un paramètre

Exemple pour régler l'intervalle à 15 secondes en mode démo :

``` text
LOG_INTERVALL=15
```

Exemple pour désactiver le capteur de luminosité :

``` text
LUMIN=0
```

Exemple pour modifier la taille maximale théorique des journaux :

``` text
FILE_MAX_SIZE=4096
```

Le terminal confirme la valeur enregistrée. Les paramètres sont
conservés dans la Flash interne et relus au prochain démarrage.

## 15.6 Vérifier l'état du système

Commandes utiles :

``` text
HELP
VERSION
PILE
```

-   `HELP` affiche les paramètres.
-   `VERSION` donne la version et le numéro de lot.
-   `PILE` affiche l'estimation de la pile utilisée.

## 15.7 Interpréter les données

Chaque ligne affichée correspond à une acquisition. Les champs sont
séparés par des points-virgules :

``` text
date;heure;temperature_C;humidite_pct;pression_hPa;luminosite;niveau_lumiere
```

-   `NA` signifie que la donnée n'est pas disponible, désactivée ou non
    retenue.
-   `faible`, `moyenne` et `forte` décrivent la catégorie de luminosité
    selon les seuils configurés.
-   La pression est affichée en hPa, la température en °C et l'humidité
    en %.

## 15.8 Que faire en cas de problème ?

  -----------------------------------------------------------------------
  Symptôme                            Vérification / action
  ----------------------------------- -----------------------------------
  LED rouge/bleue                     Vérifier le branchement et
                                      l'alimentation du RTC, ainsi que le
                                      bus I2C

  LED rouge/verte                     Vérifier le BME280, son
                                      alimentation, son adresse et les
                                      connexions I2C

  LED rouge/verte avec rouge plus     Une mesure dépasse une plage de
  court                               cohérence configurée

  LED rouge/blanche                   Le stockage simulé a atteint la
                                      limite théorique ; cela ne signale
                                      pas une écriture réelle sur SD

  Date ou heure `NA`                  Vérifier le DS1307 et régler
                                      date/heure depuis CONFIGURATION

  Valeurs BME `NA`                    Vérifier le capteur et le paramètre
                                      `TIMEOUT`

  Pas de texte dans le terminal       Vérifier le port série, le câble
                                      USB et le débit 115200

  Le mode ne change pas               Vérifier le bouton utilisé et
                                      maintenir l'appui au moins 5
                                      secondes
  -----------------------------------------------------------------------

------------------------------------------------------------------------

# 16. Compilation et téléversement

Le fichier `platformio.ini` configure :

-   la plateforme `ststm32` ;
-   la carte `nucleo_l476rg` ;
-   le framework `stm32cube` ;
-   le moniteur série à 115200 bauds ;
-   le téléversement avec le protocole `mbed` ;
-   le débogage avec `stlink`.

Depuis le dossier `build`, les commandes PlatformIO habituelles sont :

``` bash
pio run
pio run --target upload
pio device monitor
```

Les scripts `compiler_televerser.sh` et `compiler_televerser.bat` sont
également fournis dans le dossier `build`.

------------------------------------------------------------------------

# 17. Glossaire

  -----------------------------------------------------------------------
  Terme                               Définition
  ----------------------------------- -----------------------------------
  ADC                                 Analog-to-Digital Converter :
                                      convertisseur analogique-numérique

  BCD                                 Binary-Coded Decimal : codage
                                      décimal binaire utilisé par le RTC

  BME280                              Capteur combinant température,
                                      pression et humidité

  GPIO                                General-Purpose Input/Output :
                                      entrée/sortie numérique

  HAL                                 Hardware Abstraction Layer : couche
                                      logicielle d'accès au matériel

  I2C                                 Inter-Integrated Circuit : bus
                                      série à deux lignes principales

  ISR                                 Interrupt Service Routine : routine
                                      de traitement d'interruption

  RTC                                 Real-Time Clock : horloge temps
                                      réel

  SD                                  Secure Digital : support de
                                      stockage amovible

  SPI                                 Serial Peripheral Interface :
                                      interface série synchrone

  UART                                Universal Asynchronous
                                      Receiver-Transmitter : liaison
                                      série asynchrone

  USART                               Universal Synchronous/Asynchronous
                                      Receiver-Transmitter

  `NA`                                Valeur textuelle indiquant une
                                      donnée non disponible
  -----------------------------------------------------------------------

------------------------------------------------------------------------

# 18. Évolutions nécessaires avant une mise en service complète

Pour passer d'une simulation à un système opérationnel, les étapes
suivantes sont recommandées :

1.  Câbler correctement le lecteur SD et vérifier son interface
    électrique.
2.  Ajouter et configurer un pilote de carte SD compatible avec le
    matériel retenu.
3.  Intégrer un système de fichiers, par exemple FatFs.
4.  Implémenter le montage, la création, l'ouverture, l'écriture, la
    synchronisation et la fermeture des fichiers.
5.  Corriger la gestion des révisions afin que le nom de fichier utilisé
    corresponde réellement à la révision courante.
6.  Vérifier les erreurs d'écriture, de carte absente, de carte pleine
    et de retrait inattendu.
7.  Tester la persistance des journaux après redémarrage et coupure
    d'alimentation.
8.  Réaliser des mesures réelles de consommation, de durée d'acquisition
    et de charge CPU avant de publier des performances mesurées.

------------------------------------------------------------------------

# 19. Résumé

Le programme 3W réalise l'acquisition de données météorologiques avec un
STM32 NUCLEO-L476RG, un BME280, un RTC DS1307 et un capteur analogique
de luminosité. Il propose quatre modes de fonctionnement, une commande
par boutons avec anti-rebond logiciel, une console série de
configuration, une indication d'état par LED RGB et une sauvegarde
persistante des paramètres dans la Flash interne.

La gestion des journaux journaliers est présente sous forme de
**simulation** dans `stockage.c`. La carte SD ne reçoit pas encore les
mesures dans cette version. Cette distinction doit être conservée dans
toute présentation technique ou documentation destinée à l'utilisateur
final.
