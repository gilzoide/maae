/** @file img.h
 * .mos format definitions and operations
 */
 
#ifndef IMG_H
#define IMG_H

#include <ncurses.h>
#include <panel.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * "Image" in Nmos mosaic format
 */
typedef struct {
	int height;				///< img height
	int	width;				///< img width
	char *mosaic;			/**< a height * width sized string: the drawing itself */
	unsigned char *attr;	/**< a height * width sized array with the attributes for each char.
							 * It's bits are masked, from most to less significative as the color 
							 * (6 bits), bold (1 bit on/off) and underline (1 bit on/off).
							 */
} Image;

/**
 * Wraps the image, list and curses interface
 * 
 * The Image structured as a double linked list, for sequencial matters ["video",
 * as we like to call it] and curses interface.
 */
typedef struct mosimg_t {
	Image img;	///< the asc art itself
	struct mosimg_t *prev;	///< previous MOSIMG
	struct mosimg_t *next;	///< next MOSIMG
	WINDOW *win;	///< window to show the Image
	PANEL *pan;	///< panel for showing the window
} MOSIMG;


/**
 * Inline function that measures the total size of an image
 * 
 * @param[in] img Image to be sized
 * 
 * @return image size: height * width
 */
inline int ImgSize (MOSIMG *img);

/** 
 * Create a new @ref MOSIMG, allocating the necessary memory
 * 
 * @param[in] new_height New Image's height
 * @param[in] new_width New Image's width
 * 
 * @return New @ref MOSIMG, clean and unlinked
 * @return __NULL__ if allocation failed
 */
MOSIMG *NewImg (int new_height, int new_width);

/**
 * Saves the image in a file
 * 
 * 
 * The file has a header with the mosaic dimensions, the asc art itself and it's attributes.
 * 
 * @note A .mosi file is hybrid.
 * The dimensions and @ref Image::mosaic are text, so they can be viewed in any text editor. 
 * The @ref Image::attr part is binary, so expect some weird stuff while oppening it as text.
 * 
 * @param[in] image The image to be saved
 * @param[in] file_name The new file name
 * 
 * @return 0 on success, -1 on failure
 */
int SaveImg (MOSIMG *image, const char *file_name);

/// Destroy an image, deallocating the used memory
void FreeImg (MOSIMG *img);

#endif
