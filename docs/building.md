# Building from Source

<details><summary>Linux</summary>

Note that ```pkg-config``` is an optional dependency for finding libsodium,
although we have a fallback if necessary.

### Installing dependencies on Debian and Ubuntu

```
sudo apt-get install git rpm cmake g++ libsdl2-dev libsodium-dev
```

### Installing dependencies on Fedora

```
sudo dnf install cmake git glibc-devel SDL2-devel libsodium-devel libasan libubsan
```

### Installing dependencies on Alpine Linux

```
sudo apk add git cmake g++ sdl2-dev libsodium-dev
```

### Compiling

```bash
git clone https://github.com/pionere/devilutionx
cd devilutionx
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(getconf _NPROCESSORS_ONLN)
```

### Cross-compiling for arm64 (aarch64) on Debian or Ubuntu

First, set up the dependencies for cross-compilation:

```bash
Packaging/nix/debian-cross-aarch64-prep.sh
```

Then, build DevilutionX using the cross-compilation CMake toolchain file:

```bash
cmake -S. -Bbuild-aarch64-rel \
  -DCMAKE_TOOLCHAIN_FILE=../CMake/platforms/aarch64-linux-gnu.toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCPACK=ON \
  -DDEVILUTIONX_SYSTEM_LIBFMT=OFF
cmake --build build-aarch64-rel -j $(getconf _NPROCESSORS_ONLN) --target package
```

</details>

<details><summary>macOS</summary>

