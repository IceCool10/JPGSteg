#include "FileWriter.h"

FileWriter::FileWriter() {
    this->inputFile = NULL;
}

FileWriter::FileWriter(FILE* inputFile) {
    this->inputFile = inputFile;
}


FileWriter::FileWriter(const char* fileName, const char* mode) {

    this->inputFile = fopen(fileName,mode);

}
