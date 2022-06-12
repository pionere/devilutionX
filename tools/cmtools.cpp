#include "cmtools_c.h"

size_t GetFileSize(const char* filename)
{
    struct stat stat_buf;
    int rc = stat(filename, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

bool ReadPNG(const char *pngname, png_image_data &data)
{
	const int alphaByte = 255;
	FILE *fp;

	fp = fopen(pngname, "rb");
	if (fp == NULL)
		return false;
	/*int number = 0;
	png_const_bytep header;
	const int number = 8;

	if (fread(&header, 1, number, fp) != number) {
		return 0;
	}

	if (!png_sig_cmp(header, 0, number)) {
		return 0;
	}*/

	png_structp png_ptr = png_create_read_struct
		(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr) {
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fclose(fp);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_infop end_info;
		png_destroy_read_struct(&png_ptr, &info_ptr,
			&end_info);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);

	//png_set_sig_bytes(png_ptr, number);

	png_read_info(png_ptr, info_ptr);

	png_int_32 bit_depth, color_type, interlace_type, compression_type, filter_method;
	png_get_IHDR(png_ptr, info_ptr, &data.width, &data.height,
		&bit_depth, &color_type, &interlace_type,
		&compression_type, &filter_method);

	// tell libpng to strip 16 bit/color files down to 8 bits/color
	png_set_strip_16(png_ptr);

	/* Expand data to 24-bit RGB, or 8-bit grayscale, with alpha if available. */
	if (color_type & PNG_COLOR_MASK_PALETTE)
	{
		png_set_expand(png_ptr);
	}

	/* Expand the grayscale to 24-bit RGB if necessary. */
	if (!(color_type & PNG_COLOR_MASK_COLOR))
	{
		png_set_gray_to_rgb(png_ptr);
	}

	// expand paletted or RGB images with transparency to full alpha channels
	// so the data will be available as RGBA quartets
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
	}

	// if there is no alpha information, fill with alphaByte
	if (!(color_type & PNG_COLOR_MASK_ALPHA))
	{
		png_set_filler(png_ptr, alphaByte, PNG_FILLER_AFTER );
	}

	// expand pictures with less than 8bpp to 8bpp
	if (bit_depth < 8) {
		png_set_packing(png_ptr);
	}

	png_set_interlace_handling(png_ptr);

	// update structure with the above settings
	png_read_update_info(png_ptr, info_ptr);

	png_bytep buffer = (png_bytep)png_malloc(png_ptr, data.height * data.width * sizeof(RGBA));

	png_bytep *row_pointers = (png_bytep*)malloc(data.height * sizeof(void*));
	for (int i = 0; i < data.height; i++)
		row_pointers[i] = buffer + i * data.width * sizeof(RGBA);

	png_read_image(png_ptr, row_pointers);

	png_read_end(png_ptr, (png_infop)NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	fclose(fp);
	data.row_pointers = row_pointers;
	data.data_ptr = buffer;
	data.fixColorMask = NULL;
	return true;
}

bool WritePNG(const char *pngname, png_image_data &data)
{
	const int alphaByte = 255;
	const int bitDepth = 8;
	FILE *fp;

	fp = fopen(pngname, "wb");
	if (fp == NULL)
		return false;

	png_structp png_ptr = png_create_write_struct
		(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);

	if (png_ptr == NULL) {
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(fp);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);

	/* turn on or off filtering, and/or choose
       specific filters.  You can use either a single
       PNG_FILTER_VALUE_NAME or the bitwise OR of one
       or more PNG_FILTER_NAME masks.
     */
    /*png_set_filter(png_ptr, 0,
       PNG_FILTER_NONE  | PNG_FILTER_VALUE_NONE |
       PNG_FILTER_SUB   | PNG_FILTER_VALUE_SUB  |
       PNG_FILTER_UP    | PNG_FILTER_VALUE_UP   |
       PNG_FILTER_AVG   | PNG_FILTER_VALUE_AVG  |
       PNG_FILTER_PAETH | PNG_FILTER_VALUE_PAETH|
       PNG_ALL_FILTERS  | PNG_FAST_FILTERS);*/
	png_set_IHDR(png_ptr, info_ptr, data.width, data.height,
       bitDepth, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT); /* PNG_INTRAPIXEL_DIFFERENCING for MNG */

	// png_set_PLTE(png_ptr, info_ptr, palette, num_palette);

	/*png_write_info(png_ptr, info_ptr);

	png_write_image(png_ptr, data.row_pointers);

	png_write_end(png_ptr, info_ptr);*/
	png_set_rows(png_ptr, info_ptr, data.row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
	return true;
}

void CleanupImageData(png_image_data* imagedata, int numimages)
{
	for (int n = 0; n < numimages; n++) {
		free(imagedata[n].data_ptr);
		free(imagedata[n].row_pointers);
		free(imagedata[n].fixColorMask);
	}
	free(imagedata);
}

BYTE* LoadPal(const char* palFile)
{
	BYTE* result = (BYTE*)malloc(256 * 3);

	FILE* f = fopen(palFile, "rb");

	fread(result, 1, 256 * 3, f);

	fclose(f);

	return result;
}

BYTE GetPalColor(RGBA &data, BYTE *palette, int numcolors, int offset, int numfixcolors)
{
	int res = -1;
	//int best = abs(data.r - 0) + 
	//		   abs(data.g - 0) + 
	//		   abs(data.b - 0)
	int best = (data.r - 0) * (data.r - 0) + 
			   (data.g - 0) * (data.g - 0) + 
			   (data.b - 0) * (data.b - 0);

	palette += numfixcolors * 3;
	for (int i = numfixcolors; i < numcolors; i++, palette += 3) {
		//int dist = abs(data.r - palette[0]) + 
		//		   abs(data.g - palette[1]) + 
		//		   abs(data.b - palette[2]);
		int dist = (data.r - palette[0]) * (data.r - palette[0]) + 
				   (data.g - palette[1]) * (data.g - palette[1]) + 
				   (data.b - palette[2]) * (data.b - palette[2]);
		if (dist < best) {
			best = dist;
			res = i;
		}
	}
	if (res == -1)
		res = 0;
	else
		res += offset;

	return res;
}

RGBA GetPNGColor(BYTE col, BYTE *palette, int coloroffset, int numfixcolors)
{
	RGBA result;

	if (col < coloroffset) {
		result.r = 0;
		result.g = 0;
		result.b = 0;
		result.a = 255;
	} else {
		col -= coloroffset;
		result.r = palette[col * 3 + 0];
		result.g = palette[col * 3 + 1];
		result.b = palette[col * 3 + 2];
		result.a = (col != 0 && col < numfixcolors) ? col : 255;
	}
	return result;
}

static void BilinearInterpolateColors(RGBA* c0, RGBA* cR, int dx, RGBA* cD, int dy, RGBA* cDR, int len, RGBA& res)
{
	res.r = (c0->r * (len - dx) * (len - dy) + cR->r * dx * (len - dy) + cD->r * (len - dx) * dy + cDR->r * dx * dy) / (len * len);
	res.g = (c0->g * (len - dx) * (len - dy) + cR->g * dx * (len - dy) + cD->g * (len - dx) * dy + cDR->g * dx * dy) / (len * len);
	res.b = (c0->b * (len - dx) * (len - dy) + cR->b * dx * (len - dy) + cD->b * (len - dx) * dy + cDR->b * dx * dy) / (len * len);
}

static RGBA BilinearInterpolate(RGBA* c0, RGBA* cR, int dx, RGBA* cD, int dy, RGBA* cDR, int len/*, BYTE* palette, int numcolors, int coloroffset, int numfixcolors*/)
{
	RGBA res;
	res.a = 255;
	//res.r = 0;
	//res.g = 0;
	//res.b = 0;

	if (cR->a != 255) {
		if (cD->a != 255) {
			return *c0; // preserve if pixels on the right and down are transparent
		}
		if (cDR->a != 255) {
			// interpolate down
			res.r = (c0->r * (len - dy) + cD->r * dy) / len;
			res.g = (c0->g * (len - dy) + cD->g * dy) / len;
			res.b = (c0->b * (len - dy) + cD->b * dy) / len;
		} else {
			// interpolate down and down-right
			RGBA cR_;
			cR_.r = (c0->r + cDR->r) / 2;
			cR_.g = (c0->g + cDR->g) / 2;
			cR_.b = (c0->b + cDR->b) / 2;
			BilinearInterpolateColors(c0, &cR_, dx, cD, dy, cDR, len, res);
		}
	} else if (cD->a != 255) {
		if (cDR->a != 255) {
			// interpolate right
			res.r = (c0->r * (len - dx) + cR->r * dx) / len;
			res.g = (c0->g * (len - dx) + cR->g * dx) / len;
			res.b = (c0->b * (len - dx) + cR->b * dx) / len;
		} else {
			// interpolate right and down-right
			RGBA cD_;
			cD_.r = (c0->r + cDR->r) / 2;
			cD_.g = (c0->g + cDR->g) / 2;
			cD_.b = (c0->b + cDR->b) / 2;
			BilinearInterpolateColors(c0, cR, dx, &cD_, dy, cDR, len, res);
		}
	} else {
		if (cDR->a != 255) {
			// interpolate down and right
			RGBA cDR_;
			cDR_.r = (cR->r + cD->r) / 2;
			cDR_.g = (cR->g + cD->g) / 2;
			cDR_.b = (cR->b + cD->b) / 2;
			BilinearInterpolateColors(c0, cR, dx, cD, dy, &cDR_, len, res);
		} else {
			// full bilinear interpolation
			BilinearInterpolateColors(c0, cR, dx, cD, dy, cDR, len, res);
		}
	}

	/*if (numfixcolors != 0 && palette != NULL) {
		// do not interpolate 'protected' colors
		BYTE col = GetPalColor(res, palette, numcolors, coloroffset, 0);
		if (col != 0 && col < numfixcolors)
			return *c0;
	}*/
	return res;
}

typedef enum pattern_pixel {
	PTN_ALPHA,
	PTN_COLOR,
	PTN_DNC,
};

typedef struct UpscalePatterns {
	BYTE* pattern;
	bool (*fnc)(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata);
} UpscalePatterns;
/*
  c c c    cc cc cc    cc  cc  cc
  a c c X  cc cc cc -> cc  cc  cc
  a a c    aa cc cc    aa [cc] cc
           aa cc cc    aa [Ac] cc
		   aa aa cc    aa  aa  cc
		   aa aa cc    aa  aa  cc
 */
BYTE patternLineDownRight[] = {
	3, 3,
	PTN_COLOR, PTN_COLOR, PTN_COLOR,
	PTN_ALPHA, PTN_COLOR, PTN_COLOR,
	PTN_ALPHA, PTN_ALPHA, PTN_COLOR,
};
bool LineDownRight(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y + 1];
	RGBA* dst = (RGBA*)imagedata.row_pointers[(y + 1) * multiplier];
	BYTE* fm = &imagedata.fixColorMask[(y + 1) * multiplier * imagedata.width];

	src += x + 1;
	dst += (x + 1) * multiplier;
	fm += (x + 1) * multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < multiplier; xx++, dst++, fm++) {
			if (yy > xx) {
				*dst = { 0 };
				*fm = 0;

			}
		}
		dst += imagedata.width - multiplier;
		fm += imagedata.width - multiplier;
	}

	return true;
}

