#pragma once
#ifndef _DECODER_H_
#define _DECODER_H_

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <memory>
using namespace std;

class Decoder {

    private:

    protected:
        string message;

    public:
        virtual std::unique_ptr<uint8_t[]> Compress(uint32_t *outputSize) = 0;       
        virtual std::unique_ptr<uint8_t[]> Decompress(uint32_t *outputSize) = 0;       
        virtual ~Decoder() {  }

};


#endif
