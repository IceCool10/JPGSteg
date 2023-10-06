#pragma once
#ifndef _LZMADECODER_H_
#define _LZMADECODER_H_

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <memory>
#include "lzma/LzmaEnc.h"
#include "lzma/LzmaDec.h"
using namespace std;

static void *_lzmaAlloc(ISzAllocPtr, size_t size) {
    return new uint8_t[size];
}
static void _lzmaFree(ISzAllocPtr, void *addr) {
    if (!addr)
        return;

    delete[] reinterpret_cast<uint8_t *>(addr);
}

static ISzAlloc _allocFuncs = {
    _lzmaAlloc, _lzmaFree
};

class LzmaDecoder {

    private:
        string message;
    public:

        LzmaDecoder() = delete;
        explicit LzmaDecoder(string message);
        std::unique_ptr<uint8_t[]> lzmaCompress(uint32_t *outputSize);
        std::unique_ptr<uint8_t[]> lzmaDecompress(uint32_t *outputSize);

};

#endif
