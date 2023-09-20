#include <stdio.h>
#include <string.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include "/opt/libjpeg-turbo/include/jpeglib.h"
#include "JPEGFile.h"
using namespace std;

int ReadJPEGFile (char * filename)
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
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

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }
  printf("Here 1\n");
  /* Step 1: allocate and initialize JPEG decompression object */
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  /* We set up the normal JPEG error routines, then override error_exit. */
  /*
  cinfo.err = jpeg_std_error(&jerr.pub);
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

    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
   */
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
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

  (void) jpeg_start_decompress(&cinfo);
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
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  printf("Here 2\n");
  unsigned long long x = cinfo.output_width;
  unsigned long long y = cinfo.output_height;

  unsigned long long data_size = x * y * 3;

  unsigned char* jdata = (unsigned char *)malloc(data_size);
  while (cinfo.output_scanline < cinfo.output_height) // loop
  {
    // Enable jpeg_read_scanlines() to fill our jdata array
    buffer[0] = (unsigned char *)jdata +  // secret to method
            3* cinfo.output_width * cinfo.output_scanline; 

    jpeg_read_scanlines(&cinfo, buffer, 1);
    printf("output_scanline : %16llX, output_height : %16llX\n", cinfo.output_scanline, cinfo.output_height);
    printf("data_size : %16llX\n", data_size);
    printf("buffer in while: \n");
    for(unsigned long long i = 0; i < 0x100; i++) {
        printf("%02X ", (unsigned char)jdata[i]);
        if(i % 16 == 0xF)
            printf("\n");
    }
    /*
    for(unsigned long long i = 0; i < data_size; i++) {
        printf("%02X ", (unsigned char)jdata[i]);
        if (i % 16 == 15) {
            printf("\n");
        }
    }
     */
  }
  printf("\n\n\nAAAA\n\n\n");
  for(unsigned long long i = 0; i < 0x100; i++) {
      printf("%02X ", (unsigned char)jdata[i]);
      if (i % 16 == 15) {
          printf("\n");
      }
  }

    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
  /*
  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
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

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

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

