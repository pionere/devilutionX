#include <png.h>
#include <zlib.h>

typedef unsigned int DWORD;
typedef unsigned char BYTE;

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

static BYTE GetColor(RGBA &data, BYTE *palette, int numcolors)
{
	BYTE res = 0;
	int best = INT_MAX;

	for (int i = 0; i < numcolors; i++, palette += 3) {
		int dist = abs(data.r - palette[0]) + 
				   abs(data.g - palette[1]) + 
				   abs(data.b - palette[2]);
		if (dist < best) {
			best = dist;
			res = i;
		}
	}

	return res;
}

static bool PNG2Cel(const char *pngname, const char *celname)
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

    if (!png_ptr)
       return false;

	png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
       png_destroy_read_struct(&png_ptr,
           (png_infopp)NULL, (png_infopp)NULL);
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

	png_uint_32 width, height;
	png_int_32 bit_depth, color_type, interlace_type, compression_type, filter_method;
	png_get_IHDR(png_ptr, info_ptr, &width, &height,
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

	png_bytep buffer = (png_bytep)png_malloc(png_ptr, height * width * 4);

	png_bytep *row_pointers = (png_bytep*)malloc(height * sizeof(void*));
	for (int i = 0; i < height; i++)
		row_pointers[i] = buffer + i * width * 4;

	png_read_image(png_ptr, row_pointers);

	png_read_end(png_ptr, (png_infop)NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr,
       (png_infopp)NULL);

	// convert to cel
	BYTE *buf = (BYTE *)malloc(12 + height * (2 * width));
	memset(buf, 0, 12 + height * (2 * width));
	buf[0] = 1;
	buf[4] = 0xC;
	BYTE *pBuf = &buf[12];
	BYTE *pHead;
	for (int i = 1; i <= height; i++) {
		pHead = pBuf;
		pBuf++;
		bool alpha = false;
		RGBA* data = (RGBA*)row_pointers[height - i];
		for (int j = 0; j < width; j++) {
			if (data[j].a == 255) {
				if (alpha || *pHead >= 126) {
					pHead = pBuf;
					pBuf++;
				}
				++*pHead;
				*pBuf = GetColor(data[j], &diapal[0][0], 128) + 128;
				pBuf++;
				alpha = false;
			} else {
				if ((!alpha && j != 0) || (char)*pHead <= -127) {
					pHead = pBuf;
					pBuf++;
				}
				--*pHead;
				alpha = true;
			}
		}
	}
	*(DWORD*)&buf[8] = pBuf - buf;
	fp = fopen(celname, "wb");
	fwrite(buf, 1, pBuf - buf, fp);
	fclose(fp);
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

int main()
{
	//PNG2Cel("f:\\menu.png", "f:\\menu.cel");

	//Cel2Cel("f:\\menu.cel", 1, "f:\\menu_.cel", "f:\\menu__.cel");
	//Cel2Cel("f:\\menu__.cel", 3, "f:\\menum_.cel", "f:\\menum__.cel");
	//Cel2Cel("f:\\menum__.cel", 4, "f:\\menum.cel", "f:\\menum___.cel");

	//Cel2Cel("f:\\lifeflask.cel", 1, "f:\\lifeflask_.cel", "f:\\lifeflask__.cel");
	//Cel2Cel("f:\\manaflask.cel", 1, "f:\\manaflask_.cel", "f:\\manaflask__.cel");
	//Cel2Cel("f:\\lifeflask__.cel", 3, "f:\\manaflask__.cel", "f:\\lifemana___.cel");
}