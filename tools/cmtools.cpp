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

void UpscalePNGImages(png_image_data* imagedata, int numimage, int multiplier, BYTE* palette, int numcolors, int coloroffset, int numfixcolors)
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

	// cleanup memory
	CleanupImageData(orimg_data, numimage);
}
