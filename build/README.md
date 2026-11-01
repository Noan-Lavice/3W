# Worldwide Weather Watcher — Station météo embarquée (version 4.0)

Carte **NUCLEO-L476RG**, PlatformIO, framework **STM32Cube (HAL)**, code en **C**.
Structure du workshop « Pile d'exécution » : `HAL_Init()` → `SystemClock_Config()` → initialisations → `while (1)`.

## Montage

Empilement : NUCLEO → shield SD → Grove Base Shield (interrupteur sur **3V3**).

| Module | Prise | Broche STM32 |
|---|---|---|
| Double bouton (rouge / vert) | D5 | PB4 / PB10 |
| LED RGB (P9813) | D7 | PA8 (CI) / PA9 (DI) |
| Capteur de luminosité | A0 | PA0 |
| BME280 (adresse 0x76) | I2C du Base Shield | PB8 / PB9 |
| Horloge RTC DS1307 (adresse 0x68) | I2C du shield SD | PB8 / PB9 |

**Pas de GPS** sur notre montage. **Carte SD non câblée** (il manque les 3 fils ICSP) :
l'enregistrement est simulé, chaque ligne qui serait écrite est affichée dans la console
avec le nom du fichier et sa taille.

## Les fichiers (dans l'ordre de lecture conseillé)

| Fichier | Rôle |
|---|---|
| `include/config.h` | Broches, adresses, durées, `NA`, `MODE_DEMO` |
| `src/main.c` | Les 4 modes et la boucle principale |
| `src/systeme.c` | Code du workshop : horloge 80 MHz, mesure de la pile, interruptions |
| `src/console.c` | Console série : `printf` et lecture du clavier |
| `src/boutons.c` | Boutons : anti-rebond, appui long de 5 s |
| `src/led.c` | LED : couleur du mode et clignotement des erreurs |
| `src/i2c_bus.c` | Bus I2C (RTC + BME280) |
| `src/rtc_ds1307.c` | Date et heure |
| `src/bme280.c` | Température, humidité, pression (formules de la datasheet Bosch) |
| `src/luminosite.c` | Capteur de luminosité (ADC) |
| `src/capteurs.c` | Une mesure complète : TIMEOUT, NA, erreurs, ligne de texte |
| `src/stockage.c` | Fichiers `AAMMJJ_R.LOG` (simulés) |
| `src/parametres.c` | Paramètres en Flash et commandes du mode configuration |

## Les modes

| Mode | Comment y aller | LED | Ce qu'il fait |
|---|---|---|---|
| Standard | Démarrage normal | Verte | Une mesure toutes les `LOG_INTERVALL`, enregistrée |
| Configuration | Tenir le **rouge**, appuyer sur reset, continuer à tenir le rouge 1 s | Jaune | Pas de mesure, commandes dans la console. Retour en standard avec `QUITTER` ou après 30 min sans rien taper |
| Maintenance | **Rouge** 5 s depuis standard (rouge 5 s pour revenir) | Orange | Une mesure toutes les 5 s, affichée mais pas enregistrée : la carte SD peut être retirée |
| Économique | **Vert** 5 s depuis standard (rouge 5 s pour revenir) | Bleue | Mesures 2 fois moins souvent |

## Les erreurs (clignotement à 1 Hz)

Si plusieurs erreurs arrivent en même temps, la LED montre la plus importante (ordre du sujet).

| Erreur | LED | Quand |
|---|---|---|
| Horloge RTC | rouge / bleu | L'horloge ne répond pas |
| Capteur | rouge / vert | Le BME280 n'a pas répondu (`TIMEOUT`) 2 mesures de suite |
| Données incohérentes | rouge / vert (vert 2 fois plus long) | Valeur en dehors des seuils MIN / MAX |
| Carte SD pleine | rouge / blanc | Plus de révision libre (après `_9`) |
| Accès carte SD | rouge / blanc (blanc 2 fois plus long) | Prévu dans `led.c`, mais jamais déclenché : la carte SD est simulée |

Pas d'erreur GPS : on n'a pas de GPS.

## Les fichiers de mesures

- Une ligne par mesure : `2026-10-23;14:20:00;25.1;55.0;1006.5;224;faible`
  (date ; heure ; température °C ; humidité % ; pression hPa ; luminosité 0-1023 ; faible / moyenne / forte).
- Un capteur qui ne répond pas, ou qui est désactivé, donne `NA`.
- L'humidité vaut `NA` si la température n'est pas entre `HYGR_MINT` et `HYGR_MAXT`.
- Nom du fichier : `AAMMJJ_R.LOG` (ex : `261023_0.LOG`). On écrit toujours dans la révision 0.
  Quand elle dépasse `FILE_MAX_SIZE`, elle est archivée en `_1`, `_2` … `_9`. Après `_9` : erreur carte pleine.
  Un nouveau jour donne un nouveau fichier.

## Les paramètres (gardés dans la Flash, dernière page : 0x080FF800)

