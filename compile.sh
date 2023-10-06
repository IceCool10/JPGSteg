#!/bin/bash
g++ -o main main.cpp JPEGFile.cpp lzma/CpuArch.c lzma/LzFind.c lzma/LzmaDec.c lzma/LzmaEnc.c LzmaDecoder.cpp -ljpeg -std=c++14 -D_7ZIP_ST

