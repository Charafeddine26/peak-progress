# Peak Progress — Guide d'Onboarding

Bienvenue dans l'équipe ! Ce document t'explique comment fonctionne **Peak Progress** de A à Z. Tu n'as pas besoin de coder — juste de comprendre et pouvoir expliquer le projet.

---

## 1. Vue d'ensemble

### Le concept

Peak Progress est un **objet tangible** qui transforme une habitude quotidienne (méditation, sport, lecture…) en escalade de montagne. Chaque fois que tu enregistres une activité via l'app web, un petit grimpeur monte physiquement le long d'une montagne imprimée en 3D. Quand tu atteins le sommet, une nouvelle montagne plus difficile se débloque.

### L'objet physique

L'objet se compose de :

1. **La montagne 3D** — une forme imprimée en 3D avec un rail le long duquel un petit personnage (le grimpeur) se déplace grâce à un fil et une poulie
2. **Le servo-moteur** — un petit moteur qui tire le fil pour faire monter ou descendre le grimpeur
3. **Le socle** — contient l'Arduino et les câbles

### L'expérience utilisateur

```
Appuyer dans l'app web → L'Arduino reçoit via Bluetooth → Le grimpeur monte → Sommet atteint → Nouvelle montagne !
```

### Les 2 composants du système

| Composant | Rôle | Technologie |
|-----------|------|-------------|
| **Arduino** | Cerveau : logique, mémoire, servo, Bluetooth | Arduino Uno WiFi Rev.2 |
| **Web App** | Interface utilisateur : dashboard + commandes | Page web (HTML/JS) via Web Bluetooth |

### Schéma des connexions

```
┌──────────────────────────────────┐
│          ARDUINO                 │
│       Uno WiFi Rev.2             │
│                                  │
│  • Servo-moteur (grimpeur)       │
│  • EEPROM (mémoire permanente)   │
│  • BLE (Bluetooth Low Energy)    │
└───────────┬──────────────────────┘
            │ Bluetooth (BLE)
            │ bidirectionnel
            ▼
┌──────────────────────────────────┐
│          WEB APP                 │
│  (téléphone ou PC)               │
│                                  │
│  • Bouton "Log Activity"         │
│  • Dashboard en temps réel       │
│  • Liste des montagnes           │
└──────────────────────────────────┘
```

---

## 2. Le Parcours d'une Activité

C'est le fil rouge de ce document. On va suivre ce qui se passe quand l'utilisateur **appuie sur "Log Activity"** dans l'app web, étape par étape, fichier par fichier.

---

### Étape 1 — L'action utilisateur

**L'app web (navigateur)**

L'utilisateur ouvre la page web sur son téléphone (Chrome/Android) ou son PC. Il appuie sur le bouton « Log Activity ». L'app envoie alors une commande via Bluetooth à l'Arduino.

Côté web, c'est du JavaScript — pas besoin de comprendre le code. Ce qu'il faut retenir : l'app envoie l'octet `0x01` (le code pour « enregistrer une activité ») dans la `characteristic` de commande BLE.

---

### Étape 2 — La réception BLE (Bluetooth)

**Fichier : `ble_service.h`**

L'Arduino reçoit la commande Bluetooth. Le `BLE` (Bluetooth Low Energy) fonctionne avec un système de « boîtes à lettres » numériques appelées `characteristic`. L'app web écrit dans la boîte de commande, et l'Arduino la lit.

```cpp
#include <ArduinoBLE.h>

// ── Identifiants uniques du service BLE ──
#define SERVICE_UUID       "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHAR_PROGRESS_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"  // Données → téléphone
#define CHAR_COMMAND_UUID  "19B10003-E8F2-537E-4F6C-D104768A1214"  // Commandes ← téléphone

// ── Codes de commande ──
#define CMD_LOG_ACTIVITY   0x01   // "Enregistrer une activité"
#define CMD_RESET          0x02   // "Réinitialiser la progression"

// ── Les objets BLE ──
BLEService peakService(SERVICE_UUID);
BLECharacteristic progressChar(CHAR_PROGRESS_UUID, BLERead | BLENotify, 8); // Lecture + notification
BLEByteCharacteristic commandChar(CHAR_COMMAND_UUID, BLEWrite);              // Écriture

// ── Initialisation : rendre l'Arduino visible en Bluetooth ──
inline bool initBLE() {
  if (!BLE.begin()) return false;            // Démarrer le Bluetooth

  BLE.setLocalName("PeakProgress");          // Nom visible par le téléphone
  BLE.setAdvertisedService(peakService);

  peakService.addCharacteristic(progressChar);
  peakService.addCharacteristic(commandChar);

  BLE.addService(peakService);
  BLE.advertise();                           // Commencer à se rendre visible
  return true;
}
```

