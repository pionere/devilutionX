rem call VsDevCmd.bat

mkdir build
cd build

rem prepare SDL2
rem - checkout
git clone https://github.com/pionere/SDL.git
git -C SDL checkout ede1535a4be9aa4f0d28160e4d3bb9c3d685523c
rem - configure
powershell "(Get-Content SDL\VisualC-WinRT\SDL-UWP.vcxproj ) -replace '<PreprocessorDefinitions>(.*)</PreprocessorDefinitions>','<PreprocessorDefinitions>SDL_LOCALE_DISABLED;SDL_MISC_DISABLED;SDL_POWER_DISABLED;SDL_SENSOR_DISABLED;SDL_FILE_DISABLED;SDL_AUDIO_DRIVER_DISK_DISABLED;SDL_AUDIO_DRIVER_DUMMY_DISABLED;SDL_VIDEO_DRIVER_DUMMY_DISABLED;SDL_DYN_MEMFUNCS_DISABLED;SDL_BLIT_0_DISABLED;SDL_BLIT_A_DISABLED;SDL_BLIT_N_DISABLED;SDL_BLIT_N_RGB565_DISABLED;SDL_BLIT_AUTO_DISABLED;SDL_BLIT_SLOW_DISABLED;SDL_RLE_ACCEL_DISABLED;SDL_BLIT_TRANSFORM_DISABLED;SDL_YUV_FORMAT_DISABLED;SDL_GESTURES_DISABLED;SDL_ANALYZE_MACROS_DISABLED;SDL_ASSERTIONS_DISABLED;SDL_LOGGING_DISABLED;SDL_VERBOSE_ERROR_DISABLED;SDL_SANITIZE_ACCESS_DISABLED;${1}</PreprocessorDefinitions>' | Set-Content -encoding ASCII SDL\VisualC-WinRT\SDL-UWP.vcxproj"
rem - build
msbuild /p:PlatformToolset=v143;TargetPlatformVersion=10.0.26100.0;TargetPlatformMinVersion=10.0.14393.0;Configuration=Release;Platform=x64;ConfigurationType=StaticLibrary SDL\VisualC-WinRT\SDL-UWP.vcxproj
if errorlevel 1 goto error

rem prepare devilutionx
rem - configure
cmake .. -DUWP_LIB=1 -DUWP_SDL2_DIR="%CD%/SDL" -DCMAKE_BUILD_TYPE=x64-Release %*
if errorlevel 1 goto error
rem - build
msbuild /p:PlatformToolset=v143;TargetPlatformVersion=10.0.26100.0;TargetPlatformMinVersion=10.0.14393.0;Configuration=Release;Platform=x64 DevilutionX.sln
if errorlevel 1 goto error

powershell "Get-Content ..\uwp-project\Package.appxmanifest.template | %% {$_ -replace '__PROJECT_VERSION__',$(& {git describe --tags --abbrev=0})} | Out-File -FilePath ..\uwp-project\Package.appxmanifest -encoding ASCII"
rem - build the game
msbuild /p:PlatformToolset=v143;TargetPlatformVersion=10.0.26100.0;TargetPlatformMinVersion=10.0.14393.0;Configuration=Release;Platform=x64;AppxBundle=Always;AppxBundlePlatforms=x64 ..\uwp-project\devilutionx.sln
if errorlevel 1 goto error

rem create package
powershell "New-Item -Path uwp-project\pkg -ItemType Directory -Force; Get-Childitem -Path uwp-project\AppxPackages, uwp-project\Release, uwp-patcher\Release -Include Microsoft.VCLibs.x64.*.appx, devil*_x64.appx -File -Recurse | Copy-Item -Destination uwp-project\pkg\\"
rem powershell "Get-Childitem -Path uwp-project\AppxPackages, uwp-project\Release -Include Microsoft.VCLibs.x64.*.appx, devil*_x64.appx -File -Recurse | Get-Unique -AsString | Compress-Archive -DestinationPath devilutionx.zip"
powershell "Compress-Archive -Path uwp-project\pkg\* -DestinationPath devilutionx.zip"

:error
