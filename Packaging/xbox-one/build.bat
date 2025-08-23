rem call VsDevCmd.bat

set TargetPlatformVersion=10.0.26100.0
set TargetPlatformMinVersion=10.0.10069.0
set PlatformToolset=v143
set Platform=x64
set Configuration=Release
set AppVersion=1.0.0.0

set BuildParams=PlatformToolset=%PlatformToolset%;TargetPlatformVersion=%TargetPlatformVersion%;TargetPlatformMinVersion=%TargetPlatformMinVersion%;Configuration=%Configuration%;Platform=%Platform%

mkdir build
cd build

rem prepare SDL2
rem - checkout
git clone https://github.com/pionere/SDL.git
git -C SDL checkout 63ce2c16c00778ceb4dde5676658f259f0a61e59
rem - configure
powershell "(Get-Content SDL\VisualC-WinRT\SDL-UWP.vcxproj ) -replace '<PreprocessorDefinitions>(.*)</PreprocessorDefinitions>','<PreprocessorDefinitions>SDL_LOCALE_DISABLED;SDL_MISC_DISABLED;SDL_POWER_DISABLED;SDL_SENSOR_DISABLED;SDL_FILE_DISABLED;SDL_AUDIO_DRIVER_DISK_DISABLED;SDL_AUDIO_DRIVER_DUMMY_DISABLED;SDL_VIDEO_DRIVER_DUMMY_DISABLED;SDL_DYN_MEMFUNCS_DISABLED;SDL_BLIT_0_DISABLED;SDL_BLIT_A_DISABLED;SDL_BLIT_N_DISABLED;SDL_BLIT_N_RGB565_DISABLED;SDL_BLIT_AUTO_DISABLED;SDL_BLIT_SLOW_DISABLED;SDL_RLE_ACCEL_DISABLED;SDL_BLIT_TRANSFORM_DISABLED;SDL_YUV_FORMAT_DISABLED;SDL_GESTURES_DISABLED;SDL_ANALYZE_MACROS_DISABLED;SDL_ASSERTIONS_DISABLED;SDL_LOGGING_DISABLED;SDL_VERBOSE_ERROR_DISABLED;SDL_SANITIZE_ACCESS_DISABLED;${1}</PreprocessorDefinitions>' | Set-Content -encoding ASCII SDL\VisualC-WinRT\SDL-UWP.vcxproj"
rem - build
msbuild /p:%BuildParams%;ConfigurationType=StaticLibrary SDL\VisualC-WinRT\SDL-UWP.vcxproj
if errorlevel 1 goto error

rem prepare devilutionx
rem - configure
cmake .. -DUWP_LIB=1 -DUWP_SDL2_DIR="%CD%/SDL" -DCMAKE_BUILD_TYPE=%Platform%-%Configuration% %*
if errorlevel 1 goto error
rem - build
msbuild /p:%BuildParams% DevilutionX.sln
if errorlevel 1 goto error

powershell "(Get-Content ..\uwp-project\Package.appxmanifest.template ) -replace '__PROJECT_VERSION__', '%AppVersion%' -replace '__PLATFORM_MIN_VERSION__','%TargetPlatformMinVersion%' -replace '__PLATFORM_MAX_VERSION__','%TargetPlatformVersion%' | Set-Content -encoding ASCII ..\uwp-project\Package.appxmanifest"
rem - build the game
msbuild /p:%BuildParams%;AppxBundle=Always;AppxBundlePlatforms=%Platform% ..\uwp-project\devilutionx.sln
if errorlevel 1 goto error

rem build the patcher
IF EXIST devil_patcher.dir (
  powershell "(Get-Content ..\uwp-project\Patcher.appxmanifest.template ) -replace '__PROJECT_VERSION__','%AppVersion%' -replace '__PLATFORM_MIN_VERSION__','%TargetPlatformMinVersion%' -replace '__PLATFORM_MAX_VERSION__','%TargetPlatformVersion%' | Set-Content -encoding ASCII ..\uwp-project\Patcher.appxmanifest"

  msbuild /p:%BuildParams%;AppxBundle=Always;AppxBundlePlatforms=%Platform% ..\uwp-project\devil_patcher.sln
  if errorlevel 1 goto error
)

rem create package
powershell "New-Item -Path uwp-project\pkg -ItemType Directory -Force; Get-Childitem -Path uwp-project\AppxPackages, uwp-project\Release, uwp-patcher\Release -Include Microsoft.VCLibs.%Platform%.*.appx, devil*_%Platform%.appx -File -Recurse | Copy-Item -Destination uwp-project\pkg\\"
rem powershell "Get-Childitem -Path uwp-project\AppxPackages, uwp-project\Release -Include Microsoft.VCLibs.x64.*.appx, devil*_x64.appx -File -Recurse | Get-Unique -AsString | Compress-Archive -DestinationPath devilutionx.zip"
powershell "Compress-Archive -Path uwp-project\pkg\* -DestinationPath devilutionx.zip"

:error