> **C++ : c'est quoi le `BLE` ?**
>
> `BLE` = Bluetooth Low Energy. C'est une version économe en énergie du Bluetooth, idéale pour les objets connectés. Le fonctionnement :
>
> 1. L'Arduino **annonce** sa présence (comme un phare)
> 2. Le téléphone **se connecte** au service « PeakProgress »
> 3. Le téléphone **écrit** dans la boîte de commande pour déclencher une action
> 4. L'Arduino **publie** ses données dans la boîte de progression, et le téléphone les lit
>
> Les `UUID` sont des identifiants uniques (comme des numéros de téléphone) pour chaque boîte.

> **C++ : c'est quoi `#include` et les fichiers `.h` ?**
>
> Imagine un livre de recettes divisé en chapitres. `#include "progress.h"` revient à dire « ouvre le chapitre *progress* et lis-le ici ». Les fichiers `.h` (header) contiennent du code réutilisable. Ça évite de tout mettre dans un seul fichier énorme.
>
> `#include <ArduinoBLE.h>` avec des `< >` inclut une bibliothèque externe (fournie par le système).
> `#include "progress.h"` avec des `" "` inclut un fichier de notre projet.

---

### Étape 3 — La boucle principale et le traitement des commandes

**Fichier : `arduino_main.ino`**

L'Arduino tourne en boucle infinie. À chaque tour, il vérifie si une commande BLE est arrivée. Si oui, il l'exécute.

```cpp
#include "mountains.h"
#include "progress.h"
#include "servo_control.h"
#include "ble_service.h"

UserProgress userProgress;   // La fiche de progression de l'utilisateur

// ── Fonction de démarrage (une seule fois) ──
void setup() {
  loadProgress(userProgress);                  // Charger la sauvegarde
  checkAndUnlockMountains(userProgress);       // Vérifier les montagnes débloquées

  initServo();                                 // Initialiser le servo
  positionClimberForCurrentProgress(userProgress);  // Placer le grimpeur

  if (initBLE()) {                             // Démarrer le Bluetooth
    updateAllBLE(userProgress);                // Publier les données
  }
}

// ── Boucle infinie ──
void loop() {
  uint8_t bleCmd = checkBLECommand();          // Vérifier si commande reçue

  if (bleCmd != 0x00) {
    handleBLECommand(bleCmd);                  // Exécuter la commande
  }
}

// ── Traitement des commandes BLE ──
void handleBLECommand(uint8_t cmd) {
  switch (cmd) {
    case CMD_LOG_ACTIVITY:                     // 0x01 → enregistrer activité
      logActivity();
      break;
    case CMD_RESET:                            // 0x02 → réinitialiser
      resetProgress(userProgress);
      descendToBase();
      updateAllBLE(userProgress);
      break;
  }
}
```

> **C++ : c'est quoi `setup()` et `loop()` ?**
>
> Dans Arduino, chaque programme a deux fonctions obligatoires :
> - `setup()` s'exécute **une seule fois** au démarrage (comme préparer ses affaires avant une randonnée)
> - `loop()` s'exécute **en boucle infinie** ensuite (comme marcher pas après pas)
>
> Tout le comportement du programme vient de ce que tu mets dans `loop()`.

