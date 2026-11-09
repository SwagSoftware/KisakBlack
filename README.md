# KisakBlack

## About the project
An open source fully-buildable reimplementation of Call of Duty Black ops's Multi-Player .exe

Aimed towards mod developers and BLOPS enthusiasts.

### Development Blog
Learn about the Development of KisakBlack here: [https://lwss.github.io/Kisak-Black/](https://lwss.github.io/Kisak-Black/)

## Current Requirements
- Windows OS
- Visual Studio 2022
- CMake >= 3.16
- [DirectX SDK 2010](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
- Steam with a copy of [Call of Duty: Black Ops](https://store.steampowered.com/app/42700/Call_of_Duty_Black_Ops/)


## How to build
1) Install the above requirements and Clone repo
2) create a `/build` folder
3) `CMAKE .. -G"Visual Studio 17 2022" -DCMAKE_GENERATOR_PLATFORM=WIN32`
4) Open the generated .sln file
5) Build Debug or Release (Visual Studio dropdown)
6) Run `powershell -NoProfile -ExecutionPolicy Bypass -File setup.ps1 [-GameDir "D:\Games\Call of Duty Black Ops"] [-Config Debug]`
   (from the repo root, after the first build created `build/`; default config is Release).
   This does **not** copy the ~11 GB install. It records the install folder in `gamedir.txt`, links `zone\` and `main\video`
   into `build/(BUILD_TYPE)/` with directory junctions, and copies only the small files the exe needs next to it
   (`localization.txt`, the in-tree DLLs, the DirectX 9 runtime DLLs). `main\*.iwd` are read straight from the install via `+set fs_cdpath`.
   The install is only read, never changed. Without `-GameDir` it is looked up in Steam.
7) Run the game via the Visual Studio play button (re-run CMake after setup.ps1 so the debugger gets `fs_cdpath`) or `Play.bat [Debug]`.
   Starting the .exe by hand needs `+set fs_cdpath "<your install>"`.

To undo setup: `rmdir build\Release\zone build\Release\main\video` (never `Remove-Item -Recurse` on a junction) and delete `gamedir.txt`.


```
Keep in Mind: This is a ~20 year old game with some known exploits. We will try to fix these as we become aware of them.
However, there is a non-zero chance of some type of binary exploitation when playing online. Use a sandbox (Sandboxie?) for peace of mind. 
```

## Known Issues
(Use the **[issues](https://github.com/SwagSoftware/KisakBlack/issues)** section)

## Troubleshooting
- ***Can't Connect to Dedicated Server*** :
  -  Check `net_ip` and `net_port`, the server will increment the port if the preferred one isn't available but the client won't sweep upwards.
 - ***DLL Error upon launch*** :
   - You didn't copy over the necessary runtime DLL's

## FAQ
- Can we use AI in this project?
  - Yes you can, but you're still responsible for whatever you commit. In general, you should have the AI be assisting you, and not carrying you. We have started using AI to help de-bug, and it's been extremely helpful.

## Credits and Special Thanks
- ***All Original BLOPS Developers (for creating one of the best games of all time)***
- https://github.com/SwagSoftware/KisakCOD (Some code re-used)
- https://github.com/PJayB/jk3src (Jedi Academy fork with .sln)
- https://github.com/voron00/CoD2rev_Server - Useful yacc code for the gsc scripting here
- https://github.com/shiversoftdev/BO3Enhanced - Viewed as reference code for some of the Steam API Auth
- [RAD Game Tools](https://www.radgametools.com/) for their Bink library.


## Discord
[Join the KisakCOD Discord](https://discord.gg/9uqntRWMA3)
