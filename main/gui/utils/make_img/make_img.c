/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2016
 *  
 *  This code is included on the disc that is included with the book
 *   GUI: The Graphic User Interface, and is for that purpose only.  You 
 *   have the right to use it for learning purposes only.  You may not
 *   modify it for redistribution for any other purpose unless you have
 *   written permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *
 * Last update:  4 July 2016
 *
 * usage:
 *   make_img files.txt
 * 
 * See the included files.txt file for an example of the resource file
 *
 * See Chapter 12 of the book for more information on how to use this utility.
 *
 * This utility will take a list of filenames and include those
 *   files in a single list of images for use with GUI_DEMO.EXE.
 *   See the code for more information on why we do this.
 *
 * Notes:
 *  - Please see the notes.txt file for information on the .BMP files
 *    and the format of the image.sys file.
 *
 *  - Remember, I didn't write this utility to be complete or robust.
 *    I wrote it to simply make an image file for use with this book.
 *    Please consider this if you add or modify to this utility.
 *
 *  - This routine does not allow you to specify the delay time for
 *    a multi-image image.  This is something you will have to add if
 *    you wish to do so.
 *
 *  Thank you for your purchase and interest in my work.
 *
 * compile using gcc
 *  gcc -Os make_img.c -o make_img.exe -s
 */

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "..\include\ctype.h"

#include "make_img.h"

#define COMP_TYPE   1   // 0 = no compression, 1 = RLE compression