> **C++ : c'est quoi une variable ?**
>
> Une variable, c'est une boîte avec une étiquette. Par exemple :
> - `uint8_t bleCmd` crée une boîte nommée `bleCmd` qui stocke un nombre de 0 à 255
> - `UserProgress userProgress` crée une fiche de progression (voir étape 4)
>
> Les types courants dans notre code :
> - `int` — nombre entier classique
> - `uint8_t` — nombre positif de 0 à 255 (1 octet)
> - `uint16_t` — nombre positif de 0 à 65 535 (2 octets)
> - `bool` — vrai (`true`) ou faux (`false`)

---

### Étape 4 — La logique de progression

**Fichier : `progress.h` + `arduino_main.ino`**

Quand l'Arduino reçoit la commande `CMD_LOG_ACTIVITY`, il appelle `logActivity()`. C'est là que la magie opère : on incrémente les compteurs, on bouge le grimpeur, et on vérifie si le sommet est atteint.

```cpp
// ── La structure qui stocke toute la progression de l'utilisateur ──
struct UserProgress {
  uint16_t totalSessionsAllTime;       // Total de sessions depuis le début (0-65535)
  uint8_t  currentMountainIndex;       // Quelle montagne on grimpe (0-8)
  uint8_t  sessionsOnCurrentMountain;  // Sessions faites sur cette montagne
  uint8_t  summitsReached;             // Nombre de sommets atteints au total
  uint16_t unlockedBitfield;           // Quelles montagnes sont débloquées (bitfield)
  uint8_t  currentStreakDays;          // Jours consécutifs actifs
  uint8_t  longestStreakDays;          // Record de jours consécutifs
};
```

```cpp
// ── dans arduino_main.ino — la fonction appelée à chaque activité ──
void logActivity() {
  const Mountain &mtn = MOUNTAIN_LIBRARY[userProgress.currentMountainIndex];
  uint8_t totalSess = SESSIONS_FOR_MOUNTAIN(mtn);

  if (userProgress.sessionsOnCurrentMountain >= totalSess) return; // Déjà au sommet

  userProgress.sessionsOnCurrentMountain++;   // +1 session
  userProgress.totalSessionsAllTime++;        // +1 au compteur global
  userProgress.currentStreakDays++;            // +1 jour de suite

  if (userProgress.currentStreakDays > userProgress.longestStreakDays) {
    userProgress.longestStreakDays = userProgress.currentStreakDays;  // Nouveau record !
  }

  // Faire monter le grimpeur
  int angle = calculateServoAngle(userProgress.sessionsOnCurrentMountain, totalSess);
  moveClimberSmooth(angle);

  saveProgress(userProgress);    // Sauvegarder en mémoire permanente

  // Vérifier si on est au sommet
  if (userProgress.sessionsOnCurrentMountain >= totalSess) {
    summitReached();             // Célébration !
  }

  updateAllBLE(userProgress);   // Mettre à jour le Bluetooth
}
```

> **C++ : c'est quoi un `struct` ?**
>
> Un `struct`, c'est un **formulaire**. Au lieu d'avoir 7 variables séparées qui traînent, on les regroupe dans une fiche. `UserProgress` est notre fiche qui contient tout ce qu'on sait sur la progression de l'utilisateur.
>
> Pour accéder à un champ : `userProgress.summitsReached` (le point `.` signifie « le champ *summitsReached* de la fiche *userProgress* »).

> **C++ : c'est quoi `if/else` ?**
>
> C'est une décision. Comme un embranchement sur un sentier :
> ```cpp
> if (sessions >= totalSess) {    // SI on a fait assez de sessions...
>   summitReached();              //   ...on est au sommet !
> }                               // SINON on continue
> ```
> Le code entre `{ }` ne s'exécute **que si** la condition entre `( )` est vraie.

> **C++ : c'est quoi une fonction ?**
>
> Une fonction, c'est une **recette réutilisable**. `logActivity()` est une recette qui dit « voici les étapes à faire quand quelqu'un enregistre une activité ». On peut l'appeler depuis n'importe où avec juste son nom : `logActivity();`
>
> Une fonction peut aussi prendre des **paramètres** (ingrédients) :
> ```cpp
> int calculateServoAngle(uint8_t sessions, uint8_t totalSessions)
> //                      ^^^^^^^^^^^^^^   ^^^^^^^^^^^^^^^^^^^^^
> //                      ingrédient 1     ingrédient 2
> ```

