#include "JPEGFile.h"
#include "/opt/libjpeg-turbo/include/turbojpeg.h"
#include "/opt/libjpeg-turbo/include/jpeglib.h"
using namespace std;

#define NUMBER_OF_BITS 8

JPEGFile::JPEGFile(const char* filename, const char* outputFile) {

    this->filename = filename;
    this->outputFile = outputFile;
    this->jdata = nullptr;
    this->DctCoeff = nullptr;
    this->image_width = 0;
    this->image_height = 0;
    this->data_size = 0;
    this->messageLength = 0;
}

bool JPEGFile::HideMessage(const char* message) {

    this->lzmaDecoder = new LzmaDecoder(string(message));
    if (!this->lzmaDecoder) {
        return false;
    }
    unsigned int compressedSize = 0;
    unique_ptr<unsigned char[]> compressedMessagePtr = this->lzmaDecoder->Compress(&compressedSize);
    
    if (compressedSize * NUMBER_OF_BITS >= data_size - sizeof(size_t) * NUMBER_OF_BITS) {
        return false;
    }

    const unsigned char* compressedMessage = compressedMessagePtr.get();

    size_t linearDCTOffset = 0;
    
    for(size_t i = 0; i < compressedSize; i++) {
        char c = compressedMessage[i];
        for(unsigned short bit = 0; bit < NUMBER_OF_BITS; bit++) {
            while(LinDctCoeffs[linearDCTOffset] <= 1) {
                linearDCTOffset++;
            }
            LinDctCoeffs[linearDCTOffset] = ((LinDctCoeffs[linearDCTOffset] & 0xFE)  | ((c >> (NUMBER_OF_BITS - bit - 1)) & 1));
            linearDCTOffset++;
            this->messageLength++; // (CMD) Message length as bits
        }
    }
    /*
    printf("\nSTART\n");
    for(size_t i = 0; i < (sizeof(size_t) * 8); i++) {
        printf("%02X ", (LinDctCoeffs[i] & 1));
    }
    */
    //printf("\nthis->messageLength : %16llX\n", this->messageLength);
    linearDCTOffset = data_size - 1;
    for(size_t i = 0; i < (sizeof(size_t) * NUMBER_OF_BITS); i++) {
        while(LinDctCoeffs[linearDCTOffset] <= 1) {
            linearDCTOffset--;
        }
        LinDctCoeffs[linearDCTOffset - i] = ((LinDctCoeffs[linearDCTOffset - i] & 0xFE) | (this->messageLength & 1));
        this->messageLength >>= 1;
    }

    return true;
}

bool JPEGFile::DecodeMessage() {
    
    size_t msgLen = 0;

    size_t linearDCTOffset = data_size - 1;
    for(size_t i = 0; i < (sizeof(size_t) * NUMBER_OF_BITS); i++) {
        while(LinDctCoeffs[linearDCTOffset] <= 1) {
            linearDCTOffset--;
        }
        msgLen |= (LinDctCoeffs[linearDCTOffset - (sizeof(size_t) * NUMBER_OF_BITS) + i] & 1);

        msgLen <<=1;
    }

    //printf("\nthis->msgLen : %16llX\n", msgLen);
    string message(msgLen, '\0');
    linearDCTOffset = 0;

    for(size_t i = 0; i < msgLen / NUMBER_OF_BITS; i++) {
        for(unsigned short bit = 0; bit < NUMBER_OF_BITS; bit++) {
            while(LinDctCoeffs[linearDCTOffset] <= 1) {
                linearDCTOffset++;
            }
            message[i] |= ((LinDctCoeffs[linearDCTOffset++] & 1) << (NUMBER_OF_BITS - bit - 1));
        }

    }
    
    this->lzmaDecoder = new LzmaDecoder(message);
    if (!this->lzmaDecoder) {
        return false;
    }


    unsigned int decompressedSize = 0;
    unique_ptr<unsigned char[]> decompressedMessagePtr = this->lzmaDecoder->Decompress(&decompressedSize);
    unsigned char* decompressedMessage = decompressedMessagePtr.get();
    decompressedMessage[decompressedSize] = '\0';

    printf("[*] HIDDEN MESSAGE : %s\n", decompressedMessage);
    return true;
}

unsigned int JPEGFile::DivRoundUp(unsigned int x, unsigned int y) {
    unsigned int z = y--;
    return ((x + y) / z);
}

