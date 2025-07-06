

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <winbase.h>
#include <ShellApi.h>
#include <windows.h>


// Definitions
// cwd
#define CWD_MAX 200
// Include
#define INCLUDE_FILENAME_MAX 300
// Includes
#define INCLUDES_MAX 1000

#define MEMORY_FIRST 1

const char INTERNAL_LIB_OPEN = '<';
const char INTERNAL_LIB_CLOSE = '>';
const char USER_LIB_OPEN = '"';
const char USER_LIB_CLOSE = '"';
const char PREPROCESSOR_TAG = '#';
const char INPUT_FILE_EXT[] = ".gpl";


// Enums
enum { INTERNAL_LIB, USER_LIB };


// Structs
typedef struct Include {
	char filename[INCLUDE_FILENAME_MAX];
	int libtype;
} Include;


// Global
Include includes[INCLUDES_MAX];
int includesLen = 0;
char *directory = NULL;
int clArgs;
int PragmaArgs = 0;
int memorygpl = 0;
char memorygplname[] = "memory.gpl";


void view_debug(const char* pszFileName) {
    ShellExecuteA(GetDesktopWindow(), "open", pszFileName, NULL, NULL, SW_SHOW);
}


// Append include to an includes array
int Include_append(Include p) {
	if(includesLen) {
		for(int i = 0; i < includesLen; i++) {
			if(!strcmp(includes[i].filename, p.filename)) return 1;
		}
	}
	includes[includesLen] = p;
	includesLen++;
	return 0;
}


char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
	for (char* p = current_pos; (current_pos = strchr(str, find)) != NULL; *current_pos = replace);
    return str;
}


// Parse files recursively
int ParseFile(char *filename) {


	// Open source file
	FILE *f = fopen(filename, "r");
	if(f == NULL) {
		printf("ERROR: Couldn't open file %s\n", filename);
		return 1;
	}


	// Parse source file
	int c;
	while((c = fgetc(f)) != EOF) {


		// Found Preprocess statement
		if(c == PREPROCESSOR_TAG) {


			// Get preprocess statement name
			int i = 0;
			char buffer1[INCLUDE_FILENAME_MAX];
			while(isalpha(c = fgetc(f))) {
				buffer1[i] = c;
				i++;
			}
			buffer1[i] = 0;
			while(isspace(c = fgetc(f)));


			// If its include...
			if(!strcmp(buffer1, "include")) {


				// Instantiate include struct
				Include p;


				// Get the file name
				char buffer2[INCLUDE_FILENAME_MAX];
				i = 0;
				while(isalpha(c) || isdigit(c) || c == USER_LIB_OPEN || c == USER_LIB_CLOSE || c == INTERNAL_LIB_OPEN || c == INTERNAL_LIB_CLOSE || c == '.') {
					buffer2[i] = c;
					c = fgetc(f);
					i++;
				}
				buffer2[i] = 0;


				// Get the libtype
				if((buffer2[0] == INTERNAL_LIB_OPEN) && (buffer2[strlen(buffer2) - 1] == INTERNAL_LIB_CLOSE)) {
					p.libtype = INTERNAL_LIB;
				}
				else if((buffer2[0] == USER_LIB_OPEN) && (buffer2[strlen(buffer2) - 1] == USER_LIB_CLOSE)) {
					p.libtype = USER_LIB;
				}
				else {
					printf("ERROR: Invalid use of #include in %s\n", filename);
					return 1;
				}


				// Copy the filename into struct
				char fName[INCLUDE_FILENAME_MAX];
				memset(fName, 0, sizeof(fName));

				for(int i = 1; i < strlen(buffer2) - 1; i++) {
					fName[i - 1] = buffer2[i];
				}
				fName[i - 2] = 0;

				if(!strcmp(fName, memorygplname)) memorygpl = 1;

				memset(p.filename, 0, sizeof(p.filename)); 
				if(p.libtype == INTERNAL_LIB) {
					strncat(p.filename, directory, strlen(directory) + 1);
					strncat(p.filename, "\\gpclib\\", sizeof("\\gpclib\\"));
					strncat(p.filename, fName, strlen(fName) + 1);

				}
				else {
					strncat(p.filename, fName, strlen(fName) + 1);
				}


				// Try to append the include struct to the array of includes
				if(!Include_append(p)) {
					// If new file found, parse that file before finsishing this one
					if(ParseFile(p.filename)) return 1;
				}
			}
			else if(!strcmp(buffer1, "pragma")) {
				char PragmaDirective[100];
				memset(PragmaDirective, 0, sizeof(PragmaDirective));
				int ccount = 0;
				while(!isspace(c)) {
					PragmaDirective[ccount] = c;
					ccount++;
					c = fgetc(f);
				}
				if(!strcmp(PragmaDirective, "args")) {
					memorygpl = 1;
					PragmaArgs = 1;
				}
			}
		}
	}


	// Close file descriptors
	fclose(f);
	return 0;
}


char *strndup(const char *s, size_t n) {
    char *p;
    size_t n1;

    for (n1 = 0; n1 < n && s[n1] != '\0'; n1++)
        continue;
    p = malloc(n + 1);
    if (p != NULL) {
        memcpy(p, s, n1);
        p[n1] = '\0';
    }
    return p;
}


