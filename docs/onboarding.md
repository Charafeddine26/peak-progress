# Peak Progress — Guide d'Onboarding

Bienvenue dans l'équipe ! Ce document t'explique comment fonctionne **Peak Progress** de A à Z. Tu n'as pas besoin de coder — juste de comprendre et pouvoir expliquer le projet.

---

## 1. Vue d'ensemble

### Le concept

Peak Progress est un **objet tangible** qui transforme une habitude quotidienne (méditation, sport, lecture…) en escalade de montagne. Chaque fois que tu touches l'objet, un petit grimpeur monte physiquement le long d'une montagne imprimée en 3D. Quand tu atteins le sommet, une nouvelle montagne plus difficile se débloque.

### L'objet physique

L'objet se compose de trois éléments :

1. **La montagne 3D** — une forme imprimée en 3D avec un rail le long duquel un petit personnage (le grimpeur) se déplace grâce à un fil et une poulie
2. **Le servo-moteur** — un petit moteur qui tire le fil pour faire monter ou descendre le grimpeur
3. **Le socle** — contient l'électronique : l'Arduino, le Circuit Playground, les câbles

### L'expérience utilisateur

```
Toucher le pad → Le grimpeur monte un peu → Les LEDs s'allument → Atteindre le sommet → Nouvelle montagne !
```

### Les 3 composants du système

| Composant | Rôle | Carte |
|-----------|------|-------|
| **Circuit Playground** | Capteur tactile + LEDs + buzzer | Adafruit Circuit Playground |
| **Arduino** | Cerveau : logique, mémoire, servo, Bluetooth | Arduino Uno WiFi Rev.2 |
| **Web App** | Dashboard sur téléphone via Bluetooth | Page web (HTML/JS) |

### Schéma des connexions

```
┌─────────────────────┐          câble série          ┌──────────────────────┐
│  Circuit Playground │ ◄──────────────────────────► │      Arduino         │
│                     │   (TX/RX, 9600 baud)          │   Uno WiFi Rev.2     │
│  • Pad capacitif    │                               │                      │
│  • 10 LEDs NeoPixel │                               │  • Servo-moteur      │
│  • Buzzer           │                               │  • EEPROM (mémoire)  │
└─────────────────────┘                               │  • BLE (Bluetooth)   │
                                                      └──────────┬───────────┘
                                                                 │ Bluetooth
                                                                 ▼
                                                      ┌──────────────────────┐
                                                      │     Web App          │
                                                      │  (téléphone/PC)      │
                                                      │  • Dashboard         │
                                                      │  • Historique        │
                                                      └──────────────────────┘
```

---

## 2. Le Parcours d'une Touche

C'est le fil rouge de ce document. On va suivre ce qui se passe quand l'utilisateur **touche le pad**, étape par étape, fichier par fichier.

---

### Étape 1 — Le toucher (détection capacitive)

**Fichier : `circuit_playground.ino`**

Quand tu poses ton doigt sur le pad n°3 du Circuit Playground, le capteur mesure la capacitance (une propriété électrique qui change au contact de la peau). Si la valeur dépasse un seuil, c'est un toucher valide.

```cpp
// ── Configuration ──
#define TOUCH_PAD        3       // Numéro du pad capacitif utilisé
#define TOUCH_THRESHOLD  500     // Seuil : au-dessus = toucher détecté
#define DEBOUNCE_MS      1500    // Anti-rebond : ignore les touches trop rapides (1.5s)

// ── Fonction principale au démarrage ──
void setup() {
  CircuitPlayground.begin();         // Initialise le Circuit Playground
  Serial.begin(9600);               // Ouvre la communication série
  CircuitPlayground.setBrightness(30); // Luminosité des LEDs
}

// ── Boucle infinie (tourne en permanence) ──
void loop() {
  handleTouch();               // Vérifie si on touche le pad
  processSerialCommands();     // Écoute les ordres de l'Arduino
}

// ── Détection du toucher ──
void handleTouch() {
  uint16_t capValue = CircuitPlayground.readCap(TOUCH_PAD);  // Lit la capacitance

  if (capValue > TOUCH_THRESHOLD) {                          // Seuil dépassé ?
    unsigned long now = millis();                             // Temps actuel en ms
    if (now - lastTouchTime >= DEBOUNCE_MS) {                // Assez de temps écoulé ?
      lastTouchTime = now;
      Serial.println('T');     // Envoie "T" à l'Arduino = "Toucher détecté !"
    }
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
> - `#define TOUCH_THRESHOLD 500` crée une constante nommée `TOUCH_THRESHOLD` qui vaut 500
> - `uint16_t capValue` crée une boîte nommée `capValue` qui stocke un nombre entier positif (de 0 à 65 535)
>
> Les types courants dans notre code :
> - `int` — nombre entier classique
> - `uint8_t` — nombre positif de 0 à 255 (1 octet)
> - `uint16_t` — nombre positif de 0 à 65 535 (2 octets)
> - `bool` — vrai (`true`) ou faux (`false`)
> - `unsigned long` — très grand nombre positif (pour le temps en millisecondes)

