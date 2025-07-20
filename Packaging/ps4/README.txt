# devilutionX PS4 port

## Prerequisites
- A Playstation 4 capable of running homebrew.
- Game assets from the Diablo game (diabdat.mpq)
 + To run the Diablo: Hellfire expansion you will need hellfire.mpq, hfmonk.mpq, hfmusic.mpq, hfvoice.mpq.

## Installation
- Install the devilutionX PS4 pkg
- Copy the game assets (e.g., via ftp) to /user/data/diasurgical/devilution/
- Run the patcher to patch the assets in devilx.mpq
- Run the game

## Known limitations
 - No networking

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
Install the [PacBrew openorbis SDK][pacbrew-openorbis], then run the following
 bash script.
```console
devilutionX$ ./Packaging/ps4/build.sh
```

[pacbrew-openorbis]: https://github.com/PacBrew/pacbrew-packages