int main(int argc, char *argv[]) {

	// Get directory of executable
	char buf[INCLUDE_FILENAME_MAX];
	GetModuleFileName(NULL, buf, sizeof(buf));
	char *lastSlash = NULL;
	lastSlash = strrchr(buf, '\\'); // you need escape character
	directory = strndup(buf, lastSlash - buf);


	// Check only one filename given
	if(argc < 2) {
		printf("ERROR: Please provide a file name\n");
		return 1;
	}


	clArgs = argc - 2;


	// Check file is of type '.gpl'
	char *str = strstr(argv[1], INPUT_FILE_EXT);
	if(str == NULL) {
		printf("ERROR: Not a %s file\n", INPUT_FILE_EXT);
		return 1;
	}
	else if(str - argv[1] != strlen(argv[1]) - strlen(INPUT_FILE_EXT)) {
		printf("ERROR: Not a %s file\n", INPUT_FILE_EXT);
		return 1;
	}


	// Check source file
	char inputFile[INCLUDE_FILENAME_MAX];
	memset(inputFile, 0, sizeof(inputFile));
	strncpy(inputFile, argv[1], strlen(argv[1]) + 1);

	FILE *f = fopen(inputFile, "r");
	if(f == NULL) {
		printf("ERROR: Couldn't open source file %s\n", inputFile);
		return 1;
	}
	fclose(f);
	


	// Open destination file
	char outputFile[INCLUDE_FILENAME_MAX];
	//strncat(outputFile, argv[2], strlen(argv[2]) + 1);
	//strncat(outputFile, ".gpc", sizeof(".gpc"));
	strncat(outputFile, argv[1], strlen(argv[1]) + 1);
	outputFile[strlen(outputFile) - 1] = 'c';

	FILE *fout = fopen(outputFile, "w");
	if(fout == NULL) {
		printf("ERROR: Couldn't create destination file %s\n", outputFile);
		fclose(fout);
		return 1;
	}


	if(!strcmp(inputFile, outputFile)) {
		printf("ERROR: Input filename and output filename are matching.\n");
		fclose(fout);
		return 1;
	}



	// Spoof first file as the source file
	Include p;
	p.libtype = USER_LIB;
	strncpy(p.filename, inputFile, strlen(inputFile) + 1);
	Include_append(p);


	// Parse the file
	int parse = ParseFile(inputFile);
	if(parse) return 1;



	// Write to file
	for(int i = 0; i < includesLen; i++) {

		if(!i && memorygpl && MEMORY_FIRST) {
			fputs("\n // Memory base pointers\n", fout);
			fputs("int deref[0];\nint HEAP[HEAP_SIZE];\nint ALLOCATED[HEAP_SIZE / 32 + 1];\n\n", fout);
		}

		if(!i && PragmaArgs) {
			Include a;
			strncpy(a.filename, directory, strlen(directory) + 1);
			strncat(a.filename, "\\gpclib\\", sizeof("\\gpclib\\"));
			strncat(a.filename, "memory.gpl", sizeof("memory.gpl"));
			a.libtype = INTERNAL_LIB;
			Include_append(a);

			Include s;
			strncpy(s.filename, directory, strlen(directory) + 1);
			strncat(s.filename, "\\gpclib\\", sizeof("\\gpclib\\"));
			strncat(s.filename, "string.gpl", sizeof("string.gpl"));
			s.libtype = INTERNAL_LIB;
			Include_append(s);


			fputs("\n// Command Line Args\n", fout);

			char argcBuf[100];
			sprintf(argcBuf, "define ARGC = %d;\n", clArgs);
			fputs(argcBuf, fout);

			char argvBuf[1000];
			char argvElement[100];
			sprintf(argvBuf, "const string ARGV[] = {");

			if(clArgs) {
				for(int j = 0; j < clArgs; j++) {
					if(j) strncat(argvBuf, ", ", sizeof(", "));
					memset(argvElement, 0, sizeof(argvElement));
					sprintf(argvElement, "\"%s\"", argv[2 + j]);
					strncat(argvBuf, argvElement, strlen(argvElement) + 1);
				}
			}
			else {
				strncat(argvBuf, "\"\"", sizeof("\"\""));
			}
			fputs(argvBuf, fout);
			fputs("};\n", fout);

			fputs("int argc, argv; init { argv = memory_alloc(ARGC); for(argc=0; argc<ARGC; argc++) { deref[argv + argc] = string(ARGV[argc]); }}\n", fout);
		}


		// Add some spacing
		if(i) fputs("\n\n\n\n", fout);
		if(i == 1) {
			fputs("\n\n\n// Library code //\n\n\n", fout);
		}


		// Open source files
		FILE *f = fopen(includes[i].filename, "r");
		if(f == NULL) {
			printf("ERROR: Couldn't open file %s\n", includes[i].filename);
			return 1;
		}


		// Write source file text to destination file
		int c;
		while((c = fgetc(f)) != EOF) {

			if(c == EOF) break;

			// Get rid of preprocessor statements from final output
			if(c == PREPROCESSOR_TAG) {
				while((c = fgetc(f)) != '\n');
				continue;
			}

			fputc(c, fout);
		}

		fclose(f);
	}
	fclose(fout);

	view_debug(outputFile);

	// Return success
	return 0;
}