---

### Étape 2 — La communication série

**Fichier : `serial_protocol.h`**

Le Circuit Playground envoie la lettre `T` suivie d'un retour à la ligne via un câble physique (TX/RX). L'Arduino écoute et réagit.

```cpp
#ifndef SERIAL_PROTOCOL_H    // Protection : n'inclure ce fichier qu'une seule fois
#define SERIAL_PROTOCOL_H

#include <Arduino.h>          // Inclut les fonctions de base Arduino
#include "mountains.h"        // Inclut notre fichier de définition des montagnes
#include "progress.h"         // Inclut notre fichier de progression

// ── L'Arduino vérifie si le CP a envoyé un "T" ──
inline bool serialTouchDetected() {
  if (Serial.available()) {          // Y a-t-il des données à lire ?
    char c = Serial.read();          // Lire un caractère
    if (c == 'T') {                  // C'est un toucher ?
      while (Serial.available()) Serial.read();  // Vider le reste
      return true;                   // Oui, toucher détecté !
    }
  }
  return false;                      // Non, rien reçu
}

// ── L'Arduino envoie des ordres au CP ──
inline void sendMelodyCommand(MelodyType melody) {
  Serial.print('M');                 // Lettre de commande
  Serial.println((int)melody);      // Numéro de mélodie (0 à 4)
}

inline void sendSummitAnimation(uint8_t tier) {
  Serial.print('S');                 // "S" pour summit
  Serial.println(tier);             // Tier de la montagne (1 à 4)
}
```

> **C++ : c'est quoi `#include` et les fichiers `.h` ?**
>
> Imagine un livre de recettes divisé en chapitres. `#include "progress.h"` revient à dire « ouvre le chapitre *progress* et lis-le ici ». Les fichiers `.h` (header) contiennent du code réutilisable. Ça évite de tout mettre dans un seul fichier énorme.
>
> `#include <Arduino.h>` avec des `< >` inclut un fichier de la bibliothèque Arduino (fourni par le système).
> `#include "progress.h"` avec des `" "` inclut un fichier de notre projet.

> **C++ : c'est quoi `Serial` ?**
>
> `Serial` est le moyen de communication entre deux cartes via un câble. C'est comme un talkie-walkie textuel :
> - `Serial.print('M')` — envoie la lettre M
> - `Serial.println(42)` — envoie le nombre 42 suivi d'un retour à la ligne
> - `Serial.read()` — lit un caractère reçu
> - `Serial.available()` — vérifie s'il y a quelque chose à lire

---

### Étape 3 — La logique de progression

**Fichier : `progress.h` + `arduino_main.ino`**

Quand l'Arduino reçoit le `T`, il appelle la fonction `logActivity()`. C'est là que la magie opère : on incrémente les compteurs, on bouge le grimpeur, et on vérifie si le sommet est atteint.

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
// ── dans arduino_main.ino — la fonction appelée à chaque toucher ──
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
> Une fonction, c'est une **recette réutilisable**. `logActivity()` est une recette qui dit « voici les 10 étapes à faire quand quelqu'un touche le pad ». On peut l'appeler depuis n'importe où avec juste son nom : `logActivity();`
>
> Une fonction peut aussi prendre des **paramètres** (ingrédients) :
> ```cpp
> int calculateServoAngle(uint8_t sessions, uint8_t totalSessions)
> //                      ^^^^^^^^^^^^^^   ^^^^^^^^^^^^^^^^^^^^^
> //                      ingrédient 1     ingrédient 2
> ```

---

### Étape 4 — La sauvegarde EEPROM

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
> Sur Arduino, la mémoire est très limitée. Utiliser le type le plus petit possible permet d'économiser de la place. `totalSessionsAllTime` utilise `uint16_t` parce qu'on pourrait dépasser 255 sessions au total, mais `currentMountainIndex` utilise `uint8_t` car il n'y a que 9 montagnes.

---

### Étape 5 — Le mouvement du servo

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