---

### Étape 5 — La sauvegarde EEPROM

**Fichier : `progress.h`**

L'`EEPROM` est une mémoire permanente intégrée à l'Arduino. Même si on débranche le câble USB, les données restent. C'est comme graver dans la pierre au lieu d'écrire au crayon.

```cpp
#define EEPROM_MAGIC       0xA5    // Nombre magique pour détecter le premier démarrage
#define EEPROM_ADDR_MAGIC  0       // Adresse 0 : le nombre magique
#define EEPROM_ADDR_DATA   1       // Adresse 1+ : les données de progression

// ── Sauvegarder la progression ──
inline void saveProgress(const UserProgress &prog) {
  EEPROM.put(EEPROM_ADDR_DATA, prog);   // Écrit toute la struct en mémoire
}

// ── Charger la progression au démarrage ──
inline void loadProgress(UserProgress &prog) {
  uint8_t magic = EEPROM.read(EEPROM_ADDR_MAGIC);  // Lire le nombre magique

  if (magic != EEPROM_MAGIC) {
    // Premier démarrage ! Aucune donnée sauvegardée.
    initProgressDefaults(prog);                      // Valeurs par défaut
    EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);  // Marquer comme initialisé
    saveProgress(prog);
    return;
  }

  EEPROM.get(EEPROM_ADDR_DATA, prog);  // Charger les données sauvegardées
}

// ── Valeurs par défaut ──
inline void initProgressDefaults(UserProgress &prog) {
  memset(&prog, 0, sizeof(UserProgress));     // Tout mettre à zéro
  prog.unlockedBitfield = 0x0001;             // Débloquer la première montagne (bit 0)
}
```

> **C++ : c'est quoi `EEPROM` ?**
>
> `EEPROM` = Electrically Erasable Programmable Read-Only Memory. C'est une petite mémoire (1 Ko sur notre Arduino) qui **survit aux redémarrages**. On l'utilise pour sauvegarder la progression de l'utilisateur.
>
> - `EEPROM.put(adresse, données)` — écrit des données à une adresse
> - `EEPROM.get(adresse, données)` — lit des données depuis une adresse
> - `EEPROM.read(adresse)` — lit un seul octet

> **C++ : c'est quoi `uint8_t` et `uint16_t` ?**
>
> Ce sont des types de nombres précis en taille :
> - `uint8_t` = nombre positif sur **8 bits** (1 octet) → de 0 à 255
> - `uint16_t` = nombre positif sur **16 bits** (2 octets) → de 0 à 65 535
>
> Sur Arduino, la mémoire est très limitée. Utiliser le type le plus petit possible permet d'économiser de la place. `totalSessionsAllTime` utilise `uint16_t` parce qu'on pourrait dépasser 255 sessions, mais `currentMountainIndex` utilise `uint8_t` car il n'y a que 9 montagnes.

---

### Étape 6 — Le mouvement du servo

**Fichier : `servo_control.h`**

Le servo-moteur fait physiquement monter le grimpeur le long de la montagne. L'angle va de 20° (camp de base, en bas) à 170° (sommet, en haut). Le mouvement est **progressif** : 1° à la fois pour un effet visuel fluide.

