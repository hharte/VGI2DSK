/*************************************************************************
 *                                                                       *
 * $Id: VG2DSK.c 2604 2015-03-07 18:57:43Z hharte $                      *
 *                                                                       *
 * Copyright (c) 2007-2015 Howard M. Harte                               *
 * All Rights Reserved                                                   *
 * hharte@magicandroidapps.com                                           *
 *                                                                       *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY *
 * KIND, EITHER EXPRESSED OR  IMPLIED, INCLUDING BUT NOT  LIMITED TO THE *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR *
 * PURPOSE.                                                              *
 *                                                                       *
 * Module Description:                                                   *
 *     Fixed sector skew on Single-sided Vector disks imaged with CPT    *
 * running later Double-sided CP/M.                                      *
 *                                                                       *
 * Environment:                                                          *
 *     User mode only                                                    *
 *                                                                       *
 *************************************************************************/

#include <basetyps.h>
#include <stdio.h>
#include <stdlib.h>
#include <wtypes.h>
#include <fcntl.h>

unsigned char *ibuf;

typedef unsigned char uint8;
	
// THese skew tables are to re-skew a Micropolis FD Controller Disk image made with an FD/HD Controller using CPT.  The FDHD uses a different skew.
uint8 skew_lut[16] = {
	0,
	11,
	6,
	1,	// 3
	12,
	7,
	2,	// 6
	13,
	8,
	3,	// 9
	14,
	9,
	4,	// 12
	15,
	10,
	5	// 15
};

uint8 deskew_lut[16] = {
	0,
	13,
	10,
	7,	// 3
	4,
	1,
	14,	// 6
	11,
	8,
	5,	// 9
	2,
	15,
	12,	// 12
	9,
	6,
	3	// 15
};

// FD/HD System Tracks sector skew table
uint8 fdhd_skew_lut[16] = {
	0,
	3,
	6,
	9,
	12,
	15,
	2,
	5,
	8,
	11,
	14,
	1,
	4,
	7,
	10,
	13
};

// FD/HD Data Tracks sector skew table
uint8 fdhd_skew_dat_lut[16] = {
	0,
	5,
	10,
	15,
	4,
	9,
	14,
	3,
	8,
	13,
	2,
	7,
	12,
	1,
	6,
	11
};

unsigned int ntracks = 77; // 154;


int main(int argc, char* argv[])
{
	
	FILE *istream;
	FILE *ostream;

	unsigned int i,j,sec_offset,skew_sect;
	unsigned int data_origin;

	printf("Vec VGI2DSK %d tracks\n\n", ntracks);

	if(argc < 3) {
		printf("Usage is: %s <infile.cpt> <ofile.dsk>\n", argv[0]);
		return(-1);
	}

	if((fopen_s(&istream, argv[1], "rb")) != 0) {
		printf("Cannot open input file %s\n", argv[1]);
		return NULL;
	}


	if((fopen_s(&ostream, argv[2], "wb")) != 0) {
		printf("Cannot open input file %s for writing\n", argv[2]);
		return NULL;
	}

	ibuf = malloc(8192);

	_set_fmode(_O_BINARY);
#if 0
	do {
		fread(ibuf, 1, 1, istream);
		if(ibuf[0] != 0x1a) putchar(ibuf[0]);
	}
	while (ibuf[0] != 0x1a);

	fread(ibuf, 31, 1, istream);

	do {
		fread(ibuf, 1, 1, istream);
	}
	while ((ibuf[0] != 0xFF) && !feof(istream));
#endif
	data_origin = ftell(istream);

	_set_fmode(_O_BINARY);

	printf("Data Origin at 0x%x\n", data_origin);
#if 0
	fread(ibuf, 8192, 1, istream);
	fwrite(ibuf, 8192, 1, ostream);

	for(i=0;i<32;i++) {
		fseek(istream, off_tbl[i]+  data_origin, 0);
		fread(ibuf, 1, 256, istream);
		fwrite(ibuf, 1, 256, ostream);
	}

	fseek(istream, data_origin + 16384, 0);

	for(i=0;i<73;i++) {
		fread(ibuf, 1, 4096, istream);
		fwrite(ibuf, 1, 4096, ostream);
	}
#endif

	for(i=0;i<ntracks;i++) {
		for(j=0;j<16;j++) {
#if MDSK_CPT_RESKEW
			if (i < 3) {
				skew_sect = j; //skew_lut[j];
			} else {
				skew_sect = j; //deskew_lut[j];
			}
#else // De-skew an FDHD Image made with CWVG
			if (ntracks == 154) {
			if (i < 4) {
				skew_sect = fdhd_skew_lut[j];
			} else {
				skew_sect = fdhd_skew_dat_lut[j];
			}
			} else {
				skew_sect = j;
			}
#endif // MDSK_CPT_RESKEW

#ifdef CPT_IMAGE
			sec_offset = (i * 4096) + skew_sect * 256;
#else		// else we use the actual physical sector including metadata.
			sec_offset = (i * 4384) + skew_sect * 274;
#endif /* CPT_IMAGE */
			fseek(istream, data_origin + sec_offset, 0);
			fread(ibuf, 1, 274, istream);
			fwrite(&ibuf[13], 1, 256, ostream);
			printf(".");
		} // j
	} // i

	fclose(istream);
	fclose(ostream);

	free(ibuf);

}
