#pragma once
#ifndef _JPEGFILE_H_
#define _JPEGFILE_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <vector>
#include "/opt/libjpeg-turbo/include/jpeglib.h"
#include "jerror.h"
#include "LzmaDecoder.h"
using namespace std;

class JPEGFile {

    private:
        unsigned char* jdata;
        const char* filename;
        const char* outputFile;
        unsigned long long image_width;
        unsigned long long image_height;
        unsigned long long data_size;
        size_t messageLength;
        unsigned int* HeightInBlocks;
        unsigned int* WidthInBlocks;
        jvirt_barray_ptr *DctCoeff;

        static const unsigned int CoeffPerBlock = 64;
        unsigned int DivRoundUp(unsigned int x, unsigned int y);
        vector<signed short> LinDctCoeffs;

        struct jpeg_compress_struct cinfo ;
        struct jpeg_decompress_struct decinfo ;
        LzmaDecoder* lzmaDecoder;
        

    public:
        JPEGFile() = delete;
        explicit JPEGFile(const char* filename, const char* outputFile);
        int ReadJPEGFile();
        bool HideMessage(const char* message);
        bool DecodeMessage();
        void WriteJPEGFile (int quality, int data_precision);
        ~JPEGFile();
};

#endif