/*
  c c c    cc cc cc    cc  cc  cc
  c c a X  cc cc cc -> cc  cc  cc
  c a a    cc cc aa    cc [cc] aa
           cc cc aa    cc [cA] aa
		   cc aa aa    cc  aa  aa
		   cc aa aa    cc  aa  aa
 */
BYTE patternLineDownLeft[] = {
	3, 3,
	PTN_COLOR, PTN_COLOR, PTN_COLOR,
	PTN_COLOR, PTN_COLOR, PTN_ALPHA,
	PTN_COLOR, PTN_ALPHA, PTN_ALPHA,
};
bool LineDownLeft(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y + 1];
	RGBA* dst = (RGBA*)imagedata.row_pointers[(y + 1) * multiplier];
	BYTE* fm = &imagedata.fixColorMask[(y + 1) * multiplier * imagedata.width];

	src += x + 1;
	dst += (x + 1) * multiplier;
	fm += (x + 1) * multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < multiplier; xx++, dst++, fm++) {
			if (yy >= multiplier - xx) {
				*dst = { 0 };
				*fm = 0;
			}
		}
		dst += imagedata.width - multiplier;
		fm += imagedata.width - multiplier;
	}

	return true;
}

/*
  a a c    aa aa cc     aa [aa] cc
  a c c X  aa aa cc ->  aa [aC] cc
  c c c    aa cc cc    [aa] cc  cc
           aa cc cc    [aC] cc  cc
		   cc cc cc     cc  cc  cc
		   cc cc cc     cc  cc  cc
 */
