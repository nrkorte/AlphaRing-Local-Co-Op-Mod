## Alpha Ring
A Modding Tool for MCC

> ### ⬇️ Just want to play? [Download INSTALL-AlphaRing-Local-Co-Op.zip](https://github.com/nrkorte/AlphaRing-Local-Co-Op-Mod/raw/master/INSTALL-AlphaRing-Local-Co-Op.zip)
> It contains the mod (`WTSAPI32.dll`) and a step-by-step `HOW TO INSTALL.txt` that shows how to find the right game folder, install the mod and set up splitscreen.

> This is a continuation of [WinterSquire/AlphaRing](https://github.com/WinterSquire/AlphaRing), which has been archived.
> It is based on the 1.3528.0.0 release and adds:
> * Player 1 can use a controller in splitscreen (on by default), so all players can use controllers.
> * Splitscreen settings, player names, controllers and profiles can be saved and are loaded at startup.

### Showcase

| | |
|--|--|
| Camera Tool (H3) <br> ![Camera](https://github.com/WinterSquire/AlphaRing/assets/135317392/d359b2e8-5302-430f-be0d-bc065e63f546) | Object Browser (H3) <br> ![Object](https://github.com/WinterSquire/AlphaRing/assets/135317392/0bce1af7-354f-4d9d-92f7-eb2d46d8ae37) |
| 8 Players Campaign <br> ![Splitscreen 8 players](https://github.com/WinterSquire/AlphaRing/assets/135317392/7d9f4281-892a-47e2-8e0c-845a965e5d11) | Splitscreen With [Mod](https://steamcommunity.com/sharedfiles/filedetails/?id=3153235187) (By [Priception](https://steamcommunity.com/id/priception)) <br> ![H4](https://github.com/WinterSquire/AlphaRing/assets/135317392/5359868c-c5db-4300-9805-84c61b0bd8ee) |

### Features
* Splitscreen (all games)
* Camera Tool (H3)
* Object Browser (H3)

### Installation
Make sure you have the latest [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe) installed.

Download [INSTALL-AlphaRing-Local-Co-Op.zip](https://github.com/nrkorte/AlphaRing-Local-Co-Op-Mod/raw/master/INSTALL-AlphaRing-Local-Co-Op.zip) and follow `HOW TO INSTALL.txt` inside it.

In short: place `WTSAPI32.dll` into the "Halo The Master Chief Collection\mcc\binaries\win64" directory and launch the game with EAC off. Built for MCC 1.3528.0.0.

For Running on Steam Deck/Linux, add the following command in the Steam Game Launch Options:
``` 
WINEDLLOVERRIDES="WTSAPI32=n,b" %command%
```

### Switching the mod on and off
The mod only works with anti-cheat (EAC) off. To play online with EAC on, you don't have to uninstall it. `tools/ToggleAlphaRing` builds a small `Toggle-AlphaRing.exe` that switches it off and on with a double-click.

**Build it (one time):**
1. Download or clone this repository.
2. Double-click `tools/ToggleAlphaRing/build.bat`. It uses the C# compiler that ships with Windows 10/11, so nothing else needs to be installed.
3. `Toggle-AlphaRing.exe` appears in the same folder. You can move it anywhere, such as your desktop.

**Use it:**
1. Close MCC.
2. Double-click `Toggle-AlphaRing.exe` and click `Yes` on the administrator prompt.
3. A message shows the new state:
   * `DISABLED`: launch MCC normally, with anti-cheat on.
   * `ENABLED`: launch MCC with anti-cheat off.

When the mod is disabled, the exe renames `WTSAPI32.dll` to `WTSAPI32.dll.disabled`, so the game skips it. Running the exe again renames it back. The exe finds MCC automatically through your Steam libraries, including libraries on other drives. It works with Steam installs only.

If the mod isn't installed yet, put `WTSAPI32.dll` in the same folder as `Toggle-AlphaRing.exe` and run it; it copies the DLL into the game folder for you.

| File | Purpose |
|--|--|
| `ToggleAlphaRing.cs` | Source code for the toggle |
| `app.manifest` | Makes the exe ask for administrator rights, which game folders under `Program Files` need |
| `build.bat` | Compiles the two files above into `Toggle-AlphaRing.exe` |

### Usage
Toggle menu: `F4` or `Controller Back` + `Controller Start`

To navigate using Controller use the `Right Stick` to move the mouse and `RB` to click.

When the menu is open, game input is disabled.

#### Splitscreen
1. Open the menu and select `Splitscreen` in the menu bar.
2. Click `Enable` and set `Players` to the number of players.
3. In each player's tab, choose which controller that player uses under `Input`.
4. Click `Save` to keep these settings for next time.

By default every player uses a controller (Player 1 → Controller 1, Player 2 → Controller 2, ...).
To let Player 1 use keyboard and mouse instead, turn on `Options` → `Enable K/M for player1`; Players 2-4 then move to Controllers 1-3.
If two players end up swapped, change their `Input` selection, since Windows numbers controllers in the order it detects them.

To keep other players' in-game settings (sensitivity, button layout, etc.), use `Load Profile` in each player's tab while in game, adjust it under `Profile` / `Gamepad Mapping`, then click `Save`.

Settings are saved to `alpha_ring/splitscreen.json` in the MCC install folder and loaded automatically at startup. Delete this file to reset to defaults.

### Bugs Report
Submit it in the [Issues](https://github.com/WinterSquire/AlphaRing/issues) page.

### Credits
- [Assembly](https://github.com/XboxChaos/Assembly) for the tag group research.
- [Blender](https://github.com/blender/blender) for the bezier curve calculation.
- [Priception](https://github.com/Priception) for adding UI controller support and helping with the interface and crash issue.
