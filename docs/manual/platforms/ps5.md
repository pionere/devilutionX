# devilutionX PS5 port

## Prerequisites
- A Playstation 5 capable of running the [ps5-payload-websrv][websrv] homebrew.
- Game assets from the Diablo game (diabdat.mpq).
 + To run the Diablo: Hellfire expansion you will need hellfire.mpq, hfmonk.mpq, hfmusic.mpq, hfvoice.mpq.

## Installation
- Copy the game assets (e.g., via ftp) to /data/homebrew/devilutionX
- Launch the [ps5-payload-websrv][websrv] homebrew
- Launch the patcher and the game from your browser at http://PS5-IP:8080, or using the
  [companion launcher][launcher] from the PS5 menu system
  1. Run the patcher to patch the assets in devilx.mpq
  2. Run the game

## Controls
- D-pad: move hero
- ○: attack nearby enemies, talk to townspeople and merchants, pickup/place
     items in the inventory, OK while in main menu
- ×: select spell, back while in menus
- △: pickup items, open nearby chests and doors, use item in the inventory
- □: cast spell, delete character while in main menu
- L1: use health item from belt
- R1: use mana potion from belt
- L2: toggle character sheet
- R2: toggle inventory
- Left stick: move hero
- Right stick: move cursor
- L3: toggle auto map
- R3: click with cursor

## Building from Source
Install the [PacBrew PS5 SDK][pacbrew], then run the following
 bash script:
```console
devilutionX$ ./Packaging/ps5/build.sh
```

[pacbrew]: https://github.com/ps5-payload-dev/pacbrew-repo
[websrv]: https://github.com/ps5-payload-dev/websrv
[launcher]: https://github.com/ps5-payload-dev/websrv/blob/master/homebrew/IV9999-FAKE00000_00-HOMEBREWLOADER01.pkg?raw=true