BYTE patternLineUpRight[] = {
	3, 3,
	PTN_ALPHA, PTN_ALPHA, PTN_COLOR,
	PTN_ALPHA, PTN_COLOR, PTN_COLOR,
	PTN_COLOR, PTN_COLOR, PTN_COLOR,
};
bool LineUpRight(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x + 1;
	dst += (x + 1) * multiplier;
	fm += (x + 1) * multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < multiplier; xx++, dst++, fm++) {
			if (yy >= multiplier - xx) {
				dst->a = 255;
				if (*(fm + imagedata.width * multiplier) != 0) {
					if (*(fm + multiplier) != 0) {
						// both colors are fixed -> use one and fix the color
						*dst = *(src + 1);
						*fm = *(fm + multiplier);
					} else {
						// only bottom is fixed -> use the right
						*dst = *(src + 1);
					}
				} else {
					if (*(fm + multiplier) != 0) {
						// only right is fixed -> use the bottom
						*dst = *(src + orimg_data.width);
					} else {
						// neither color is fixed -> interpolate
						dst->r = ((src + 1)->r + (src + orimg_data.width)->r) / 2;
						dst->g = ((src + 1)->g + (src + orimg_data.width)->g) / 2;
						dst->b = ((src + 1)->b + (src + orimg_data.width)->b) / 2;
					}
				}
			}
		}
		dst += imagedata.width - multiplier;
		fm += imagedata.width - multiplier;
	}

	dst -= multiplier;
	fm -= multiplier;
	src += orimg_data.width - 1;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < multiplier; xx++, dst++, fm++) {
			if (yy >= multiplier - xx) {
				dst->a = 255;
				if (*(fm + imagedata.width * multiplier) != 0) {
					if (*(fm + multiplier) != 0) {
						// both colors are fixed -> use one and fix the color
						*dst = *(src + 1);
						*fm = *(fm + multiplier);
					} else {
						// only bottom is fixed -> use the right
						*dst = *(src + 1);
					}
				} else {
					if (*(fm + multiplier) != 0) {
						// only right is fixed -> use the bottom
						*dst = *(src + orimg_data.width);
					} else {
						// neither color is fixed -> interpolate
						dst->r = ((src + 1)->r + (src + orimg_data.width)->r) / 2;
						dst->g = ((src + 1)->g + (src + orimg_data.width)->g) / 2;
						dst->b = ((src + 1)->b + (src + orimg_data.width)->b) / 2;
					}
				}
			}
		}
		dst += imagedata.width - multiplier;
		fm += imagedata.width - multiplier;
	}
	return true;
}

/*
  c a a    cc aa aa    cc [aa] aa
  c c a X  cc aa aa -> cc [Ca] aa
  c c c    cc cc aa    cc  cc [aa]
           cc cc aa    cc  cc [Ca]
		   cc cc cc    cc  cc  cc
		   cc cc cc    cc  cc  cc
 */
BYTE patternLineUpLeft[] = {
	3, 3,
	PTN_COLOR, PTN_ALPHA, PTN_ALPHA,
	PTN_COLOR, PTN_COLOR, PTN_ALPHA,
	PTN_COLOR, PTN_COLOR, PTN_COLOR,
};
bool LineUpLeft(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x + 1;
	dst += (x + 1) * multiplier;
	fm += (x + 1) * multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < multiplier; xx++, dst++, fm++) {
			if (yy > xx) {
				dst->a = 255;
				if (*(fm + imagedata.width * multiplier) != 0) {
					if (*(fm - multiplier) != 0) {
						// both colors are fixed -> use one and fix the color
						*dst = *(src - 1);
						*fm = *(fm - multiplier);
					} else {
						// only bottom is fixed -> use the left
						*dst = *(src - 1);
					}
				} else {
					if (*(fm - multiplier) != 0) {
						// only left is fixed -> use the bottom
						*dst = *(src + orimg_data.width);
					} else {
						// neither color is fixed -> interpolate
						dst->r = ((src - 1)->r + (src + orimg_data.width)->r) / 2;
						dst->g = ((src - 1)->g + (src + orimg_data.width)->g) / 2;
						dst->b = ((src - 1)->b + (src + orimg_data.width)->b) / 2;
					}
				}
			}
		}
		dst += imagedata.width - multiplier;
		fm += imagedata.width - multiplier;
	}

	dst += multiplier;
	fm += multiplier;
	src += orimg_data.width + 1;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < multiplier; xx++, dst++, fm++) {
			if (yy > xx) {
				dst->a = 255;
				if (*(fm + imagedata.width * multiplier) != 0) {
					if (*(fm - multiplier) != 0) {
						// both colors are fixed -> use one and fix the color
						*dst = *(src - 1);
						*fm = *(fm - multiplier);
					} else {
						// only bottom is fixed -> use the left
						*dst = *(src - 1);
					}
				} else {
					if (*(fm - multiplier) != 0) {
						// only left is fixed -> use the bottom
						*dst = *(src + orimg_data.width);
					} else {
						// neither color is fixed -> interpolate
						dst->r = ((src - 1)->r + (src + orimg_data.width)->r) / 2;
						dst->g = ((src - 1)->g + (src + orimg_data.width)->g) / 2;
						dst->b = ((src - 1)->b + (src + orimg_data.width)->b) / 2;
					}
				}
			}
		}
		dst += imagedata.width - multiplier;
		fm += imagedata.width - multiplier;
	}

	return true;
}

/*
  c c c c    cc cc cc cc    cc  cc cc  cc
  a c c c X  cc cc cc cc -> cc  cc cc  cc
  a a a c    aa cc cc cc    aa [cc cc] cc
             aa cc cc cc    aa [AA cc] cc
		     aa aa aa cc    aa  aa aa  cc
		     aa aa aa cc    aa  aa aa  cc
 */
BYTE patternSlowDownRight[] = {
	4, 3,
	PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR,
	PTN_ALPHA, PTN_COLOR, PTN_COLOR, PTN_COLOR,
	PTN_ALPHA, PTN_ALPHA, PTN_ALPHA, PTN_COLOR,
};
bool SlowDownRight(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x;
	dst += x * multiplier;
	fm += x * multiplier;

	// move to [1; 1]
	src += orimg_data.width + 1;
	dst += (imagedata.width + 1) * multiplier;
	fm += (imagedata.width + 1) * multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy > xx / 2) {
				*dst = { 0 };
				*fm = 0;
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}
	return true;
}

/*
  c c c c    cc cc cc cc    cc  cc cc  cc
  c c c a X  cc cc cc cc -> cc  cc cc  cc
  c a a a    cc cc cc aa    cc [cc cc] cc
             cc cc cc aa    cc [cc AA] aa
		     cc aa aa aa    cc  aa aa  aa
		     cc aa aa aa    cc  aa aa  aa
 */
BYTE patternSlowDownLeft[] = {
	4, 3,
	PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR,
	PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_ALPHA,
	PTN_COLOR, PTN_ALPHA, PTN_ALPHA, PTN_ALPHA,
};
bool SlowDownLeft(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x;
	dst += x * multiplier;
	fm += x * multiplier;

	// move to [1; 1]
	src += orimg_data.width + 1;
	dst += (imagedata.width + 1) * multiplier;
	fm += (imagedata.width + 1) * multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy >= multiplier - xx / 2) {
				*dst = { 0 };
				*fm = 0;
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}

	return true;
}

/*
  a a a c    aa aa aa cc    aa [aa aa] cc
  a c c c X  aa aa aa cc -> aa [aa CC] cc
  c c c c    aa cc cc cc    aa  cc cc  cc
             aa cc cc cc    aa  cc cc  cc
		     cc cc cc cc    cc  cc cc  cc
		     cc cc cc cc    cc  cc cc  cc
 */
