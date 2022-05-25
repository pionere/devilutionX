/*
 * Utility functions to manipulate Diablo assets. Its main features are:
 *  Cel2PNG: convert regular CEL file to PNG
 *  PNG2Cel: convert PNG to regular CEL file
 *  UpscaleCel: (integer) upscale regular CEL file
 *  CelComp2PNG: convert compiled CEL file to PNG
 *  PNG2CelComp: convert PNG to compiled CEL file
 *  UpscaleCelComp: (integer) upscale compiled CEL file
 *  Cl2PNG: convert CL2 file to PNG
 *  PNG2Cl2: convert PNG to CL2 file
 *  UpscaleCl2: (integer) upscale CL2 file
 *  Cel2Cel: merge two regular CEL files
 */
#include <png.h>
#include <zlib.h>
#include <stdint.h>
#include <algorithm>
#include <string>
#include <set>
#include <map>

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef unsigned char BYTE;

#define PNG_TRANSFORM_VFLIP 0x10000
#define PNG_TRANSFORM_HFLIP 0x20000

#define SUB_HEADER_SIZE		0x0A

template<class T, int N>
constexpr int lengthof(T (&arr)[N])
{
	return N;
}

static WORD SwapLE16(WORD w)
{
	WORD v = 1;
	if (((BYTE*)&v)[1] == 0)
		return w;
	return ((w >> 8) & 0x00FF) | ((w << 8) & 0xFF00);
}

static DWORD SwapLE32(DWORD dw)
{
	DWORD v = 1;
	if (((BYTE*)&v)[3] == 0)
		return dw;
	return ((dw >> 24) & 0xFF) | ((dw << 24) & 0xFF000000) | ((dw >> 8) & 0x00FF00) | ((dw << 8) & 0xFF0000);
}

static bool stringicomp(const char* str1, const char* str2)
{
	int len = strlen(str1); 
	if (len != strlen(str2))
		return false;
	for (int i = 0; i < len; i++) {
		if (str1[i] == str2[i])
			continue;
		if (str1[i] <= 'Z' && str1[i] >= 'A') {
			if (str2[i] == str1[i] - 'A' + 'a')
				continue;
		} else if (str1[i] <= 'z' && str1[i] >= 'a') {
			if (str2[i] == str1[i] - 'a' + 'A')
				continue;
		}
		return false;
	}
	return true;
}

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

typedef struct RGBA {
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE a;
} RGBA;

static BYTE GetPalColor(RGBA &data, BYTE *palette, int numcolors, int offset, int numfixcolors)
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

typedef struct png_image_data {
	png_uint_32 width;
	png_uint_32 height;
	png_bytep *row_pointers;
	png_bytep data_ptr;
	BYTE* fixColorMask;
} png_image_data;

static void CleanupImageData(png_image_data* imagedata, int numimages)
{
	for (int n = 0; n < numimages; n++) {
		free(imagedata[n].data_ptr);
		free(imagedata[n].row_pointers);
		free(imagedata[n].fixColorMask);
	}
	free(imagedata);
}

static bool ReadPNG(const char *pngname, png_image_data &data)
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

static void PNGFlip(png_image_data &imagedata, bool vertical)
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

