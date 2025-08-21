rem call VsDevCmd.bat

mkdir build
cd build

rem prepare SDL2
rem - checkout
git clone https://github.com/pionere/SDL.git
git -C SDL checkout ede1535a4be9aa4f0d28160e4d3bb9c3d685523c
rem - build
msbuild /p:PlatformToolset=v143;TargetPlatformVersion=10.0.26100.0;TargetPlatformMinVersion=10.0.14393.0;ConfigurationType=StaticLibrary;Configuration=Release;Platform=x64 SDL\VisualC-WinRT\SDL-UWP.vcxproj

rem prepare devilutionx
rem - configure
cmake -DUWP_LIB=1 -DUWP_SDL2_DIR="%CD%/SDL" -DCMAKE_BUILD_TYPE=x64-Release ..
rem - build
msbuild /p:Configuration=Release;Platform=x64 DevilutionX.sln

powershell "Get-Content ..\uwp-project\Package.appxmanifest.template | %% {$_ -replace '__PROJECT_VERSION__',$(& {git describe --tags --abbrev=0})} | Out-File -FilePath ..\uwp-project\Package.appxmanifest -encoding ASCII"
rem - build the game
msbuild /p:Configuration=Release;Platform=x64;AppxBundle=Always;AppxBundlePlatforms=x64 ..\uwp-project\devilutionx.sln

rem create package
powershell "New-Item -Path uwp-project\pkg -ItemType Directory -Force; Get-Childitem -Path uwp-project\AppxPackages, uwp-project\Release, uwp-patcher\Release -Include Microsoft.VCLibs.x64.*.appx, devil*_x64.appx -File -Recurse | Copy-Item -Destination uwp-project\pkg\\"
rem powershell "Get-Childitem -Path uwp-project\AppxPackages, uwp-project\Release -Include Microsoft.VCLibs.x64.*.appx, devil*_x64.appx -File -Recurse | Get-Unique -AsString | Compress-Archive -DestinationPath devilutionx.zip"
powershell "Compress-Archive -Path uwp-project\pkg\* -DestinationPath devilutionx.zip"
