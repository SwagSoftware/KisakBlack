# KisakBlack

## About the project
An open source fully-buildable reimplementation of Call of Duty Black ops's Multi-Player .exe

Aimed towards mod developers and BLOPS enthusiasts.

### Development Blog
Learn about the Development of KisakBlack here: [https://lwss.github.io/Kisak-Black/](https://lwss.github.io/Kisak-Black/)

## Building

Both native targets require a Steam copy of [Call of Duty: Black Ops](https://store.steampowered.com/app/42700/Call_of_Duty_Black_Ops/) and its game data.

### Windows x86

Install Visual Studio 2022, CMake 3.16+, and the [DirectX SDK (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=6812), then run:

```sh
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release
```

Copy the game data and required runtime DLLs beside the executable before launching it.

### Linux x86 / OpenGL

Install CMake and the 32-bit GCC, SDL2, GLEW, Mesa GL/GLU, OpenAL, Speex, and VPX development packages, then run:

```sh
./build_linux.sh
```

The executable is written to `build_linux/blackops`.


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