```cpp
#define SERVO_PIN        8      // Broche Arduino connectée au servo
#define SERVO_MIN_ANGLE  20     // Position camp de base (en bas)
#define SERVO_MAX_ANGLE  170    // Position sommet (en haut)
#define SERVO_STEP_DELAY 15     // 15ms entre chaque degré = mouvement fluide

Servo climberServo;              // L'objet qui contrôle le servo
int currentServoAngle = SERVO_MIN_ANGLE;  // Position actuelle

// ── Calculer l'angle en fonction de la progression ──
inline int calculateServoAngle(uint8_t sessions, uint8_t totalSessions) {
  if (totalSessions == 0) return SERVO_MIN_ANGLE;
  if (sessions >= totalSessions) return SERVO_MAX_ANGLE;  // Sommet !

  int range = SERVO_MAX_ANGLE - SERVO_MIN_ANGLE;  // 170 - 20 = 150°
  return SERVO_MIN_ANGLE + (range * (int)sessions) / (int)totalSessions;
  // Exemple : 3 sessions sur 7 → 20 + (150 × 3) / 7 = 84°
}

// ── Mouvement fluide degré par degré ──
inline void moveClimberSmooth(int targetAngle) {
  targetAngle = constrain(targetAngle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  if (targetAngle == currentServoAngle) return;

  climberServo.attach(SERVO_PIN);  // Activer le servo

  int step = (targetAngle > currentServoAngle) ? 1 : -1;  // Monter ou descendre ?

  while (currentServoAngle != targetAngle) {
    currentServoAngle += step;                // Avancer d'1°
    climberServo.write(currentServoAngle);    // Envoyer la position au servo
    delay(SERVO_STEP_DELAY);                  // Attendre 15ms
  }

  climberServo.detach();   // Désactiver pour économiser de l'énergie
}
```

