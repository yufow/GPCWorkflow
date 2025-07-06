## GPCWorkflow



### DESCRIPTION

GPL stands for Game Pack Library. This is an attempt to try and create a library of frequently used, high quality GPC code. This repo comes
with a set of tools that can be used in tandem with this library such as a preprocessor (also called GPL) and a syntax highlighter for notepad++.



### SETUP

- Compile gpl.c with gcc:   `gcc gpl.c -o gpl`
- Install Notepad++ and import GamePackLibrary.xml as a user defined language
- I reccommend using Npp dark mode
- Add the NppExec plugin to get an integrated terminal in Notepad++
- Add gpl.exe to path



### .GPL FILE FEATURES

- Import module source code from gpclib:		`#include <module.gpl>`
- Import module documentation from gpclib:		`#include <module.doc>`
- Import files from your own directories: 		`#include "filepath"`
- Load command line arguments into script:		`#pragma args`



### USING GPL.EXE

- Building a .gpl file:						`gpl filename.gpl`
- Build a file with command line args:		`gpl filename.gpl arg1 arg2 arg3`
- The output file will be a .gpc file and will be automatically opened in ZenStudio