BYTE patternSlowUpRight[] = {
	4, 3,
	PTN_ALPHA, PTN_ALPHA, PTN_ALPHA, PTN_COLOR,
	PTN_ALPHA, PTN_COLOR, PTN_COLOR, PTN_COLOR,
	PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR,
};
bool SlowUpRight(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x;
	dst += x * multiplier;
	fm += x * multiplier;

	// move to [1; 0]
	src += 1;
	dst += multiplier;
	fm += multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy >= multiplier - xx / 2) {
				dst->a = 255;
				if (*(fm + imagedata.width * multiplier) != 0) {
					if (*(fm + (xx < multiplier ? 2 : 1) * multiplier) != 0) {
						// both colors are fixed -> use one and fix the color
						*dst = *(src + 2);
						*fm = *(fm + (xx < multiplier ? 2 : 1) * multiplier);
					} else {
						// only bottom is fixed -> use the right
						*dst = *(src + 2);
					}
				} else {
					if (*(fm + (xx < multiplier ? 2 : 1) * multiplier) != 0) {
						// only right is fixed -> use the bottom
						*dst = *(src + orimg_data.width);
					} else {
						// neither color is fixed -> interpolate
						dst->r = ((src + 2)->r + (src + orimg_data.width)->r) / 2;
						dst->g = ((src + 2)->g + (src + orimg_data.width)->g) / 2;
						dst->b = ((src + 2)->b + (src + orimg_data.width)->b) / 2;
					}
				}
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}
	return true;
}

/*
  c a a a    cc aa aa aa    cc [aa aa] aa
  c c c a X  cc aa aa aa -> cc [CC aa] aa
  c c c c    cc cc cc aa    cc  cc cc  aa
             cc cc cc aa    cc  cc cc  aa
		     cc cc cc cc    cc  cc cc  cc
		     cc cc cc cc    cc  cc cc  cc
 */
BYTE patternSlowUpLeft[] = {
	4, 3,
	PTN_COLOR, PTN_ALPHA, PTN_ALPHA, PTN_ALPHA,
	PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_ALPHA,
	PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR,
};
bool SlowUpLeft(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x;
	dst += x * multiplier;
	fm += x * multiplier;

	// move to [1; 0]
	src += 1;
	dst += multiplier;
	fm += multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy > xx / 2) {
				dst->a = 255;
				if (*(fm + imagedata.width * multiplier) != 0) {
					if (*(fm - (xx < multiplier ? 1 : 2) * multiplier) != 0) {
						// both colors are fixed -> use one and fix the color
						*dst = *(src - 1);
						*fm = *(fm - (xx < multiplier ? 1 : 2) * multiplier);
					} else {
						// only bottom is fixed -> use the left
						*dst = *(src - 1);
					}
				} else {
					if (*(fm - (xx < multiplier ? 1 : 2) * multiplier) != 0) {
						// only left is fixed -> use the bottom
						*dst = *(src + orimg_data.width);
					} else {
						// neither color is fixed -> interpolate
						dst->r = ((src - 1)->r + (src + orimg_data.width)->r) / 2;
						dst->g = ((src - 1)->g + (src + orimg_data.width)->g) / 2;
						dst->b = ((src - 1)->b + (src + orimg_data.width)->b) / 2;
					}
				}
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}

	return true;
}

/*
  c c c    cc cc cc    cc  cc  cc
  a c c X  cc cc cc -> cc  cc  cc
  a c c    aa cc cc    aa [cc] cc
  a a c    aa cc cc    aa [cc] cc
           aa cc cc    aa [Ac] cc
           aa cc cc    aa [Ac] cc
		   aa aa cc    aa  aa  aa
		   aa aa cc    aa  aa  aa
 */
BYTE patternFastDownRight[] = {
	3, 4,
	PTN_COLOR, PTN_COLOR, PTN_COLOR,
	PTN_ALPHA, PTN_COLOR, PTN_COLOR,
	PTN_ALPHA, PTN_COLOR, PTN_COLOR,
	PTN_ALPHA, PTN_ALPHA, PTN_COLOR,
};
bool FastDownRight(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x;
	dst += x * multiplier;
	fm += x * multiplier;

	// move to [1; 1]
	src += orimg_data.width + 1;
	dst += (imagedata.width + 1) * multiplier;
	fm += (imagedata.width + 1) * multiplier;

	for (int yy = 0; yy < 2 * multiplier; yy++) {
		for (int xx = 0; xx < multiplier; xx++, dst++, fm++) {
			if (yy > xx * 2) {
				*dst = { 0 };
				*fm = 0;
			}
		}
		dst += imagedata.width - multiplier;
		fm += imagedata.width - multiplier;
	}

	return true;
}

/*
  c c c    cc cc cc    cc  cc  cc
  c c a X  cc cc cc -> cc  cc  cc
  c c a    cc cc aa    cc [cc] aa
  c a a    cc cc aa    cc [cc] aa
           cc cc aa    cc [cA] aa
           cc cc aa    cc [cA] aa
		   cc aa aa    cc  aa  aa
		   cc aa aa    cc  aa  aa
 */
BYTE patternFastDownLeft[] = {
	3, 4,
	PTN_COLOR, PTN_COLOR, PTN_COLOR,
	PTN_COLOR, PTN_COLOR, PTN_ALPHA,
	PTN_COLOR, PTN_COLOR, PTN_ALPHA,
	PTN_COLOR, PTN_ALPHA, PTN_ALPHA,
};
bool FastDownLeft(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x;
	dst += x * multiplier;
	fm += x * multiplier;

	// move to [1; 1]
	src += orimg_data.width + 1;
	dst += (imagedata.width + 1) * multiplier;
	fm += (imagedata.width + 1) * multiplier;

	for (int yy = 0; yy < 2 * multiplier; yy++) {
		for (int xx = 0; xx < multiplier; xx++, dst++, fm++) {
			if (yy >= 2 * multiplier - 2 * xx) {
				*dst = { 0 };
				*fm = 0;
			}
		}
		dst += imagedata.width - multiplier;
		fm += imagedata.width - multiplier;
	}

	return true;
}

/*
  a a c    aa aa cc     aa  aa cc
  a c c X  aa aa cc ->  aa  aa cc
  a c c    aa cc cc    [aa] cc cc
  c c c    aa cc cc    [aa] cc cc
           aa cc cc    [aC] cc cc
           aa cc cc    [aC] cc cc
		   cc cc cc     cc  cc cc
		   cc cc cc     cc  cc cc
 */
