/*************************************************************************
 *                                                                       *
 * $Id: CPT2VGI.c 1702 2007-12-17 07:22:07Z Hharte $                     *
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
 *                                                                       *
 * Module Description:                                                   *
 *     Convert 274-byte VGI image to 275                                 *
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

unsigned int ntracks = 154; // 77

static byte ROMimage[] = {
    /*00*/  0x44, 0x01, 0x01, 0x01, 0x82, 0x84, 0x78, 0xE6,
    /*08*/  0x07, 0x4F, 0x00, 0x31, 0x30, 0x00, 0x21, 0x29,
    /*10*/  0x00, 0xE5, 0x21, 0x2C, 0xC2, 0xE5, 0x21, 0x77,
    /*18*/  0x13, 0xE5, 0x21, 0xC9, 0x1A, 0xE5, 0xCD, 0x28,
    /*20*/  0x00, 0x21, 0x30, 0x00, 0x5B, 0x52, 0x44, 0x54,
    /*28*/  0x5D, 0x3A, 0x27, 0x00, 0x57, 0xC3, 0x29, 0x00,
    /*30*/  0x14, 0x14, 0x1E, 0x15, 0x1A, 0x26, 0x30, 0xCD,
    /*38*/  0xD9, 0x00, 0x42, 0x05, 0x0A, 0xCD, 0xD4, 0x00,
    /*40*/  0x2E, 0x0D, 0x2D, 0xCA, 0x43, 0x00, 0xCD, 0xD7,
    /*48*/  0x00, 0x1A, 0xE6, 0x40, 0xCA, 0x42, 0x00, 0x3E,
    /*50*/  0x0A, 0xF5, 0xCD, 0xC1, 0x00, 0x1E, 0x20, 0x1A,
    /*58*/  0xE6, 0x01, 0xC2, 0x63, 0x00, 0xCD, 0xC5, 0x00,
    /*60*/  0xC3, 0x55, 0x00, 0x2E, 0x04, 0xCD, 0xE7, 0x00,
    /*68*/  0x1E, 0x10, 0x1A, 0xE6, 0x04, 0xCA, 0x68, 0x00,
    /*70*/  0x3E, 0x09, 0x3D, 0xC2, 0x72, 0x00, 0x1A, 0xE6,
    /*78*/  0x20, 0xC2, 0x84, 0x00, 0xCD, 0xC1, 0x00, 0x2E,
    /*80*/  0x08, 0xCD, 0xE7, 0x00, 0x06, 0xA3, 0x1E, 0x10,
    /*88*/  0x05, 0xCA, 0xF4, 0x00, 0x1A, 0x0F, 0xD2, 0x88,
    /*90*/  0x00, 0x1E, 0x40, 0x1A, 0x67, 0x2E, 0x00, 0x36,
    /*98*/  0x59, 0x07, 0x47, 0x23, 0x1A, 0x77, 0xA8, 0x07,
    /*A0*/  0x47, 0x2C, 0xC2, 0x9C, 0x00, 0x24, 0x1A, 0x77,
    /*A8*/  0xA8, 0x07, 0x47, 0x2C, 0xC2, 0xA6, 0x00, 0x1A,
    /*B0*/  0xA8, 0xC2, 0xF4, 0x00, 0x25, 0x2E, 0x03, 0x71,
    /*B8*/  0x2D, 0x36, 0x59, 0xC2, 0xB8, 0x00, 0x2E, 0x0A,
    /*C0*/  0xE9, 0x3E, 0x20, 0x81, 0x4F, 0x0A, 0x3E, 0x10,
    /*C8*/  0x81, 0x4F, 0x0A, 0x3E, 0xF0, 0x81, 0x4F, 0x0A,
    /*D0*/  0x79, 0xE6, 0x0F, 0x4F, 0xCD, 0xD7, 0x00, 0x26,
    /*D8*/  0x01, 0x1E, 0x11, 0x1A, 0x1D, 0x1A, 0xB7, 0xF2,
    /*E0*/  0xDD, 0x00, 0x25, 0xC2, 0xD9, 0x00, 0xC9, 0xCD,
    /*E8*/  0xD7, 0x00, 0x1E, 0x35, 0x1A, 0xE6, 0x0F, 0xBD,
    /*F0*/  0xC2, 0xE7, 0x00, 0xC9, 0xF1, 0x3D, 0xF5, 0xC2,
    /*F8*/  0x55, 0x00, 0xC3, 0xFA, 0x00, 0x52, 0x44, 0x54,
};


typedef union {
	struct {
		uint8 sync;
		uint8 header[2];
		uint8 unused[10];
		uint8 data[256];
		uint8 checksum;
		uint8 ecc[4];
		uint8 ecc_valid;
	} u;
	uint8 raw[275];

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

	unsigned int i,j;
	unsigned int data_origin;

	printf("Vec 274-byte VGI to 275-byte VGI Conversion: tracks %d\n\n", ntracks);

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

	data_origin = ftell(istream);

	_set_fmode(_O_BINARY);

	printf("Data Origin at 0x%x\n", data_origin);

	fwrite(&ROMimage[0], 1, 256, ostream);
	fclose(ostream);
	exit(0);

	for(i=0;i<ntracks;i++) {
		for(j=0;j<16;j++) {

			memset(&sdata.raw[0], 0, sizeof(sdata));
			fread(&sdata.raw, 1, 274, istream);

			if(sdata.u.ecc[0] || sdata.u.ecc[1] || sdata.u.ecc[2] || sdata.u.ecc[3]) {
				printf("E");
				sdata.u.ecc_valid = 0xAA;
			} else {
				printf(".");
				sdata.u.ecc_valid = 0x00;
			}

			fwrite(&sdata.raw[0], 1, 275, ostream);
			
		} // j
	} // i

	fclose(istream);
	fclose(ostream);

	free(ibuf);

}