int JPEGFile::ReadJPEGFile ()
{
    /* This struct contains the JPEG decompression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     */
    /* We use our private extension JPEG error handler.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    //struct my_error_mgr jerr;
    /* More stuff */
    FILE * infile;		/* source file */
    JSAMPARRAY buffer;		/* Output row buffer */
    int row_stride;		/* physical row width in output buffer */

    /* In this example we want to open the input file before doing anything else,
     * so that the setjmp() error recovery below can assume the file is open.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to read binary files.
     */

    if ((infile = fopen(this->filename, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", this->filename);
        return 0;
    }
    /* Step 1: allocate and initialize JPEG decompression object */
    struct jpeg_error_mgr jerr;
    decinfo.err = jpeg_std_error(&jerr);
    /* We set up the normal JPEG error routines, then override error_exit. */
    /*
       decinfo.err = jpeg_std_error(&jerr.pub);
       jerr.pub.error_exit = my_error_exit;
     */
    /* Establish the setjmp return context for my_error_exit to use. */
    /*
       if (setjmp(jerr.setjmp_buffer)) {
     */ 
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    /*

       jpeg_destroy_decompress(&decinfo);
       fclose(infile);
       return 0;
       }
     */
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&decinfo);

    /* Step 2: specify data source (eg, a file) */

    jpeg_stdio_src(&decinfo, infile);

    /* Step 3: read file parameters with jpeg_read_header() */

    (void) jpeg_read_header(&decinfo, TRUE);

    // (CMD) ReadDCT coefficients
    this->DctCoeff = jpeg_read_coefficients(&decinfo);

    unsigned int maxVSampleFactorW = 0;
    unsigned int maxVSampleFactorH = 0;

    for (unsigned int icomp = 0; icomp < decinfo.num_components; icomp++) {
        if (maxVSampleFactorW < decinfo.comp_info[icomp].v_samp_factor) {
            maxVSampleFactorW = decinfo.comp_info[icomp].v_samp_factor;
        }
        if (maxVSampleFactorH < decinfo.comp_info[icomp].h_samp_factor) {
            maxVSampleFactorH = decinfo.comp_info[icomp].h_samp_factor;
        }
    }

    this->HeightInBlocks = new unsigned int[decinfo.num_components] ;
	this->WidthInBlocks = new unsigned int[decinfo.num_components] ;

    for (unsigned short icomp = 0 ; icomp < decinfo.num_components ; icomp++) {
		HeightInBlocks[icomp] = this->DivRoundUp(decinfo.image_height * decinfo.comp_info[icomp].v_samp_factor, 8 * maxVSampleFactorW) ;
		WidthInBlocks[icomp] = this->DivRoundUp(decinfo.image_width * decinfo.comp_info[icomp].h_samp_factor,	8 * maxVSampleFactorH) ;
	}

    unsigned long totalNumCoeff = 0;
    for(unsigned int icomp = 0; icomp < decinfo.num_components; icomp++) {
        totalNumCoeff += JPEGFile::CoeffPerBlock * (HeightInBlocks[icomp] * WidthInBlocks[icomp]) ;
    }

    
    LinDctCoeffs.resize(totalNumCoeff);
    this->data_size = totalNumCoeff;
    //printf("this->data_size : %16llX\n", this->data_size);

    unsigned int linindex = 0;
    for (unsigned short icomp = 0 ; icomp < decinfo.num_components ; icomp++) {
		unsigned int currow = 0 ;
		while (currow < HeightInBlocks[icomp]) {
			unsigned int naccess = 1 ;
			JBLOCKARRAY array = (*(decinfo.mem->access_virt_barray))
				((j_common_ptr) &decinfo, DctCoeff[icomp], currow, naccess, FALSE) ;
			for (unsigned int irow = 0 ; irow < naccess ; irow++) {
				for (unsigned int iblock = 0 ; iblock < WidthInBlocks[icomp] ; iblock++) {
					for (unsigned int icoeff = 0 ; icoeff < CoeffPerBlock ; icoeff++) {
						LinDctCoeffs[linindex] = array[irow][iblock][icoeff] ;

						linindex++ ;
					}
				}
			}
			currow += naccess ;
		}
	}


    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.txt for more info.
     */

    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */

    /* Step 5: Start decompressor */

    //(void) jpeg_start_decompress(&decinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    /* JSAMPLEs per row in output buffer */

    //row_stride = decinfo.output_width * decinfo.output_components;

    /* Make a one-row-high sample array that will go away when done with image */

    //buffer = (*decinfo.mem->alloc_sarray)((j_common_ptr) &decinfo, JPOOL_IMAGE, row_stride, 1);


    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable decinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    this->image_width  = decinfo.output_width;
    this->image_height = decinfo.output_height;

    //this->data_size = this->image_width * this->image_height * 3;
    //printf("data_size : %16llX\n", data_size);

    //this->jdata = (unsigned char *)malloc(data_size);

/*
    if(!this->jdata) {
        return 0;
    }
    */

    //while (decinfo.output_scanline < decinfo.output_height) // loop
    //{
        // Enable jpeg_read_scanlines() to fill our jdata array
        //buffer[0] = (unsigned char *)jdata +  // secret to method
            //3* decinfo.output_width * decinfo.output_scanline; 

        //jpeg_read_scanlines(&decinfo, buffer, 1);
        /*
           for(unsigned long long i = 0; i < data_size; i++) {
           printf("%02X ", (unsigned char)jdata[i]);
           if (i % 16 == 15) {
           printf("\n");
           }
           }
         */
    //}

    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    /*
       while (decinfo.output_scanline < decinfo.output_height) {
       (void) jpeg_read_scanlines(&decinfo, buffer, 1);
     */
    /* Assume put_scanline_someplace wants a pointer and sample count. */
    /*
       printf("row_stride : %16X\n", row_stride);
       for (int i = 0; i < row_stride; i++) {
       printf("%02X ", buffer[i]);
       if (i % 16 == 15) {
       printf("\n");
       }
       }
    //put_scanline_someplace(buffer[0], row_stride);
    }

     */
    /* Step 7: Finish decompression */

    //(void) jpeg_finish_decompress(&decinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    //jpeg_destroy_decompress(&decinfo);

    /* After finish_decompress, we can close the input file.
     * Here we postpone it until after no more JPEG errors are possible,
     * so as to simplify the setjmp error logic above.  (Actually, I don't
     * think that jpeg_destroy can do an error exit, but why assume anything...)
     */
    fclose(infile);

    /* At this point you may want to check to see whether any corrupt-data
     * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
     */

    /* And we're done! */
    return 1;
}

void JPEGFile::WriteJPEGFile (int quality, int data_precision)
{
    /* This struct contains the JPEG compression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     * It is possible to have several such structures, representing multiple
     * compression/decompression processes, in existence at once.  We refer
     * to any one struct (and its associated working data) as a "JPEG object".
     */
    /* This struct represents a JPEG error handler.  It is declared separately
     * because applications often want to supply a specialized error handler
     * (see the second half of this file for an example).  But here we just
     * take the easy way out and use the standard error handler, which will
     * print a message on stderr and call exit() if compression fails.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct jpeg_error_mgr jerr;
    JSAMPARRAY image_buffer = NULL;
    /* Points to large array of R,G,B-order data */
    J12SAMPARRAY image_buffer12 = NULL;
    /* More stuff */
    FILE * outfile;		/* target file */
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
    J12SAMPROW row_pointer12[1];
    int row_stride;		/* physical row width in image buffer */

    /* Step 1: allocate and initialize JPEG compression object */

    /* We have to set up the error handler first, in case the initialization
     * step fails.  (Unlikely, but it could happen if you are out of memory.)
     * This routine fills in the contents of struct jerr, and returns jerr's
     * address which we place into the link field in cinfo.
     */
    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);
    jpeg_copy_critical_parameters(&decinfo, &cinfo);

    /* Step 2: specify data destination (eg, a file) */
    /* Note: steps 2 and 3 can be done in either order. */

    /* Here we use the library-supplied code to send compressed data to a
     * stdio stream.  You can also write your own code to do something else.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to write binary files.
     */
    if ((outfile = fopen(this->outputFile, "wb")) == NULL) {
        fprintf(stderr, "can't open %s\n", this->outputFile);
        exit(1);
    }
    jpeg_stdio_dest(&cinfo, outfile);
    jpeg_write_coefficients (&cinfo, this->DctCoeff);
    unsigned int linindex = 0 ;
    for (unsigned short icomp = 0 ; icomp < cinfo.num_components ; icomp++) {

        unsigned int currow = 0 ;
        while (currow < HeightInBlocks[icomp]) {
            unsigned int naccess = 1 ;
            JBLOCKARRAY array = (*(cinfo.mem->access_virt_barray))
                ((j_common_ptr) &cinfo, DctCoeff[icomp], currow, naccess, TRUE) ;
            for (unsigned int irow = 0 ; irow < naccess ; irow++) {
                for (unsigned int iblock = 0 ; iblock < WidthInBlocks[icomp] ; iblock++) {
                    for (unsigned int icoeff = 0 ; icoeff < CoeffPerBlock ; icoeff++) {
                        array[irow][iblock][icoeff] = LinDctCoeffs[linindex] ;
                        linindex++ ;
                    }
                }
            }
            currow += naccess ;
        }
    }


    jpeg_finish_compress (&cinfo) ;
    jpeg_destroy_compress(&cinfo);
    jpeg_finish_decompress (&decinfo) ;
    jpeg_destroy_decompress(&decinfo);
}

JPEGFile::~JPEGFile() {
    if(this->jdata != nullptr) {
        free(jdata);
        jdata = nullptr;
    }
    if (WidthInBlocks) {
		delete[] WidthInBlocks ;
	}
	if (HeightInBlocks) {
		delete[] HeightInBlocks ;
	}
    if (this->lzmaDecoder) {
        delete this->lzmaDecoder;
    }
}