Make sure you have [Homebrew](https://brew.sh/) installed, then run:

```bash
brew bundle install
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(sysctl -n hw.physicalcpu)
```

</details>
<details><summary>iOS</summary>

Make sure you have [Homebrew](https://brew.sh/) installed, then run:

```bash
brew install cmake
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=../CMake/ios.toolchain.cmake  -DENABLE_BITCODE=0 -DPLATFORM=OS64
cmake --build build -j $(sysctl -n hw.physicalcpu) --config Release
cd build
rm -rf Payload devilutionx.ipa
mkdir -p Payload
mv devilutionx.app Payload
zip -r devilutionx.ipa Payload
```

For testing with the Simulator instead run the following:

```bash
cmake -S. -Bbuild -G Xcode -DCMAKE_TOOLCHAIN_FILE=../CMake/ios.toolchain.cmake -DPLATFORM=SIMULATOR64
```

Then open the generated Xcode project and run things from there.
</details>
<details><summary>FreeBSD</summary>

### Installing dependencies

```
pkg install cmake libsodium
```

### Compiling

```bash
cmake -S. -Bbuild. -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(sysctl -n hw.ncpu)
```

</details>
<details><summary>NetBSD</summary>

### Installing dependencies

```
pkgin install cmake libsodium
```

### Compiling

```bash
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(sysctl -n hw.ncpu)
```

</details>

<details><summary>OpenBSD</summary>

### Installing dependencies

```
pkg_add cmake libsodium gmake
```

### Compiling

```bash
cmake -S. -Bbuild -DCMAKE_MAKE_PROGRAM=gmake -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(sysctl -n hw.ncpuonline)
```

</details>

<details><summary>Windows via MinGW</summary>

<details><summary>Installing Windows Subsystem for Linux</summary>

Note: We currently recommend using Ubuntu 24.04 for the MinGW build. The following instructions will install the recommended version of Ubuntu on WSL.

If you are building on Windows and do not have WSL already setup this will install WSL and Ubuntu (Requires Windows 10 2004 or higher or Windows 11)

In an Administrator Command Prompt or Powershell

```wsl --install -d Ubuntu-24.04```

Reboot

Wait for Command Prompt to pop up and say installing when complete enter your new Linux password

You can launch WSL anytime by typing wsl or ubuntu in a Command Prompt or Powershell or in the Start Menu launch the Ubuntu App

### Setup git and clone DevilutionX

In a WSL terminal run these commands to get the source code for DevilutionX

```
sudo apt install git
git clone https://github.com/pionere/devilutionx
cd devilutionx
```
</details>

### Installing dependencies on WSL, Debian and Ubuntu

### 32-bit

In addition to the 32-bit MinGW build tools, the build process depends on the 32-bit MinGW Development Libraries for [SDL2](https://www.libsdl.org/download-2.0.php) and [libsodium](https://github.com/jedisct1/libsodium/releases). These dependencies will need to be placed in the appropriate subfolders under `/usr/i686-w64-mingw32`. This can be done automatically by running [`Packaging/windows/mingw-prep.sh`](/Packaging/windows/mingw-prep.sh).

```bash
# Install the 32-bit MinGW build tools
sudo apt install cmake gcc-mingw-w64-i686 g++-mingw-w64-i686 pkg-config-mingw-w64-i686 git wget

# Download the 32-bit development libraries for SDL2 and libsodium
# and place them in subfolders under /usr/i686-w64-mingw32
Packaging/windows/mingw-prep.sh
```

### 64-bit

In addition to the 64-bit MinGW build tools, the build process depends on the 64-bit MinGW Development Libraries of [SDL2](https://www.libsdl.org/download-2.0.php) and [libsodium](https://github.com/jedisct1/libsodium/releases). These dependencies will need to be placed in the appropriate subfolders under `/usr/x86_64-w64-mingw32`. This can be done automatically by running [`Packaging/windows/mingw-prep64.sh`](/Packaging/windows/mingw-prep64.sh).

```bash
# Install the 64-bit MinGW build tools
sudo apt install cmake gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64 pkg-config-mingw-w64-x86-64 git wget

# Download the 64-bit development libraries for SDL2 and libsodium
# and place them in subfolders under /usr/x86_64-w64-mingw32
Packaging/windows/mingw-prep64.sh
```

### Compiling

By compiling the `package` target, the build will produce the `devilutionx.zip` archive which should contain all the dlls necessary to run the game. If you encounter any errors suggesting a dll is missing, try extracting the dlls from the zip archive.

### 32-bit

```bash
# Configure the project to statically link sdl2 and libsodium
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=../CMake/mingwcc.toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release -DDEVILUTIONX_SYSTEM_SDL2=OFF \
    -DDEVILUTIONX_SYSTEM_LIBSODIUM=OFF

# Build the "package" target which produces devilutionx.zip
# containing all the necessary dlls to run the game
cmake --build build -j $(getconf _NPROCESSORS_ONLN) --target package
```

### 64-bit

```bash
# Configure the project to statically link sdl2 and libsodium
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=../CMake/mingwcc64.toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release -DDEVILUTIONX_SYSTEM_SDL2=OFF \
    -DDEVILUTIONX_SYSTEM_LIBSODIUM=OFF

# Build the "package" target which produces devilutionx.zip
# containing all the necessary dlls to run the game
cmake --build build -j $(getconf _NPROCESSORS_ONLN) --target package
```

Note: If your `(i686|x86_64)-w64-mingw32` directory is not in `/usr` (e.g. when on Debian), the mingw-prep scripts and the CMake
command won't work. You need adjust the mingw-prep scripts and pass `-DCROSS_PREFIX=/path` to CMake to set the path to the parent
of the `(i686|x86_64)-w64-mingw32` directory.
</details>
<details><summary>Windows via Visual Studio</summary>

### Installing dependencies

Make sure to install the `C++ CMake tools for Windows` component for Visual Studio.

Execute the following commands (via cmd or powershell):
1. Install [Git for Windows](https://gitforwindows.org/)

   ```
   git clone https://github.com/microsoft/vcpkg
   ```

2. Setup [vckpg](https://github.com/microsoft/vcpkg#quick-start-windows)

   ```
   cd vcpkg
   bootstrap-vcpkg.bat
   vcpkg integrate install
   ```

3. Install the required dependencies

   For the 64-bit version:

   ```
   vcpkg install sdl2:x64-windows libsodium:x64-windows
   ```

   For the 32-bit version:

   ```
   vcpkg install sdl2:x86-windows libsodium:x86-windows
   ```

*Note*
You can download the libraries manually from [SDL2](https://www.libsdl.org/download-2.0.php) and [Libsodium](https://github.com/jedisct1/libsodium/releases).

### Compiling

* **Through Open->CMake in Visual Studio**

1. Go to `File -> Open -> CMake`, select `CMakeLists.txt` from the project root.
2. Select the `x64-Release` configuration (or `x86-Release` for 32 bit builds, `-Debug` for debug builds).
3. Select `Build devilution.exe` from the `Build` menu.

* **Through GCC/WSL in Visual Studio**

1. Ensure the WSL environment has the build pre-requisites for both devilutionX (see "Installing dependencies on Debian and Ubuntu" under the "Linux" section above) and [WSL remote development](https://docs.microsoft.com/en-us/cpp/linux/connect-to-your-remote-linux-computer?view=msvc-160#connect-to-wsl).
2. Select the `WSL-GCC-x64-Debug` configuration.
3. Select `Build devilution` from the `Build` menu.

* **Through cmake-gui**

1. Input the path to devilutionx source directory at `Where is the source code:` field.
2. Input the path where the binaries would be placed at `Where to build the binaries:` field. If you want to place them inside source directory it's preferable to do so inside directory called `build` to avoid the binaries being added to the source tree.
3. It's recommended to input `Win32` in `Optional Platform for Generator`, otherwise it will default to x64 build.
4. In case you're using `vcpkg` select `Specify toolchain file for cross-compiling` and select the file `scripts/buildsystems/vcpkg.cmake` from `vcpkg` directory otherwise just go with `Use default native compilers`.
5. In case you need to select any paths to dependencies manually do this right in cmake-gui window.
6. Press `Generate` and open produced `.sln` file using Visual Studio.
7. Use build/debug etc. commands inside Visual Studio Solution like with any normal Visual Studio project.

</details>

<details><summary>Android</summary>

### Installing dependencies

Install [Android Studio](https://developer.android.com/studio)
After first launch configuration, go to "Configure -> SDK Manager -> SDK Tools".
Select "NDK (Side by side)" and "CMake" checkboxes and click "OK".

### Compiling

Click "Open Existing Project" and choose "android-project" folder in DevilutionX root folder.
Wait until Gradle sync is completed.
In Android Studio, go to "Build -> Make Project" or use the shortcut Ctrl+F9
You can find the compiled APK in `/android-project/app/build/outputs/apk/`
</details>

<details><summary>Nintendo Switch</summary>

### Installing dependencies

https://devkitpro.org/wiki/Getting_Started

- Install (dkp-)pacman: https://devkitpro.org/wiki/devkitPro_pacman

- Install required packages with (dkp-)pacman:

```
sudo (dkp-)pacman -S --needed - < Packaging/switch/packages.txt
```

### Compiling

```bash
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/Switch.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(getconf _NPROCESSORS_ONLN)
```

The nro-file will be generated in the build folder. Test with an emulator (RyuJinx) or real hardware.

[Nintendo Switch manual](/docs/manual/platforms/switch.md)
</details>

<details><summary>Nintendo 3DS</summary>

### Installing dependencies

https://devkitpro.org/wiki/Getting_Started

- Install (dkp-)pacman: https://devkitpro.org/wiki/devkitPro_pacman

- Install required packages with (dkp-)pacman:

```
sudo (dkp-)pacman -S \
    devkitARM general-tools 3dstools libctru \
    citro3d 3ds-sdl 3ds-libpng 3ds-bzip2 \
    3ds-cmake 3ds-pkg-config picasso 3dslink
```

- Download or compile [bannertool](https://github.com/diasurgical/bannertool/releases) and [makerom](https://github.com/jakcron/Project_CTR/releases)
  - Copy binaries to: `/opt/devkitpro/tools/bin/`

### Compiling

_If you are compiling using MSYS2, you will need to run `export MSYS2_ARG_CONV_EXCL=-D` before compiling.
Otherwise, MSYS will sanitize file paths in compiler flags which will likely lead to errors in the build._

```bash
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(getconf _NPROCESSORS_ONLN)
```

The output files will be generated in the build folder.

[Nintendo 3DS manual](/docs/manual/platforms/3ds.md)
</details>

<details><summary>PlayStation Vita</summary>

### Compiling

```bash
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=${VITASDK}/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

[PlayStation Vita manual](/docs/manual/platforms/vita.md)
</details>


<details><summary>PlayStation 4</summary>

### Installing dependencies

Install [PacBrew openorbis SDK](https://github.com/PacBrew/pacbrew-packages)

### Compiling

```console
devilutionX$ ./Packaging/ps4/build.sh
```

[PlayStation 4 manual](/docs/manual/platforms/ps4.md)
</details>


<details><summary>Haiku</summary>

### Installing dependencies on 32 bit Haiku

```
pkgman install cmake_x86 devel:libsdl2_x86 devel:libsodium_x86
```

### Installing dependencies on 64 bit Haiku

```
pkgman install cmake devel:libsdl2 devel:libsodium
```

### Compiling on 32 bit Haiku

```bash
setarch x86 #Switch to secondary compiler toolchain (GCC8+)
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(getconf _NPROCESSORS_ONLN)
```

### Compiling on 64 bit Haiku

No setarch required, as there is no secondary toolchain on x86_64, and the primary is GCC8+

```
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(getconf _NPROCESSORS_ONLN)
```

</details>

<details><summary>OpenDingux / RetroFW</summary>

DevilutionX uses buildroot-based toolchains to build packages for OpenDingux and RetroFW.

For OpenDingux / RetroFW builds, `mksquashfs` needs to be installed on your machine.

To build, run the following command:

~~~ bash
TOOLCHAIN=<path/to/toolchain> Packaging/OpenDingux/build.sh <platform>
~~~

Replace `<platform>` with one of: `lepus`, `retrofw`, `rg99`, `rg350`, or `gkd350h`.

For example:

~~~ bash
TOOLCHAIN=/opt/gcw0-toolchain Packaging/OpenDingux/build.sh rg350
~~~

You can download the prebuilt toolchains for `x86_64` hosts here:

* OpenDingux: https://github.com/OpenDingux/buildroot/releases
* RetroFW: https://github.com/Poligraf/retrofw_buildroot_gcc11/releases

Remember to run `./relocate-sdk.sh` in the toolchain directory after unpacking it.

End-user manuals are available here:

* [RetroFW manual](/docs/manual/platforms/retrofw.md)
* [RG-350 manual](/docs/manual/platforms/rg350.md)
* [GKD350h manual](/docs/manual/platforms/gkd350h.md)

</details>

<details><summary>Clockwork PI GameShell</summary>

You can either call

~~~ bash
Packaging/cpi-gamesh/build.sh
~~~

to install dependencies and build the code.

Or you create a new directory under `/home/cpi/apps/Menu` and copy [the file](../Packaging/cpi-gamesh/__init__.py) there. After restarting the UI, you can download and compile the game directly from the device itself. See [the readme](../Packaging/cpi-gamesh/readme.md) for more details.
</details>

<details><summary>Amiga via Docker</summary>

### Build the container from the repo root

~~~ bash
docker build -f Packaging/amiga/Dockerfile -t devilutionx-amiga .
~~~

### Build DevilutionX Amiga binary

~~~ bash
docker run -u "$(id -u "$USER"):$(id -g "$USER")" --rm -v "${PWD}:/work" devilutionx-amiga
~~~

The command above builds DevilutionX in release mode.
For other build options, you can run the container interactively:

~~~ bash
docker run -u "$(id -u "$USER"):$(id -g "$USER")" -ti --rm -v "${PWD}:/work" devilutionx-amiga bash
~~~

See the `CMD` in `Packaging/amiga/Dockerfile` for reference.

To actually start DevilutionX, increase the stack size to 50KiB in Amiga.
You can do this by selecting the DevilutionX icon, then hold right mouse button and
select Icons -> Information in the top menu.
</details>

<details><summary><b>32-bit building on 64-bit platforms</b></summary><blockquote>

<details><summary>Linux</summary>

Note that ```pkg-config``` is an optional dependency for finding libsodium,
although we have a fallback if necessary.

### Installing dependencies on Debian and Ubuntu

```
sudo apt-get install git rpm cmake g++-multilib libsdl2-dev:i386 libsodium-dev libsodium-dev:i386
```

### Compiling

```bash
git clone https://github.com/pionere/devilutionx
cd devilutionx
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../CMake/32bit.cmake
cmake --build build -j $(getconf _NPROCESSORS_ONLN)
```

</details>

<details><summary>MacOS</summary>

### Installing dependencies

Install [Xcode 9.4.1 and Xcode Command Line tools](https://developer.apple.com/download/more/?=xcode%209.4.1), this is the last version with **32 bits** support.

Note: Be sure that your to select the command line Xcode if you have more then one installed:

```
$ sudo xcode-select --switch /Applications/Xcode.app
```

Install the build tools using [Homebrew](https://brew.sh/):

```
brew install automake autoconf libtool
```

Get SDL2 and Libsodium:

```
./xcode-build.sh --get-libs
```

### Compiling

```
./xcode-build.sh --build-libs
./xcode-build.sh --build-project
./xcode-build.sh --package
```

</details>

<details><summary>Windows via MinGW</summary>

### Installing dependencies on Debian and Ubuntu

Download and place the 32bit MinGW Development Libraries of [SDL2](https://www.libsdl.org/download-2.0.php) and [Libsodium](https://github.com/jedisct1/libsodium/releases) in `/user/i686-w64-mingw32`. This can be done automatically by running `Packaging/windows/mingw-prep.sh`

```
sudo apt-get install cmake gcc-mingw-w64-i686 g++-mingw-w64-i686 wget git
```

### Compiling

```
git clone https://github.com/pionere/devilutionx
cd devilutionx
Packaging/windows/mingw-prep.sh  
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=../CMake/mingwcc.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(getconf _NPROCESSORS_ONLN)
```

</details>

</blockquote></details>

<details><summary><b>CMake build options</b></summary>

### General

- `-DCMAKE_BUILD_TYPE=Release` change build type to release and optimize for distribution.
- `-DVERSION_NUM=XXX` set version number (project version) to the desired value.
- `-DUSE_SDL1=ON` build for SDL v1 instead of v2, not all features are supported under SDL v1, notably upscaling.
- `-DCMAKE_TOOLCHAIN_FILE=../CMake/32bit.cmake` generate 32bit builds on 64bit platforms (remember to use the `linux32` command if on Linux).
- `-DNOSOUND=ON` disable sound support
- `-DSTREAM_ALL_AUDIO=ON` stream all the audio. For extremely RAM-constrained platforms
- `-DNOWIDESCREEN=ON` disable widescreen support
- `-DNONET=ON` disable network support, this also removes the need for the ASIO and Sodium.
- `-DINET_MODE=ON` enable validation of network messages
- `-DADAPTIVE_NETUPDATE=OFF` disable adaptive network
- `-DNETENCRYPT=OFF` disable encryption of network messages
- `-DTCPIP=OFF` disable tcp/ip support
- `-DNOHOSTING=OFF` enable host-only games
- `-DHOSTONLY=ON` disable support for non host-only games
- `-DHELLFIRE=ON` build Hellfire version
- `-DHAS_JOYSTICK=0` disable joystick support
- `-DHAS_DPAD=0` disable dpad support
- `-DHAS_KBCTRL=0` disable keyboard-controller support
- `-DHAS_GAMECTRL=0` disable game-controller support
- `-DHAS_TOUCHPAD=0` disable touchpad support
- `-DASSET_MPL=2` use upscaled assets, requires devilx_hdX.mpq (e.g. devilx_hd2.mpq)
- `-DSCREEN_WIDTH=640` hardcode screen width to 640 pixel
- `-DSCREEN_HEIGHT=480` hardcode screen height to 480 pixel
- `-DMPQONE="hellone.mpq"` The name of the merged MPQ file. ("diablone.mpq" in case of non-hellfire game)
- `-DCREATE_MPQONE=OFF` Merge the .mpq files to "hellone.mpq". Takes a few minutes, but required to be done only once.
- `-DUSE_MPQONE=OFF` Force the use of a merged .mpq file.
- `-DUSE_PATCH=OFF` build a separate binary to manipulate MPQ files (expect it to be used before the game is launched).

### Debug builds

- `-DDEBUG=OFF` disable debug mode of the Diablo engine.
- `-DASAN=OFF` disable address sanitizer.
- `-DUBSAN=OFF` disable undefined behavior sanitizer.

</details>
