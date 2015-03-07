/*************************************************************************
 *                                                                       *
 * $Id: CPT2VGI.c 1702 2007-12-17 07:22:07Z Hharte $                     *
 *                                                                       *
 * Copyright (c) 2007-2008 Harte Technologies, LLC.                      *
 * All Rights Reserved                                                   *
 * http://www.hartetec.com                                               *
 *                                                                       *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF HARTE TECHNOLOGIES,LLC *
 * The  copyright notice above does not evidence any actual  or intended *
 * publication of such source  code.  The contents of this file may  not *
 * be  disclosed to third  parties, copied  or duplicated  in  any form, *
 * in whole or  in part, without  the prior  written permission of Harte *
 * Technologies, LLC.                                                    *
 *                                                                       *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY *
 * KIND, EITHER EXPRESSED OR  IMPLIED, INCLUDING BUT NOT  LIMITED TO THE *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR *
 * PURPOSE.                                                              *
 *                                                                       *
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

unsigned int ntracks = 77; // 154;

typedef union {
	struct {
		uint8 sync;
		uint8 header[2];
		uint8 unused[10];
		uint8 data[256];
		uint8 checksum;
		uint8 ecc[4];
	} u;
	uint8 raw[274];

} SECTOR_FORMAT;

SECTOR_FORMAT sdata;

static uint8 cy;
uint8 adc(uint8 sum, uint8 a1)
{
	unsigned int total;

	total = sum + a1 + cy;

	if(total > 0xFF) {
		cy = 1;
	} else {
		cy = 0;
	}

	return(total & 0xFF);
}


int main(int argc, char* argv[])
{
	
	FILE *istream;
	FILE *ostream;

	unsigned int i,j,k,sec_offset,skew_sect;
	unsigned int data_origin, checksum;

	printf("Vec CPT2VGI %d tracks\n\n", ntracks);

	if(argc < 3) {
		printf("Usage is: %s <infile.cpt> <ofile.dsk>\n", argv[0]);
		return(-1);
	}

	if((fopen_s(&istream, argv[1], "rb")) != NULL) {
		printf("Cannot open input file %s\n", argv[1]);
		return NULL;
	}


	if((fopen_s(&ostream, argv[2], "wb")) != NULL) {
		printf("Cannot open input file %s for writing\n", argv[2]);
		return NULL;
	}

	ibuf = malloc(8192);

	_set_fmode(_O_BINARY);


	// Skip over CPT Header on input stream
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

	data_origin = ftell(istream);

	_set_fmode(_O_BINARY);

	printf("Data Origin at 0x%x\n", data_origin);

	for(i=0;i<ntracks;i++) {
		for(j=0;j<16;j++) {
			if (i < 4) {
				skew_sect = skew_lut[j];
			} else {
				skew_sect = deskew_lut[j];
			}

			
			sec_offset = (i * 4096) + skew_sect * 256;
			fseek(istream, data_origin + sec_offset, 0);


			memset(&sdata.raw[0], 0, sizeof(sdata));
			fread(&sdata.u.data, 1, 256, istream);

			sdata.u.sync = 0xFF;
			sdata.u.header[0] = i;
			sdata.u.header[1] = j;

			adc(0,0); // clear Carry bit
			checksum = 0;
			
			// Checksum everything except the sync byte
			for(k=1;k<269;k++) {
				checksum = adc(checksum, sdata.raw[k]);
			}

			sdata.u.checksum = checksum & 0xFF;

			fwrite(&sdata.raw[0], 1, 274, ostream);
			printf(".");
		} // j
	} // i

	fclose(istream);
	fclose(ostream);

	free(ibuf);

}


