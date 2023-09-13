#include "JPEGFile.h"
using namespace std;

JPEGFile::JPEGFile(const char* filename, const char* outputFile) {

    this->filename = filename;
    this->outputFile = outputFile;
    this->jdata = nullptr;
    this->image_width = 0;
    this->image_height = 0;
    this->data_size = 0;
    this->messageLength = 0;
}

bool JPEGFile::HideMessage(const char* message) {

    string msg = string(message);
    size_t msgLength = msg.length();
    if (msgLength * 8 >= data_size) {
        return false;
    }

    size_t bitOffset = 0;
    
    for(size_t i = 0; i < msgLength; i++) {
        char c = msg[i];
        for(int bit = 0; bit < 8; bit++) {
            jdata[bitOffset++] = ((jdata[bitOffset] >> 1) << 0) | ((c >> (8 - bit - 1)) & 1);
        }
    }
    this->messageLength = bitOffset;

    return true;
}

int JPEGFile::ReadJPEGFile ()
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

    if ((infile = fopen(this->filename, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", this->filename);
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
    this->image_width  = cinfo.output_width;
    this->image_height = cinfo.output_height;

    this->data_size = this->image_width * this->image_height * 3;
    printf("data_size : %16llX\n", data_size);

    this->jdata = (unsigned char *)malloc(data_size);
    if(!this->jdata) {
        return 0;
    }

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

void JPEGFile::WriteJPEGFile (int quality)
{
  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  struct jpeg_compress_struct cinfo;
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
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

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = image_width; 	/* image width and height, in pixels */
  cinfo.image_height = image_height;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & jdata[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
  /* After finish_compress, we can close the output file. */
  fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  /* And we're done! */
}

JPEGFile::~JPEGFile() {
    if(this->jdata != nullptr) {
        free(jdata);
        jdata = nullptr;
    }
}
