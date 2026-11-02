# 3W -- Worldwide Weather Watcher

## Guide d'utilisation

**Carte :** STM32 NUCLEO-L476RG\
**Version du programme :** 4.0

------------------------------------------------------------------------

## 1. À quoi sert le système ?

3W est une station météo qui relève régulièrement : - la température ; -
l'humidité de l'air ; - la pression atmosphérique ; - la luminosité ; -
la date et l'heure.

Les résultats sont affichés sur un ordinateur. Une LED indique le mode
actif ou signale certains problèmes.

## 2. Démarrer le système

1.  Reliez la carte à l'ordinateur avec le câble USB.
2.  Ouvrez un terminal série sur le port de la carte.
3.  Réglez la vitesse sur **115200 bauds**.
4.  Allumez ou redémarrez la carte.
5.  Observez les messages dans le terminal.

Le système démarre normalement en mode STANDARD. Pour démarrer en mode
CONFIGURATION, maintenez le bouton rouge pendant le démarrage.

## 3. Les modes de fonctionnement

  -----------------------------------------------------------------------
  Couleur de la LED       Mode                    Fonctionnement
  ----------------------- ----------------------- -----------------------
  Verte                   STANDARD                Mesures régulières,
                                                  toutes les 10 secondes
                                                  par défaut.

  Bleue                   ECONOMIQUE              Mesures moins
                                                  fréquentes, toutes les
                                                  20 secondes par défaut.

  Orange                  MAINTENANCE             Mesures plus
                                                  fréquentes, toutes les
                                                  5 secondes.

  Jaune                   CONFIGURATION           Modification des
                                                  réglages depuis le
                                                  terminal.
  -----------------------------------------------------------------------

### Changer de mode avec les boutons

Maintenez le bouton concerné pendant **5 secondes**.

  Action                            Résultat
  --------------------------------- -----------------------
  Bouton rouge depuis STANDARD      Passe en MAINTENANCE.
  Bouton vert depuis STANDARD       Passe en ECONOMIQUE.
  Bouton rouge depuis MAINTENANCE   Retour en STANDARD.
  Bouton rouge depuis ECONOMIQUE    Retour en STANDARD.

En mode CONFIGURATION, saisissez `QUITTER` dans le terminal pour revenir
en STANDARD. Le retour se fait aussi automatiquement après 30 minutes
sans activité.

## 4. Lire les mesures

Chaque ligne affichée dans le terminal correspond à une mesure.

  Information         Signification
  ------------------- ------------------------------------------
  Date et heure       Moment où la mesure a été effectuée
  Température (°C)    Température de l'air
  Humidité (%)        Humidité relative de l'air
  Pression (hPa)      Pression atmosphérique
  Luminosité          Valeur mesurée par le capteur de lumière
  Niveau de lumière   Catégorie : faible, moyenne ou forte

`NA` signifie que la donnée n'est pas disponible ou que la mesure
correspondante est désactivée.

## 5. Régler la date et l'heure

Entrez en mode CONFIGURATION, puis saisissez les commandes dans le
terminal.

Exemple :

``` text
CLOCK=14:30:00
DATE=10,23,2026
DAY=FRI
```

Remplacez les valeurs par la date et l'heure souhaitées.

-   `CLOCK` règle l'heure au format heures:minutes:secondes.
-   `DATE` règle la date au format mois,jour,année.
-   `DAY` règle le jour : `MON`, `TUE`, `WED`, `THU`, `FRI`, `SAT` ou
    `SUN`.

## 6. Modifier les réglages

Les réglages se modifient en mode CONFIGURATION. Saisissez le nom du
réglage, suivi de `=` et de la nouvelle valeur.

Exemples :

``` text
LOG_INTERVALL=15
LUMIN=0
FILE_MAX_SIZE=4096
```

  Réglage           Utilité
  ----------------- ----------------------------------------------------
  `LOG_INTERVALL`   Intervalle entre les mesures en mode STANDARD.
  `LUMIN`           Active (`1`) ou désactive (`0`) la luminosité.
  `TEMP_AIR`        Active (`1`) ou désactive (`0`) la température.
  `HYGR`            Active (`1`) ou désactive (`0`) l'humidité.
  `PRESSURE`        Active (`1`) ou désactive (`0`) la pression.
  `FILE_MAX_SIZE`   Taille maximale prévue pour un journal de mesures.

Le programme vérifie que les valeurs saisies sont autorisées. Les
réglages sont conservés dans la mémoire interne de la carte après
redémarrage.

### Commandes utiles

  Commande    Utilité
  ----------- --------------------------------------------------------
  `HELP`      Affiche l'aide et les réglages disponibles.
  `VERSION`   Affiche la version du programme.
  `PILE`      Affiche une estimation de la mémoire de pile utilisée.
  `RESET`     Rétablit les réglages par défaut.
  `QUITTER`   Quitte le mode CONFIGURATION.

**Attention :** `RESET` remplace les réglages personnalisés par les
réglages par défaut.

## 7. Comprendre les alertes lumineuses

En cas de problème, la LED peut clignoter en alternant le rouge avec une
autre couleur.

  -----------------------------------------------------------------------
  Clignotement                        Signification
  ----------------------------------- -----------------------------------
  Rouge et bleu                       Problème de communication avec
                                      l'horloge.

  Rouge et vert                       Problème de communication avec le
                                      capteur météo.

  Rouge et vert avec un rythme        Une mesure est en dehors des
  différent                           limites prévues.

  Rouge et blanc                      La limite de taille prévue pour le
                                      stockage est atteinte.
  -----------------------------------------------------------------------

En cas d'alerte, vérifiez les branchements et les messages du terminal.
Si le problème persiste, demandez une vérification technique.

## 8. Stockage des mesures

Le programme prévoit de regrouper les mesures d'une même journée dans un
journal. Le nom prévu suit le format `AAMMJJ_R.LOG`. Si la taille
maximale est atteinte, une nouvelle révision est prévue.

**Important : dans cette version, l'enregistrement sur la carte SD n'est
pas encore opérationnel.** Le programme simule la gestion des fichiers
et affiche des informations dans le terminal, mais il n'écrit pas
réellement les mesures sur une carte SD. Les mesures affichées ne sont
donc pas conservées sur la carte SD.

## 9. En cas de problème

  -----------------------------------------------------------------------
  Problème                            Que faire ?
  ----------------------------------- -----------------------------------
  Aucun message dans le terminal      Vérifier le câble USB, le port
                                      série et la vitesse de 115200
                                      bauds.

  Date ou heure `NA`                  Vérifier l'horloge et régler la
                                      date et l'heure en mode
                                      CONFIGURATION.

  Mesures météo `NA`                  Vérifier le branchement du capteur
                                      météo.

  LED clignotante                     Consulter le tableau des alertes et
                                      vérifier les connexions.

  Un bouton ne change pas le mode     Le maintenir appuyé pendant au
                                      moins 5 secondes.

  Un réglage est refusé               Vérifier la commande et la valeur
                                      avec `HELP`.
  -----------------------------------------------------------------------

## 10. Memo

-   **Vert :** STANDARD.
-   **Bleu :** ECONOMIQUE.
-   **Orange :** MAINTENANCE.
-   **Jaune :** CONFIGURATION.
-   Maintenir un bouton pendant 5 secondes pour changer de mode.
-   Utiliser un terminal série réglé à 115200 bauds.
-   `QUITTER` permet de sortir du mode CONFIGURATION.
-   La carte SD ne sauvegarde pas encore réellement les mesures dans
    cette version.

------------------------------------------------------------------------

*Guide destiné à l'utilisateur final du système 3W.*
