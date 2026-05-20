# PlayerTracker — Plugin AsaApi pour le tracking des connexions ARK

Plugin C++ qui hook `PostLogin` et `Logout` sur le serveur ARK, capture nom + EOSID + position (X/Y/Z) du joueur, et l'écrit dans une table MySQL partagée. Le bot Discord ChaosArk poll cette table pour pousser les events vers les salons Discord.

## Schéma MySQL

Le plugin crée automatiquement la table au premier démarrage :

```sql
CREATE TABLE IF NOT EXISTS player_events (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  event_type VARCHAR(10),     -- 'join' ou 'leave'
  server_name VARCHAR(64),    -- "Aberration", "Forglar"...
  eos_id VARCHAR(64),
  player_name VARCHAR(128),
  pos_x DOUBLE NULL,
  pos_y DOUBLE NULL,
  pos_z DOUBLE NULL,
  created_at BIGINT,
  INDEX idx_created (created_at)
);
```

## Installation

1. Compiler `PlayerTracker.vcxproj` (Release | x64) avec Visual Studio. Mêmes prérequis qu'EOSBanManager : AsaApi v1.21 cloné + libmysql.
2. Créer le dossier serveur :
   ```
   ShooterGame\Binaries\Win64\ArkApi\Plugins\PlayerTracker\
   ```
3. Copier `PlayerTracker.dll`, `PluginInfo.json`, `config.json`.
4. Éditer `config.json` :
   ```json
   {
     "MySQL": { "Host": "127.0.0.1", "Port": 3306, "User": "ark_ban", "Password": "...", "Database": "ark_bans", "Table": "player_events" },
     "ServerName": "Aberration"
   }
   ```
   ⚠️ **`ServerName` doit être différent pour chaque map du cluster** — c'est ce qui apparaîtra dans Discord. Si tu utilises le même `Plugin-PlayerTracker/` sur plusieurs serveurs, fais bien attention à modifier ce champ pour chaque.

5. Redémarre le serveur ARK. Console doit afficher :
   ```
   [PlayerTracker][INFO ] PlayerTracker chargé (server_name=Aberration)
   [PlayerTracker][INFO ] DB OK, table `player_events` prête
   ```

## Vérif rapide

- Connecte-toi : `SELECT * FROM ark_bans.player_events ORDER BY id DESC LIMIT 5;` → tu dois voir une ligne `event_type='join'` avec ta position.
- Déconnecte-toi : nouvelle ligne `event_type='leave'`.

## Permissions MySQL pour le bot

Le bot Discord (`ark_ban` user) lit cette table. S'il n'y a pas accès :
```sql
GRANT SELECT, DELETE ON ark_bans.player_events TO 'ark_ban'@'localhost';
FLUSH PRIVILEGES;
```
(Le DELETE permet au bot de nettoyer les events traités si besoin.)
