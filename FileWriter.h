#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <fstream>
#include <unistd.h>
#include <stdio.h>

class FileWriter {
	private:
        FILE* inputFile; 
    public:	
        FileWriter();
        FileWriter(FILE* inputFile);
        FileWriter(const char* fileName, const char* mode);
        ~FileWriter();
};

#endif