BYTE patternFastUpRight[] = {
	3, 4,
	PTN_ALPHA, PTN_ALPHA, PTN_COLOR,
	PTN_ALPHA, PTN_COLOR, PTN_COLOR,
	PTN_ALPHA, PTN_COLOR, PTN_COLOR,
	PTN_COLOR, PTN_COLOR, PTN_COLOR,
};
bool FastUpRight(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x;
	dst += x * multiplier;
	fm += x * multiplier;

	// move to [0; 1]
	src += orimg_data.width;
	dst += imagedata.width * multiplier;
	fm += imagedata.width * multiplier;

	for (int yy = 0; yy < 2 * multiplier; yy++) {
		for (int xx = 0; xx < multiplier; xx++, dst++, fm++) {
			if (yy >= 2 * multiplier - 2 * xx) {
				dst->a = 255;
				if (*(fm + imagedata.width * multiplier * (yy < multiplier ? 2 : 1)) != 0) {
					if (*(fm + multiplier) != 0) {
						// both colors are fixed -> use one and fix the color
						*dst = *(src + 1);
						*fm = *(fm + multiplier);
					} else {
						// only bottom is fixed -> use the right
						*dst = *(src + 1);
					}
				} else {
					if (*(fm + multiplier) != 0) {
						// only right is fixed -> use the bottom
						*dst = *(src + orimg_data.width * 2);
					} else {
						// neither color is fixed -> interpolate
						dst->r = ((src + 1)->r + (src + orimg_data.width * 2)->r) / 2;
						dst->g = ((src + 1)->g + (src + orimg_data.width * 2)->g) / 2;
						dst->b = ((src + 1)->b + (src + orimg_data.width * 2)->b) / 2;
					}
				}
			}
		}
		dst += imagedata.width - multiplier;
		fm += imagedata.width - multiplier;
	}

	return true;
}

/*
  c a a    cc aa aa    cc aa  aa
  c c a X  cc aa aa -> cc aa  aa
  c c a    cc cc aa    cc cc [aa]
  c c c    cc cc aa    cc cc [aa]
           cc cc aa    cc cc [Ca]
           cc cc aa    cc cc [Ca]
		   cc cc cc    cc cc  cc
		   cc cc cc    cc cc  cc
 */
BYTE patternFastUpLeft[] = {
	3, 4,
	PTN_COLOR, PTN_ALPHA, PTN_ALPHA,
	PTN_COLOR, PTN_COLOR, PTN_ALPHA,
	PTN_COLOR, PTN_COLOR, PTN_ALPHA,
	PTN_COLOR, PTN_COLOR, PTN_COLOR,
};
bool FastUpLeft(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x;
	dst += x * multiplier;
	fm += x * multiplier;

	// move to [2; 1]
	src += orimg_data.width + 2;
	dst += (imagedata.width + 2) * multiplier;
	fm += (imagedata.width + 2) * multiplier;

	for (int yy = 0; yy < 2 * multiplier; yy++) {
		for (int xx = 0; xx < multiplier; xx++, dst++, fm++) {
			if (yy > 2 * xx) {
				dst->a = 255;
				if (*(fm + imagedata.width * multiplier * (yy < multiplier ? 2 : 1)) != 0) {
					if (*(fm - multiplier) != 0) {
						// both colors are fixed -> use one and fix the color
						*dst = *(src - 1);
						*fm = *(fm - multiplier);
					} else {
						// only bottom is fixed -> use the left
						*dst = *(src - 1);
					}
				} else {
					if (*(fm - multiplier) != 0) {
						// only left is fixed -> use the bottom
						*dst = *(src + orimg_data.width * 2);
					} else {
						// neither color is fixed -> interpolate
						dst->r = ((src - 1)->r + (src + orimg_data.width * 2)->r) / 2;
						dst->g = ((src - 1)->g + (src + orimg_data.width * 2)->g) / 2;
						dst->b = ((src - 1)->b + (src + orimg_data.width * 2)->b) / 2;
					}
				}
			}
		}
		dst += imagedata.width - multiplier;
		fm += imagedata.width - multiplier;
	}

	return true;
}

/*
  a a a a    aa aa aa aa     aa  aa  aa aa
  a a c c    aa aa aa aa     aa  aa  aa aa
  c c c c X  aa aa cc cc -> [aa  aa] cc cc
  a a c c    aa aa cc cc    [aa  CC] cc cc
  a a a a    cc cc cc cc    [cc  cc] cc cc
		     cc cc cc cc    [AA  cc] cc cc
		     aa aa cc cc     aa  aa  cc cc
		     aa aa cc cc     aa  aa  cc cc
		     aa aa aa aa     aa  aa  aa aa
		     aa aa aa aa     aa  aa  aa aa
 */
BYTE patternLeftTriangle[] = {
	4, 5,
	PTN_ALPHA, PTN_ALPHA, PTN_ALPHA, PTN_ALPHA,
	PTN_ALPHA, PTN_ALPHA, PTN_COLOR, PTN_COLOR,
	PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR,
	PTN_ALPHA, PTN_ALPHA, PTN_COLOR, PTN_COLOR,
	PTN_ALPHA, PTN_ALPHA, PTN_ALPHA, PTN_ALPHA,
};
bool LeftTriangle(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x;
	dst += x * multiplier;
	fm += x * multiplier;

	// not on the left border and [-1; 2] is not alpha
	if (x != 0 && (src + orimg_data.width * 2 - 1)->a == 255) {
		return false;
	}

	// move to [0; 1]
	src += orimg_data.width;
	dst += imagedata.width * multiplier;
	fm += imagedata.width * multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy >= multiplier - xx / 2) {
				dst->a = 255;
				if (*(fm + imagedata.width * multiplier) != 0) {
					if (*(fm + (xx < multiplier ? 2 : 1) * multiplier) != 0) {
						// both colors are fixed -> use one and fix the color
						*dst = *(src + 2);
						*fm = *(fm + (xx < multiplier ? 2 : 1) * multiplier);
					} else {
						// only bottom is fixed -> use the right
						*dst = *(src + 2);
					}
				} else {
					if (*(fm + (xx < multiplier ? 2 : 1) * multiplier) != 0) {
						// only right is fixed -> use the bottom
						*dst = *(src + orimg_data.width);
					} else {
						// neither color is fixed -> interpolate
						dst->r = ((src + 2)->r + (src + orimg_data.width)->r) / 2;
						dst->g = ((src + 2)->g + (src + orimg_data.width)->g) / 2;
						dst->b = ((src + 2)->b + (src + orimg_data.width)->b) / 2;
					}
				}
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}

	src += orimg_data.width;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy > xx / 2) {
				*dst = { 0 };
				*fm = 0;
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}

	return true;
}

/*
  a a a a    aa aa aa aa    aa aa  aa aa
  c c a a    aa aa aa aa    aa aa  aa aa
  c c c c X  cc cc aa aa -> cc cc [aa aa]
  c c a a    cc cc aa aa    cc cc [CC aa]
  a a a a    cc cc cc cc    cc cc [cc cc]
		     cc cc cc cc    cc cc [cc AA]
		     cc cc aa aa    cc cc  aa aa
		     cc cc aa aa    cc cc  aa aa
		     aa aa aa aa    aa aa  aa aa
		     aa aa aa aa    aa aa  aa aa
 */