typedef struct cel_image_data {
	DWORD dataSize;
	DWORD width;
	DWORD height;
	bool clipped;
	BYTE* data;
} cel_image_data;
static bool WritePNG2Cel(png_image_data* imagedata, int numimage, cel_image_data* celdata, bool multi, const char* celname, BYTE* palette, int numcolors, int coloroffset, int numfixcolors)
{
	int HEADER_SIZE = 4 + 4 + numimage * 4;
	int maxsize = HEADER_SIZE;

	for (int n = 0; n < numimage; n++) {
		if (celdata[n].clipped)
			maxsize += SUB_HEADER_SIZE;
		png_image_data *image_data = &imagedata[n];
		maxsize += image_data->height * (2 * image_data->width);
	}

	BYTE *buf = (BYTE *)malloc(maxsize);
	memset(buf, 0, maxsize);
	*(DWORD*)&buf[0] = SwapLE32(numimage);
	*(DWORD*)&buf[4] = SwapLE32(HEADER_SIZE);
	BYTE *pBuf = &buf[HEADER_SIZE];
	for (int n = 0; n < numimage; n++) {
		// add optional {CEL FRAME HEADER}
		BYTE *pHeader = pBuf;
		if (celdata[n].clipped) {
			pBuf[0] = SUB_HEADER_SIZE;
			pBuf[1] = 0x00;
			*(DWORD*)&pBuf[2] = 0;
			*(DWORD*)&pBuf[6] = 0;
			pBuf += SUB_HEADER_SIZE;
		}
		// convert to cel
		png_image_data *image_data = &imagedata[n];
		BYTE *pHead;
		for (int i = 1; i <= image_data->height; i++) {
			pHead = pBuf;
			pBuf++;
			bool alpha = false;
			RGBA* data = (RGBA*)image_data->row_pointers[image_data->height - i];
			if (i == 32 + 1 && celdata[n].clipped) { // TODO: write more entries if necessary?
				*(WORD*)(&pHeader[(i / 32) * 2]) = SwapLE16(pHead - pHeader);//pHead - buf - SUB_HEADER_SIZE;
			}
			for (int j = 0; j < image_data->width; j++) {
				if (data[j].a == 255) {
					// add opaque pixel
					if (alpha || *pHead > 126) {
						pHead = pBuf;
						pBuf++;
					}
					++*pHead;
					if (imagedata->fixColorMask != NULL && imagedata->fixColorMask[(image_data->height - i) * imagedata->width + j] != 0)
						*pBuf = imagedata->fixColorMask[(image_data->height - i) * imagedata->width + j];
					else
						*pBuf = GetPalColor(data[j], palette, numcolors, coloroffset, numfixcolors);
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
		*(DWORD*)&buf[4 + 4 * (n + 1)] = SwapLE32(pBuf - buf);
	}

	// write to file
	bool result = false;
	FILE *fp = fopen(celname, "wb");
	if (fp != NULL) {
		fwrite(buf, 1, pBuf - buf, fp);
		fclose(fp);
		result = true;
	}

	// cleanup
	free(buf);
	CleanupImageData(imagedata, multi ? 1 : numimage);
	free(celdata);
	return result;
}

/**
 * Convert PNG file to regular CEL
 * @param pngnames: the list of PNG file names
 * @param numimage: the number of frames.
 * @param multi: false - numimage equals to the number of PNG files, true - a single PNG file is split to the number of frames
 * @param celname: the name of the output CEL file
 * @param clipped: whether the optional frame header is added
 * @param palette: the palette to use
 * @param numcolors: the number of colors in the palette
 * @param coloroffset: offset to be added to the selected color
 * @return true if the function succeeds
 */
bool PNG2Cel(const char** pngnames, int numimage, bool multi, const char *celname, bool clipped, BYTE *palette, int numcolors, int coloroffset)
{
	png_image_data* imagedata = (png_image_data*)malloc(sizeof(png_image_data) * numimage);
	if (multi) {
		if (!ReadPNG(pngnames[0], imagedata[0])) {
			free(imagedata);
			return false;
		}
		png_image_data *image_data = &imagedata[0];
		if ((image_data->height % numimage) != 0) {
			CleanupImageData(imagedata, 1);
			return false;
		}
		image_data->height /= numimage;
		for (int n = 1; n < numimage; n++) {
			png_image_data *img_data = &imagedata[n];
			img_data->width = image_data->width;
			img_data->height = image_data->height;
			img_data->row_pointers = &image_data->row_pointers[n * image_data->height];
			img_data->data_ptr = NULL;
		}
	} else {
		for (int n = 0; n < numimage; n++) {
			if (!ReadPNG(pngnames[n], imagedata[n])) {
				CleanupImageData(imagedata, n - 1);
				return false;
			}
		}
	}

	cel_image_data *celdata = (cel_image_data*)malloc(sizeof(cel_image_data) * numimage);
	for (int n = 0; n < numimage; n++) {
		celdata[n].clipped = clipped;
	}
	return WritePNG2Cel(imagedata, numimage, celdata, multi, celname, palette, numcolors, coloroffset, 0);
}

typedef struct celcmp_image_data {
	DWORD dataSize;
	DWORD width;
	DWORD height;
	bool clipped;
	int groupSize;
	BYTE* data;
} celcmp_image_data;
static bool WritePNG2CelComp(png_image_data* imagedata, int numimage, celcmp_image_data* celdata,  bool multi, const char* celname, BYTE *palette, int numcolors, int coloroffset)
{
	// calculate header size
	int groupNum = 0;
	int headerSize = 0;
	for (int i = 0; i < numimage; ) {
		int ni = celdata[i].groupSize;
		headerSize += 4 + 4 * (ni + 1);
		i += ni;
		groupNum++;
	}
	//if (groupNum > 1) {
		headerSize += sizeof(DWORD) * groupNum;
	//}
	// estimate data size
	int maxsize = headerSize;
	for (int n = 0; n < numimage; n++) {
		png_image_data *image_data = &imagedata[n];
		if (celdata[n].clipped)
			maxsize += SUB_HEADER_SIZE;
		maxsize += image_data->height * (2 * image_data->width);
	}	

	BYTE *buf = (BYTE *)malloc(maxsize);
	memset(buf, 0, maxsize);

	BYTE* pBuf = &buf[sizeof(DWORD) * groupNum];
	int idx = 0;
	for (int i = 0; i < groupNum; i++) {
		DWORD ni = celdata[i].groupSize;
		*(DWORD*)&buf[i * sizeof(DWORD)] = SwapLE32(pBuf - buf);

		BYTE* hdr = pBuf;
		*(DWORD*)&hdr[0] = SwapLE32(ni);
		*(DWORD*)&hdr[4] = SwapLE32(4 + 4 * (ni + 1));

		pBuf += 4 + 4 * (ni + 1);
		for (int n = 0; n < ni; n++, idx++) {
			// add optional {CEL FRAME HEADER}
			BYTE *pHeader = pBuf;
			if (celdata[idx].clipped) {
				pBuf[0] = SUB_HEADER_SIZE;
				pBuf[1] = 0x00;
				*(DWORD*)&pBuf[2] = 0;
				*(DWORD*)&pBuf[6] = 0;
				pBuf += SUB_HEADER_SIZE;
			}
			// convert to cel
			png_image_data *image_data = &imagedata[idx];
			BYTE *pHead;
			for (int i = 1; i <= image_data->height; i++) {
				pHead = pBuf;
				pBuf++;
				bool alpha = false;
				RGBA* data = (RGBA*)image_data->row_pointers[image_data->height - i];
				if (i == 32 + 1 && celdata[idx].clipped) { // TODO: write more entries if necessary?
					*(WORD*)(&pHeader[(i / 32) * 2]) = SwapLE16(pHead - pHeader);//pHead - buf - SUB_HEADER_SIZE;
				}
				for (int j = 0; j < image_data->width; j++) {
					if (data[j].a == 255) {
						// add opaque pixel
						if (alpha || *pHead > 126) {
							pHead = pBuf;
							pBuf++;
						}
						++*pHead;
						*pBuf = GetPalColor(data[j], palette, numcolors, coloroffset, 0);
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
			*(DWORD*)&hdr[4 + 4 * (n + 1)] = SwapLE32(pBuf - hdr);
		}
	}
	// write to file
	bool result = false;
	FILE *fp = fopen(celname, "wb");
	if (fp != NULL) {
		fwrite(buf, 1, pBuf - buf, fp);
		fclose(fp);
		result = true;
	}

	// cleanup
	free(buf);
	CleanupImageData(imagedata, multi ? 1 : numimage);
	free(celdata);
	return result;
}

/**
 * Convert PNG file to compiled CEL
 * @param pngnames: the list of PNG file names
 * @param numimage: the number of frames.
 * @param multi: false - numimage equals to the number of PNG files, true - a single PNG file is split to the number of frames
 * @param celname: the name of the output CEL file
 * @param clipped: whether the optional frame header is added
 * @param groupSize: the number of frames per group (numimage / number of groups)
 * @param palette: the palette to use
 * @param numcolors: the number of colors in the palette
 * @param coloroffset: offset to be added to the selected color
 * @return true if the function succeeds
 */
bool PNG2CelComp(const char** pngnames, int numimage, bool multi, const char *celname, bool clipped, int groupSize, BYTE *palette, int numcolors, int coloroffset)
{
	png_image_data *imagedata = (png_image_data*)malloc(sizeof(png_image_data) * numimage);
	if (multi) {
		if (!ReadPNG(pngnames[0], imagedata[0])) {
			free(imagedata);
			return false;
		}
		png_image_data *image_data = &imagedata[0];
		if ((image_data->height % numimage) != 0) {
			CleanupImageData(imagedata, 1);
			return false;
		}
		image_data->height /= numimage;
		for (int n = 1; n < numimage; n++) {
			png_image_data *img_data = &imagedata[n];
			img_data->width = image_data->width;
			img_data->height = image_data->height;
			img_data->row_pointers = &image_data->row_pointers[n * image_data->height];
			img_data->data_ptr = NULL;
		}
	} else {
		for (int n = 0; n < numimage; n++) {
			if (!ReadPNG(pngnames[n], imagedata[n])) {
				CleanupImageData(imagedata, n - 1);
				return false;
			}
		}
	}

	celcmp_image_data *celdata = (celcmp_image_data*)malloc(sizeof(celcmp_image_data) * numimage);
	for (int n = 0; n < numimage; n++) {
		celdata[n].clipped = clipped;
		celdata[n].groupSize = groupSize;
	}
	return WritePNG2CelComp(imagedata, numimage, celdata, multi, celname, palette, numcolors, coloroffset);
}

struct cl2_image_data {
	DWORD dataSize;
	DWORD width;
	DWORD height;
	int groupSize;
	BYTE* data;
};
static bool WritePNG2Cl2(png_image_data *imagedata, int numimage, cl2_image_data* celdata, const char* celname, BYTE *palette, int numcolors, int coloroffset)
{
	const int RLE_LEN = 4; // number of matching colors to switch from bmp encoding to RLE

	// calculate header size
	int groupNum = 0;
	int headerSize = 0;
	for (int i = 0; i < numimage; ) {
		int ni = celdata[i].groupSize;
		headerSize += 4 + 4 * (ni + 1);
		i += ni;
		groupNum++;
	}
	if (groupNum > 1) {
		headerSize += sizeof(DWORD) * groupNum;
	}
	// estimate data size
	int maxsize = headerSize;
	for (int n = 0; n < numimage; n++) {
		png_image_data *image_data = &imagedata[n];
		maxsize += SUB_HEADER_SIZE;
		maxsize += image_data->height * (2 * image_data->width);
	}

	BYTE *buf = (BYTE *)malloc(maxsize);
	memset(buf, 0, maxsize);

	BYTE* hdr = buf;
	if (groupNum > 1) {
		// add optional {CL2 GROUP HEADER}
		int offset = groupNum * 4;
		for (int i = 0; i < groupNum; i++, hdr += 4) {
			*(DWORD*)&hdr[0] = offset;
			DWORD ni = celdata[i].groupSize;
			offset += 4 + 4 * (ni + 1);
		}
	}

	BYTE* pBuf = &buf[headerSize];
	int idx = 0;
	for (int i = 0; i < groupNum; i++) {
		DWORD ni = celdata[i].groupSize;
		*(DWORD*)&hdr[0] = SwapLE32(ni);
		*(DWORD*)&hdr[4] = SwapLE32(pBuf - hdr);

		for (int n = 0; n < ni; n++, idx++) {
			// convert one image to cl2-data
			png_image_data *image_data = &imagedata[idx];
			BYTE *pHeader = pBuf;
			// add CL2 FRAME HEADER
			pBuf[0] = SUB_HEADER_SIZE;
			pBuf[1] = 0x00;
			*(DWORD*)&pBuf[2] = 0;
			*(DWORD*)&pBuf[6] = 0;
			pBuf += SUB_HEADER_SIZE;

			BYTE *pHead = pBuf;
			BYTE col, lastCol;
			BYTE colMatches = 0;
			bool alpha = false;
			bool first = true;
			for (int i = 1; i <= image_data->height; i++) {
				RGBA* data = (RGBA*)image_data->row_pointers[image_data->height - i];
				if (i == 32 + 1) { // TODO: write more entries if necessary?
					pHead = pBuf;
					*(WORD*)(&pHeader[(i / 32) * 2]) = SwapLE16(pHead - pHeader);//pHead - buf - SUB_HEADER_SIZE;

					colMatches = 0;
					alpha = false;
					first = true;
				}
				for (int j = 0; j < image_data->width; j++) {
					if (data[j].a == 255) {
						// add opaque pixel
						// assert(image_data->fixColorMask == NULL);
						col = GetPalColor(data[j], palette, numcolors, coloroffset, 0);
						if (alpha || first || col != lastCol)
							colMatches = 1;
						else
							colMatches++;
						if (colMatches < RLE_LEN || (char)*pHead <= -127) {
							/*if (colMatches == RLE_LEN - 1 && pBuf - pHead == RLE_LEN - 1) {
								// RLE encode the whole 'line'
								memset(pBuf - (RLE_LEN - 2), 0, RLE_LEN - 2);
								*pHead += RLE_LEN - 2;
								if (*pHead != 0) {
									pHead = pBuf - (RLE_LEN - 2);
								}
								*pHead = -65 - (RLE_LEN - 2);
								pBuf = pHead + 1;
							} else*/ {
								// bmp encoding
								if (alpha || (char)*pHead <= -65) {
									pHead = pBuf;
									pBuf++;
									colMatches = 1;
								}
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
					first = false;
				}
			}
			*(DWORD*)&hdr[4 + 4 * (n + 1)] = SwapLE32(pBuf - hdr);
		}
		hdr += 4 + 4 * (ni + 1);
	}
	// write to file
	bool result = false;
	FILE *fp = fopen(celname, "wb");
	if (fp != NULL) {
		fwrite(buf, 1, pBuf - buf, fp);
		fclose(fp);
		result = true;
	}
	// cleanup
	free(buf);
	CleanupImageData(imagedata, numimage);
	free(celdata);
	return result;
}

bool PNG2Cl2(const char** pngnames, int numimage, int transform, const char* celname, BYTE *palette, int numcolors, int coloroffset)
{
	png_image_data *imagedata = (png_image_data*)malloc(sizeof(png_image_data) * numimage);
	for (int n = 0; n < numimage; n++) {
		if (!ReadPNG(pngnames[n], imagedata[n])) {
			CleanupImageData(imagedata, n - 1);
			return false;
		}
		if (transform & PNG_TRANSFORM_HFLIP)
			PNGFlip(imagedata[n], false);
		if (transform & PNG_TRANSFORM_VFLIP)
			PNGFlip(imagedata[n], true);
	}

	cl2_image_data* celdata = (cl2_image_data*)malloc(sizeof(cl2_image_data) * numimage);
	for (int n = 0; n < numimage; n++) {
		celdata[n].groupSize = numimage;
	}
	return WritePNG2Cl2(imagedata, numimage, celdata, celname, palette, numcolors, coloroffset);
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
	DWORD v = SwapLE32(srcCount + destCount);
	// write dummy header data (only the first entry is valid)
	for (int i = 0; i < v + 2; i++)
		fwrite(&v, 4, 1, frData);
	v = SwapLE32(v);

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
		DWORD cv = SwapLE32(curs);
		fwrite(&cv, 4, 1, frData);
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

static RGBA GetPNGColor(BYTE col, BYTE *palette, int coloroffset, int numfixcolors)
{
	RGBA result;

	if (col < coloroffset) {
		// assert(col == 0);
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

static void CelBlitSafe(RGBA *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth, int bufferWidth, BYTE *palette, int coloroffset, int numfixcolors)
{
	int i, w, BUFFER_WIDTH;
	char width;
	BYTE *src;
	RGBA* dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	w = nWidth;
	BUFFER_WIDTH = bufferWidth;

	for (; src != &pRLEBytes[nDataSize]; dst -= BUFFER_WIDTH + w) {
		for (i = w; i != 0;) {
			width = *src++;
			if (width >= 0) {
				i -= width;
				while (width-- != 0) {
					*dst = GetPNGColor(*src++, palette, coloroffset, numfixcolors);
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

static void Cl2BlitSafe(RGBA *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth, int bufferWidth, BYTE *palette, int coloroffset, int numfixcolors)
{
	int w, BUFFER_WIDTH;
	char width;
	BYTE fill;
	BYTE *src;
	RGBA *dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	w = nWidth;
	BUFFER_WIDTH = bufferWidth;

	while (nDataSize != 0) { 
		width = *src++;
		nDataSize--;
		if (width < 0) {
			width = -width;
			if (width > 65) {
				width -= 65;
				nDataSize--;
				RGBA fill = GetPNGColor(*src++, palette, coloroffset, numfixcolors);
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
					*dst = GetPNGColor(*src, palette, coloroffset, numfixcolors);
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

static bool CelValidWidth(int w, int pixels, std::map<int, int>& lineBreaks, std::map<int, int>& colorBreaks)
{
	// check if width is consistent with the number of pixels
	if (pixels % w != 0)
		return false;
	// check if every line-break is consistent with the width
	for (auto lit = lineBreaks.begin(); lit != lineBreaks.end(); ++lit) {
		if (lit->second % w != 0)
			return false;
	}
	// check if there is a break (line or color) at every block-end
	for (int i = w; i < pixels; i += w) {
		auto lit = lineBreaks.begin();
		for ( ; lit != lineBreaks.end(); ++lit) {
			if (lit->second == i)
				break;
		}
		if (lit != lineBreaks.end())
			continue;
		auto cit = colorBreaks.begin();
		for ( ; cit != colorBreaks.end(); ++cit) {
			if (cit->second == i)
				break;
		}
		if (cit == colorBreaks.end())
			return false;
	}
	return true;
}

/*
 * collect line breaks of a CEL-frame.
 * @param frameData: the BYTE array of the frame
 * @param frameLen: the length of the BYTE array
 * @param lineBreaks: the collected definite line break
 * @param colorBreaks: the collected possible line break
 * @param pixels: the number of pixels in the image
 * @return true if parsing is successful
 */
static bool CelCollectLineBreaks(BYTE* frameData, int frameLen, std::map<int, int>& lineBreaks, std::map<int, int>& colorBreaks, int* pixels)
{
	BYTE* frameEnd = &frameData[frameLen];
	BYTE* frameBegin = frameData;
	int counter = 0;
	char lastLen;
	bool alpha = false;

	while (frameData < frameEnd) {
		char len = *frameData;
		if (counter != 0) {
			if (len >= 0) {
				if (!alpha && lastLen != CHAR_MAX) {
					lineBreaks[(size_t)frameData - (size_t)frameBegin] = counter;
				} else {
					colorBreaks[(size_t)frameData - (size_t)frameBegin] = counter;
				}
				frameData += len + 1;
				counter += len;
			} else {
				if (alpha && lastLen != CHAR_MIN) {
					lineBreaks[(size_t)frameData - (size_t)frameBegin] = counter;
				} else {
					colorBreaks[(size_t)frameData - (size_t)frameBegin] = counter;
				}
				frameData++;
				counter -= len;
			}
		} else {
			if (len >= 0) {
				frameData += len + 1;
				counter += len;
			} else {
				frameData++;
				counter -= len;
			}
		}
		lastLen = len;
		alpha = len < 0;
	}
	*pixels = counter;
	return frameData == frameEnd;
}

static int CelGetFrameWidth(bool clipped, BYTE* frameData, int frameLen)
{
	int pixels;
	std::map<int, int> lineBreaks, colorBreaks;
	if (clipped) {
		if (!CelCollectLineBreaks(&frameData[SUB_HEADER_SIZE], frameLen - SUB_HEADER_SIZE, lineBreaks, colorBreaks, &pixels)) {
			return 0; // failed to parse
		}
		int offset = SwapLE16(*(WORD*)&frameData[2]);
		if (offset != 0 && offset > SUB_HEADER_SIZE) {
			offset -= SUB_HEADER_SIZE;
			int w;
			auto lit = lineBreaks.find(offset);
			if (lit != lineBreaks.end()) {
				w = lit->second;
			} else {
				auto cit = colorBreaks.find(offset);
				if (cit != colorBreaks.end())
					w = cit->second;
				else
					w = 1; // dummy value to skip the further testing
			}
			if (w % 32 == 0) {
				w /= 32;
				if (CelValidWidth(w, pixels, lineBreaks, colorBreaks))
					return w;
			}
		}
	} else {
		if (!CelCollectLineBreaks(&frameData[0], frameLen, lineBreaks, colorBreaks, &pixels)) {
			return 0; // failed to parse
		}
	}
	if (!lineBreaks.empty()) {
		lineBreaks[0] = 0;
		lineBreaks[frameLen] = pixels;
		// the length between two linebreaks could be the real width
		for (auto lit = lineBreaks.begin(); lit != lineBreaks.end(); lit++) {
			auto llit = lit;
			for (llit++; llit != lineBreaks.end(); llit++) {
				int w = (llit->second - lit->second);
				if (CelValidWidth(w, pixels, lineBreaks, colorBreaks)) {
					// check if there is a break in between as possible 'hidden' linebreak
					auto cit = colorBreaks.begin();
					for ( ; cit != colorBreaks.end(); cit++) {
						if (cit->second > lit->second)
							break; // skip preceeding color-breaks
					}
					auto ccit = cit;
					for ( ; cit != colorBreaks.end(); cit++) {
						if (cit->second > llit->second)
							continue; // skip color-breaks after the second linebreak
						int ww = std::min(llit->second - cit->second, cit->second - lit->second);
						if (CelValidWidth(ww, pixels, lineBreaks, colorBreaks))
							break;
					}
					if (cit == colorBreaks.end())
						return w;
				}
			}
		}
	}
	// find the smallest possible width
	for (int i = 1; i <= pixels / 2; i++) {
		if (CelValidWidth(i, pixels, lineBreaks, colorBreaks))
			return i;
	}
	return pixels;
}

static bool IsCelFrameClipped(BYTE* frameData, int frameLen)
{
	if (frameData[0] != SUB_HEADER_SIZE || frameData[1] != 0)
		return false; // wrong header
	if (frameLen <= SUB_HEADER_SIZE)
		return false; // not enough data
	int pixels;
	std::map<int, int> lineBreaks, colorBreaks;
	if (!CelCollectLineBreaks(&frameData[0], frameLen, lineBreaks, colorBreaks, &pixels))
		return true; // can not be parsed as a non-clipped frame
	lineBreaks.clear(); colorBreaks.clear();
	if (!CelCollectLineBreaks(&frameData[SUB_HEADER_SIZE], frameLen - SUB_HEADER_SIZE, lineBreaks, colorBreaks, &pixels))
		return false; // can not be parsed as a clipped frame
	// validate CEL FRAME HEADER
	int lastOffset = 0; int nWidth = 0;
	for (int i = 1; i < SUB_HEADER_SIZE / 2; i++) {
		int offset = SwapLE16(*(WORD*)&frameData[i * 2]);
		if (offset == 0)
			break;
		if (offset >= frameLen)
			return false; // invalid header
		offset = offset - SUB_HEADER_SIZE;
		if (offset <= lastOffset)
			return false; // invalid header
		int w;
		auto lit = lineBreaks.find(offset);
		if (lit != lineBreaks.end()) {
			w = lit->second;
		} else {
			auto cit = colorBreaks.find(offset);
			if (cit == colorBreaks.end())
				return false; // no line break at the specified location
			w = cit->second;
		}
		if (w % (32 * i) != 0)
			return false; // line break at a wrong location
		w /= 32 * i;
		if (nWidth != w && nWidth != 0)
			return false; // mismatching widths
		if (w == 0 || pixels % w != 0)
			return false; // wrong width
		nWidth = w;
		lastOffset = offset;
	}
	if (nWidth != 0) {
		return CelValidWidth(nWidth, pixels, lineBreaks, colorBreaks);
	}
	// commented out, because pixels as width is always valid
	//nWidth = CelGetFrameWidth(true, frameData, frameLen);
	//return nWidth != 0;
	return true;
}

static cel_image_data* ReadCelData(const char* celname, int* nImage, BYTE** oBuf)
{
	FILE *f = fopen(celname, "rb");

	if (f == NULL)
		return false;

	// read the file into memory
	DWORD numimage;
	fread(&numimage, 4, 1, f);
	numimage = SwapLE32(numimage);

	int headerSize = 4 + 4 + 4 * numimage;
	cel_image_data *celdata = (cel_image_data *)malloc(sizeof(cel_image_data) * (numimage + 1));
	DWORD dataSize;
	for (int i = 0; i <= numimage; i++) {
		fread(&dataSize, 4, 1, f);
		dataSize = SwapLE32(dataSize);
		celdata[i].dataSize = dataSize;
	}

	BYTE *buf = (BYTE *)malloc(dataSize);
	fread(buf, 1, dataSize - headerSize, f);

	fclose(f);

	// prepare celdata info
	BYTE *src = buf;
	for (int i = 0; i < numimage; i++) {
		celdata[i].dataSize = celdata[i + 1].dataSize - celdata[i].dataSize;
		celdata[i].clipped = IsCelFrameClipped(src, celdata[i].dataSize);
		celdata[i].width = CelGetFrameWidth(celdata[i].clipped, src, celdata[i].dataSize);
		// skip optional {CEL FRAME HEADER}
		if (celdata[i].clipped) {
			src += SUB_HEADER_SIZE;
			celdata[i].dataSize -= SUB_HEADER_SIZE;
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
		int nWidth = celdata[i].width;
		if (nWidth == 0 || pixels % nWidth != 0) {
			free(buf);
			free(celdata);
			return false;
		}
		celdata[i].height = pixels / nWidth;
	}

	*nImage = numimage;
	*oBuf = buf;
	return celdata;
}

/*
 * Convert a regular CEL file to PNG(s)
 * @param celname: the path of the CEL file
 * @param nCel: the frame which should be exported. 0 to export the whole content of the CEL file
 * @param multi:
 *        false: each frame is written to a separate png file
 *        true: the whole content is written to a single png file
 * @param destFolder: the output folder
 * @param palette: the palette to be used
 * @param coloroffset: the offset to be applied when selecting a color from the palette
 * @return true if the function succeeds
 */
bool Cel2PNG(const char* celname, int nCel, bool multi, const char* destFolder, BYTE *palette, int coloroffset)
{
	int numimage;
	BYTE* buf;
	cel_image_data* celdata = ReadCelData(celname, &numimage, &buf);
	if (celdata == NULL)
		return false;

	// write the png(s)
	if (multi) {
		// all in one
		//  find the required width
		DWORD width = 0;
		for (int i = 0; i < numimage; i++) {
			width = std::max(celdata[i].width, width);
		}
		// blit the frames to png_image_data
		png_image_data imagedata;
		imagedata.width = width;
		imagedata.height = 0;
		imagedata.data_ptr = NULL;
		imagedata.row_pointers = NULL;
		for (int i = 0; i < numimage; i++) {
			// prepare pngdata
			imagedata.height += celdata[i].height;
			imagedata.data_ptr = (png_bytep)realloc(imagedata.data_ptr, sizeof(RGBA) * imagedata.height * imagedata.width);
			imagedata.row_pointers = (png_bytep*)realloc(imagedata.row_pointers, imagedata.height * sizeof(void*));
			RGBA *imagerows = (RGBA *)imagedata.data_ptr;
			for (int n = 0; n < imagedata.height; n++) {
				imagedata.row_pointers[n] = (png_bytep)&imagerows[imagedata.width * n];
			}
			RGBA* lastLine = (RGBA*)imagedata.row_pointers[imagedata.height - 1];
			//lastLine += imagedata.width * (imagedata.height - 1);
			CelBlitSafe(lastLine, celdata[i].data, celdata[i].dataSize, celdata[i].width, imagedata.width, palette, coloroffset, 0);
			// imagedata.fixColorMask = NULL;
		}

		// write a single png
		char destFile[256];
		int idx = strlen(celname) - 1;
		while (idx > 0 && celname[idx] != '\\' && celname[idx] != '/')
			idx--;
		int fnc = snprintf(destFile, 246, "%s%s", destFolder, &celname[idx + 1]);
		snprintf(&destFile[fnc - 3], 10, "png");

		if (!WritePNG(destFile, imagedata)) {
			free(imagedata.row_pointers);
			free(imagedata.data_ptr);
			free(buf);
			free(celdata);
			return false;
		}
		free(imagedata.row_pointers);
		free(imagedata.data_ptr);
	} else {
		// one png per frame
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
				CelBlitSafe(lastLine, celdata[i].data, celdata[i].dataSize, imagedata.width, imagedata.width, palette, coloroffset, 0);
				// imagedata.fixColorMask = NULL;

				// write a single png
				char destFile[256];
				int idx = strlen(celname) - 1;
				while (idx > 0 && celname[idx] != '\\' && celname[idx] != '/')
					idx--;
				int fnc = snprintf(destFile, 236, "%s%s", destFolder, &celname[idx + 1]);
				destFile[fnc - 4] = '_';
				snprintf(&destFile[fnc], 20, "_frame%04d.png", i);

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
	}
	// cleanup
	free(buf);
	free(celdata);
	return true;
}

static size_t GetFileSize(const char* filename)
{
    struct stat stat_buf;
    int rc = stat(filename, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

static celcmp_image_data* ReadCelCompData(const char* celname, int* nImage, BYTE** oBuf)
{
	FILE *f = fopen(celname, "rb");
	if (f == NULL)
		return NULL;

	// read the file into memory
	DWORD numGroups;
	fread(&numGroups, 4, 1, f);
	numGroups = SwapLE32(numGroups);
	numGroups /= 4;

	DWORD* groupOffsets = (DWORD*)malloc(sizeof(DWORD) * numGroups);

	groupOffsets[0] = 0;
	for (int i = 1; i < numGroups; i++) {
		fread(&groupOffsets[i], 4, 1, f);
		groupOffsets[i] = SwapLE32(groupOffsets[i]) - 4 * numGroups;
	}

	DWORD dataSize = GetFileSize(celname);

	BYTE *buf = (BYTE *)malloc(dataSize - numGroups * 4);
	fread(buf, 1, dataSize - numGroups * 4, f);

	fclose(f);
	celcmp_image_data *celdata = NULL;
	DWORD numimage = 0;
	for (int n = 0; n < numGroups; n++) {
		BYTE *src = buf + groupOffsets[n];
		DWORD ni = SwapLE32(*(DWORD*)src);
		src += 4;
		int headerSize = 4 + 4 + 4 * ni;
		DWORD imgOffset = numimage;
		numimage += ni;
		celdata = (celcmp_image_data *)realloc(celdata, sizeof(cel_image_data) * (numimage + 1));
		DWORD dataSize;
		for (int i = imgOffset; i <= imgOffset + ni; i++) {
			dataSize = SwapLE32(*(DWORD*)src);
			src += 4;
			celdata[i].dataSize = dataSize;
		}

		// prepare celdata info
		for (int i = imgOffset; i < imgOffset + ni; i++) {
			// celdata[i].width = nWidth;
			celdata[i].dataSize = celdata[i + 1].dataSize - celdata[i].dataSize;
			// skip optional {CEL FRAME HEADER}
			celdata[i].clipped = IsCelFrameClipped(src, celdata[i].dataSize);
			celdata[i].width = CelGetFrameWidth(celdata[i].clipped, src, celdata[i].dataSize);
			if (celdata[i].clipped) {
				src += SUB_HEADER_SIZE;
				celdata[i].dataSize -= SUB_HEADER_SIZE;
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
				free(groupOffsets);
				free(buf);
				free(celdata);
				return NULL;
			}
			int nWidth = celdata[i].width;
			if (nWidth == 0 || pixels % nWidth != 0) {
				free(groupOffsets);
				free(buf);
				free(celdata);
				return NULL;
			}
			celdata[i].height = pixels / nWidth;
		}
	}
	free(groupOffsets);
	*nImage = numimage;
	*oBuf = buf;
	return celdata;
}

/*
 * Convert a compiled CEL file to PNG(s)
 * @param celname: the path of the CEL file
 * @param nCel: the frame which should be exported. 0 to export the whole content of the CEL file
 * @param multi:
 *        0: each frame is written to a separate png file
 *        1: each group is written to a separate png file
 *        2: the whole content is written to a single png file
 * @param destFolder: the output folder
 * @param palette: the palette to be used
 * @param coloroffset: the offset to be applied when selecting a color from the palette
 * @return true if the function succeeds
 */
bool CelComp2PNG(const char* celname, int nCel, int multi, const char* destFolder, BYTE *palette, int coloroffset)
{
	int numimage;
	BYTE* buf;
	celcmp_image_data* celdata = ReadCelCompData(celname, &numimage, &buf);
	if (celdata == NULL)
		return false;

	// write the png(s)
	if (multi) {
		if (multi == 1) {
			// one png per group
			int groupIdx = 0;
			for (int i = 0; i < numimage; groupIdx++) {
				// find out the required width of the group
				DWORD width = 0;
				for (int n = i; n < i + celdata[i].groupSize; n++) {
					width = std::max(celdata[n].width, width);
				}

				// blit the frames to png_image_data
				png_image_data imagedata;
				imagedata.width = width;
				imagedata.height = 0;
				imagedata.data_ptr = NULL;
				imagedata.row_pointers = NULL;

				for (int j = i; j < i + celdata[i].groupSize; j++) {
					// prepare pngdata
					imagedata.height += celdata[j].height;
					imagedata.data_ptr = (png_bytep)realloc(imagedata.data_ptr, sizeof(RGBA) * imagedata.height * imagedata.width);
					imagedata.row_pointers = (png_bytep*)realloc(imagedata.row_pointers, imagedata.height * sizeof(void*));
					RGBA *imagerows = (RGBA *)imagedata.data_ptr;
					for (int n = 0; n < imagedata.height; n++) {
						imagedata.row_pointers[n] = (png_bytep)&imagerows[imagedata.width * n];
					}
					RGBA* lastLine = (RGBA*)imagedata.row_pointers[imagedata.height - 1];
					//lastLine += imagedata.width * (imagedata.height - 1);
					CelBlitSafe(lastLine, celdata[j].data, celdata[j].dataSize, celdata[j].width, imagedata.width, palette, coloroffset, 0);
					// imagedata.fixColorMask = NULL;
				}

				// write a single png
				char destFile[256];
				int idx = strlen(celname) - 1;
				while (idx > 0 && celname[idx] != '\\' && celname[idx] != '/')
					idx--;
				int fnc = snprintf(destFile, 246, "%s%s", destFolder, &celname[idx + 1]);
				snprintf(&destFile[fnc - 3], 10, "_%02d.png", groupIdx);

				if (!WritePNG(destFile, imagedata)) {
					free(imagedata.row_pointers);
					free(imagedata.data_ptr);
					free(buf);
					free(celdata);
					return false;
				}
				free(imagedata.row_pointers);
				free(imagedata.data_ptr);

				i += celdata[i].groupSize;
			}
		} else {
			// one png per cel
			//  find the required width
			DWORD width = 0;
			for (int i = 0; i < numimage; i++) {
				width = std::max(celdata[i].width, width);
			}
			// blit the frames to png_image_data
			png_image_data imagedata;
			imagedata.width = width;
			imagedata.height = 0;
			imagedata.data_ptr = NULL;
			imagedata.row_pointers = NULL;
			for (int i = 0; i < numimage; i++) {
				// prepare pngdata
				imagedata.height += celdata[i].height;
				imagedata.data_ptr = (png_bytep)realloc(imagedata.data_ptr, sizeof(RGBA) * imagedata.height * imagedata.width);
				imagedata.row_pointers = (png_bytep*)realloc(imagedata.row_pointers, imagedata.height * sizeof(void*));
				RGBA *imagerows = (RGBA *)imagedata.data_ptr;
				for (int n = 0; n < imagedata.height; n++) {
					imagedata.row_pointers[n] = (png_bytep)&imagerows[imagedata.width * n];
				}
				RGBA* lastLine = (RGBA*)imagedata.row_pointers[imagedata.height - 1];
				//lastLine += imagedata.width * (imagedata.height - 1);
				CelBlitSafe(lastLine, celdata[i].data, celdata[i].dataSize, celdata[i].width, imagedata.width, palette, coloroffset, 0);
				// imagedata.fixColorMask = NULL;
			}

			// write a single png
			char destFile[256];
			int idx = strlen(celname) - 1;
			while (idx > 0 && celname[idx] != '\\' && celname[idx] != '/')
				idx--;
			int fnc = snprintf(destFile, 246, "%s%s", destFolder, &celname[idx + 1]);
			snprintf(&destFile[fnc - 3], 10, "png");

			if (!WritePNG(destFile, imagedata)) {
				free(imagedata.row_pointers);
				free(imagedata.data_ptr);
				free(buf);
				free(celdata);
				return false;
			}
			free(imagedata.row_pointers);
			free(imagedata.data_ptr);
		}
	} else {
		// one png per frame
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
				CelBlitSafe(lastLine, celdata[i].data, celdata[i].dataSize, imagedata.width, imagedata.width, palette, coloroffset, 0);
				// imagedata.fixColorMask = NULL;

				// write a single png
				char destFile[256];
				int idx = strlen(celname) - 1;
				while (idx > 0 && celname[idx] != '\\' && celname[idx] != '/')
					idx--;
				int fnc = snprintf(destFile, 236, "%s%s", destFolder, &celname[idx + 1]);
				destFile[fnc - 4] = '_';
				snprintf(&destFile[fnc], 20, "_frame%04d.png", i);

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
	}
	// cleanup
	free(buf);
	free(celdata);
	return true;
}

static bool Cl2IsMono(const char* celname)
{
	bool result = true;
	int numimage;

	FILE* f = fopen(celname, "rb");
	if (f == NULL)
		return false;

	fread(&numimage, 4, 1, f);
	numimage = SwapLE32(numimage);
	// first offset of a non-mono CL2 must point to the end end of the group-header with size of 4 * numgroups
	if (numimage % 4 == 0) {
		result = false;
		int lastOffset, i;
		for (i = 0; i < numimage; i++) {
			int offset;
			if (fread(&offset, 4, 1, f) != 1) {
				break;
			}
			offset = SwapLE32(offset);
			if (i == 0) {
				// first offset must point to the end of the header
				if (offset != 4 + 4 * (numimage + 1))
					break;
			} else {
				// subsequent offsets must be greater than the previous one
				if (offset <= lastOffset)
					break;
			}
			lastOffset = offset;
		}
		if (i == numimage) {
			// last entry in the header must match the file-size
			int fileSize;
			if (fread(&fileSize, 4, 1, f) == 1) {
				fileSize = SwapLE32(fileSize);
				result = fileSize == GetFileSize(celname);
			}
		}
	}

	fclose(f);
	return result;
}

static cl2_image_data* ReadCl2Data(const char* celname, int* nImage, BYTE** oBuf)
{
	bool mono = Cl2IsMono(celname);

	FILE *f = fopen(celname, "rb");

	if (f == NULL)
		return NULL;

	// read the file into memory
	DWORD numimage, dataSize;
	int headerSize, groupSize;
	cl2_image_data *celdata;
	DWORD* groupOffsets;
	if (!mono) {
		// CL2 with groups
		fread(&headerSize, 4, 1, f);
		headerSize = SwapLE32(headerSize);
		fseek(f, headerSize, SEEK_SET);
		numimage = 0;
		groupSize = headerSize / 4;
		groupOffsets = (DWORD*)malloc(groupSize * sizeof(DWORD));
		for (int i = 0; i < groupSize; i++) {
			fseek(f, i * 4, SEEK_SET);

			DWORD offset;
			fread(&offset, 4, 1, f);
			offset = SwapLE32(offset);
			groupOffsets[i] = offset - groupSize * 4;
			fseek(f, offset, SEEK_SET);

			DWORD ni;
			fread(&ni, 4, 1, f);
			ni = SwapLE32(ni);
			numimage += ni;

			fseek(f, ni * 4, SEEK_CUR);
			fread(&dataSize, 4, 1, f);
			dataSize = offset + SwapLE32(dataSize);
		}

		fseek(f, groupSize * 4, SEEK_SET);
	} else {
		// mono CL2 -> prepare fake group
		fread(&numimage, 4, 1, f);
		numimage = SwapLE32(numimage);
		groupSize = 1;
		groupOffsets = (DWORD*)malloc(groupSize * sizeof(DWORD));
		groupOffsets[0] = 0;
		fseek(f, numimage * 4, SEEK_CUR);
		fread(&dataSize, 4, 1, f);
		dataSize = SwapLE32(dataSize);

		fseek(f, 0, SEEK_SET);
	}

	BYTE *buf = (BYTE *)malloc(dataSize);
	fread(buf, 1, dataSize, f);

	fclose(f);

	celdata = (cl2_image_data *)malloc(sizeof(cl2_image_data) * (numimage + 1));
	// prepare celdata info
	int i = 0;
	for (int n = 0; n < groupSize; n++) {
		BYTE* cur = &buf[groupOffsets[n]];
		DWORD ni = *(DWORD*)cur;
		ni = SwapLE32(ni);
		BYTE *src = cur + SwapLE32(*(DWORD*)&cur[4]);
	for (int j = 0; j < ni; j++, i++) {
		celdata[i].data = src;
		celdata[i].dataSize = *(DWORD*)&cur[(j + 2) * 4] - *(DWORD*)&cur[(j + 1) * 4];
		//celdata[i].width = nWidth;
		celdata[i].width = 0;
		celdata[i].groupSize = ni;
		// skip frame-header
		WORD subHeaderSize = SwapLE16(*(WORD*)src);
		int blockOffset = SwapLE16(*(WORD*)&src[2]) - subHeaderSize;
		src += subHeaderSize;
		celdata[i].data += subHeaderSize;
		celdata[i].dataSize -= subHeaderSize;

		int pixels = 0;
		while (src < &celdata[i].data[celdata[i].dataSize]) {
			char width = *src++;
			if (width >= 0) {
				// transparent
				pixels += width;
				if (blockOffset != 0) {
					blockOffset -= 1;
					if (blockOffset == 0) {
						// calculate width based on https://github.com/savagesteel/d1-file-formats/blob/master/PC-Mac/CL2.md#41-cl2-frame-header
						celdata[i].width = pixels / 32;
					}
				}
			} else {
				width = -width;
				if (width > 65) {
					// fill
					pixels += width - 65;
					src++;
					if (blockOffset != 0) {
						blockOffset -= 2;
						if (blockOffset == 0) {
							celdata[i].width = pixels / 32;
						}
					}
				} else {
					// bmp
					pixels += width;
					src += width;
					if (blockOffset != 0) {
						blockOffset -= width + 1;
						if (blockOffset == 0) {
							celdata[i].width = pixels / 32;
						}
					}
				}
			}
		}
		if (src != &celdata[i].data[celdata[i].dataSize]) {
			free(buf);
			free(groupOffsets);
			free(celdata);
			return NULL;
		}
		int nWidth = celdata[i].width;
		if (nWidth == 0 || pixels % nWidth != 0) {
			free(buf);
			free(groupOffsets);
			free(celdata);
			return NULL;
		}
		celdata[i].height = pixels / nWidth;
	}
	}
	free(groupOffsets);

	*nImage = numimage;
	*oBuf = buf;
	return celdata;
}

/*
 * Convert a CL2 file to PNG(s)
 * @param celname: the path of the CL2 file
 * @param nCel: the frame which should be exported. 0 to export the whole content of the CL2 file
 * @param multi:
 *        0: each frame is written to a separate png file
 *        1: each group is written to a separate png file
 *        2: the whole content is written to a single png file
 * @param destFolder: the output folder
 * @param palette: the palette to be used
 * @param coloroffset: the offset to be applied when selecting a color from the palette
 * @return true if the function succeeds
 */
bool Cl2PNG(const char* celname, int nCel, int multi, const char* destFolder, BYTE *palette, int coloroffset)
{
	int numimage;
	BYTE* buf;
	cl2_image_data* celdata = ReadCl2Data(celname, &numimage, &buf);
	if (celdata == NULL)
		return false;

	// write the png(s)
	if (multi) {
		if (multi == 1) {
			// one per group
			int groupIdx = 0;
			for (int i = 0; i < numimage; groupIdx++) {
				// find out the required width of the group
				DWORD width = 0;
				for (int n = i; n < i + celdata[i].groupSize; n++) {
					width = std::max(celdata[n].width, width);
				}

				// blit the frames to png_image_data
				png_image_data imagedata;
				imagedata.width = width;
				imagedata.height = 0;
				imagedata.data_ptr = NULL;
				imagedata.row_pointers = NULL;

				for (int j = i; j < i + celdata[i].groupSize; j++) {
					// prepare pngdata
					imagedata.height += celdata[j].height;
					imagedata.data_ptr = (png_bytep)realloc(imagedata.data_ptr, sizeof(RGBA) * imagedata.height * imagedata.width);
					imagedata.row_pointers = (png_bytep*)realloc(imagedata.row_pointers, imagedata.height * sizeof(void*));
					RGBA *imagerows = (RGBA *)imagedata.data_ptr;
					for (int n = 0; n < imagedata.height; n++) {
						imagedata.row_pointers[n] = (png_bytep)&imagerows[imagedata.width * n];
					}
					RGBA* lastLine = (RGBA*)imagedata.row_pointers[imagedata.height - 1];
					//lastLine += imagedata.width * (imagedata.height - 1);
					Cl2BlitSafe(lastLine, celdata[j].data, celdata[j].dataSize, celdata[j].width, imagedata.width, palette, coloroffset, 0);
					// imagedata.fixColorMask = NULL;
				}

				// write a single png
				char destFile[256];
				int idx = strlen(celname) - 1;
				while (idx > 0 && celname[idx] != '\\' && celname[idx] != '/')
					idx--;
				int fnc = snprintf(destFile, 246, "%s%s", destFolder, &celname[idx + 1]);
				snprintf(&destFile[fnc - 3], 10, "_%02d.png", groupIdx);

				if (!WritePNG(destFile, imagedata)) {
					free(imagedata.row_pointers);
					free(imagedata.data_ptr);
					free(buf);
					free(celdata);
					return false;
				}
				free(imagedata.row_pointers);
				free(imagedata.data_ptr);

				i += celdata[i].groupSize;
			}
		} else {
			// all in one
			//  find the required width
			DWORD width = 0;
			for (int i = 0; i < numimage; i++) {
				width = std::max(celdata[i].width, width);
			}
			// blit the frames to png_image_data
			png_image_data imagedata;
			imagedata.width = width;
			imagedata.height = 0;
			imagedata.data_ptr = NULL;
			imagedata.row_pointers = NULL;
			for (int i = 0; i < numimage; i++) {
				// prepare pngdata
				imagedata.height += celdata[i].height;
				imagedata.data_ptr = (png_bytep)realloc(imagedata.data_ptr, sizeof(RGBA) * imagedata.height * imagedata.width);
				imagedata.row_pointers = (png_bytep*)realloc(imagedata.row_pointers, imagedata.height * sizeof(void*));
				RGBA *imagerows = (RGBA *)imagedata.data_ptr;
				for (int n = 0; n < imagedata.height; n++) {
					imagedata.row_pointers[n] = (png_bytep)&imagerows[imagedata.width * n];
				}
				RGBA* lastLine = (RGBA*)imagedata.row_pointers[imagedata.height - 1];
				//lastLine += imagedata.width * (imagedata.height - 1);
				Cl2BlitSafe(lastLine, celdata[i].data, celdata[i].dataSize, celdata[i].width, imagedata.width, palette, coloroffset, 0);
				// imagedata.fixColorMask = NULL;
			}

			// write a single png
			char destFile[256];
			int idx = strlen(celname) - 1;
			while (idx > 0 && celname[idx] != '\\' && celname[idx] != '/')
				idx--;
			int fnc = snprintf(destFile, 246, "%s%s", destFolder, &celname[idx + 1]);
			snprintf(&destFile[fnc - 3], 10, "png");

			if (!WritePNG(destFile, imagedata)) {
				free(imagedata.row_pointers);
				free(imagedata.data_ptr);
				free(buf);
				free(celdata);
				return false;
			}
			free(imagedata.row_pointers);
			free(imagedata.data_ptr);
		}
	} else {
		// one png per frame
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
				Cl2BlitSafe(lastLine, celdata[i].data, celdata[i].dataSize, imagedata.width, imagedata.width, palette, coloroffset, 0);
				// imagedata.fixColorMask = NULL;

				// write a single png
				char destFile[256];
				int idx = strlen(celname) - 1;
				while (idx > 0 && celname[idx] != '\\' && celname[idx] != '/')
					idx--;
				int fnc = snprintf(destFile, 236, "%s%s", destFolder, &celname[idx + 1]);
				destFile[fnc - 4] = '_';
				snprintf(&destFile[fnc], 20, "_frame%04d.png", i);

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

static RGBA Interpolate(RGBA* c0, RGBA* c1, int idx, int len, BYTE* palette, int numcolors, int coloroffset, int numfixcolors)
{
	if (c1->a != 255)
		return *c0; // preserve tranparent pixels

	RGBA res;
	res.a = 255;
	res.r = (c0->r * (len - idx) + c1->r * idx) / len;
	res.g = (c0->g * (len - idx) + c1->g * idx) / len;
	res.b = (c0->b * (len - idx) + c1->b * idx) / len;
	/*if (numfixcolors != 0 && palette != NULL) {
		// do not interpolate 'protected' colors
		BYTE col = GetPalColor(res, palette, numcolors, coloroffset);
		if (col != 0 && col < numfixcolors)
			return *c0;
	}*/
	return res;
}

static void UpscalePNGImages(png_image_data* imagedata, int numimage, int multiplier, BYTE* palette, int numcolors, int coloroffset, int numfixcolors)
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
		for (int y = 0; y < imagedata[i].height / multiplier - 1; y++) {
			RGBA* p0 = (RGBA*)imagedata[i].row_pointers[y * multiplier];
			for (int x = 0; x < imagedata[i].width / multiplier - 1; x++, p0 += multiplier) {
				if (p0->a != 255)
					continue; // skip transparent pixels
				// skip 'protected' colors
				if (imagedata[i].fixColorMask != NULL && imagedata[i].fixColorMask[x + y * imagedata[i].width] != 0)
					continue;

				RGBA* p1 = p0 + multiplier;
				for (int j = 0; j < multiplier; j++) {
					for (int k = 1; k < multiplier; k++) {
						RGBA* pp = p0 + j * imagedata[i].width + k;
						*pp = Interpolate(p0, p1, k, multiplier, palette, numcolors, coloroffset, numfixcolors);
					}
				}
					for (int k = 1; k < multiplier; k++) {
						RGBA* pp = p0 + k * imagedata[i].width;
						*pp = Interpolate(p0, p1, k, multiplier, palette, numcolors, coloroffset, numfixcolors);
					}
			}
		}
	}

	// cleanup memory
	CleanupImageData(orimg_data, numimage);
}

void UpscaleCel(const char* celname, int multiplier, BYTE* palette, int numcolors, int coloroffset, int numfixcolors, const char* resCelName)
{
	int numimage;
	BYTE* buf;
	cel_image_data* celdata = ReadCelData(celname, &numimage, &buf);
	if (celdata == NULL)
		return;

	// prepare pngdata
	png_image_data* imagedata = (png_image_data*)malloc(sizeof(png_image_data) * numimage);
	for (int i = 0; i < numimage; i++) {
		imagedata[i].width = celdata[i].width;
		imagedata[i].height = celdata[i].height;
		RGBA *imagerows = (RGBA *)malloc(sizeof(RGBA) * imagedata[i].height * imagedata[i].width);
		imagedata[i].row_pointers = (png_bytep*)malloc(imagedata[i].height * sizeof(void*));
		for (int n = 0; n < imagedata[i].height; n++) {
			imagedata[i].row_pointers[n] = (png_bytep)&imagerows[imagedata[i].width * n];
		}
		imagedata[i].data_ptr = (png_bytep)imagerows;
		RGBA* lastLine = (RGBA*)imagedata[i].row_pointers[imagedata[i].height - 1];
		RGBA* dst = &lastLine[0];
		CelBlitSafe(dst, celdata[i].data, celdata[i].dataSize, imagedata[i].width, imagedata[i].width, palette, coloroffset, numfixcolors);

		// prepare meta-info
		imagedata[i].fixColorMask = (BYTE*)malloc(sizeof(BYTE) * imagedata[i].height * imagedata[i].width);
		memset(imagedata[i].fixColorMask, 0, sizeof(BYTE) * imagedata[i].height * imagedata[i].width);
		BYTE* dstB = &imagedata[i].fixColorMask[(imagedata[i].height - 1) * imagedata[i].width];
		for (int y = 0; y < imagedata[i].height; y++) {
			for (int x = 0; x < imagedata[i].width; x++) {
				if (dst[x].a != 0 && dst[x].a != 255) {
					dstB[x] = dst[x].a;
					dst[x].a = 255;
				}
			}
			dstB -= imagedata[i].width;
			dst -= imagedata[i].width;
		}
	}

	free(buf);

	// upscale the png data
	UpscalePNGImages(imagedata, numimage, multiplier, palette, numcolors, coloroffset, numfixcolors);

	// convert pngs back to cel
	WritePNG2Cel(imagedata, numimage, celdata, false, resCelName, palette, numcolors, coloroffset, numfixcolors);
}

void UpscaleCelComp(const char* celname, int multiplier, BYTE* palette, int numcolors, int coloroffset, const char* resCelName)
{
	int numimage;
	BYTE* buf;
	celcmp_image_data* celdata = ReadCelCompData(celname, &numimage, &buf);
	if (celdata == NULL)
		return;

	// prepare pngdata
	png_image_data* imagedata = (png_image_data*)malloc(sizeof(png_image_data) * numimage);
	for (int i = 0; i < numimage; i++) {
		imagedata[i].width = celdata[i].width;
		imagedata[i].height = celdata[i].height;
		RGBA *imagerows = (RGBA *)malloc(sizeof(RGBA) * imagedata[i].height * imagedata[i].width);
		imagedata[i].row_pointers = (png_bytep*)malloc(imagedata[i].height * sizeof(void*));
		for (int n = 0; n < imagedata[i].height; n++) {
			imagedata[i].row_pointers[n] = (png_bytep)&imagerows[imagedata[i].width * n];
		}
		imagedata[i].data_ptr = (png_bytep)imagerows;
		RGBA* lastLine = (RGBA*)imagedata[i].row_pointers[imagedata[i].height - 1];
		//lastLine += imagedata.width * (imagedata.height - 1);
		// blit to the bottom right
		CelBlitSafe(&lastLine[0], celdata[i].data, celdata[i].dataSize, imagedata[i].width, imagedata[i].width, palette, coloroffset, 0);
		imagedata[i].fixColorMask = NULL;
	}

	free(buf);

	// upscale the png data
	UpscalePNGImages(imagedata, numimage, multiplier, NULL, 0, 0, 0);

	// convert pngs back to cel
	WritePNG2CelComp(imagedata, numimage, celdata, false, resCelName, palette, numcolors, coloroffset);
}

void UpscaleCl2(const char* celname, int multiplier, BYTE* palette, int numcolors, int coloroffset, const char* resCelName)
{
	int numimage;
	BYTE* buf;
	cl2_image_data* celdata = ReadCl2Data(celname, &numimage, &buf);
	if (celdata == NULL)
		return;

	png_image_data* imagedata = (png_image_data*)malloc(sizeof(png_image_data) * numimage);
	for (int i = 0; i < numimage; i++) {
		// prepare pngdata
		imagedata[i].width = celdata[i].width;
		imagedata[i].height = celdata[i].height;
		RGBA *imagerows = (RGBA *)malloc(sizeof(RGBA) * imagedata[i].height * imagedata[i].width);
		imagedata[i].row_pointers = (png_bytep*)malloc(imagedata[i].height * sizeof(void*));
		for (int n = 0; n < imagedata[i].height; n++) {
			imagedata[i].row_pointers[n] = (png_bytep)&imagerows[imagedata[i].width * n];
		}
		imagedata[i].data_ptr = (png_bytep)imagerows;
		RGBA* lastLine = (RGBA*)imagedata[i].row_pointers[imagedata[i].height - 1];
		Cl2BlitSafe(&lastLine[0], celdata[i].data, celdata[i].dataSize, imagedata[i].width, imagedata[i].width, palette, coloroffset, 0);
		imagedata[i].fixColorMask = NULL;
	}

	free(buf);

	// upscale the png data
	UpscalePNGImages(imagedata, numimage, multiplier, NULL, 0, 0, 0);

	// convert pngs back to cl2
	WritePNG2Cl2(imagedata, numimage, celdata, resCelName, palette, numcolors, coloroffset);
}

int main()
{
	/*{ // upscale regular cel files of listfiles.txt
		- fails if the output-folder structure is not prepared
		- skips Levels(dungeon tiles), gendata(cutscenes) and cow.CEL manually
	// #include <fstream>
	std::ifstream input("f:\\listfiles.txt");

	std::string line;
	while (std::getline(input, line)) {
		size_t ls = line.size();
		if (ls <= 4)
			continue;
		if (line[0] == '_')
			continue;
		if (!stringicomp(line.substr(line.length() - 4, 4).c_str(), ".CEL"))
			continue;
		if (ls > 7 && stringicomp(line.substr(0, 7).c_str(), "gendata"))
			continue;
		if (ls > 7 && stringicomp(line.substr(0, 6).c_str(), "Levels"))
			continue;
		if (ls > 7 && stringicomp(line.substr(1, 6).c_str(), "Levels"))
			continue;
		if (ls > 7 && stringicomp(line.substr(line.length() - 7, 7).c_str(), "cow.CEL"))
			continue;

		char path[256];
		snprintf(path, 256, "F:\\MPQE\\Work\\%s", line.c_str());
		char outpath[256];
		snprintf(outpath, 256, "F:\\outcel\\%s", line.c_str());
		UpscaleCel(path, 2, &diapal[0][0], 128, 128, 0, outpath);
	}
	input.close();
	}*/
	/*{ // upscale objects with level-specific palette (fails if the output-folder structure is not prepared)
		const char* celPalPairs[][4] = {
			// celname,				  palette,				  numcolors, numfixcolors (protected colors)
			{ "Objects\\L1Doors.CEL", "Levels\\L1Data\\L1_1.PAL", "128", "0" },
			{ "Objects\\L2Doors.CEL", "Levels\\L2Data\\L2_1.PAL", "128", "0" },
			{ "Objects\\L3Doors.CEL", "Levels\\L3Data\\L3_1.PAL", "128", "32" },
			{ "Objects\\L5Door.CEL", "NLevels\\L5Data\\L5base.PAL", "128", "32" },
			{ "Objects\\L5Books.CEL", "NLevels\\L5Data\\L5base.PAL", "256", "32" },
			{ "Objects\\L5Lever.CEL", "NLevels\\L5Data\\L5base.PAL", "128", "32" },
			{ "Objects\\L5Light.CEL", "NLevels\\L5Data\\L5base.PAL", "128", "32" },
			{ "Objects\\L5Sarco.CEL", "NLevels\\L5Data\\L5base.PAL", "256", "32" },
			{ "Objects\\Urnexpld.CEL", "NLevels\\L5Data\\L5base.PAL", "256", "32" },
			{ "Objects\\Urn.CEL", "NLevels\\L5Data\\L5base.PAL", "256", "32" },
		};
		for (int i = 0; i < lengthof(celPalPairs); i++) {
			char path[256];
			snprintf(path, 256, "f:\\MPQE\\Work\\%s", celPalPairs[i][1]);
			BYTE* pal = LoadPal(path);
			snprintf(path, 256, "f:\\MPQE\\Work\\%s", celPalPairs[i][0]);
			char outpath[256];
			snprintf(outpath, 256, "F:\\outcel\\%s", celPalPairs[i][0]);
			UpscaleCel(path, 2, pal, atoi(celPalPairs[i][2]), 0, atoi(celPalPairs[i][3]), outpath);
			free(pal);
		}
	}*/
	/*{ // upscale special cells of the levels (fails if the output-folder structure is not prepared)
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\Levels\\TownData\\Town.PAL");
		UpscaleCel("f:\\MPQE\\Work\\Levels\\TownData\\TownS.CEL", 2, pal, 128, 0, 0,
			"f:\\outcel\\Levels\\TownData\\TownS.CEL");
		free(pal);
	}
	{
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\Levels\\L1Data\\L1_1.PAL");
		UpscaleCel("f:\\MPQE\\Work\\Levels\\L1Data\\L1S.CEL", 2, pal, 128, 0, 0,
			"f:\\outcel\\Levels\\L1Data\\L1S.CEL");
		free(pal);
	}
	{
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\Levels\\L2Data\\L2_1.PAL");
		UpscaleCel("f:\\MPQE\\Work\\Levels\\L2Data\\L2S.CEL", 2, pal, 128, 0, 0,
			"f:\\outcel\\Levels\\L2Data\\L2S.CEL");
		free(pal);
	}
	{
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\NLevels\\L5Data\\L5base.PAL");
		UpscaleCel("f:\\MPQE\\Work\\NLevels\\L5Data\\L5S.CEL", 2, pal, 128, 0, 32, 
			"f:\\outcel\\NLevels\\L5Data\\L5S.CEL");
		free(pal);
	}*/
	/*{ // upscale cutscenes
		const char* celPalPairs[][2] = {
			{ "Gendata\\Cut2.CEL", "Gendata\\Cut2.pal" },
			{ "Gendata\\Cut3.CEL", "Gendata\\Cut3.pal" },
			{ "Gendata\\Cut4.CEL", "Gendata\\Cut4.pal" },
			{ "Gendata\\Cutgate.CEL", "Gendata\\Cutgate.pal" },
			{ "Gendata\\Cutl1d.CEL", "Gendata\\Cutl1d.pal" },
			{ "Gendata\\Cutportl.CEL", "Gendata\\Cutportl.pal" },
			{ "Gendata\\Cutportr.CEL", "Gendata\\Cutportr.pal" },
			{ "Gendata\\Cutstart.CEL", "Gendata\\Cutstart.pal" },
			{ "Gendata\\Cuttt.CEL", "Gendata\\Cuttt.pal" },
			{ "NLevels\\CutL5.CEL", "NLevels\\CutL5.pal" },
			{ "NLevels\\CutL6.CEL", "NLevels\\CutL6.pal" },
		};
		for (int i = 0; i < lengthof(celPalPairs); i++) {
			char path[256];
			snprintf(path, 256, "f:\\MPQE\\Work\\%s", celPalPairs[i][1]);
			BYTE* pal = LoadPal(path);
			snprintf(path, 256, "f:\\MPQE\\Work\\%s", celPalPairs[i][0]);
			char outpath[256];
			snprintf(outpath, 256, "F:\\outcel\\%s", celPalPairs[i][0]);
			UpscaleCel(path, 2, pal, 256, 0, 0, outpath);
			free(pal);
		}
	}*/
	// UpscaleCelComp("F:\\MPQE\\Work\\towners\\animals\\cow.CEL", 1, &diapal[0][0], 128, 128, "F:\\outcel\\towners\\animals\\cow.cel");
	/*{ // upscale non-standard CELs of the menu (converted from PCX)
		const char* menuCELs[] = {
			"ui_art\\mainmenu.CEL", "ui_art\\title.CEL", "ui_art\\logo.CEL", "ui_art\\smlogo.CEL"
		};
		BYTE* pal = LoadPal("f:\\Diablo\\Work\\ui_art\\menu.PAL");
		for (int i = 0; i < lengthof(menuCELs); i++) {
			char path[256];
			snprintf(path, 256, "f:\\Diablo\\Work\\%s", menuCELs[i]);
			char outpath[256];
			snprintf(outpath, 256, "f:\\outcel\\%s", menuCELs[i]);
			UpscaleCel(path, 2, pal, 256, 0, 0, outpath);
		}
		free(pal);

		const char* diaCELs[] = { "ui_art\\black.CEL", "ui_art\\heros.CEL",
			"ui_art\\selconn.CEL", "ui_art\\selgame.CEL", "ui_art\\selhero.CEL",
			// "ui_art\\focus.CEL", "ui_art\\focus16.CEL", "ui_art\\focus42.CEL",
			// "ui_art\\lrpopup.CEL", "ui_art\\spopup.CEL", "ui_art\\srpopup.CEL", "ui_art\\smbutton.CEL"
			// "ui_art\\prog_bg.CEL", "ui_art\\prog_fil.CEL",
			// "ui_art\\sb_arrow.CEL", "ui_art\\sb_bg.CEL", "ui_art\\sb_thumb.CEL",
		};
		for (int i = 0; i < lengthof(diaCELs); i++) {
			char path[256];
			snprintf(path, 256, "f:\\Diablo\\Work\\%s", diaCELs[i]);
			char outpath[256];
			snprintf(outpath, 256, "f:\\outcel\\%s", diaCELs[i]);
			UpscaleCel(path, 2, &diapal[0][0], 128, 128, 0, outpath);
		}

		pal = LoadPal("f:\\Diablo\\Work\\ui_art\\credits.PAL");
		UpscaleCel("f:\\Diablo\\Work\\ui_art\\credits.CEL", 2, pal, 256, 0, 0,
			"f:\\outcel\\ui_art\\credits.CEL");
		free(pal);
	}*/
	/*{ // upscale all cl2 files of listfiles.txt (fails if the output-folder structure is not prepared)
	// #include <fstream>
		std::ifstream input("f:\\listfiles.txt");

		std::string line;
		while (std::getline(input, line)) {
			size_t ls = line.size();
			if (ls <= 4)
				continue;
			if (line[0] == '_')
				continue;
			if (!stringicomp(line.substr(line.length() - 4, 4).c_str(), ".CL2"))
				continue;

			char path[256];
			snprintf(path, 256, "f:\\MPQE\\Work\\%s", line.c_str());
			char outpath[256];
			snprintf(outpath, 256, "f:\\outcl2\\%s", line.c_str());
			UpscaleCl2(path, 2, &diapal[0][0], 128, 128, outpath);
		}
		input.close();
	}*/

	//UpscaleCl2("f:\\plrgfx\\rogue\\rhbat.CL2", 2, &diapal[0][0], 128, 128, "f:\\rhbat.CL2");

	//Cl2PNG("f:\\plrgfx\\rogue\\rhbat.CL2", 0, 1, "f:\\", &diapal[0][0], 128);
	//Cl2PNG("f:\\Farrow1.CL2", 0, 0, "f:\\", &diapal[0][0], 128);

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

	Cel2PNG("f:\\MPQE\\Work\\data\\char.CEL", 0, false, "f:\\", &diapal[0][0], 128);
	CelComp2PNG("f:\\MPQE\\Work\\towners\\animals\\cow.CEL", 0, 2, "f:\\", &diapal[0][0], 128);

	Cel2PNG("f:\\SpellBkB.CEL", 0, false, "f:\\", &diapal[0][0], 128);

	const char* filenames[] = { "f:\\Farrow1.png" };
	PNG2Cl2(filenames, 1, PNG_TRANSFORM_IDENTITY, "f:\\Farrow1.CL2", &diapal[0][0], 128, 128);

	/*{ // sample code to patch a sol file
		FILE *f = fopen("f:\\town.sol", "rb");
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
	}*/
	/*{ // sample code to convert CEL to PNGs, then convert the PNGs back to CEL
		BYTE* pal = LoadPal("f:\\L1_1.PAL");
		Cel2PNG("f:\\L1Doors.CEL", 0, false, "f:\\", pal, 256);
		const char* doors[] = {
			"f:\\L1Doors_CEL_frame0000.png",
			"f:\\L1Doors_CEL_frame0001.png",
			"f:\\L1Doors_CEL_frame0002.png",
			"f:\\L1Doors_CEL_frame0003.png"};
		PNG2Cel(doors, 4, false, "f:\\L1Doors_per.CEL", true, pal, 256, 0);
		free(pal);
	}*/
	/*{ // sample code to convert CEL to a PNG with multiple frames, then convert the PNG back to CEL
		BYTE* pal = LoadPal("f:\\L1_1.PAL");
		Cel2PNG("f:\\L1Doors.CEL", 0, true, "f:\\", pal, 256);
		const char* doors[] = {
			"f:\\L1Doors.png" };
		PNG2Cel(doors, 4, true, "f:\\L1Doors_mul.CEL", true, pal, 256, 0);
		free(pal);
	}*/
	/*{ // sample code to convert compiled CEL to a PNG with multiple frames, then convert the PNG back to CEL
		CelComp2PNG("f:\\MPQE\\Work\\towners\\animals\\cow.CEL", 0, 2, "f:\\", &diapal[0][0], 128);
		const char* pnames[] = { "f:\\cow.png" };
		PNG2CelComp(pnames, 8 * 12, true, "f:\\cow_out.CEL", true, 12, &diapal[0][0], 128, 128);
	}*/
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