#include <png.h>
#include <zlib.h>

typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

#define PNG_TRANSFORM_VFLIP 0x10000
#define PNG_TRANSFORM_HFLIP 0x20000

BYTE diapal[128][3] = {
{ 159, 159, 255}, { 87, 87, 255}, { 36, 36, 254}, { 1, 1, 239}, 
{ 0, 0, 189}, { 0, 0, 138}, { 0, 0, 87}, { 0, 0, 25}, 
{ 255, 159, 159}, { 255, 87, 87}, { 254, 36, 36}, { 240, 0, 0}, 
{ 189, 0, 0}, { 145, 0, 0}, { 90, 0, 0}, { 35, 0, 0}, 
{ 255, 253, 159}, { 255, 252, 87}, { 254, 251, 36}, { 240, 236, 0}, 
{ 195, 195, 0}, { 134, 134, 0}, { 87, 85, 0}, { 25, 25, 0}, 
{ 254, 190, 160}, { 255, 140, 87}, { 254, 105, 36}, { 240, 76, 0}, 
{ 199, 65, 0}, { 143, 46, 0}, { 87, 26, 0}, { 30, 7, 0}, 
{ 232, 202, 202}, { 215, 178, 178}, { 202, 158, 158}, { 189, 143, 143}, 
{ 179, 128, 128}, { 168, 113, 113}, { 165, 90, 90}, { 156, 73, 73}, 
{ 139, 65, 65}, { 121, 57, 57}, { 104, 49, 49}, { 86, 41, 41}, 
{ 68, 33, 33}, { 51, 25, 25}, { 27, 14, 14}, { 12, 7, 7}, 
{ 200, 205, 234}, { 178, 183, 215}, { 159, 165, 198}, { 147, 153, 185}, 
{ 135, 141, 172}, { 121, 127, 160}, { 102, 112, 153}, { 88, 99, 141}, 
{ 78, 88, 125}, { 67, 76, 111}, { 57, 65, 95}, { 47, 54, 80}, 
{ 37, 43, 65}, { 25, 30, 45}, { 13, 17, 27}, { 5, 7, 12}, 
{ 255, 227, 164}, { 238, 209, 140}, { 221, 196, 126}, { 204, 183, 117}, 
{ 188, 168, 108}, { 171, 154, 99}, { 152, 139, 93}, { 135, 126, 84}, 
{ 120, 111, 73}, { 105, 96, 63}, { 91, 81, 52}, { 72, 64, 39}, 
{ 57, 49, 29}, { 49, 40, 22}, { 26, 20, 8}, { 20, 11, 0}, 
{ 255, 226, 179}, { 244, 201, 150}, { 231, 179, 126}, { 220, 159, 112}, 
{ 208, 140, 98}, { 199, 123, 82}, { 204, 97, 51}, { 199, 75, 31}, 
{ 177, 67, 27}, { 155, 59, 24}, { 133, 50, 19}, { 111, 41, 16}, 
{ 90, 34, 12}, { 63, 23, 8}, { 37, 14, 3}, { 15, 5, 0}, 
{ 255, 189, 189}, { 244, 150, 150}, { 232, 125, 125}, { 224, 108, 108}, 
{ 216, 91, 91}, { 207, 73, 73}, { 199, 56, 56}, { 191, 39, 39}, 
{ 169, 34, 34}, { 147, 30, 30}, { 124, 25, 25}, { 102, 21, 21}, 
{ 79, 17, 17}, { 57, 13, 13}, { 35, 9, 9}, { 12, 5, 5}, 
{ 243, 243, 243}, { 222, 222, 222}, { 204, 204, 204}, { 184, 184, 184}, 
{ 163, 163, 163}, { 148, 148, 148}, { 133, 133, 133}, { 115, 115, 115}, 
{ 102, 102, 102}, { 89, 89, 89}, { 76, 76, 76}, { 61, 61, 61}, 
{ 46, 46, 46}, { 30, 30, 30}, { 17, 17, 17}, { 255, 255, 255}, 
};

struct RGBA {
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE a;
};

