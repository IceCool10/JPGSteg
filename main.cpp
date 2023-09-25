#include <stdio.h>
#include <string.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include "/opt/libjpeg-turbo/include/jpeglib.h"
#include "JPEGFile.h"
using namespace std;

void help(const char* prog) {

    printf("%s [-e / -d] -m [MESSAGE] -i [INPUT_JPG_FILE] -o [OUTPUT_FILE]\n", prog);
}


int main(int argc, char** argv) {

	if (argc <= 2) {
        help(argv[0]);
		return -1;
	}
    
    const char* inputFile   = nullptr;
    const char* outputFile  = nullptr;
    const char* message     = nullptr;

    bool encode = false;
    bool decode = false;

    for (int i = 0; i < argc; i++) {
        if (string(argv[i]) == "-o") {
            if((i + 1) < argc) {
                outputFile = argv[i+1];
            }
        }

        else if(string(argv[i]) == "-i") {
            if((i + 1) < argc) {
                inputFile = argv[i+1];
            }
            else {
                help(argv[0]);
                return -1;
            }
        }

        else if(string(argv[i]) == "-m") {
            if((i + 1) < argc) {
                message = argv[i+1];
            }
        }

        else if(string(argv[i]) == "-e") {
            encode = true;
        }
        else if(string(argv[i]) == "-d") {
            decode = true;
        }
    }

    if (encode == decode) {
        help(argv[0]);
        return -1;
    }

    if ((encode == true) && ((message == nullptr) || (outputFile == nullptr))) {
        help(argv[0]);
        return -1;
    }

    JPEGFile* jpegFile = new JPEGFile(inputFile, outputFile);
    if (jpegFile == nullptr) {
        printf("[-] ERROR allocating JPEGFile\n");
        return -1;
    }

    if(encode) {
        printf("[*] Encoding...\n");
        if(jpegFile->ReadJPEGFile() == 0) {
            delete jpegFile;
            return -1;
        }

        printf("[+] ReadJPEG OK\n");
        jpegFile->HideMessage(message);
        printf("[+] HideMessage OK\n");
        jpegFile->WriteJPEGFile(100, 12);       
        printf("[+] WriteJPEGFile OK\n");
    }
    else if(decode) {

        printf("[*] Decoding...\n");
        if(jpegFile->ReadJPEGFile() == 0) {
            delete jpegFile;
            return -1;
        }
        printf("[+] ReadJPEG OK\n");

        jpegFile->DecodeMessage();
    }

    delete jpegFile;
    
	return 0;
}

