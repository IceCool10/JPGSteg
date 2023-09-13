#pragma once
#ifndef _JPEGFILE_H_
#define _JPEGFILE_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include "jpeglib.h"

class JPEGFile {

    private:
        unsigned char* jdata;
        const char* filename;
        const char* outputFile;
        unsigned long long image_width;
        unsigned long long image_height;
        unsigned long long data_size;
        size_t messageLength;

    public:
        JPEGFile() = delete;
        explicit JPEGFile(const char* filename, const char* outputFile);
        int ReadJPEGFile();
        bool HideMessage(const char* message);
        void WriteJPEGFile (int quality);
        ~JPEGFile();
};

#endif