| Paramètre | Défaut | Min | Max | Rôle |
|---|---|---|---|---|
| `LOG_INTERVALL` | 10 | 1 | 1440 | Minutes entre 2 mesures (secondes si `MODE_DEMO = 1`) |
| `FILE_MAX_SIZE` | 2048 | 512 | 32768 | Taille max d'un fichier (octets) |
| `TIMEOUT` | 30 | 1 | 300 | Temps max pour qu'un capteur réponde (secondes) |
| `LUMIN` | 1 | 0 | 1 | Capteur de luminosité activé |
| `LUMIN_LOW` | 255 | 0 | 1023 | En dessous : « faible » |
| `LUMIN_HIGH` | 768 | 0 | 1023 | Au-dessus : « forte » |
| `TEMP_AIR` | 1 | 0 | 1 | Température activée |
| `MIN_TEMP_AIR` | -10 | -40 | 85 | Seuil bas (°C) |
| `MAX_TEMP_AIR` | 60 | -40 | 85 | Seuil haut (°C) |
| `HYGR` | 1 | 0 | 1 | Humidité activée |
| `HYGR_MINT` | 0 | -40 | 85 | Humidité prise en compte au-dessus de cette température |
| `HYGR_MAXT` | 50 | -40 | 85 | … et en dessous de celle-ci |
| `PRESSURE` | 1 | 0 | 1 | Pression activée |
| `PRESSURE_MIN` | 850 | 300 | 1100 | Seuil bas (hPa) |
| `PRESSURE_MAX` | 1080 | 300 | 1100 | Seuil haut (hPa) |

## Les commandes du mode configuration (moniteur série, 115200 bauds)

Majuscules ou minuscules, puis Entrée.

- `NOM=valeur` : un des 15 paramètres ci-dessus (ex : `LOG_INTERVALL=5`, `LOG_INTERVAL=5` marche aussi).
  La valeur doit être un nombre entier entre le min et le max, sinon un message d'erreur s'affiche.
- `CLOCK=HH:MM:SS`, `DATE=MOIS,JOUR,ANNEE`, `DAY=MON` (TUE, WED, THU, FRI, SAT, SUN)
- `RESET` (valeurs par défaut), `VERSION` (version et numéro de lot), `PILE` (pile utilisée),
  `HELP` (liste des paramètres et des commandes), `QUITTER` (retour au mode standard)

`MODE_DEMO = 1` dans `config.h` : `LOG_INTERVALL` est compté en secondes pour la démo
(mettre `0` pour des minutes, comme le sujet).

## Ce qui vient du workshop « Pile d'exécution »

| Élément du workshop | Où dans le projet |
|---|---|
| `HAL_Init()` puis `SystemClock_Config()` en premier | `main()` dans `main.c` |
| Horloge HSI 16 MHz → PLL → 80 MHz, `FLASH_LATENCY_4` | `SystemClock_Config()` dans `systeme.c` |
| `SysTick_Handler` (avec `HAL_IncTick()`), `NMI_Handler`, `HardFault_Handler` | `systeme.c` |
| `get_sp()` en assembleur inline et `SRAM_END` | `systeme.c` et `systeme.h` |
| `pile_utilisee = SRAM_END - SP`, affichée avec `%lu` | `systeme.c`, affichée au démarrage et avec la commande `PILE` |
| UART2 sur PA2 / PA3, 115200 bauds, par le ST-Link | `console_init()` dans `console.c` |
| `HAL_UART_Transmit` pour envoyer le texte | `_write()` dans `console.c` (utilisée par `printf`) |
| Variables globales plutôt que locales (pas sur la pile) | `mesure` et `ligne` dans `capteurs.c`, variables de `main.c` |
| Pas de fonction récursive | Tout est fait avec des boucles |
| `volatile` | Boucle d'attente de `led.c` |
| Débogage `-O0 -g` avec le ST-Link | `platformio.ini` |

Pour voir la pile grandir avec le débogueur : mettre un breakpoint dans `bme280_lire()` (`bme280.c`).
Le CALL STACK montre alors `bme280_lire ← mesurer_bme ← capteurs_mesurer ← mesurer ← main`.
Avec F11 sur la ligne `i2c_lire(...)`, on entre dans `i2c_lire()` et SP diminue dans REGISTERS.

## Compiler et téléverser

Dans VS Code avec PlatformIO : bouton **Build** puis **Upload**, puis **Serial Monitor** (115200 bauds).
Ou lancer `compiler_televerser.bat` (Windows) / `compiler_televerser.sh` (Linux, macOS).

`build_flags = -Wl,-u,_printf_float` dans `platformio.ini` est obligatoire : sans lui, `printf("%.1f")`
n'affiche rien sur la carte (la petite bibliothèque C utilisée ne gère pas les nombres à virgule par défaut).

## Différences avec le sujet (choix du groupe)

- Pas de GPS, donc pas d'erreur GPS et rien à désactiver en mode économique à part la fréquence des mesures.
- Carte SD simulée (voir plus haut).
- En mode économique, rouge 5 s ramène au mode standard. Le mode maintenance s'ouvre donc depuis le mode standard.
- Commandes en plus : `HELP`, `PILE`, `QUITTER`.
- `MODE_DEMO` : intervalle en secondes pour la démonstration.

## Limites connues

- Pendant qu'on attend un capteur qui ne répond pas (jusqu'à `TIMEOUT` secondes), les boutons ne sont pas lus.
- Si les deux boutons sont vus appuyés exactement à la même milliseconde, c'est le rouge qui compte.
- La carte SD est simulée : rien n'est vraiment écrit.