int main(int argc, char *argv[]) {
  
  struct STATIC_IMAGES_HDR hdr;
  struct STATIC_IMAGE *image_hdr = NULL;
  int img_cnt = 0, entry_size_cnt = 0, entry_data_size = 0, image_id;
  int errors = 0;
  bit32u width, height;
  bit8u *data = NULL, *p;
  bit32u *pixel = NULL, *px;
  struct BMP_FILE_HDR bmp_file_hdr;
  
  FILE *src, *targ, *rsrc;
  struct S_RESOURCE *resources;
  char filename[NAME_LEN_MAX];
  
  // print start string
  fprintf(stderr, strtstr);
  
  // we need to parse the command line and get the parameters found
  parse_command(argc, argv, filename);
  
  // now retrieve the resource file's contents
  resources = parse_resource(filename);
  if (!resources) {
    printf("Error with Resource file. '%s'\n", filename);
    if (resources) free(resources);
    return -1;
  }
  
  // find the ID resource file and open it
  if ((rsrc = fopen(resources->id_filename, "r+")) == NULL) {
    printf("...Error opening ID resource file '%s'\n", resources->id_filename);
    return -1;
  }
  
  // clear out the header
  memset(&hdr, 0, sizeof(struct STATIC_IMAGES_HDR));
  
  // loop through the images
  int i;
  for (i=0; i<resources->file_cnt; i++) {
    if (resources->files[i].param1 > 0)
      printf(" Adding: %s", resources->files[i].filename);
    else
      printf("         %s", resources->files[i].filename);
     if ((src = fopen(resources->files[i].filename, "r+b")) == NULL) {
      printf("...Error opening file...\n");
      errors++;
      continue;
    }
    
    // (re)allocate the memory for the entries
    if (img_cnt >= entry_size_cnt) {
      entry_size_cnt += 32;
      image_hdr = (struct STATIC_IMAGE *) realloc(image_hdr, entry_size_cnt * sizeof(struct STATIC_IMAGE));
      if (image_hdr == NULL) {
        printf("Error allocating memory for entries...\n");
        fclose(rsrc);
        return -1;
      }
    }
    
    // found and opened source file.
    fread(&bmp_file_hdr, 1, sizeof(struct BMP_FILE_HDR), src);
    
    // check to make sure this is a .BMP file and is the
    //  format we are expecting.
    if ((bmp_file_hdr.bmp_hdr.bf_id != 0x4D42) ||
        (bmp_file_hdr.bmp_hdr.bf_resv0 || bmp_file_hdr.bmp_hdr.bf_resv1)) {
      printf("\n  ...Did not find valid BMP header at beginning of file.\n");
      errors++;
      fclose(src);
      continue;
    }
    
    if ((bmp_file_hdr.bmp_info.bi_size != 40) ||       // header size
        (bmp_file_hdr.bmp_info.bi_planes > 1) ||       // we only want 1 plane
        (bmp_file_hdr.bmp_info.bi_compression > 0) ||  // with no compression
        (bmp_file_hdr.bmp_info.bi_bitcount != 24) ||   // must be 24-bit image (RGB)
        (bmp_file_hdr.bmp_info.bi_sizeimage == 0)) {
      printf("\n  ...Did not find valid BMP Info header, or BMP doesn't use 24 bits per pixel.\n");
      printf("\n (%i %i %i %i %i)\n", bmp_file_hdr.bmp_info.bi_size, bmp_file_hdr.bmp_info.bi_planes,
        bmp_file_hdr.bmp_info.bi_compression, bmp_file_hdr.bmp_info.bi_bitcount, bmp_file_hdr.bmp_info.bi_sizeimage);
      errors++;
      fclose(src);
      continue;
    }
    
    // find the ID value from the ids.h file given
    image_id = resource_id(rsrc, resources->files[i].ID_name);
    
    // is this part of the previous image
    if (resources->files[i].param1 > 0) {
      // initialize data
      memset(&image_hdr[img_cnt], 0, sizeof(struct STATIC_IMAGE));  // clear it out first
      image_hdr[img_cnt].id = (bit32u) image_id;                    // ID dword
      width = image_hdr[img_cnt].width = bmp_file_hdr.bmp_info.bi_width;    // width of image in pixels
      height = image_hdr[img_cnt].height = bmp_file_hdr.bmp_info.bi_height; // height of image in pixels
      image_hdr[img_cnt].offset = entry_data_size;                          // we will patch this offset when we are nearly done below
      strncpy(image_hdr[img_cnt].name, resources->files[i].filename, STATIC_NAME_LEN - 1);
      image_hdr[img_cnt].count = (bit8u) resources->files[i].param1;
      
      // display size and the ID value we are using
      printf(" -- (%i x %i)", width, height);
      if (image_id > -1) {
        printf(" -- [%s = %i]\n", resources->files[i].ID_name, image_id);
      } else {
        printf("\n   *Did not find ID name: %s  Using default: %i\n", resources->files[i].ID_name, resources->files[i].param0);
        errors++;
        image_id = resources->files[i].param0;
      }
      img_cnt++;  // increment the count
    } else {
      if ((bmp_file_hdr.bmp_info.bi_width != width) ||
          (bmp_file_hdr.bmp_info.bi_height != height)) {
        printf("  Warning: Images sizes do not match. Using stored size...");
        errors++;
      }
      puts("");
    }
    
    // allocate memory to store this bmp file to
    data = (bit8u *) malloc(bmp_file_hdr.bmp_info.bi_sizeimage);
    if (data == NULL) {
      printf("...Failed to allocate the memory. Aborting...\n");
      fclose(src);
      fclose(rsrc);
      return -1;
    }
    
    // move to start of image (should be 54)
    fseek(src, bmp_file_hdr.bmp_hdr.bf_offbits, SEEK_SET);
    size_t read = fread(data, 1, bmp_file_hdr.bmp_info.bi_sizeimage, src);
    if (read != bmp_file_hdr.bmp_info.bi_sizeimage) {
      printf("...Failed to read the file. Aborting...\n");
      fclose(src);
      fclose(rsrc);
      exit(-1);
    }
    
    // (re)allocate the memory for the image data
    pixel = (bit32u *) realloc(pixel, entry_data_size + ((width * height) * sizeof(bit32u)));
    px = (bit32u *) ((bit8u *) pixel + entry_data_size);
    entry_data_size += ((width * height) * sizeof(bit32u));
    if (pixel == NULL) {
      printf("...Failed to allocate the memory. Aborting...\n");
      fclose(src);
      fclose(rsrc);
      return -1;
    }
    
    // now convert the image to our PIXEL format and write to the image file.
    // since the BMP is not stored from top left to bottom right, we have to
    //  read the memory as such, making it a bit slower to do.  However, you
    //  will only run this program once or twice, right?
    const int bytes_per_row = ((width * 3) + 3) & ~0x3;
    bit8u red, grn, blu;
    for (int y = height - 1; y > -1; y--) {
      for (int x = 0; x < width; x++) {
        p = data + ((y * bytes_per_row) + (x * 3));
        blu = p[0];
        grn = p[1];
        red = p[2];
        if ((red == 1) && (grn == 1) && (blu == 1))
          *px++ = 0xFF000000;
        else
          *px++ = GUITRGB(0, red, grn, blu);
      }
    }
    // free the data we used to hold this .bmp file
    free(data);
    
    // close the source file, add the size, and continue
    fclose(src);
  }
  
  // create target file
  if ((targ = fopen(resources->targ_filename, "w+b")) == NULL) {
    printf("Error creating target file: '%s'\n", resources->targ_filename);
    return -1;
  }
  printf("Creating resource file:\n '%s'\n", resources->targ_filename);
  
  // write the header
  hdr.magic = STATIC_IMAGES_MAGIC;
  hdr.total_size = sizeof(struct STATIC_IMAGES_HDR) + (img_cnt * sizeof(struct STATIC_IMAGE)) + entry_data_size;
  hdr.total_images = (bit32u) img_cnt;
  hdr.comp_type = COMP_TYPE;
  fwrite(&hdr, 1, sizeof(struct STATIC_IMAGES_HDR), targ);
  
  // patch the offsets in each entry
  for (i=0; i<img_cnt; i++)
    image_hdr[i].offset += (sizeof(struct STATIC_IMAGES_HDR) + (sizeof(struct STATIC_IMAGE) * img_cnt));
  
  // write entry headers
  fwrite(image_hdr, sizeof(struct STATIC_IMAGE), img_cnt, targ);
  
  // write pixel data
#if (COMP_TYPE == 0)      // no compression
  fwrite(pixel, 1, entry_data_size, targ);
#elif (COMP_TYPE == 1)  // RLE compression
  rle_compress(pixel, entry_data_size, targ);
  puts("Compressing image using RLE encoding...");
#else
  #error "COMP_TYPE must be 0 or 1 only"
#endif

  free(image_hdr);
  free(pixel);
  
  fclose(targ);
  fclose(rsrc);
  
  printf("\nStored %i total files, with %i errors...\n", img_cnt, errors);
  
  return 0;
}