> **C++ : fonctions avec paramètres et `map()`**
>
> `calculateServoAngle(sessions, totalSessions)` est une fonction qui prend deux ingrédients et renvoie un résultat (l'angle). C'est comme une formule mathématique :
>
> ```
> angle = base + (plage × sessions) / sessionsTotal
> ```
>
> La fonction `constrain(valeur, min, max)` empêche une valeur de sortir d'un intervalle. Si `targetAngle` vaut 200, `constrain` le ramène à 170 (le maximum).

---

### Étape 6 — Le retour visuel (LEDs et mélodies)

**Fichiers : `serial_protocol.h` + `circuit_playground.ino`**

Après chaque toucher, l'Arduino envoie des commandes au Circuit Playground pour allumer les LEDs et jouer des sons. Le nombre de LEDs allumées reflète la progression sur la montagne actuelle.

**Côté Arduino — envoi des commandes :**

```cpp
// ── Allumer les LEDs selon la progression ──
inline void sendProgressLEDs(const UserProgress &prog, const Mountain &mtn) {
  uint8_t totalSess = SESSIONS_FOR_MOUNTAIN(mtn);

  // Calculer combien de LEDs allumer (sur 10)
  int ledsToLight = ((int)prog.sessionsOnCurrentMountain * NUM_LEDS) / totalSess;
  ledsToLight = constrain(ledsToLight, 0, NUM_LEDS);

  sendClearLEDs();   // D'abord tout éteindre

  for (int i = 0; i < ledsToLight; i++) {
    uint32_t color = getMountainColor(mtn, i);  // Couleur de la palette
    sendLEDCommand(i, color);                    // Allumer la LED i
  }
}
```

**Côté Circuit Playground — réception et exécution :**

```cpp
// ── Écouter les commandes série ──
void processSerialCommands() {
  if (!Serial.available()) return;

  char cmd = Serial.read();         // Lire la première lettre

  switch (cmd) {
    case 'L': parseLEDCommand(); break;    // "L" = allumer une LED
    case 'C': clearAllLEDs(); break;       // "C" = tout éteindre
    case 'M': parseMelodyCommand(); break; // "M" = jouer une mélodie
    case 'W': weeklyAnimation(); break;    // "W" = animation hebdomadaire
    case 'S': parseSummitCommand(); break; // "S" = célébration sommet
  }
}
```

> **C++ : protocole texte et `parsing`**
>
> Les deux cartes communiquent avec un **protocole** simple : des lettres suivies de données. Par exemple :
> - `L3:00FF99` = « LED numéro 3, couleur vert clair »
> - `M2` = « joue la mélodie numéro 2 (triumphante) »
> - `S4` = « animation de sommet, tier 4 »
>
> Le `parsing`, c'est l'action de **décoder** ces messages. Le `switch` est comme un aiguillage de train : selon la première lettre reçue, le programme prend un chemin différent.

---

### Étape 7 — La notification BLE (Bluetooth)

**Fichier : `ble_service.h`**

L'Arduino envoie la progression au téléphone via `BLE` (Bluetooth Low Energy). L'app web peut ainsi afficher le dashboard en temps réel sans fil.

```cpp
#define SERVICE_UUID       "19B10000-E8F2-537E-4F6C-D104768A1214"  // Identifiant du service
#define CHAR_PROGRESS_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"  // Identifiant des données
#define CHAR_COMMAND_UUID  "19B10003-E8F2-537E-4F6C-D104768A1214"  // Identifiant des commandes

// ── Objets BLE ──
BLEService peakService(SERVICE_UUID);                                    // Le service
BLECharacteristic progressChar(CHAR_PROGRESS_UUID, BLERead | BLENotify, 8); // Données (lecture + notification)
BLEByteCharacteristic commandChar(CHAR_COMMAND_UUID, BLEWrite);          // Commandes (écriture)

// ── Initialisation BLE ──
inline bool initBLE() {
  if (!BLE.begin()) return false;       // Démarrer le Bluetooth

  BLE.setLocalName("PeakProgress");     // Nom visible par le téléphone
  BLE.setAdvertisedService(peakService);

  peakService.addCharacteristic(progressChar);  // Ajouter les caractéristiques
  peakService.addCharacteristic(commandChar);

  BLE.addService(peakService);
  BLE.advertise();   // Commencer à se rendre visible
  return true;
}

// ── Envoyer la progression au téléphone ──
inline void updateBLEProgress(const UserProgress &prog) {
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

> **C++ : c'est quoi le `BLE` ?**
>
> `BLE` = Bluetooth Low Energy. C'est une version économe en énergie du Bluetooth, idéale pour les objets connectés. Le fonctionnement :
>
> 1. L'Arduino **annonce** sa présence (comme un phare)
> 2. Le téléphone **se connecte** au service `PeakProgress`
> 3. L'Arduino **publie** ses données dans des `characteristic` (des boîtes à lettres numériques)
> 4. Le téléphone **lit** ces boîtes ou reçoit des **notifications** automatiques quand elles changent
>
> Les `UUID` sont des identifiants uniques (comme des numéros de téléphone) qui permettent au téléphone de trouver le bon service et les bonnes données.

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

### Étape 8 — L'affichage web

**Web App (`mobile-app/`)**

L'app web est une **page web** (pas une app native) qui se connecte à l'Arduino via Web Bluetooth directement depuis le navigateur. Elle affiche en temps réel la progression du grimpeur.

L'app web :
- Se connecte en Bluetooth à « PeakProgress »
- Lit les 8 octets de la `characteristic` de progression
- Met à jour le dashboard : montagne actuelle, sessions, streak, sommets
- Permet d'envoyer des commandes (log activité, reset) via la `characteristic` de commande
- Fonctionne aussi en **mode démo** sans Arduino pour tester l'interface

L'app est purement descriptive ici — pas besoin de comprendre le code JavaScript.

---

## 4. Le Système de Montagnes

### Tableau des 9 montagnes (4 tiers)

| # | Nom | Tier | Sessions | Semaines | Débloquée après | Palette LED | Mélodie |
|---|-----|------|----------|----------|-----------------|-------------|---------|
| 0 | Colline Locale | 1 | 7 | 1 | — (départ) | Bleu → Vert | Simple |
| 1 | Petit Sommet | 1 | 7 | 1 | 1 sommet | Bleu → Cyan → Vert | Simple |
| 2 | Mont d'Entraînement | 1 | 7 | 1 | 2 sommets | Violet → Or | Modérée |
| 3 | Mont Blanc | 2 | 14 | 2 | 3 sommets | Glace → Blanc → Or | Triomphante |
| 4 | Matterhorn | 2 | 14 | 2 | 4 sommets | Violet foncé → Rouge | Triomphante |
| 5 | Kilimanjaro | 3 | 21 | 3 | 5 sommets | Dégradé chaud | Épique |
| 6 | Denali | 3 | 21 | 3 | 6 sommets | Arctique scintillant | Épique |
| 7 | Everest | 4 | 28 | 4 | 7 sommets | Arc-en-ciel | Légendaire |
| 8 | K2 | 4 | 28 | 4 | 8 sommets | Feu pulsé | Légendaire |

### Sessions par tier

- **Tier 1** — 7 sessions (1 semaine) : Colline, Petit Sommet, Mont d'Entraînement
- **Tier 2** — 14 sessions (2 semaines) : Mont Blanc, Matterhorn
- **Tier 3** — 21 sessions (3 semaines) : Kilimanjaro, Denali
- **Tier 4** — 28 sessions (4 semaines) : Everest, K2

### Logique de déverrouillage

Le champ `unlockedBitfield` est un nombre de 16 bits. Chaque bit correspond à une montagne. Quand l'utilisateur atteint un sommet, la fonction `checkAndUnlockMountains()` vérifie pour chaque montagne si le nombre de sommets atteints suffit pour la débloquer :

```cpp
inline void checkAndUnlockMountains(UserProgress &prog) {
  for (uint8_t i = 0; i < NUM_MOUNTAINS; i++) {
    if (prog.summitsReached >= MOUNTAIN_LIBRARY[i].unlockAfterSummits) {
      unlockMountain(prog, i);   // Activer le bit correspondant
    }
  }
}
```

Exemple : après avoir gravi la Colline Locale (1 sommet), le Petit Sommet (qui demande 1 sommet) se débloque automatiquement.

### Palettes LED et mélodies par tier

Chaque montagne a sa propre palette de 10 couleurs pour les LEDs NeoPixel. Les couleurs sont de plus en plus spectaculaires :
- **Tier 1** : dégradés simples (bleu-vert, violet-or)
- **Tier 2** : couleurs froides et élégantes (glace, violet foncé)
- **Tier 3** : couleurs chaudes et intenses (orange, arctique)
- **Tier 4** : effets spéciaux (arc-en-ciel, feu)

Les mélodies suivent la même progression : de 3 notes simples (tier 1) à une fanfare de 8 notes (tier 4).

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

2. **Dashboard** — Vue principale : montagne actuelle, barre de progression, nombre de sessions, streak, servo position. Se met à jour en temps réel quand on touche le pad

3. **Montagnes** — Liste des 9 montagnes avec leur statut (verrouillée, débloquée, en cours, complétée). Affiche le tier, le nombre de sessions requises et la palette de couleurs

4. **Historique** — Résumé de l'activité : total de sessions, sommets atteints, meilleur streak, progression globale

5. **Paramètres** — Options : réinitialiser la progression, déconnecter le Bluetooth, informations sur le projet

### Mode démo

Le mode démo simule un Arduino virtuel pour tester l'interface sans matériel. Les données sont fictives mais le comportement est identique : on peut « toucher » le pad virtuel, voir le grimpeur monter, et atteindre des sommets.

---

## 6. Glossaire & Schéma Récapitulatif

### Schéma du flux de données

```
┌──────────────────┐       "T\n"        ┌──────────────────────────────────┐
│                  │ ──────────────────► │                                  │
│  CIRCUIT         │                     │          ARDUINO                 │
│  PLAYGROUND      │ ◄────────────────── │       Uno WiFi Rev.2             │
│                  │  "L3:00FF99\n"      │                                  │
│  Capteur tactile │  "M2\n"            │  1. Reçoit le "T"                │
│  10 LEDs         │  "S4\n"            │  2. logActivity()                │
│  Buzzer          │                     │  3. Incrémente sessions          │
│                  │                     │  4. Bouge le servo               │
└──────────────────┘                     │  5. Sauvegarde EEPROM            │
                                         │  6. Envoie LEDs + mélodie au CP  │
                                         │  7. Met à jour BLE               │
                                         │                                  │
                                         └───────────┬──────────────────────┘
                                                     │ BLE (Bluetooth)
                                                     │ 8 octets de données
                                                     ▼
                                         ┌──────────────────────────────────┐
                                         │          WEB APP                 │
                                         │                                  │
                                         │  Lit les données BLE             │
                                         │  Affiche le dashboard            │
                                         │  Peut envoyer des commandes      │
                                         │  (log activité, reset)           │
                                         └──────────────────────────────────┘
```

### Glossaire

| Terme | Définition |
|-------|------------|
| `Arduino` | Micro-contrôleur, le cerveau du système. Gère la logique, la mémoire et le Bluetooth |
| `BLE` | Bluetooth Low Energy — communication sans fil économe en énergie entre l'Arduino et le téléphone |
| `Bitfield` | Un nombre où chaque bit (0 ou 1) représente un état oui/non. Ici, chaque bit = une montagne débloquée ou non |
| `Baud rate` | Vitesse de communication série, en bits par seconde. Notre projet utilise 9600 baud |
| `Capacitif` | Type de capteur qui détecte le contact de la peau grâce aux propriétés électriques du corps humain |
| `Characteristic` | En BLE, une « boîte à lettres » numérique dans laquelle l'Arduino publie des données que le téléphone peut lire |
| `Circuit Playground` | Carte Adafruit avec capteurs, LEDs et buzzer intégrés. Sert d'interface utilisateur (entrée tactile + sortie visuelle/sonore) |
| `Debounce` | Anti-rebond : délai minimum entre deux détections pour éviter les faux positifs |
| `EEPROM` | Mémoire permanente de l'Arduino (1 Ko). Les données survivent aux redémarrages |
| `Header (`.h`)` | Fichier de code réutilisable qu'on inclut avec `#include`. Contient des définitions et des fonctions |
| `LED NeoPixel` | LED RGB programmable individuellement. Le Circuit Playground en a 10 en cercle |
| `loop()` | Fonction Arduino qui s'exécute en boucle infinie après `setup()` |
| `Parsing` | Action de décoder un message texte pour en extraire les données utiles |
| `Servo` | Moteur qui tourne à un angle précis (0°-180°). Ici, il fait monter/descendre le grimpeur |
| `Serial` | Communication filaire entre deux cartes via câble TX/RX |
| `setup()` | Fonction Arduino qui s'exécute une seule fois au démarrage |
| `Streak` | Série de jours consécutifs où l'utilisateur a été actif |
| `struct` | Regroupement de variables dans une seule structure (comme un formulaire) |
| `Tier` | Niveau de difficulté d'une montagne (1 à 4). Plus le tier est élevé, plus il faut de sessions |
| `UUID` | Identifiant unique universel. En BLE, chaque service et chaque `characteristic` a son propre UUID |
| `Web Bluetooth` | API du navigateur qui permet à une page web de se connecter en Bluetooth sans installer d'app |
