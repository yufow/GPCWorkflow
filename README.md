## GPCWorkflow



### SETUP

- Compile gpl.c with gcc:   `gcc gpl.c -o gpl`
- Install notepad++ and import GamePackLibrary.xml as a user defined language
- Add gpl to path



### GPL FILE FEATURES

- Import modules from gpclib:		 			`#include <module>`
- Import files from your own directories: 		`#include "filepath"`
- Load command line arguments into script:		`#pragma args`



### USING GPL

- Building a gpl file:	`gpl filename.gpl`
- Build a file with cl args:					`gpl filename.gpl arg1 arg2 arg3`