/* compress a string of pixels using the RLE encoding, writing them to
 *  the file pointed to by fp
 *
 * pixel is a pointer to the pixel data
 *  size is the count in bytes
 *    fp is the file pointer to write the compressed data to
 *
 *  * we write the pixels in big-endian form for this to work *
 *  this relies upon the fact that our pixels will all have the
 *   high byte (the transparent byte) either 0x00 or 0xFF.
 *  if a 0x00 or 0xFF is found in the rle stream, it is a the
 *   actual pixel.  If anything else if found in the stream, it
 *   is a byte count and the next four bytes is the pixel value.
 *  this is an RLE of pixels, not bytes
 *
 */
void rle_compress(bit32u *pixel, int size, FILE *fp) {
  int c;
  bit32u ch;
  size /= sizeof(bit32u);  // we need the count of pixels not bytes
  
  while (size) {
    c = 1;
    ch = *pixel++;
    size--;
    while ((*pixel == ch) && size && (c < 0xFE))
      c++, pixel++, size--;
    if (c > 1)
      fputc(c, fp);
    ch = ENDIAN_32(ch);  // we need to write the pixels as big-endian so that the T byte is first
    fwrite(&ch, 1, sizeof(bit32u), fp);
  }
}

/* Parse command line.  We are looking for the following items
 *  filename   - This is the path/filename of the resource file to open
 */
