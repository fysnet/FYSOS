/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * images.h
 *  
 */
#ifndef FYSOS_IMAGES
#define FYSOS_IMAGES

#pragma pack(1)



PIXEL *get_image(const char *, int *, int *, int *, int *, const bool);

bool get_bit(const bit8u *, const int);
int get_nibble(const bit8u *, const int);


#endif // FYSOS_IMAGES
