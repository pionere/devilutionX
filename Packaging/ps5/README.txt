# DevilutionX (Diablo 1) for PS5
DevilutionX is a source port of Diablo and Hellfire that strives to make it simple to run the game while providing engine improvements, bugfixes, and some optional quality of life features.

## Installation
**Requires Playstation 5 capable of running homebrew**
1. Extract the files in the zip.
2. Copy DIABDAT.MPQ from the CD or GOG-installation (or extract it from the GoG installer) to the DevilutionX folder (/data/homebrew/devilutionX).
 - To run the Diablo: Hellfire expansion you will need to also copy hellfire.mpq, hfmonk.mpq, hfmusic.mpq, hfvoice.mpq.

## Usage
- Launch the `ps5-payload-websrv` (https://github.com/ps5-payload-dev/websrv) homebrew
- Launch the patcher and the game from your browser at http://PS5-IP:8080, or using the
  `companion launcher` (https://github.com/ps5-payload-dev/websrv/blob/master/homebrew/IV9999-FAKE00000_00-HOMEBREWLOADER01.pkg?raw=true) from the PS5 menu system
  1. Run the patcher to patch the assets in devilx.mpq (required only once)
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
Install the `PacBrew PS5 SDK` (https://github.com/ps5-payload-dev/pacbrew-repo), then run the following
 bash script:
```console
devilutionX$ ./Packaging/ps5/build.sh
```

## Credits
 - See list of contributors https://github.com/pionere/devilutionX/graphs/contributors

## Resources

Discord: https://discord.gg/YQKCAYQ
GitHub: https://github.com/pionere/devilutionX

Check out the manual for what features are available and how best to take advantage of them: https://github.com/pionere/devilutionX/wiki
For a full list of changes see our changelog: https://github.com/pionere/devilutionX/blob/master/docs/CHANGELOG.md

## Legal
This software is being released to the Public Domain. No assets of Diablo are being provided. You must own a copy of Diablo and have access to the assets beforehand in order to use this software.

Battle.net® - Copyright © 1996 Blizzard Entertainment, Inc. All rights reserved. Battle.net and Blizzard Entertainment are trademarks or registered trademarks of Blizzard Entertainment, Inc. in the U.S. and/or other countries.

Diablo® - Copyright © 1996 Blizzard Entertainment, Inc. All rights reserved. Diablo and Blizzard Entertainment are trademarks or registered trademarks of Blizzard Entertainment, Inc. in the U.S. and/or other countries.

This software is in no way associated with or endorsed by Blizzard Entertainment®.