void parse_command(int argc, char *argv[], char *filename) {
  
  int i;
  const char *s;
  
  strcpy(filename, "");
  
  for (i=1; i<argc; i++) {
    s = argv[i];
    if (*s == '/') {
      s++;
      if (strcmp(s, "?") == 0)
        printf(" Usage: make_img filename.txt\n");
      else
        printf(" Unknown switch parameter: /%s\n", s);
    } else
      strcpy(filename, s);
  }
}

/* This routine will search the ID resource file for the #define and
 *  if found, returns the value specified for the #define name.
 * It returns -1 if not found.
 *
 * It expects the file to be in the form of:
 *
 *    #define   NAME    value
 *
 * It may contain comments within the file, all three items must
 *  be on the same line, and no comments inbetween any of the three items.
 *
 * This is a simple parser.  Please make sure you do not have any
 *  exotic forms within your ids.h file.
 *
 */
int resource_id(FILE *src, const char *id_name) {
  char define[128];
  char name[128];
  int value;
  
  rewind(src);
  
  while (!feof(src)) {
    fscanf(src, "%s", define);
    if (!feof(src) && (strcmp(define, "#define") == 0)) {
      fscanf(src, "%s", name);
      if (!feof(src) && (strcmp(name, id_name) == 0))
        if (fscanf(src, "%i", &value) == 1)
          return value;
    }
  }
  
  return -1;
}

void get_a_string(FILE *fp, char *str) {
  char ch, *t = str;
  int c = NAME_LEN_MAX - 1;
  
  // skip all leading spaces
  while (!feof(fp)) {
    ch = fgetc(fp);
    if (ch == ' ')
      continue;
    else
      break;
  }
  
  while (1) {
    if (feof(fp))
      break;
    if (ch == 13)
      continue;
    if (strchr("\xA,=#", ch))
      break;
    *t++ = ch;
    if (--c == 0)
      break;
    ch = fgetc(fp);
  }
  
  // if it was the '#' char, skip to eol
  if (ch == '#') {
    while (!feof(fp)) {
      ch = fgetc(fp);
      if (ch == 10)
        break;
    }
  }
  
  // kill all trailing spaces
  while (t >= str) {
    t--;
    if (*t != ' ') {
      t++;
      break;
    }
  }
  
  // asciiz it
  *t = 0;
}

/*
 *  # line is a comment
 *  ID_name, filename_to_use, param0, param1
 *
 */
struct S_RESOURCE *parse_resource(const char *filename) {
  
  struct S_RESOURCE *r;
  int cnt = 0;
  int limit = 30;  // start with allowing 30 files
  char str[NAME_LEN_MAX];
  FILE *fp;
  
  if ((fp = fopen(filename, "r")) == NULL) {
    printf("\nError opening resource file.");
    return NULL;
  }
  
  r = (struct S_RESOURCE *) calloc(sizeof(struct S_RESOURCE) * limit, 1);
  while (!feof(fp)) {
    get_a_string(fp, str);
    if (strcmp(str, "imgfile") == 0) {
      get_a_string(fp, str);
      strcpy(r->targ_filename, str);
    } else if (strcmp(str, "idfile") == 0) {
      get_a_string(fp, str);
      strcpy(r->id_filename, str);
    } else if (strlen(str) > 0) {
      strcpy(r->files[cnt].ID_name, str);
      get_a_string(fp, r->files[cnt].filename);
      get_a_string(fp, str);
      r->files[cnt].param0 = strtoul(str, NULL, 0);
      get_a_string(fp, str);
      r->files[cnt].param1 = strtoul(str, NULL, 0);
      cnt++;
      if (cnt == limit) {
        limit += 20;
        r = (struct S_RESOURCE *) realloc(r, sizeof(struct S_RESOURCE) * limit);
      }
    }
  }
  
  // close the file
  fclose(fp);
  
  r->file_cnt = cnt;
  return r;
}