static BYTE GetPalColor(RGBA &data, BYTE *palette, int numcolors, int offset)
{
	int res = -1;
	int best = abs(data.r - 0) + 
			   abs(data.g - 0) + 
			   abs(data.b - 0)

	for (int i = 0; i < numcolors; i++, palette += 3) {
		int dist = abs(data.r - palette[0]) + 
				   abs(data.g - palette[1]) + 
				   abs(data.b - palette[2]);
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

struct png_image_data {
	png_uint_32 width;
	png_uint_32 height;
	png_bytep *row_pointers;
};

static bool ReadPNG(const char *pngname, png_image_data &data)
{
	const int alphaByte = 255;
	FILE *fp;

	fp = fopen(pngname, "rb");
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
	return true;
}

void PNGFlip(png_image_data &imagedata, bool vertical)
{
	if (vertical) {
		RGBA tmp;
		for (int j = 0; j < imagedata.height; j++) {
			RGBA *imagerow = (RGBA *)imagedata.row_pointers[j];
			for (int i = 0; i < imagedata.width / 2; i++) {
				tmp = imagerow[i];
				imagerow[i] = imagerow[imagedata.width - i - 1];
				imagerow[imagedata.width - i - 1] = tmp;
			}
		}
	} else {
		png_bytep tmp;
		for (int i = 0; i < imagedata.height / 2; i++) {
			tmp = imagedata.row_pointers[i];
			imagedata.row_pointers[i] = imagedata.row_pointers[imagedata.height - i - 1];
			imagedata.row_pointers[imagedata.height - i - 1] = tmp;
		}
	}
}

/**
 * Convert PNG file to CEL
 * @param pngnames: the list of PNG file names
 * @param numimage: the number of frames.
 * @param multi: false - numimage equals to the number of PNG files, true - a single PNG file is split to the number of frames
 * @param celname: the name of the output CEL file
 * @param clipped: whether the optional frame header is added
 * @param palette: the palette to use
 * @param numcolors: the number of colors in the palette
 * @param coloroffset: offset to be added to the selected color
 */
static bool PNG2Cel(const char** pngnames, int numimage, bool multi, const char *celname, bool clipped, BYTE *palette, int numcolors, int coloroffset)
{
	int HEADER_SIZE = 4 + 4 + numimage * 4;

	png_image_data *imagedata = (png_image_data*)malloc(sizeof(png_image_data) * numimage);
	if (multi) {
		if (!ReadPNG(pngnames[0], imagedata[0])) {
			free(imagedata);
			return false;
		}
		png_image_data *image_data = &imagedata[0];
		if ((image_data->height % numimage) != 0) {
			free(imagedata);
			return false;
		}
		image_data->height /= numimage;
		for (int n = 1; n < numimage; n++) {
			png_image_data *img_data = &imagedata[n];
			img_data->width = image_data->width;
			img_data->height = image_data->height;
			img_data->row_pointers = &image_data->row_pointers[n * image_data->height];
		}
	} else {
		for (int n = 0; n < numimage; n++) {
			if (!ReadPNG(pngnames[n], imagedata[n])) {
				while (--n >= 0)
					free(imagedata[n].row_pointers);
				free(imagedata);
				return false;
			}
		}
	}

	int maxsize = HEADER_SIZE;
	if (clipped)
		maxsize += numimage * 0x0A;
	for (int n = 0; n < numimage; n++) {
		png_image_data *image_data = &imagedata[n];
		maxsize += image_data->height * (2 * image_data->width);
	}

	BYTE *buf = (BYTE *)malloc(maxsize);
	memset(buf, 0, maxsize);
	buf[0] = numimage;
	buf[4] = HEADER_SIZE;
	BYTE *pBuf = &buf[HEADER_SIZE];
	for (int n = 0; n < numimage; n++) {
		// add optional {CEL FRAME HEADER}
		if (clipped) {
			pBuf[0] = 0x0A;
			pBuf[1] = 0x00;
			pBuf += 0x0A;
		}
		// convert to cel
		png_image_data *image_data = &imagedata[n];
		BYTE *pHead;
		for (int i = 1; i <= image_data->height; i++) {
			pHead = pBuf;
			pBuf++;
			bool alpha = false;
			RGBA* data = (RGBA*)image_data->row_pointers[image_data->height - i];
			for (int j = 0; j < image_data->width; j++) {
				if (data[j].a == 255) {
					// add opaque pixel
					if (alpha || *pHead > 126) {
						pHead = pBuf;
						pBuf++;
					}
					++*pHead;
					*pBuf = GetPalColor(data[j], palette, numcolors, coloroffset);
					pBuf++;
					alpha = false;
				} else {
					// add transparent pixel
					if (j != 0 && (!alpha || (char)*pHead == -128)) {
						pHead = pBuf;
						pBuf++;
					}
					--*pHead;
					alpha = true;
				}
			}
		}
		*(DWORD*)&buf[4 + 4 * (n + 1)] = pBuf - buf;
	}

	// write to file
	FILE *fp = fopen(celname, "wb");
	fwrite(buf, 1, pBuf - buf, fp);
	fclose(fp);

	// cleanup
	if (multi) {
		free(imagedata[0].row_pointers);
	} else {
		for (int n = 0; n < numimage; n++) {
			png_image_data *image_data = &imagedata[n];

			free(image_data->row_pointers);
		}
	}
	free(imagedata);
	return true;
}

static bool PNG2Cl2(const char** pngnames, int numimage, int transform, const char* celname, BYTE *palette, int numcolors, int coloroffset)
{
	const int SUB_HEADER_SIZE = 10;
	const int RLE_LEN = 4; // number of matching colors to switch from bmp encoding to RLE

	int HEADER_SIZE = 4 + 4 + numimage * 4;

	png_image_data *imagedata = (png_image_data*)malloc(sizeof(png_image_data) * numimage);
	for (int n = 0; n < numimage; n++) {
		if (!ReadPNG(pngnames[n], imagedata[n])) {
			while (--n >= 0)
				free(imagedata[n].row_pointers);
			free(imagedata);
			return false;
		}
		if (transform & PNG_TRANSFORM_HFLIP)
			PNGFlip(imagedata[n], false);
		if (transform & PNG_TRANSFORM_VFLIP)
			PNGFlip(imagedata[n], true);
	}

	int maxsize = HEADER_SIZE;
	for (int n = 0; n < numimage; n++) {
		png_image_data *image_data = &imagedata[n];
		maxsize += image_data->height * (2 * image_data->width);
	}

	BYTE *buf = (BYTE *)malloc(maxsize);
	memset(buf, 0, maxsize);

	// convert to cl2
	buf[0] = numimage;
	buf[4] = HEADER_SIZE;

	BYTE *pBuf = &buf[HEADER_SIZE];
	for (int n = 0; n < numimage; n++) {
		png_image_data *image_data = &imagedata[n];
		BYTE *pHeader = pBuf;
		memset(pBuf, 0, SUB_HEADER_SIZE);
		pBuf[0] = SUB_HEADER_SIZE;
		pBuf += SUB_HEADER_SIZE;

		BYTE *pHead = pBuf;
		BYTE col, lastCol;
		BYTE colMatches = 0;
		bool alpha = false;
		bool first = TRUE;
		for (int i = 1; i <= image_data->height; i++) {
			RGBA* data = (RGBA*)image_data->row_pointers[image_data->height - i];
			if (i == 32 + 1) {
				pHead = pBuf;
				*(WORD*)(&pHeader[2]) = pHead - pHeader;//pHead - buf - SUB_HEADER_SIZE;

				colMatches = 0;
				alpha = false;
				first = TRUE;
			}
			if (i == image_data->height - (32 - 1)) {
				pHead = pBuf;
				*(WORD*)(&pHeader[4]) = pHead - pHeader;//pHead - buf - SUB_HEADER_SIZE;

				colMatches = 0;
				alpha = false;
				first = TRUE;
			}
			for (int j = 0; j < image_data->width; j++) {
				if (data[j].a == 255) {
					// add opaque pixel
					col = GetPalColor(data[j], palette, numcolors, coloroffset);
					if (alpha || first || col != lastCol)
						colMatches = 1;
					else
						colMatches++;
					if (colMatches < RLE_LEN || (char)*pHead <= -127) {
						// bmp encoding
						if (alpha || (char)*pHead <= -65) {
							pHead = pBuf;
							pBuf++;
						}
						*pBuf = col;
						pBuf++;
					} else {
						// RLE encoding
						if (colMatches == RLE_LEN) {
							memset(pBuf - (RLE_LEN - 1), 0, RLE_LEN - 1);
							*pHead += RLE_LEN - 1;
							if (*pHead != 0) {
								pHead = pBuf - (RLE_LEN - 1);
							}
							*pHead = -65 - (RLE_LEN - 1);
							pBuf = pHead + 1;
							*pBuf = col;
							pBuf++;
						}
					}
					--*pHead;

					lastCol = col;
					alpha = false;
				} else {
					// add transparent pixel
					if (!alpha || (char)*pHead >= 127) {
						pHead = pBuf;
						pBuf++;
					}
					++*pHead;
					alpha = true;
				}
				first = FALSE;
			}
		}
		*(DWORD*)&buf[4 + 4 * (n + 1)] = pBuf - buf;
	}
	// write to file
	FILE *fp = fopen(celname, "wb");
	fwrite(buf, 1, pBuf - buf, fp);
	fclose(fp);
	// cleanup
	for (int n = 0; n < numimage; n++) {
		png_image_data *image_data = &imagedata[n];

		free(image_data->row_pointers);
	}
	free(imagedata);
	return true;
}

bool Cel2Cel(const char* destCelName, int nCel,
	const char* srcCelName, const char* resCelName)
{
	FILE *fsHead, *fsData;
	FILE *fdHead, *fdData;
	FILE *frData;
	
	// open source file with 2 pointers (header + data)
	fsHead = fopen(srcCelName, "rb");
	fsData = fopen(srcCelName, "rb");
	int srcCount;
	fread(&srcCount, 1, 4, fsHead);
	fread(&srcCount, 1, 4, fsData);
	int srcDataSize;
	for (int i = 0; i <= srcCount; i++)
		fread(&srcDataSize, 1, 4, fsData);

	// open dest file with 2 pointers (header + data)
	fdHead = fopen(destCelName, "rb");
	fdData = fopen(destCelName, "rb");
	int destCount;
	fread(&destCount, 1, 4, fdHead);
	fread(&destCount, 1, 4, fdData);
	int destDataSize;
	for (int i = 0; i <= destCount; i++)
		fread(&destDataSize, 1, 4, fdData);

	// create result with a single pointer (data)
	frData = fopen(resCelName, "wb");
	int v = srcCount + destCount;
	// write dummy header data (only the first entry is valid)
	for (int i = 0; i < v + 2; i++)
		fwrite(&v, 1, 4, frData);

	// allocate container for the header info
	DWORD *headBuf = (DWORD *)malloc((v + 1) * 4);

	// write entries from the dest file till nCel
	int chunkSize;
	int destCurs, srcCurs, curs;
	fread(&destCurs, 1, 4, fdHead);
	int ci = 0;
	for (int i = 1; i < nCel; i++) {
		fread(&curs, 1, 4, fdHead);
		chunkSize = curs - destCurs;
		destCurs = curs;
		BYTE *buf = (BYTE *)malloc(chunkSize);
		fread(buf, 1, chunkSize, fdData);
		fwrite(buf, 1, chunkSize, frData);
		free(buf);
		headBuf[ci] = chunkSize;
		ci++;
	}

	// write entries from the source file till end
	fread(&srcCurs, 1, 4, fsHead);
	for (int i = 0; i < srcCount; i++) {
		fread(&curs, 1, 4, fsHead);
		chunkSize = curs - srcCurs;
		srcCurs = curs;
		BYTE *buf = (BYTE *)malloc(chunkSize);
		fread(buf, 1, chunkSize, fsData);
		fwrite(buf, 1, chunkSize, frData);
		free(buf);
		headBuf[ci] = chunkSize;
		ci++;
	}
	fclose(fsHead);
	fclose(fsData);

	// write remaining entries of the dest file
	for (int i = nCel; i <= destCount; i++) {
		fread(&curs, 1, 4, fdHead);
		chunkSize = curs - destCurs;
		destCurs = curs;
		BYTE *buf = (BYTE *)malloc(chunkSize);
		fread(buf, 1, chunkSize, fdData);
		fwrite(buf, 1, chunkSize, frData);
		free(buf);
		headBuf[ci] = chunkSize;
		ci++;
	}

	fclose(fdHead);
	fclose(fdData);

	// go back to the header and write the valid info
	fseek(frData, 4, SEEK_SET);
	curs = (2 + ci) * 4;
	for (int i = 0; i <= ci; i++) {
		fwrite(&curs, 1, 4, frData);
		curs += headBuf[i];
	}

	fclose(frData);	
	free(headBuf);
	return true;
}

static bool WritePNG(const char *pngname, png_image_data &data)
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

static RGBA GetPNGColor(BYTE col, BYTE *palette, int coloroffset)
{
	RGBA result;

	col -= coloroffset;
	result.r = palette[col * 3 + 0];
	result.g = palette[col * 3 + 1];
	result.b = palette[col * 3 + 2];
	result.a = 255;
	return result;
}

static void CelBlitSafe(RGBA *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth, BYTE *palette, int coloroffset)
{
	int i, w, BUFFER_WIDTH;
	char width;
	BYTE *src;
	RGBA* dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	w = BUFFER_WIDTH = nWidth;

	for (; src != &pRLEBytes[nDataSize]; dst -= BUFFER_WIDTH + w) {
		for (i = w; i != 0;) {
			width = *src++;
			if (width >= 0) {
				i -= width;
				while (width-- != 0) {
					*dst = GetPNGColor(*src++, palette, coloroffset);
					dst++;
				}
			} else {
				while (width++ != 0) {
					*dst = { 0 };
					i--;
					dst++;
				}
			}
		}
	}
}

static void Cl2BlitSafe(RGBA *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth, BYTE *palette, int coloroffset)
{
	int w, BUFFER_WIDTH;
	char width;
	BYTE fill;
	BYTE *src;
	RGBA *dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	w = BUFFER_WIDTH = nWidth;

	while (nDataSize != 0) { 
		width = *src++;
		nDataSize--;
		if (width < 0) {
			width = -width;
			if (width > 65) {
				width -= 65;
				nDataSize--;
				RGBA fill = GetPNGColor(*src++, palette, coloroffset);
				w -= width;
				while (width != 0) {
					*dst = fill;
					dst++;
					width--;
				}
				if (w == 0) {
					w = nWidth;
					dst -= BUFFER_WIDTH + w;
				}
			} else {
				nDataSize -= width;
				w -= width;
				while (width != 0) {
					*dst = GetPNGColor(*src, palette, coloroffset);
					src++;
					dst++;
					width--;
				}
				if (w == 0) {
					w = nWidth;
					dst -= BUFFER_WIDTH + w;
				}
			}
			continue;
		}
		while (width != 0) {
			if (width > w) {
				for ( ; w != 0; w--) {
					*dst = { 0 };
					dst++;
					width--;
				}
			} else {
				for ( ; width != 0; width--) {
					*dst = { 0 };
					dst++;
					w--;
				}
			}
			if (w == 0) {
				w = nWidth;
				dst -= BUFFER_WIDTH + w;
			}
		}
	}
}

struct cel_image_data {
	DWORD dataSize;
	DWORD width;
	DWORD height;
	BYTE* data;
};
bool Cel2PNG(const char* celname, int nCel, int nWidth, const char* destFolder, BYTE *palette, int coloroffset)
{
	FILE *f = fopen(celname, "rb");

	// read the file into memory
	int numimage;
	fread(&numimage, 4, 1, f);

	int headerSize = 4 + 4 + 4 *numimage;
	cel_image_data *celdata = (cel_image_data *)malloc(sizeof(cel_image_data) * (numimage + 1));
	DWORD dataSize;
	for (int i = 0; i <= numimage; i++) {
		fread(&dataSize, 4, 1, f);
		celdata[i].dataSize = dataSize;
	}

	BYTE *buf = (BYTE *)malloc(dataSize);
	fread(buf, 1, dataSize - headerSize, f);

	fclose(f);

	// prepare celdata info
	BYTE *src = buf;
	for (int i = 0; i < numimage; i++) {
		celdata[i].width = nWidth;
		celdata[i].dataSize = celdata[i + 1].dataSize - celdata[i].dataSize;
		// skip optional {CEL FRAME HEADER}
		if (src[0] == 0x0A && src[1] == 0) {
			src += 0x0A;
			celdata[i].dataSize -= 0x0A;
		}
		celdata[i].data = src;
		int pixels = 0;
		while (src < &celdata[i].data[celdata[i].dataSize]) {
			char width = *src++;
			if (width >= 0) {
				pixels += width;
				src += width;
			} else {
				pixels -= width;
			}
		}
		if (src != &celdata[i].data[celdata[i].dataSize]) {
			free(buf);
			free(celdata);
			return false;
		}
		if (pixels % nWidth != 0) {
			free(buf);
			free(celdata);
			return false;
		}
		celdata[i].height = pixels / nWidth;
	}

	// write the png(s)
	nCel--;
	png_image_data imagedata;
	for (int i = 0; i < numimage; i++) {
		if (i == nCel || nCel < 0) {
			// prepare pngdata
			imagedata.width = celdata[i].width;
			imagedata.height = celdata[i].height;
			RGBA *imagerows = (RGBA *)malloc(sizeof(RGBA) * imagedata.height * imagedata.width);
			imagedata.row_pointers = (png_bytep*)malloc(imagedata.height * sizeof(void*));
			for (int n = 0; n < imagedata.height; n++) {
				imagedata.row_pointers[n] = (png_bytep)&imagerows[imagedata.width * n];
			}
			RGBA* lastLine = (RGBA*)imagedata.row_pointers[imagedata.height - 1];
			CelBlitSafe(lastLine, celdata[i].data, celdata[i].dataSize, imagedata.width, palette, coloroffset);

			// write a single png
			char destFile[256];
			int idx = strlen(celname) - 1;
			while (idx > 0 && celname[idx] != '\\' && celname[idx] != '/')
				idx--;
			int fnc = snprintf(destFile, 256, "%s%s", destFolder, &celname[idx + 1]);
			destFile[fnc - 4] = '_';
			snprintf(&destFile[fnc], 256, "_frame%04d.png", i);
			
			if (!WritePNG(destFile, imagedata)) {
				free(imagedata.row_pointers);
				free(imagerows);
				free(buf);
				free(celdata);
				return false;
			}
			free(imagedata.row_pointers);
			free(imagerows);
		}
	}

	// cleanup
	free(buf);
	free(celdata);
	return true;
}

bool Cl2PNG(const char* celname, int nCel, int nWidth, const char* destFolder, BYTE *palette, int coloroffset)
{
	FILE *f = fopen(celname, "rb");
	// read the file into memory
	int numimage;
	fread(&numimage, 4, 1, f);

	int headerSize = 4 + 4 + 4 *numimage;
	cel_image_data *celdata = (cel_image_data *)malloc(sizeof(cel_image_data) * (numimage + 1));
	DWORD dataSize;
	for (int i = 0; i <= numimage; i++) {
		fread(&dataSize, 4, 1, f);
		celdata[i].dataSize = dataSize;
	}

	BYTE *buf = (BYTE *)malloc(dataSize);
	fread(buf, 1, dataSize - headerSize, f);

	fclose(f);

	// prepare celdata info
	BYTE *src = buf;
	for (int i = 0; i < numimage; i++) {
		celdata[i].data = src;
		celdata[i].dataSize = celdata[i + 1].dataSize - celdata[i].dataSize;
		celdata[i].width = nWidth;
		// skip frame-header
		WORD subHeaderSize = *(WORD*)src;
		src += subHeaderSize;
		celdata[i].data += subHeaderSize;
		celdata[i].dataSize -= subHeaderSize;

		int pixels = 0;
		while (src < &celdata[i].data[celdata[i].dataSize]) {
			char width = *src++;
			if (width >= 0) {
				pixels += width;
			} else {
				width = -width;
				if (width > 65) {
					// fill
					pixels += width - 65;
					src++;
				} else {
					// bmp
					pixels += width;
					src += width;
				}
			}
		}
		if (src != &celdata[i].data[celdata[i].dataSize]) {
			free(buf);
			free(celdata);
			return false;
		}
		if (pixels % nWidth != 0) {
			free(buf);
			free(celdata);
			return false;
		}
		celdata[i].height = pixels / nWidth;
	}

	// write the png(s)
	nCel--;
	png_image_data imagedata;
	for (int i = 0; i < numimage; i++) {
		if (i == nCel || nCel < 0) {
			// prepare pngdata
			imagedata.width = celdata[i].width;
			imagedata.height = celdata[i].height;
			RGBA *imagerows = (RGBA *)malloc(sizeof(RGBA) * imagedata.height * imagedata.width);
			imagedata.row_pointers = (png_bytep*)malloc(imagedata.height * sizeof(void*));
			for (int n = 0; n < imagedata.height; n++) {
				imagedata.row_pointers[n] = (png_bytep)&imagerows[imagedata.width * n];
			}

			RGBA* lastLine = (RGBA*)imagedata.row_pointers[imagedata.height - 1];
			//lastLine += imagedata.width * (imagedata.height - 1);
			Cl2BlitSafe(lastLine, celdata[i].data, celdata[i].dataSize, imagedata.width, palette, coloroffset);
			
			// write a single png
			char destFile[256];
			int idx = strlen(celname) - 1;
			while (idx > 0 && celname[idx] != '\\' && celname[idx] != '/')
				idx--;
			int fnc = snprintf(destFile, 256, "%s%s", destFolder, &celname[idx + 1]);
			destFile[fnc - 4] = '_';
			snprintf(&destFile[fnc], 256, "_frame%04d.png", i);
			
			if (!WritePNG(destFile, imagedata)) {
				free(imagedata.row_pointers);
				free(imagerows);
				free(buf);
				free(celdata);
				return false;
			}
			free(imagedata.row_pointers);
			free(imagerows);
		}
	}

	// cleanup
	free(buf);
	free(celdata);
	return true;
}

BYTE* LoadPal(const char* palFile)
{
	BYTE* result = (BYTE*)malloc(256 * 3);

	FILE* f = fopen(palFile, "rb");

	fread(result, 1, 256 * 3, f);

	fclose(f);

	return result;
}

int main()
{
	//Cl2PNG("f:\\Farrow1.CL2", 0, 96, "f:\\", &diapal[0][0], 128);

	/*const char* sffilenames[] = {
		"f:\\Splash_CL2_frame0000.png",
		"f:\\Splash_CL2_frame0001.png",
		"f:\\Splash_CL2_frame0002.png",
		"f:\\Splash_CL2_frame0003.png",
		"f:\\Splash_CL2_frame0004.png",
		"f:\\Splash_CL2_frame0005.png",
		"f:\\Splash_CL2_frame0006.png",
		"f:\\Splash_CL2_frame0007.png"};
	PNG2Cl2(sffilenames, 8, PNG_TRANSFORM_IDENTITY, "f:\\Splash.CL2", &diapal[0][0], 128, 128);*/

	/*for (int n = 1; n < 10; n++) {
		char pffiles[4][256];
		const char* pps[4];
		for (int m = 0; m < 4; m++) {
			snprintf(pffiles[m], 256, "f:\\Marrow%d_CL2_frame%04d.png", n, m);
			pps[m] = &pffiles[m][0];
		}
		char pname[256];
		snprintf(pname, 256, "f:\\Marrow%d.CL2", n);
		PNG2Cl2(&pps[0], 4, PNG_TRANSFORM_IDENTITY, pname, &diapal[0][0], 128, 128);
	}*/

	/*for (int n = 2; n < 9; n++) {
		char pffiles[4][256];
		const char* pps[4];
		for (int m = 0; m < 4; m++) {
			snprintf(pffiles[m], 256, "f:\\Marrow%d_CL2_frame%04d.png", n, m);
			pps[m] = &pffiles[m][0];
		}
		char pname[256];
		snprintf(pname, 256, "f:\\Marrow%d.CL2", (18 - n));
		PNG2Cl2(&pps[0], 4, PNG_TRANSFORM_VFLIP, pname, &diapal[0][0], 128, 128);
	}

	const char* ffilenames[] = {
		"f:\\Farrow1_CL2_frame0000_.png",
		"f:\\Farrow1_CL2_frame0001_.png",
		"f:\\Farrow1_CL2_frame0002_.png",
		"f:\\Farrow1_CL2_frame0003_.png"};
	PNG2Cl2(ffilenames, 4, PNG_TRANSFORM_IDENTITY, "f:\\Farrow1__.CL2", &diapal[0][0], 128, 128);

	Cel2PNG("f:\\SpellBkB.CEL", 0, 76, "f:\\", &diapal[0][0], 128);

	const char* filenames[] = { "f:\\Farrow1.png" };
	PNG2Cl2(filenames, 1, PNG_TRANSFORM_IDENTITY, "f:\\Farrow1.CL2", &diapal[0][0], 128, 128);
	/*FILE *fa = fopen("f:\\Farrow1.CL2", "rb");
	int srcCount;
	fread(&srcCount, 1, 4, fa);
	int srcDataSize;
	for (int i = 0; i <= srcCount; i++)
		fread(&srcDataSize, 1, 4, fa);
	fclose(fa);
	BYTE *bufy = (BYTE*)malloc(srcDataSize);
	fa = fopen("f:\\Farrow1.CL2", "rb");
	fread(bufy, 1, srcDataSize, fa);
	fclose(fa);
	BYTE *bufo = (BYTE*)malloc(srcDataSize);
	memset(bufo, 0, srcDataSize);
	Cl2Draw(bufo, bufy, 1, 96);*/

	/*FILE *f = fopen("f:\\town.sol", "rb");
	BYTE *bufy = (BYTE*)malloc(0x4F0);
	int lenn = fread(bufy, 1, 1264, f);
	fclose(f);
	bufy[521] |= 1;
	bufy[522] |= 1;
	bufy[523] |= 1;
	bufy[524] |= 1;
	bufy[539] |= 1;
	bufy[551] |= 1;
	bufy[552] |= 1;
	f = fopen("f:\\town_.sol", "wb");
	fwrite(bufy, 1, lenn, f);
	fclose(f);
	f = NULL;*/

	/*BYTE* pal = LoadPal("f:\\L1_1.PAL");
	Cel2PNG("f:\\L1Doors.CEL", 0, 64, "f:\\", pal, 256);
	const char* doors[] = {
			"f:\\L1Doors_CEL_frame0000.png",
			"f:\\L1Doors_CEL_frame0001.png",
			"f:\\L1Doors_CEL_frame0002.png",
			"f:\\L1Doors_CEL_frame0003.png"};
	PNG2Cel(doors, 4, false, "f:\\L1Doors.CEL", true, pal, 256, 0);*/

	//PNG2Cel("f:\\inv.png", "f:\\inv.cel", false, &diapal[0][0], 128, 128);
	//PNG2Cel("f:\\quest.png", "f:\\quest.cel", false, &diapal[0][0], 128, 128);
	//PNG2Cel("f:\\char.png", "f:\\char.cel", false, &diapal[0][0], 128, 128);
	const char* fsb0[] = { "f:\\SpellBk.png" };
	PNG2Cel(fsb0, 1, false, "f:\\SpellBk.CEL", false, &diapal[0][0], 128, 128);

	const char* fsb1[] = { "f:\\SpellBkB1.png", "f:\\SpellBkB2.png",
		"f:\\SpellBkB3.png", "f:\\SpellBkB4.png"};
	PNG2Cel(fsb1, 4, false, "f:\\SpellBkB__.CEL", false, &diapal[0][0], 128, 128);
	//PNG2Cel("f:\\SpellBkB1.png", "f:\\SpellBkB1.CEL", false, &diapal[0][0], 128, 128);
	//PNG2Cel("f:\\SpellBkB2.png", "f:\\SpellBkB2.CEL", false, &diapal[0][0], 128, 128);
	//PNG2Cel("f:\\SpellBkB3.png", "f:\\SpellBkB3.CEL", false, &diapal[0][0], 128, 128);
	//PNG2Cel("f:\\SpellBkB4.png", 1, "f:\\SpellBkB4.CEL", false, &diapal[0][0], 128, 128);

	const char* pnames[] = { "f:\\heros.png" };
	PNG2Cel(pnames, 7, true, "f:\\heros.CEL", false, &diapal[0][0], 128, 128);

	Cel2Cel("f:\\SpellBkB1.cel", 2, "f:\\SpellBkB2.cel", "f:\\SpellBkB12.cel");
	Cel2Cel("f:\\SpellBkB3.cel", 2, "f:\\SpellBkB4.cel", "f:\\SpellBkB34.cel");
	Cel2Cel("f:\\SpellBkB12.cel", 3, "f:\\SpellBkB34.cel", "f:\\SpellBkB_.cel");
}