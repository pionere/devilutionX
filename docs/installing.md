# Installing

In order to run the game you need access to these MPQ files:
- The `DIABDAT.MPQ` on your CD or in the [GoG](https://www.gog.com/game/diablo) installation (you might [extract it from the GoG installer](https://github.com/diasurgical/devilutionX/wiki/Extracting-MPQs-from-the-GoG-installer)).
- _Diablo: Hellfire_ expansion also needs `hellfire.mpq`, `hfmonk.mpq`, `hfmusic.mpq`, `hfvoice.mpq`.
- [`devilx.mpq`](https://github.com/pionere/devilutionX/raw/master/Packaging/resources/devilx.mpq) from [GitHub](https://github.com/pionere/devilutionX/tree/master/Packaging/resources).
  1. Most of the cases this is attached to the released package.
  2. In case your gaming platform does not support the patcher, you need to create the patched mpq on a separate platform.

Download the latest [DevilutionX release](https://github.com/pionere/devilutionX/releases) for your system (if available) and extract the contents to a location of your choosing, or [build from source](building.md). Then follow the system-specific instructions below.

<details><summary>Windows</summary>

- Unzip the downloaded file to the desired folder
- Copy the MPQ files to the folder containing the DevilutionX exe (`devilx.mpq` should be already there)
- Run `devil_patcher.exe` to patch (and optionally merge) the assets in `.mpq` files. (`devilx.mpq` should be updated)
- Run `devilutionx.exe`

</details>

<details><summary>Linux</summary>

- Copy the MPQ files to the folder containing the DevilutionX executable (or to the data folder, which may differ depending on distro, version, and security settings, but will normally be `~/.local/share/diasurgical/devilx/`)
- Install [SDL2](https://www.libsdl.org/download-2.0.php):
  - Ubuntu/Debian/Rasbian `sudo apt install libsdl2-2.0-0`
  - Fedora `sudo dnf install SDL2`
- Run `./devil_patcher` to patch (and optionally merge) the assets in `.mpq` files. (`devilx.mpq` should be updated)
- Run `./devilutionx`

</details>

<details><summary>MacOS X</summary>

- Copy the MPQ files to the folder containing the DevilutionX application (or to the data folder, which is normally `~/Library/Application Support/diasurgical/devilx`
- Double-click `devil_patcher` to patch (and optionally merge) the assets in `.mpq` files. (`devilx.mpq` should be updated)
- Double-click `devilutionx`

</details>

<details><summary>FreeBSD</summary>

- Copy the MPQ files to the folder containing the DevilutionX application, or to the data folder `~/.local/share/diasurgical/devilution/`
- To install the port: `cd /usr/ports/games/devilutionX/ && make install clean`
- To add the package, run one of these commands:
  `pkg install games/devilutionX` || `pkg install devilutionX`
- Run `devil_patcher` to patch (and optionally merge) the assets in `.mpq` files. (`devilx.mpq` should be updated)
- Run `devilutionx`

</details>

<details><summary>iOS & iPadOS</summary>

Certain sideloading applications exist which can let you install IPA packages to your device such as AltStore (https://altstore.io/) and Sideloadly (https://sideloadly.io/). Using such a sideloading application, install the .ipa file to your iDevice.

Once the App is installed, launch it once. It will say that it cannot find the data file (.MPQ). This is ok. Close the game. Do not skip this step as it will create your Documents folder that you will need to access in later steps.

There are 3 way to get install the MPQs. Make sure to have installed the app and have launched it once before attempting to copy over the MPQ files.

~~Method 1~~ (***May no longer work***): ~~Using the files.app~~

~~- Using the files.app, navigate to "On My iPhone / On My iPad"~~

![ios_mpq_files01](https://user-images.githubusercontent.com/1339414/145088910-5664aed6-3a53-43f4-8088-834796792f68.png)

~~- Copy the MPQs to the devilutionx directory. Your directory should look like on the picture below~~

![ios_mpq_files02](https://user-images.githubusercontent.com/1339414/145089068-2c87887b-58c3-4759-9270-9d8934fe90bb.png)

~~- Go back to your home screen and launch the game~~

Method 2: Using Finder (MacOS)

On more recent versions of MacOS, iTunes is no more. Instead you can use Finder to directly copy data to and from your iDevices

- Connect your iDevice to your computer and click on the "Files" tab.

  ![ios_mpq_finder01](https://user-images.githubusercontent.com/1339414/145089218-f5424196-f345-45da-aca6-9c2b2e06cdf0.png)

- Drag and drop the MPQ files on the devilutionx directory

Method 3: Using iTunes (Windows and older MacOS)

- Connect you iDevice to your computer and launch iTunes

- Click on your device and in the files tab drag and drop the MPQ files to the devilutionx directory

</details>

<details><summary>Nintendo Switch</summary>

- Download and unzip [devilutionx-switch.zip](https://github.com/diasurgical/devilutionX/releases/latest/download/devilutionx-switch.zip).
- Copy `devilutionx.nro` in into `/switch/devilutionx`
- Copy the MPQ files to `/switch/devilutionx`.
- Launch `devilutionx.nro` by holding R the installed game. Do not use the album to launch, if you use the album, the homebrew will only have a small amount of memory available, and the touch keyboard won't work. This is true for all homebrew, not just DevilutionX.

[Nintendo Switch manual](/docs/manual/platforms/switch.md)

</details>

<details><summary>Android</summary>

- Copy the downloaded apk file to device and tap on it on device's file explorer or install via `adb install` (if USB debugging is enabled on device).
- Place the MPQ-files in `/sdcard/Android/data/org.diasurgical.devilx/files` folder
- Run `devilx patcher` to patch (and optionally merge) the assets in `.mpq` files. (`devilx.mpq` should be updated)
- Run `devilx`

</details>

<details><summary>Nintendo 3DS</summary>

Download [devilutionx.cia](https://github.com/diasurgical/devilutionX/releases/latest/download/devilutionx-3ds.cia) and place it on your SD card.
This file will be used later to install the game on the 3DS console.

Copy the MPQ files to the `devilutionx` subfolder under the `3ds` folder on your SD card (`/3ds/devilutionx`). The game will read these files from this location.
Note, all file and folder names should be lowercase.

Put the SD card back into the 3DS console and then power it on.
Use a title manager such as [FBI](https://github.com/Steveice10/FBI) to install `devilutionx.cia`.
You will no longer need `devilutionx.cia` after this step, and so it can be removed after the installation.

[Nintendo 3DS manual](/docs/manual/platforms/3ds.md)

</details>

<details><summary>Xbox One/Series</summary>

- Go to https://gamr13.github.io/ and follow the instructions in the Discord server
- Install DevilutionX and FTP-server from the given URLs
- Open DevilutionX
- Open and start the FTP-server
- Press `View` on DevilutionX and select `Manage game and add-ons`
- Go to `File info` and note `FullName`
- Copy the MPQ files to `/LOCALFOLDER/*FullName*/LocalState/diasurgical/devilution` using a FTP-client on your PC
  
![image](https://user-images.githubusercontent.com/204594/187104388-fc5648da-5629-4335-ae8b-403600721e2a.png)

</details>

<details><summary>Playstation 4</summary>

**Requires Playstation 4 capable of running homebrew**
- Install the downloaded pkg file
- Copy the MPQ files (e.g., using ftp) to `/user/data/diasurgical/devilution/`

[PS4 manual](/docs/manual/platforms/ps4.md)

</details>

<details><summary>Playstation 5</summary>

**Requires Playstation 5 capable of running the [ps5-payload-websrv][websrv] homebrew**
- Copy the game assets (e.g., via ftp) to /data/homebrew/devilutionX
- Launch the [ps5-payload-websrv][websrv] homebrew
- Launch the patcher and the game from your browser at http://PS5-IP:8080, or using the
  [companion launcher][launcher] from the PS5 menu system
  1. Run the patcher to patch the assets in devilx.mpq
  2. Run the game

[websrv]: https://github.com/ps5-payload-dev/websrv
[launcher]: https://github.com/ps5-payload-dev/websrv/blob/master/homebrew/IV9999-FAKE00000_00-HOMEBREWLOADER01.pkg?raw=true

[PS5 manual](/docs/manual/platforms/ps5.md)

</details>

<details><summary>Playstation Vita</summary>

- Install [devilutionx.vpk](https://github.com/diasurgical/devilutionX/releases/latest/download/devilutionx-vita.vpk)
- Copy the MPQ files to `ux0:/data/diasurgical/devilution/`.

[Playstation Vita manual](/docs/manual/platforms/vita.md)

</details>

<details><summary>ClockworkPi GameShell</summary>

- Copy the `__init__.py` to a newly created folder under /home/cpi/apps/Menu and run it from the menu. The folder then symbolizes the devilutionX icon.
- From this menu, you can press 'X' to clone the git repository for devilutionX and compile the code. Dependencies are installed automatically (cmake and SDL development packages).
- Once installed, 'X' pulls the updated code and does the compiling. Note that any changes made locally to the source are reverted before pulling.
- When the compile is finished, Copy the MPQ files to `/home/cpi/.local/share/diasurgical/devilution/`
- You can now play the game from the same icon.

</details>

<details><summary>GKD350h</summary>

- Copy [devilutionx-gkd350h.opk](https://github.com/diasurgical/devilutionX/releases/download/1.0.1/devilutionx-gkd350h.opk) to `/media/data/apps` or `/media/sdcard/apps/`.
- Copy the MPQ files to `/usr/local/home/.local/share/diasurgical/devilution/`

</details>

<details><summary>RetroFW</summary>

**Requires RetroFW 2.0+.**

- Copy the downloaded opk file to the apps directory
- Copy the MPQ files to `~/.local/share/diasurgical/devilution`

`~` is your home directory, `/home/retrofw` by default.

[RetroFW manual](/docs/manual/platforms/retrofw.md)

</details>

<details><summary>RG350</summary>

**Requires firmware v1.5+**

- Copy the downloaded opk file to `/media/sdcard/APPS/`
- Copy the MPQ files to `/media/home/.local/share/diasurgical/devilution/`

 **NOTE:** You can copy the MPQ files to sdcard instead and create a symlink at the expected location. To do this, SSH into your RG350 and run:

 ~~~bash
 ln -sf /media/sdcard/<path_to_MPQ> /media/home/.local/share/diasurgical/devilution/<MPQ>
 ~~~

[RG350 manual](/docs/manual/platforms/rg350.md)

</details>

<details><summary>Miyoo Mini</summary>

**Requires OnionOS to be installed**

- Activate the ports collection by using the onion installer on the device
- Copy the contents of the released .zip-file onto the root of your SD card
- Copy the MPQ files to `/Emu/PORTS/Binaries/Diablo.port/FILES_HERE/`
</details>