> **C++ : fonctions avec paramètres**
>
> `calculateServoAngle(sessions, totalSessions)` prend deux ingrédients et renvoie un résultat (l'angle). C'est comme une formule :
>
> ```
> angle = base + (plage × sessions) / sessionsTotal
> ```
>
> `constrain(valeur, min, max)` empêche une valeur de sortir d'un intervalle. Si `targetAngle` vaut 200, `constrain` le ramène à 170 (le maximum).

---

### Étape 7 — La vérification du sommet

**Fichier : `arduino_main.ino`**

Si l'utilisateur a fait assez de sessions, il atteint le sommet ! La fonction `summitReached()` débloque la prochaine montagne et redescend le grimpeur au camp de base.

```cpp
void summitReached() {
  const Mountain &mtn = MOUNTAIN_LIBRARY[userProgress.currentMountainIndex];

  delay(3000);   // Pause de célébration

  userProgress.summitsReached++;             // +1 sommet
  checkAndUnlockMountains(userProgress);     // Débloquer les nouvelles montagnes

  uint8_t nextIdx = findNextMountain(userProgress);  // Trouver la prochaine

  userProgress.currentMountainIndex = nextIdx;       // Changer de montagne
  userProgress.sessionsOnCurrentMountain = 0;        // Remettre le compteur à zéro

  saveProgress(userProgress);   // Sauvegarder
  descendToBase();              // Redescendre le grimpeur au camp de base

  updateAllBLE(userProgress);   // Informer l'app web
}
```

```cpp
// ── Débloquer les montagnes gagnées ──
inline void checkAndUnlockMountains(UserProgress &prog) {
  for (uint8_t i = 0; i < NUM_MOUNTAINS; i++) {
    if (prog.summitsReached >= MOUNTAIN_LIBRARY[i].unlockAfterSummits) {
      unlockMountain(prog, i);   // Activer le bit correspondant
    }
  }
}
```

---

### Étape 8 — La notification BLE (retour vers l'app web)

**Fichier : `ble_service.h`**

Après chaque action, l'Arduino met à jour ses données BLE. L'app web reçoit automatiquement la notification et rafraîchit le dashboard.

```cpp
inline void updateBLEProgress(const UserProgress &prog) {
  const Mountain &mtn = MOUNTAIN_LIBRARY[prog.currentMountainIndex];

  uint8_t data[8] = {
    prog.currentMountainIndex,           // Octet 0 : quelle montagne
    prog.sessionsOnCurrentMountain,      // Octet 1 : sessions faites
    SESSIONS_FOR_MOUNTAIN(mtn),          // Octet 2 : sessions nécessaires
    prog.summitsReached,                 // Octet 3 : sommets atteints
    prog.currentStreakDays,              // Octet 4 : streak actuel
    prog.longestStreakDays,              // Octet 5 : meilleur streak
    (uint8_t)(prog.totalSessionsAllTime >> 8),   // Octet 6 : total (partie haute)
    (uint8_t)(prog.totalSessionsAllTime & 0xFF)  // Octet 7 : total (partie basse)
  };

  progressChar.writeValue(data, 8);     // Envoyer les 8 octets au téléphone
}
```

> **C++ : `BLE` `characteristic` et `notify`**
>
> Une `characteristic` BLE est une boîte à lettres numérique :
> - `BLERead` — le téléphone peut lire la boîte
> - `BLENotify` — le téléphone est prévenu automatiquement quand le contenu change
> - `BLEWrite` — le téléphone peut écrire dans la boîte (pour envoyer des commandes)
>
> Ici, `progressChar` a `BLERead | BLENotify` : le téléphone peut lire les données ET reçoit une notification à chaque mise à jour. C'est ce qui permet au dashboard de se rafraîchir en temps réel.

> **C++ : c'est quoi un `bitfield` et les opérations bit à bit ?**
>
> `unlockedBitfield` est un nombre de 16 bits où **chaque bit représente une montagne** :
>
> ```
> Bit :  8  7  6  5  4  3  2  1  0
> Mtn :  K2 Ev De Ki Ma MB ME PS CL
>
> 0x0001 = 0000000000000001  → seule la Colline Locale est débloquée
> 0x000F = 0000000000001111  → les 4 premières montagnes sont débloquées
> ```
>
> - `prog.unlockedBitfield |= (1 << i)` — débloquer la montagne numéro `i`
> - `(prog.unlockedBitfield >> i) & 0x01` — vérifier si la montagne `i` est débloquée
>
> C'est un moyen très compact de stocker 9 oui/non dans un seul nombre de 2 octets.

---

## 4. Le Système de Montagnes

### Tableau des 9 montagnes (4 tiers)

| # | Nom | Tier | Sessions | Semaines | Débloquée après |
|---|-----|------|----------|----------|-----------------|
| 0 | Colline Locale | 1 | 7 | 1 | — (départ) |
| 1 | Petit Sommet | 1 | 7 | 1 | 1 sommet |
| 2 | Mont d'Entraînement | 1 | 7 | 1 | 2 sommets |
| 3 | Mont Blanc | 2 | 14 | 2 | 3 sommets |
| 4 | Matterhorn | 2 | 14 | 2 | 4 sommets |
| 5 | Kilimanjaro | 3 | 21 | 3 | 5 sommets |
| 6 | Denali | 3 | 21 | 3 | 6 sommets |
| 7 | Everest | 4 | 28 | 4 | 7 sommets |
| 8 | K2 | 4 | 28 | 4 | 8 sommets |

### Sessions par tier

- **Tier 1** — 7 sessions (1 semaine) : Colline, Petit Sommet, Mont d'Entraînement
- **Tier 2** — 14 sessions (2 semaines) : Mont Blanc, Matterhorn
- **Tier 3** — 21 sessions (3 semaines) : Kilimanjaro, Denali
- **Tier 4** — 28 sessions (4 semaines) : Everest, K2

### Logique de déverrouillage

Le champ `unlockedBitfield` est un nombre de 16 bits. Chaque bit correspond à une montagne. Quand l'utilisateur atteint un sommet, la fonction `checkAndUnlockMountains()` vérifie pour chaque montagne si le nombre de sommets atteints suffit pour la débloquer.

Exemple : après avoir gravi la Colline Locale (1 sommet), le Petit Sommet (qui demande 1 sommet) se débloque automatiquement.

### Mode test rapide (`FAST_TEST_MODE`)

Pour tester sans faire 28 sessions, on peut activer le mode test en décommentant une ligne dans `progress.h` :

```cpp
// Décommenter cette ligne pour tester rapidement (3 sessions par montagne au lieu de 7-28)
// #define FAST_TEST_MODE
```

Quand `FAST_TEST_MODE` est actif, chaque montagne ne demande que 3 sessions au lieu du nombre normal.

---

## 5. L'App Web

### Description

L'app web est une **page web** qui se connecte à l'Arduino via **Web Bluetooth** directement depuis le navigateur du téléphone. Ce n'est pas une app à installer depuis un store — il suffit d'ouvrir la page dans Chrome ou Edge.

### Limitation iOS

Apple bloque l'API Web Bluetooth sur Safari et tous les navigateurs iOS. L'app ne fonctionne donc que sur **Android** (Chrome) ou **PC** (Chrome/Edge). C'est une limitation d'Apple, pas de notre projet.

### Les 5 écrans

1. **Connexion** — Écran d'accueil avec le logo Peak Progress, bouton « Scan & Connect » pour chercher l'Arduino en Bluetooth, et bouton « Try Demo Mode » pour tester sans Arduino

2. **Dashboard** — Vue principale : montagne actuelle, barre de progression, nombre de sessions, streak. Se met à jour en temps réel quand on enregistre une activité

3. **Montagnes** — Liste des 9 montagnes avec leur statut (verrouillée, débloquée, en cours, complétée). Affiche le tier et le nombre de sessions requises

4. **Historique** — Résumé de l'activité : total de sessions, sommets atteints, meilleur streak, progression globale

5. **Paramètres** — Options : réinitialiser la progression, déconnecter le Bluetooth, informations sur le projet

### Mode démo

Le mode démo simule un Arduino virtuel pour tester l'interface sans matériel. Les données sont fictives mais le comportement est identique : on peut enregistrer des activités, voir la progression, et atteindre des sommets.

---

## 6. Glossaire & Schéma Récapitulatif

### Schéma du flux de données

```
┌──────────────────────────────────┐
│          WEB APP                 │
│  (téléphone ou PC)               │
│                                  │
│  Bouton "Log Activity"           │
│         │                        │
│         ▼                        │
│  Envoie CMD_LOG_ACTIVITY (0x01)  │
│  via BLE characteristic          │
└──────────┬───────────────────────┘
           │ BLE (Bluetooth)
           ▼
┌──────────────────────────────────┐
│          ARDUINO                 │
│       Uno WiFi Rev.2             │
│                                  │
│  1. Reçoit la commande BLE      │
│  2. logActivity()                │
│  3. Incrémente sessions + streak │
│  4. Calcule le nouvel angle      │
│  5. Bouge le servo (grimpeur)    │
│  6. Sauvegarde en EEPROM         │
│  7. Vérifie le sommet            │
│  8. Met à jour les données BLE   │
│         │                        │
└─────────┼────────────────────────┘
          │ BLE notify
          ▼
┌──────────────────────────────────┐
│          WEB APP                 │
│                                  │
│  Reçoit la notification BLE     │
│  Met à jour le dashboard         │
│  (sessions, streak, montagne)    │
└──────────────────────────────────┘
```

### Glossaire

| Terme | Définition |
|-------|------------|
| `Arduino` | Micro-contrôleur, le cerveau du système. Gère la logique, la mémoire et le Bluetooth |
| `BLE` | Bluetooth Low Energy — communication sans fil économe en énergie entre l'Arduino et le téléphone |
| `Bitfield` | Un nombre où chaque bit (0 ou 1) représente un état oui/non. Ici, chaque bit = une montagne débloquée ou non |
| `Characteristic` | En BLE, une « boîte à lettres » numérique dans laquelle on publie ou lit des données |
| `constrain()` | Fonction Arduino qui empêche une valeur de sortir d'un intervalle [min, max] |
| `EEPROM` | Mémoire permanente de l'Arduino (1 Ko). Les données survivent aux redémarrages |
| `Header (`.h`)` | Fichier de code réutilisable qu'on inclut avec `#include` |
| `loop()` | Fonction Arduino qui s'exécute en boucle infinie après `setup()` |
| `Notify` | En BLE, mécanisme qui prévient le téléphone automatiquement quand une donnée change |
| `Servo` | Moteur qui tourne à un angle précis (0°-180°). Fait monter/descendre le grimpeur |
| `setup()` | Fonction Arduino qui s'exécute une seule fois au démarrage |
| `Streak` | Série de jours consécutifs où l'utilisateur a été actif |
| `struct` | Regroupement de variables dans une seule structure (comme un formulaire) |
| `Tier` | Niveau de difficulté d'une montagne (1 à 4). Plus le tier est élevé, plus il faut de sessions |
| `UUID` | Identifiant unique universel. En BLE, chaque service et chaque `characteristic` a le sien |
| `Web Bluetooth` | API du navigateur qui permet à une page web de se connecter en Bluetooth sans installer d'app |