BYTE patternRightTriangle[] = {
	4, 5,
	PTN_ALPHA, PTN_ALPHA, PTN_ALPHA, PTN_ALPHA,
	PTN_COLOR, PTN_COLOR, PTN_ALPHA, PTN_ALPHA,
	PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR,
	PTN_COLOR, PTN_COLOR, PTN_ALPHA, PTN_ALPHA,
	PTN_ALPHA, PTN_ALPHA, PTN_ALPHA, PTN_ALPHA,
};
bool RightTriangle(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x;
	dst += x * multiplier;
	fm += x * multiplier;

	// not on the right border and [4; 2] is not alpha
	if (x + 4 != orimg_data.width && (src + orimg_data.width * 2 + 4)->a == 255) {
		return false;
	}

	// move to [2; 1]
	src += orimg_data.width + 2;
	dst += (imagedata.width + 2) * multiplier;
	fm += (imagedata.width + 2) * multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy > xx / 2) {
				dst->a = 255;
				if (*(fm + imagedata.width * multiplier) != 0) {
					if (*(fm - (xx < multiplier ? 1 : 2) * multiplier) != 0) {
						// both colors are fixed -> use one and fix the color
						*dst = *(src - 1);
						*fm = *(fm - (xx < multiplier ? 1 : 2) * multiplier);
					} else {
						// only bottom is fixed -> use the left
						*dst = *(src - 1);
					}
				} else {
					if (*(fm - (xx < multiplier ? 1 : 2) * multiplier) != 0) {
						// only left is fixed -> use the bottom
						*dst = *(src + orimg_data.width);
					} else {
						// neither color is fixed -> interpolate
						dst->r = ((src - 1)->r + (src + orimg_data.width)->r) / 2;
						dst->g = ((src - 1)->g + (src + orimg_data.width)->g) / 2;
						dst->b = ((src - 1)->b + (src + orimg_data.width)->b) / 2;
					}
				}
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}

	// dst += 2 * multiplier;
	src += orimg_data.width;// + 2;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy >= multiplier - xx / 2) {
				*dst = { 0 };
				*fm = 0;
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}

	return true;
}

/*
  a a a a a a a a    aa aa aa aa aa aa aa aa    aa aa [aa aa aa aa] aa aa
  a a c c c c a a X  aa aa aa aa aa aa aa aa -> aa aa [aa CC CC aa] aa aa
  c c c c c c c c    aa aa cc cc cc cc aa aa    aa aa  cc cc cc cc  aa aa
                     aa aa cc cc cc cc aa aa    aa aa  cc cc cc cc  aa aa
                     cc cc cc cc cc cc cc cc    cc cc  cc cc cc cc  cc cc
                     cc cc cc cc cc cc cc cc    cc cc  cc cc cc cc  cc cc
 */
BYTE patternTopTriangle[] = {
	8, 3,
	PTN_ALPHA, PTN_ALPHA, PTN_ALPHA, PTN_ALPHA, PTN_ALPHA, PTN_ALPHA, PTN_ALPHA, PTN_ALPHA,
	PTN_ALPHA, PTN_ALPHA, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_ALPHA, PTN_ALPHA,
	PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR,
};
bool TopTriangle(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	// move to [2; 0]
	src += x + 2;
	dst += (x + 2) * multiplier;
	fm += (x + 2) * multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy >= multiplier - xx / 2) {
				*dst = *(src + orimg_data.width);
				*fm = *(fm + imagedata.width * multiplier);
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}

	dst += 2 * multiplier - imagedata.width * multiplier;
	fm += 2 * multiplier - imagedata.width * multiplier;
	src += 2;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy > xx / 2) {
				*dst = *(src + orimg_data.width);
				*fm = *(fm + imagedata.width * multiplier);
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}

	return true;
}

/*
  c c c c c c c c    cc cc cc cc cc cc cc cc    cc cc  cc cc cc cc  cc cc
  a a c c c c a a X  cc cc cc cc cc cc cc cc -> cc cc  cc cc cc cc  cc cc
                     aa aa cc cc cc cc aa aa    aa aa [cc cc cc cc] aa aa
                     aa aa cc cc cc cc aa aa    aa aa [AA cc cc AA] aa aa
 */
BYTE patternBottomTriangle[] = {
	8, 2,
	PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR,
	PTN_ALPHA, PTN_ALPHA, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_COLOR, PTN_ALPHA, PTN_ALPHA,
};
bool BottomTriangle(int x, int y, int multiplier, png_image_data &orimg_data, png_image_data &imagedata)
{
	RGBA* src = (RGBA*)orimg_data.row_pointers[y];
	RGBA* dst = (RGBA*)imagedata.row_pointers[y * multiplier];
	BYTE* fm = &imagedata.fixColorMask[y * multiplier * imagedata.width];

	src += x;
	dst += x * multiplier;
	fm += x * multiplier;

	// not at the bottom border
	if (y != orimg_data.height - 2) {
		// [1; 2] .. [6; 2] is not alpha
		for (int i = 1; i < 7; i++) {
			if ((src + orimg_data.width * 2 + i)->a == 255)
				return false;
		}
	}

	// move to [2; 1]
	src += orimg_data.width + 2;
	dst += (imagedata.width + 2) * multiplier;
	fm += (imagedata.width + 2) * multiplier;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy > xx / 2) {
				*dst = { 0 };
				*fm = 0;
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}

	dst += 2 * multiplier - imagedata.width * multiplier;
	fm += 2 * multiplier - imagedata.width * multiplier;
	src += 2;

	for (int yy = 0; yy < multiplier; yy++) {
		for (int xx = 0; xx < 2 * multiplier; xx++, dst++, fm++) {
			if (yy >= multiplier - xx / 2) {
				*dst = { 0 };
				*fm = 0;
			}
		}
		dst += imagedata.width - 2 * multiplier;
		fm += imagedata.width - 2 * multiplier;
	}

	return true;
}

void UpscalePNGImages(png_image_data* imagedata, int numimage, int multiplier, BYTE* palette, int numcolors, int coloroffset, int numfixcolors, int antiAliasingMode)
{
	png_image_data* orimg_data = (png_image_data*)malloc(sizeof(png_image_data) * numimage);

	// upscale the pngs
	for (int i = 0; i < numimage; i++) {
		// prepare orimg_data
		orimg_data[i] = imagedata[i];

		imagedata[i].width *= multiplier;
		imagedata[i].height *= multiplier;
		RGBA *imagerows = (RGBA *)malloc(sizeof(RGBA) * imagedata[i].height * imagedata[i].width);
		imagedata[i].data_ptr = (png_bytep)imagerows;
		imagedata[i].row_pointers = (png_bytep*)malloc(imagedata[i].height * sizeof(void*));
		for (int n = 0; n < imagedata[i].height; n++) {
			imagedata[i].row_pointers[n] = (png_bytep)&imagerows[imagedata[i].width * n];
		}
		RGBA* src = (RGBA*)orimg_data[i].row_pointers[0];
		RGBA* dst = (RGBA*)imagedata[i].row_pointers[0];
		for (int y = 0; y < orimg_data[i].height; y++) {
			for (int x = 0; x < orimg_data[i].width; x++) {
				for (int j = 0; j < multiplier; j++) {
					*dst = *src;
					dst++;
				}
				src++;
			}
			for (int j = 0; j < multiplier - 1; j++) {
				memcpy(dst, dst - imagedata[i].width, sizeof(RGBA) * imagedata[i].width);
				dst += imagedata[i].width;
			}
		}
	}

	// upscale the meta
	for (int i = 0; i < numimage; i++) {
		if (imagedata[i].fixColorMask == NULL)
			continue;

		imagedata[i].fixColorMask = (BYTE*)malloc(sizeof(BYTE) * imagedata[i].height * imagedata[i].width);

		BYTE* src = (BYTE*)&orimg_data[i].fixColorMask[0];
		BYTE* dst = (BYTE*)&imagedata[i].fixColorMask[0];
		for (int y = 0; y < orimg_data[i].height; y++) {
			for (int x = 0; x < orimg_data[i].width; x++) {
				for (int j = 0; j < multiplier; j++) {
					*dst = *src;
					dst++;
				}
				src++;
			}
			for (int j = 0; j < multiplier - 1; j++) {
				memcpy(dst, dst - imagedata[i].width, sizeof(BYTE) * imagedata[i].width);
				dst += imagedata[i].width;
			}
		}
	}

	// resample the pixels
	for (int i = 0; i < numimage; i++) {
		for (int y = 0; y < imagedata[i].height - multiplier; y += multiplier) {
			RGBA* p0 = (RGBA*)imagedata[i].row_pointers[y];
			for (int x = 0; x < imagedata[i].width - multiplier; x += multiplier, p0 += multiplier) {
				if (p0->a != 255)
					continue; // skip transparent pixels
				// skip 'protected' colors
				if (imagedata[i].fixColorMask != NULL && imagedata[i].fixColorMask[x + y * imagedata[i].width] != 0)
					continue;

				RGBA* pR = p0 + multiplier;
				RGBA* pD = p0 + multiplier * imagedata[i].width;
				RGBA* pDR = pD + multiplier;
				for (int j = 0; j < multiplier; j++) {
					for (int k = 0; k < multiplier; k++) {
						RGBA* pp = p0 + j * imagedata[i].width + k;
						*pp = BilinearInterpolate(pp, pR, k, pD, j, pDR, multiplier/*, palette, numcolors, coloroffset, numfixcolors*/);
					}
				}
			}
		}
	}

	// apply basic anti-aliasing filters
	if (antiAliasingMode != 2) {
		UpscalePatterns patterns[16] = {
			{ patternLineDownRight, LineDownRight },
			{ patternLineDownLeft, LineDownLeft },
			{ patternLineUpRight, LineUpRight },
			{ patternLineUpLeft, LineUpLeft },
			{ patternSlowDownRight, SlowDownRight },
			{ patternSlowDownLeft, SlowDownLeft },
			{ patternSlowUpRight, SlowUpRight },
			{ patternSlowUpLeft, SlowUpLeft },
			{ patternFastDownRight, FastDownRight },
			{ patternFastDownLeft, FastDownLeft },
			{ patternFastUpRight, FastUpRight },
			{ patternFastUpLeft, FastUpLeft },
			{ patternLeftTriangle, LeftTriangle },
			{ patternRightTriangle, RightTriangle },
			{ patternTopTriangle, TopTriangle },
			{ patternBottomTriangle, BottomTriangle },
		};
		for (int i = 0; i < numimage; i++) {
			bool dummyMask = imagedata[i].fixColorMask == NULL;
			if (dummyMask) {
				imagedata[i].fixColorMask = (BYTE*)malloc(sizeof(BYTE) * imagedata[i].width * imagedata[i].height);
				memset(imagedata[i].fixColorMask, 0, sizeof(BYTE) * imagedata[i].width * imagedata[i].height);
			}
			for (int y = 0; y < orimg_data[i].height; y++) {
				RGBA* p0 = (RGBA*)orimg_data[i].row_pointers[y];
				for (int x = 0; x < orimg_data[i].width; x++, p0++) {
					for (int k = 0; k < lengthof(patterns); k++) {
						UpscalePatterns &ptn = patterns[k];
						BYTE w = ptn.pattern[0];
						BYTE h = ptn.pattern[1];
						if (orimg_data[i].width - x < w)
							continue; // pattern does not fit to width
						if (orimg_data[i].height - y < h)
							continue; // pattern does not fit to height
						BYTE *ptnCol = &ptn.pattern[2];
						RGBA* p = p0;
						bool match = true;
						for (int yy = 0; yy < h && match; yy++) {
							for (int xx = 0; xx < w && match; xx++, ptnCol++, p++) {
								switch (*ptnCol) {
								case PTN_COLOR:
									if (p->a != 255) {
										match = false;
									}
									break;
								case PTN_ALPHA:
									if (p->a == 255) {
										match = false;
									}
									break;
								case PTN_DNC:
									break;
								}
							}
							p += orimg_data[i].width - w;
						}
						if (match && ptn.fnc(x, y, multiplier, orimg_data[i], imagedata[i])) {
							break;
						}
					}
				}
			}
			if (dummyMask) {
				free(imagedata[i].fixColorMask);
				imagedata[i].fixColorMask = NULL;
			}
		}

		// postprocess min files
		if (antiAliasingMode == 1) {
			for (int i = 0; i < numimage; i++) {
				// assert(imagedata[i].fixColorMask != NULL);

				RGBA* p0 = (RGBA*)orimg_data[i].row_pointers[orimg_data[i].height - 1];
				bool leftFloorTile = true;
				for (int y = orimg_data[i].width / 2 - 2; y >= 0; y -= 2, p0 -= orimg_data[i].width + orimg_data[i].width / 2) {
					p0 += y;
					for (int x = 0; x < orimg_data[i].width / 2 - y; x++, p0++) {
						if (p0->a != 255)
							leftFloorTile = false;
					}
				}
				for (int y = 2; y != orimg_data[i].width / 2; y += 2, p0 -= orimg_data[i].width + orimg_data[i].width / 2) {
					p0 += y;
					for (int x = 0; x < orimg_data[i].width / 2 - y; x++, p0++) {
						if (p0->a != 255)
							leftFloorTile = false;
					}
				}

				if (leftFloorTile) {
					p0 = (RGBA*)imagedata[i].row_pointers[imagedata[i].height - 1];
					for (int y = imagedata[i].width / 2 - 2; y >= 0; y -= 2, p0 -= imagedata[i].width + imagedata[i].width / 2) {
						p0 += y;
						for (int x = 0; x < imagedata[i].width / 2 - y; x++, p0++) {
							if (p0->a != 255) {
								p0->a = 255;
								BYTE* fm = imagedata[i].fixColorMask + ((size_t)p0 - (size_t)imagedata[i].row_pointers[0]) / sizeof(RGBA);
								if (*(fm - imagedata[i].width * multiplier) != 0) {
									if (*(fm + multiplier) != 0) {
										// both colors are fixed -> use one and fix the color
										*p0 = *(p0 + multiplier);
										*fm = *(fm + multiplier);
									} else {
										// only bottom is fixed -> use the right
										*p0 = *(p0 + multiplier);
									}
								} else {
									if (*(fm + multiplier) != 0) {
										// only right is fixed -> use the top
										*p0 = *(p0 - imagedata[i].width * multiplier);
									} else {
										// neither color is fixed -> interpolate
										p0->r = ((p0 + multiplier)->r + (p0 - imagedata[i].width * multiplier)->r) / 2;
										p0->g = ((p0 + multiplier)->g + (p0 - imagedata[i].width * multiplier)->g) / 2;
										p0->b = ((p0 + multiplier)->b + (p0 - imagedata[i].width * multiplier)->b) / 2;
									}
								}
							}
						}
					}
					for (int y = 2; y != imagedata[i].width / 2; y += 2, p0 -= imagedata[i].width + imagedata[i].width / 2) {
						p0 += y;
						for (int x = 0; x < imagedata[i].width / 2 - y; x++, p0++) {
							if (p0->a != 255) {
								p0->a = 255;
								BYTE* fm = imagedata[i].fixColorMask + ((size_t)p0 - (size_t)imagedata[i].row_pointers[0]) / sizeof(RGBA);
								RGBA* p1 = ((RGBA*)orimg_data[i].row_pointers[orimg_data[i].height - 1 - orimg_data[i].width / 4 - (y - 2) / (2 * multiplier)]) + (x + y) / multiplier;
								if ((p1 + 1)->a != 255 || (p0 + multiplier)->a != 255 || (p0 + imagedata[i].width * multiplier)->a != 255) {
									// original color on the right transparent (top of the triangles) -> use the bottom
									*p0 = *(p0 + imagedata[i].width * multiplier);
									*fm = *(fm + imagedata[i].width * multiplier);
								} else if (*(fm + imagedata[i].width * multiplier) != 0) {
									if (*(fm + multiplier) != 0) {
										// both colors are fixed -> use one and fix the color
										*p0 = *(p0 + multiplier);
										*fm = *(fm + multiplier);
									} else {
										// only top is fixed -> use the right
										*p0 = *(p0 + multiplier);
									}
								} else {
									if (*(fm + multiplier) != 0) {
										// only right is fixed -> use the bottom
										*p0 = *(p0 + imagedata[i].width * multiplier);
									} else {
										// neither color is fixed -> interpolate
										p0->r = ((p0 + multiplier)->r + (p0 + imagedata[i].width * multiplier)->r) / 2;
										p0->g = ((p0 + multiplier)->g + (p0 + imagedata[i].width * multiplier)->g) / 2;
										p0->b = ((p0 + multiplier)->b + (p0 + imagedata[i].width * multiplier)->b) / 2;
									}
								}
							}
						}
					}
				}
				bool rightFloorTile = true;
				p0 = (RGBA*)orimg_data[i].row_pointers[orimg_data[i].height - 1];
				p0 += orimg_data[i].width / 2;
				for (int y = orimg_data[i].width / 2 - 2; y >= 0; y -= 2, p0 -= orimg_data[i].width + orimg_data[i].width / 2) {
					for (int x = 0; x < orimg_data[i].width / 2 - y; x++, p0++) {
						if (p0->a != 255)
							rightFloorTile = false;
					}
					p0 += y;
				}
				for (int y = 2; y != orimg_data[i].width / 2; y += 2, p0 -= orimg_data[i].width + orimg_data[i].width / 2) {
					for (int x = 0; x < orimg_data[i].width / 2 - y; x++, p0++) {
						if (p0->a != 255)
							rightFloorTile = false;
					}
					p0 += y;
				}
				if (rightFloorTile) {
					p0 = (RGBA*)imagedata[i].row_pointers[imagedata[i].height - 1];
					p0 += imagedata[i].width / 2;
					for (int y = imagedata[i].width / 2 - 2; y >= 0; y -= 2, p0 -= imagedata[i].width + imagedata[i].width / 2) {
						for (int x = 0; x < imagedata[i].width / 2 - y; x++, p0++) {
							if (p0->a != 255) {
								p0->a = 255;
								BYTE* fm = imagedata[i].fixColorMask + ((size_t)p0 - (size_t)imagedata[i].row_pointers[0]) / sizeof(RGBA);
								if (*(fm - imagedata[i].width * multiplier) != 0) {
									if (*(fm - multiplier) != 0) {
										// both colors are fixed -> use one and fix the color
										*p0 = *(p0 - multiplier);
										*fm = *(fm - multiplier);
									} else {
										// only bottom is fixed -> use the left
										*p0 = *(p0 - multiplier);
									}
								} else {
									if (*(fm - multiplier) != 0) {
										// only left is fixed -> use the top
										*p0 = *(p0 - imagedata[i].width * multiplier);
									} else {
										// neither color is fixed -> interpolate
										p0->r = ((p0 - multiplier)->r + (p0 - imagedata[i].width * multiplier)->r) / 2;
										p0->g = ((p0 - multiplier)->g + (p0 - imagedata[i].width * multiplier)->g) / 2;
										p0->b = ((p0 - multiplier)->b + (p0 - imagedata[i].width * multiplier)->b) / 2;
									}
								}
							}
						}
						p0 += y;
					}
					for (int y = 2; y != imagedata[i].width / 2; y += 2, p0 -= imagedata[i].width + imagedata[i].width / 2) {
						for (int x = 0; x < imagedata[i].width / 2 - y; x++, p0++) {
							if (p0->a != 255) {
								p0->a = 255;
								BYTE* fm = imagedata[i].fixColorMask + ((size_t)p0 - (size_t)imagedata[i].row_pointers[0]) / sizeof(RGBA);
								RGBA* p1 = ((RGBA*)orimg_data[i].row_pointers[orimg_data[i].height - 1 - orimg_data[i].width / 4 - (y - 2) / (2 * multiplier)]) + orimg_data[i].width / 2 + x / multiplier;
								if ((p1 - 1)->a != 255 || (p0 - multiplier)->a != 255 || (p0 + imagedata[i].width * multiplier)->a != 255) {
									// original color on the left transparent (top of the triangles) -> use the bottom
									*p0 = *(p0 + imagedata[i].width * multiplier);
									*fm = *(fm + imagedata[i].width * multiplier);
								} else if (*(fm + imagedata[i].width * multiplier) != 0) {
									if (*(fm - multiplier) != 0) {
										// both colors are fixed -> use one and fix the color
										*p0 = *(p0 - multiplier);
										*fm = *(fm - multiplier);
									} else {
										// only top is fixed -> use the left
										*p0 = *(p0 - multiplier);
									}
								} else {
									if (*(fm - multiplier) != 0) {
										// only left is fixed -> use the bottom
										*p0 = *(p0 + imagedata[i].width * multiplier);
									} else {
										// neither color is fixed -> interpolate
										p0->r = ((p0 - multiplier)->r + (p0 + imagedata[i].width * multiplier)->r) / 2;
										p0->g = ((p0 - multiplier)->g + (p0 + imagedata[i].width * multiplier)->g) / 2;
										p0->b = ((p0 - multiplier)->b + (p0 + imagedata[i].width * multiplier)->b) / 2;
									}
								}
							}
						}
						p0 += y;
					}
				}
			}
		}
	}

	// cleanup memory
	CleanupImageData(orimg_data, numimage);
}
