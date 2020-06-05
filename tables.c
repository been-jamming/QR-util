#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

//Lookup table for 2^n in GF(256)
//2 in GF(256) is the polynomial x in (Z/2Z)[x]/<x^8 + x^4 + x^3 + x^2 + 1>
uint8_t GF256_2exp[256] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
	0x1D, 0x3A, 0x74, 0xE8, 0xCD, 0x87, 0x13, 0x26,
	0x4C, 0x98, 0x2D, 0x5A, 0xB4, 0x75, 0xEA, 0xC9,
	0x8F, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0,
	0x9D, 0x27, 0x4E, 0x9C, 0x25, 0x4A, 0x94, 0x35,
	0x6A, 0xD4, 0xB5, 0x77, 0xEE, 0xC1, 0x9F, 0x23,
	0x46, 0x8C, 0x05, 0x0A, 0x14, 0x28, 0x50, 0xA0,
	0x5D, 0xBA, 0x69, 0xD2, 0xB9, 0x6F, 0xDE, 0xA1,
	0x5F, 0xBE, 0x61, 0xC2, 0x99, 0x2F, 0x5E, 0xBC,
	0x65, 0xCA, 0x89, 0x0F, 0x1E, 0x3C, 0x78, 0xF0,
	0xFD, 0xE7, 0xD3, 0xBB, 0x6B, 0xD6, 0xB1, 0x7F,
	0xFE, 0xE1, 0xDF, 0xA3, 0x5B, 0xB6, 0x71, 0xE2,
	0xD9, 0xAF, 0x43, 0x86, 0x11, 0x22, 0x44, 0x88,
	0x0D, 0x1A, 0x34, 0x68, 0xD0, 0xBD, 0x67, 0xCE,
	0x81, 0x1F, 0x3E, 0x7C, 0xF8, 0xED, 0xC7, 0x93,
	0x3B, 0x76, 0xEC, 0xC5, 0x97, 0x33, 0x66, 0xCC,
	0x85, 0x17, 0x2E, 0x5C, 0xB8, 0x6D, 0xDA, 0xA9,
	0x4F, 0x9E, 0x21, 0x42, 0x84, 0x15, 0x2A, 0x54,
	0xA8, 0x4D, 0x9A, 0x29, 0x52, 0xA4, 0x55, 0xAA,
	0x49, 0x92, 0x39, 0x72, 0xE4, 0xD5, 0xB7, 0x73,
	0xE6, 0xD1, 0xBF, 0x63, 0xC6, 0x91, 0x3F, 0x7E,
	0xFC, 0xE5, 0xD7, 0xB3, 0x7B, 0xF6, 0xF1, 0xFF,
	0xE3, 0xDB, 0xAB, 0x4B, 0x96, 0x31, 0x62, 0xC4,
	0x95, 0x37, 0x6E, 0xDC, 0xA5, 0x57, 0xAE, 0x41,
	0x82, 0x19, 0x32, 0x64, 0xC8, 0x8D, 0x07, 0x0E,
	0x1C, 0x38, 0x70, 0xE0, 0xDD, 0xA7, 0x53, 0xA6,
	0x51, 0xA2, 0x59, 0xB2, 0x79, 0xF2, 0xF9, 0xEF,
	0xC3, 0x9B, 0x2B, 0x56, 0xAC, 0x45, 0x8A, 0x09,
	0x12, 0x24, 0x48, 0x90, 0x3D, 0x7A, 0xF4, 0xF5,
	0xF7, 0xF3, 0xFB, 0xEB, 0xCB, 0x8B, 0x0B, 0x16,
	0x2C, 0x58, 0xB0, 0x7D, 0xFA, 0xE9, 0xCF, 0x83,
	0x1B, 0x36, 0x6C, 0xD8, 0xAD, 0x47, 0x8E, 0x01
};

//Lookup table for log base 2 of n in GF(256)
//Using this, we can do multiplication:
//a * b = 2^log_2(a)*2^log_2(b) = 2^((log_2(a) + log_2(b))%255)
//So we look up a and b in this table, add the values mod 255, and then exponentiate
//using the table above
uint8_t GF256_log2[256] = {
	0x00, 0xFF, 0x01, 0x19, 0x02, 0x32, 0x1A, 0xC6,
	0x03, 0xDF, 0x33, 0xEE, 0x1B, 0x68, 0xC7, 0x4B,
	0x04, 0x64, 0xE0, 0x0E, 0x34, 0x8D, 0xEF, 0x81,
	0x1C, 0xC1, 0x69, 0xF8, 0xC8, 0x08, 0x4C, 0x71,
	0x05, 0x8A, 0x65, 0x2F, 0xE1, 0x24, 0x0F, 0x21,
	0x35, 0x93, 0x8E, 0xDA, 0xF0, 0x12, 0x82, 0x45,
	0x1D, 0xB5, 0xC2, 0x7D, 0x6A, 0x27, 0xF9, 0xB9,
	0xC9, 0x9A, 0x09, 0x78, 0x4D, 0xE4, 0x72, 0xA6,
	0x06, 0xBF, 0x8B, 0x62, 0x66, 0xDD, 0x30, 0xFD,
	0xE2, 0x98, 0x25, 0xB3, 0x10, 0x91, 0x22, 0x88,
	0x36, 0xD0, 0x94, 0xCE, 0x8F, 0x96, 0xDB, 0xBD,
	0xF1, 0xD2, 0x13, 0x5C, 0x83, 0x38, 0x46, 0x40,
	0x1E, 0x42, 0xB6, 0xA3, 0xC3, 0x48, 0x7E, 0x6E,
	0x6B, 0x3A, 0x28, 0x54, 0xFA, 0x85, 0xBA, 0x3D,
	0xCA, 0x5E, 0x9B, 0x9F, 0x0A, 0x15, 0x79, 0x2B,
	0x4E, 0xD4, 0xE5, 0xAC, 0x73, 0xF3, 0xA7, 0x57,
	0x07, 0x70, 0xC0, 0xF7, 0x8C, 0x80, 0x63, 0x0D,
	0x67, 0x4A, 0xDE, 0xED, 0x31, 0xC5, 0xFE, 0x18,
	0xE3, 0xA5, 0x99, 0x77, 0x26, 0xB8, 0xB4, 0x7C,
	0x11, 0x44, 0x92, 0xD9, 0x23, 0x20, 0x89, 0x2E,
	0x37, 0x3F, 0xD1, 0x5B, 0x95, 0xBC, 0xCF, 0xCD,
	0x90, 0x87, 0x97, 0xB2, 0xDC, 0xFC, 0xBE, 0x61,
	0xF2, 0x56, 0xD3, 0xAB, 0x14, 0x2A, 0x5D, 0x9E,
	0x84, 0x3C, 0x39, 0x53, 0x47, 0x6D, 0x41, 0xA2,
	0x1F, 0x2D, 0x43, 0xD8, 0xB7, 0x7B, 0xA4, 0x76,
	0xC4, 0x17, 0x49, 0xEC, 0x7F, 0x0C, 0x6F, 0xF6,
	0x6C, 0xA1, 0x3B, 0x52, 0x29, 0x9D, 0x55, 0xAA,
	0xFB, 0x60, 0x86, 0xB1, 0xBB, 0xCC, 0x3E, 0x5A,
	0xCB, 0x59, 0x5F, 0xB0, 0x9C, 0xA9, 0xA0, 0x51,
	0x0B, 0xF5, 0x16, 0xEB, 0x7A, 0x75, 0x2C, 0xD7,
	0x4F, 0xAE, 0xD5, 0xE9, 0xE6, 0xE7, 0xAD, 0xE8,
	0x74, 0xD6, 0xF4, 0xEA, 0xA8, 0x50, 0x58, 0xAF
};

//Inverses can also be calculated by this table!
//a^-1 = 2^(255 - log_2(a))
//Note negatives in the exponent are taken mod *255*

//Addition is done by XORing the bytes
//Subtraction is the same operation as addition

unsigned char alignment_coords[41][7] = {
	{0  , 0  , 0  , 0  , 0  , 0  , 0  },
	{0  , 0  , 0  , 0  , 0  , 0  , 0  },
	{6  , 18 , 0  , 0  , 0  , 0  , 0  },
	{6  , 22 , 0  , 0  , 0  , 0  , 0  },
	{6  , 26 , 0  , 0  , 0  , 0  , 0  },
	{6  , 30 , 0  , 0  , 0  , 0  , 0  },
	{6  , 34 , 0  , 0  , 0  , 0  , 0  },
	{6  , 22 , 38 , 0  , 0  , 0  , 0  },
	{6  , 24 , 42 , 0  , 0  , 0  , 0  },
	{6  , 26 , 46 , 0  , 0  , 0  , 0  },
	{6  , 28 , 50 , 0  , 0  , 0  , 0  },
	{6  , 30 , 54 , 0  , 0  , 0  , 0  },
	{6  , 32 , 58 , 0  , 0  , 0  , 0  },
	{6  , 34 , 62 , 0  , 0  , 0  , 0  },
	{6  , 26 , 46 , 66 , 0  , 0  , 0  },
	{6  , 26 , 48 , 70 , 0  , 0  , 0  },
	{6  , 26 , 50 , 74 , 0  , 0  , 0  },
	{6  , 30 , 54 , 78 , 0  , 0  , 0  },
	{6  , 30 , 56 , 82 , 0  , 0  , 0  },
	{6  , 30 , 58 , 86 , 0  , 0  , 0  },
	{6  , 34 , 62 , 90 , 0  , 0  , 0  },
	{6  , 28 , 50 , 72 , 94 , 0  , 0  },
	{6  , 26 , 50 , 74 , 98 , 0  , 0  },
	{6  , 30 , 54 , 78 , 102, 0  , 0  },
	{6  , 28 , 54 , 80 , 106, 0  , 0  },
	{6  , 32 , 58 , 84 , 110, 0  , 0  },
	{6  , 30 , 58 , 86 , 114, 0  , 0  },
	{6  , 34 , 62 , 90 , 118, 0  , 0  },
	{6  , 26 , 50 , 74 , 98 , 122, 0  },
	{6  , 30 , 54 , 78 , 102, 126, 0  },
	{6  , 26 , 52 , 78 , 104, 130, 0  },
	{6  , 30 , 56 , 82 , 108, 134, 0  },
	{6  , 34 , 60 , 86 , 112, 138, 0  },
	{6  , 30 , 58 , 86 , 114, 142, 0  },
	{6  , 34 , 62 , 90 , 118, 146, 0  },
	{6  , 30 , 54 , 78 , 102, 126, 150},
	{6  , 24 , 50 , 76 , 102, 128, 154},
	{6  , 28 , 54 , 80 , 106, 132, 158},
	{6  , 32 , 58 , 84 , 110, 136, 162},
	{6  , 26 , 54 , 82 , 110, 138, 166},
	{6  , 30 , 58 , 86 , 114, 142, 170}
};

const unsigned int capacities[][4][2][3] = {
	{
	//Version 1
		{
			{1, 26, 19},
			{0, 0, 0}
		}, {
			{1, 26, 16},
			{0, 0, 0}
		}, {
			{1, 26, 13},
			{0, 0, 0}
		}, {
			{1, 26, 9},
			{0, 0, 0}
		}
	}, {
	//Version 2
		{
			{1, 44, 34},
			{0, 0, 0}
		}, {
			{1, 44, 28},
			{0, 0, 0}
		}, {
			{1, 44, 22},
			{0, 0, 0}
		}, {
			{1, 44, 16},
			{0, 0, 0}
		}
	}, {
	//Version 3
		{
			{1, 70, 55},
			{0, 0, 0}
		}, {
			{1, 70, 44},
			{0, 0, 0}
		}, {
			{2, 35, 17},
			{0, 0, 0}
		}, {
			{2, 35, 13},
			{0, 0, 0}
		}
	}, {
	//Version 4
		{
			{1, 100, 80},
			{0, 0, 0}
		}, {
			{2, 50, 32},
			{0, 0, 0}
		}, {
			{2, 50, 24},
			{0, 0, 0}
		}, {
			{4, 25, 9},
			{0, 0, 0}
		}
	}, {
	//Version 5
		{
			{1, 134, 108},
			{0, 0, 0}
		}, {
			{2, 67, 43},
			{0, 0, 0}
		}, {
			{2, 33, 15},
			{2, 34, 16}
		}, {
			{2, 33, 11},
			{2, 34, 12}
		}
	}, {
	//Version 6
		{
			{2, 86, 68},
			{0, 0, 0}
		}, {
			{4, 43, 27},
			{0, 0, 0}
		}, {
			{4, 43, 19},
			{0, 0, 0}
		}, {
			{4, 43, 15},
			{0, 0, 0}
		}
	}, {
	//Version 7
	//HYPE ^-^
		{
			{2, 98, 78},
			{0, 0, 0}
		}, {
			{4, 49, 31},
			{0, 0, 0}
		}, {
			{2, 32, 14},
			{4, 33, 15}
		}, {
			{4, 39, 13},
			{1, 40, 14}
		}
	}, {
	//Version 8
		{
			{2, 121, 97},
			{0, 0, 0},
		}, {
			{2, 60, 38},
			{2, 61, 39}
		}, {
			{4, 40, 18},
			{2, 41, 19}
		}, {
			{4, 40, 14},
			{2, 41, 15}
		}
	}, {
	//Version 9
		{
			{2, 146, 116},
			{0, 0, 0}
		}, {
			{3, 58, 36},
			{2, 59, 37}
		}, {
			{4, 36, 16},
			{4, 37, 17}
		}, {
			{4, 36, 12},
			{4, 37, 13}
		}
	}, {
	//Version 10
		{
			{2, 86, 68},
			{2, 87, 69}
		}, {
			{4, 69, 43},
			{1, 70, 44}
		}, {
			{6, 43, 19},
			{2, 44, 20}
		}, {
			{6, 43, 15},
			{2, 44, 16}
		}
	}, {
	//Version 11
		{
			{4, 101, 81},
			{0, 0, 0}
		}, {
			{1, 80, 50},
			{4, 81, 51}
		}, {
			{4, 50, 22},
			{4, 51, 23}
		}, {
			{3, 36, 12},
			{8, 37, 13}
		}
	}, {
	//Version 12
		{
			{2, 116, 92},
			{2, 117, 93}
		}, {
			{6, 58, 36},
			{2, 59, 37}
		}, {
			{4, 46, 20},
			{6, 47, 21}
		}, {
			{7, 42, 14},
			{4, 43, 15}
		}
	}, {
	//Version 13
		{
			{4, 133, 107},
			{0, 0, 0}
		}, {
			{8, 59, 37},
			{1, 60, 38}
		}, {
			{8, 44, 20},
			{4, 45, 21}
		}, {
			{12, 33, 11},
			{4, 34, 12}
		}
	}, {
	//Version 14
		{
			{3, 145, 115},
			{1, 146, 116}
		}, {
			{4, 64, 40},
			{5, 65, 41}
		}, {
			{11, 36, 16},
			{5, 37, 17}
		}, {
			{11, 36, 12},
			{5, 37, 13}
		}
	}, {
	//Version 15
		{
			{5, 109, 87},
			{1, 110, 88}
		}, {
			{5, 65, 41},
			{5, 66, 42}
		}, {
			{5, 54, 24},
			{7, 55, 25}
		}, {
			{11, 36, 12},
			{7, 37, 13}
		}
	}, {
	//Version 16
		{
			{5, 122, 98},
			{1, 123, 99}
		}, {
			{7, 73, 45},
			{3, 74, 46}
		}, {
			{15, 43, 19},
			{2, 44, 20}
		}, {
			{3, 45, 15},
			{13, 46, 16}
		}
	}, {
	//Version 17
		{
			{1, 135, 107},
			{5, 136, 108}
		}, {
			{10, 74, 46},
			{1, 75, 47}
		}, {
			{1, 50, 22},
			{15, 51, 23}
		}, {
			{2, 42, 14},
			{17, 43, 15}
		}
	}, {
	//Version 18
		{
			{5, 150, 120},
			{1, 151, 121}
		}, {
			{9, 69, 43},
			{4, 70, 44}
		}, {
			{17, 50, 22},
			{1, 51, 23}
		}, {
			{2, 42, 14},
			{19, 43, 15}
		}
	}, {
	//Version 19
		{
			{3, 141, 113},
			{4, 142, 114}
		}, {
			{3, 70, 44},
			{11, 71, 45}
		}, {
			{17, 47, 21},
			{4, 48, 22}
		}, {
			{9, 39, 13},
			{16, 40, 14}
		}
	}, {
	//Version 20
		{
			{3, 135, 107},
			{5, 136, 108}
		}, {
			{3, 67, 41},
			{13, 68, 42}
		}, {
			{15, 54, 24},
			{5, 55, 25}
		}, {
			{15, 43, 15},
			{10, 44, 16}
		}
	}, {
	//Version 21
		{
			{4, 144, 116},
			{4, 145, 117}
		}, {
			{17, 68, 42},
			{0, 0, 0}
		}, {
			{17, 50, 22},
			{6, 51, 23}
		}, {
			{19, 46, 16},
			{6, 47, 17}
		}
	}, {
	//Version 22
		{
			{2, 139, 111},
			{7, 140, 112}
		}, {
			{17, 74, 46},
			{0, 0, 0}
		}, {
			{7, 54, 24},
			{16, 55, 25}
		}, {
			{34, 37, 13},
			{0, 0, 0}
		}
	}, {
	//Version 23
		{
			{4, 151, 121},
			{5, 152, 122}
		}, {
			{4, 75, 47},
			{14, 76, 48}
		}, {
			{11, 54, 24},
			{14, 55, 25}
		}, {
			{16, 45, 15},
			{14, 46, 16}
		}
	}, {
	//Version 24
		{
			{6, 147, 117},
			{4, 148, 118}
		}, {
			{6, 73, 45},
			{14, 74, 46}
		}, {
			{11, 54, 24},
			{16, 55, 25}
		}, {
			{30, 46, 16},
			{2, 47, 17}
		}
	}, {
	//Version 25
		{
			{8, 132, 106},
			{4, 133, 107}
		}, {
			{8, 75, 47},
			{13, 76, 48}
		}, {
			{7, 54, 24},
			{22, 55, 25}
		}, {
			{22, 45, 15},
			{13, 46, 16}
		}
	}, {
	//Version 26
		{
			{10, 142, 114},
			{2, 143, 115}
		}, {
			{19, 74, 46},
			{4, 75, 47}
		}, {
			{28, 50, 22},
			{6, 51, 23}
		}, {
			{33, 46, 16},
			{4, 47, 17}
		}
	}, {
	//Version 27
		{
			{8, 152, 122},
			{4, 153, 123}
		}, {
			{22, 73, 45},
			{3, 74, 46}
		}, {
			{8, 53, 23},
			{26, 54, 24}
		}, {
			{12, 45, 15},
			{28, 46, 16}
		}
	}, {
	//Version 28
		{
			{3, 147, 117},
			{10, 148, 118}
		}, {
			{3, 73, 45},
			{23, 74, 46}
		}, {
			{4, 54, 24},
			{31, 55, 25}
		}, {
			{11, 45, 15},
			{31, 46, 16}
		}
	}, {
	//Version 29
		{
			{7, 146, 116},
			{7, 147, 117}
		}, {
			{21, 73, 45},
			{7, 74, 46}
		}, {
			{1, 53, 23},
			{37, 54, 24}
		}, {
			{19, 45, 15},
			{26, 46, 16}
		}
	}, {
	//Version 30
		{
			{5, 145, 115},
			{10, 146, 116}
		}, {
			{19, 75, 47},
			{10, 76, 48}
		}, {
			{15, 54, 24},
			{25, 55, 25}
		}, {
			{23, 45, 15},
			{25, 46, 26}
		}
	}, {
	//Version 31
		{
			{13, 145, 115},
			{3, 146, 116}
		}, {
			{2, 74, 46},
			{29, 75, 47}
		}, {
			{42, 54, 24},
			{1, 55, 25}
		}, {
			{23, 45, 15},
			{28, 46, 16}
		}
	}, {
	//Version 32
		{
			{17, 145, 115},
			{0, 0, 0}
		}, {
			{10, 74, 46},
			{23, 75, 47}
		}, {
			{10, 54, 24},
			{35, 55, 25}
		}, {
			{19, 45, 15},
			{35, 46, 16}
		}
	}, {
	//Version 33
		{
			{17, 145, 115},
			{1, 146, 116}
		}, {
			{14, 74, 46},
			{21, 75, 47}
		}, {
			{29, 54, 24},
			{19, 55, 25}
		}, {
			{11, 45, 15},
			{46, 46, 16}
		}
	}, {
	//Version 34
		{
			{13, 145, 115},
			{6, 146, 116}
		}, {
			{14, 74, 46},
			{23, 75, 47}
		}, {
			{44, 54, 24},
			{7, 55, 25}
		}, {
			{59, 46, 26},
			{1, 47, 17}
		}
	}, {
	//Version 35
		{
			{12, 151, 121},
			{7, 152, 122}
		}, {
			{12, 75, 47},
			{26, 76, 48}
		}, {
			{39, 54, 24},
			{14, 55, 25}
		}, {
			{22, 45, 15},
			{41, 46, 16}
		}
	}, {
	//Version 36
		{
			{6, 151, 121},
			{14, 152, 122}
		}, {
			{6, 75, 47},
			{34, 76, 48}
		}, {
			{46, 54, 24},
			{10, 55, 25}
		}, {
			{2, 45, 15},
			{64, 46, 16}
		}
	}, {
	//Version 37
		{
			{17, 152, 122},
			{4, 153, 123}
		}, {
			{29, 74, 46},
			{14, 75, 47}
		}, {
			{49, 54, 24},
			{10, 55, 25}
		}, {
			{24, 45, 15},
			{46, 46, 16}
		}
	}, {
	//Version 38
		{
			{4, 152, 122},
			{18, 153, 123}
		}, {
			{13, 74, 46},
			{32, 75, 47}
		}, {
			{48, 54, 24},
			{14, 55, 25}
		}, {
			{42, 45, 15},
			{32, 46, 16}
		}
	}, {
	//Version 39
		{
			{20, 147, 117},
			{4, 148, 118}
		}, {
			{40, 75, 47},
			{7, 76, 48}
		}, {
			{43, 54, 24},
			{22, 55, 25}
		}, {
			{10, 45, 15},
			{67, 46, 16}
		}
	}, {
	//Version 40
		{
			{19, 148, 118},
			{6, 149, 119}
		}, {
			{18, 75, 47},
			{31, 76, 48}
		}, {
			{34, 54, 24},
			{34, 55, 25}
		}, {
			{20, 45, 15},
			{61, 46, 16}
		}
	}
};
//Phew!

unsigned char qr_polynomials[31][70] = {
	{7, 0, 87, 229, 146, 149, 238, 102, 21},
	{10, 0, 251, 67, 46, 61, 118, 70, 64, 94, 32, 45},
	{13, 0, 74, 152, 176, 100, 86, 100, 106, 104, 130, 218, 206, 140, 78},
	{15, 0, 8, 183, 61, 91, 202, 37, 51, 58, 58, 237, 140, 124, 5, 99, 105},
	{16, 0, 120, 104, 107, 109, 102, 161, 76, 3, 91, 191, 147, 169, 182, 194, 225, 120},
	{17, 0, 43, 139, 206, 78, 43, 239, 123, 206, 214, 147, 24, 99, 150, 39, 243, 163, 136},
	{18, 0, 215, 234, 158, 94, 184, 97, 118, 170, 79, 187, 152, 148, 252, 179, 5, 98, 96, 153},
	{20, 0, 17, 60, 79, 50, 61, 163, 26, 187, 202, 180, 221, 225, 83, 239, 156, 164, 212, 212, 188, 190},
	{22, 0, 210, 171, 247, 242, 93, 230, 14, 109, 221, 53, 200, 74, 8, 172, 98, 80, 219, 134, 160, 105, 165, 231},
	{24, 0, 229, 121, 135, 48, 211, 117, 251, 126, 159, 180, 169, 152, 192, 226, 228, 218, 111, 117, 232, 87, 96, 227, 21},
	{26, 0, 173, 125, 158, 2, 103, 182, 118, 17, 145, 201, 111, 28, 165, 53, 161, 21, 245, 142, 13, 102, 48, 227, 153, 145, 218, 70},
	{28, 0, 168, 223, 200, 104, 224, 234, 108, 180, 110, 190, 195, 147, 205, 27, 232, 201, 21, 43, 245, 87, 42, 195, 212, 119, 242, 37, 9, 123},
	{30, 0, 41, 173, 145, 152, 216, 31, 179, 182, 50, 48, 110, 86, 239, 96, 222, 125, 42, 173, 226, 193, 224, 130, 156, 37, 251, 216, 238, 40, 192, 180},
	{32, 0, 10, 6, 106, 190, 249, 167, 4, 67, 209, 138, 138, 32, 242, 123, 89, 27, 120, 185, 80, 156, 38, 69, 171, 60, 28, 222, 80, 52, 254, 185, 220, 241},
	{34, 0, 111, 77, 146, 94, 26, 21, 108, 19, 105, 94, 113, 193, 86, 140, 163, 125, 58, 158, 229, 239, 218, 103, 56, 70, 114, 61, 183, 129, 167, 13, 98, 62, 129, 51},
	{36, 0, 200, 183, 98, 16, 172, 31, 246, 234, 60, 152, 115, 0, 167, 152, 113, 248, 238, 107, 18, 63, 218, 37, 87, 210, 105, 177, 120, 74, 121, 196, 117, 251, 113, 233, 30, 120},
	{40, 0, 59, 116, 79, 161, 252, 98, 128, 205, 128, 161, 247, 57, 163, 56, 235, 106, 53, 26, 187, 174, 226, 104, 170, 7, 175, 35, 181, 114, 88, 41, 47, 163, 125, 134, 72, 20, 232, 53, 35, 15},
	{42, 0, 250, 103, 221, 230, 25, 18, 137, 231, 0, 3, 58, 242, 221, 191, 110, 84, 230, 8, 188, 106, 96, 147, 15, 131, 139, 34, 101, 223, 39, 101, 213, 199, 237, 254, 201, 123, 171, 162, 194, 117, 50, 96},
	{44, 0, 190, 7, 61, 121, 71, 246, 69, 55, 168, 188, 89, 243, 191, 25, 72, 123, 9, 145, 14, 247, 1, 238, 44, 78, 143, 62, 224, 126, 118, 114, 68, 163, 52, 194, 217, 147, 204, 169, 37, 130, 113, 102, 73, 181},
	{46, 0, 112, 94, 88, 112, 253, 224, 202, 115, 187, 99, 89, 5, 54, 113, 129, 44, 58, 16, 135, 216, 169, 211, 36, 0, 4, 96, 60, 241, 73, 104, 234, 8, 249, 245, 119, 174, 52, 25, 157, 224, 43, 202, 223, 19, 82, 15},
	{48, 0, 228, 25, 196, 130, 211, 146, 60, 24, 251, 90, 39, 102, 240, 61, 178, 63, 46, 123, 115, 18, 221, 111, 135, 160, 182, 205, 107, 206, 95, 150, 120, 184, 91, 21, 247, 156, 140, 238, 191, 11, 94, 227, 84, 50, 163, 39, 34, 108},
	{50, 0, 232, 125, 157, 161, 164, 9, 118, 46, 209, 99, 203, 193, 35, 3, 209, 111, 195, 242, 203, 225, 46, 13, 32, 160, 126, 209, 130, 160, 242, 215, 242, 75, 77, 42, 189, 32, 113, 65, 124, 69, 228, 114, 235, 175, 124, 170, 215, 232, 133, 205},
	{52, 0, 116, 50, 86, 186, 50, 220, 251, 89, 192, 46, 86, 127, 124, 19, 184, 233, 151, 215, 22, 14, 59, 145, 37, 242, 203, 134, 254, 89, 190, 94, 59, 65, 124, 113, 100, 233, 235, 121, 22, 76, 86, 97, 39, 242, 200, 220, 101, 33, 239, 254, 116, 51},
	{54, 0, 183, 26, 201, 87, 210, 221, 113, 21, 46, 65, 45, 50, 238, 184, 249, 225, 102, 58, 209, 218, 109, 165, 26, 95, 184, 192, 52, 245, 35, 254, 238, 175, 172, 79, 123, 25, 122, 43, 120, 108, 215, 80, 128, 201, 235, 8, 153, 59, 101, 31, 198, 76, 31, 156},
	{56, 0, 106, 120, 107, 157, 164, 216, 112, 116, 2, 91, 248, 163, 36, 201, 202, 229, 6, 144, 254, 155, 135, 208, 170, 209, 12, 139, 127, 142, 182, 249, 177, 174, 190, 28, 10, 85, 239, 184, 101, 124, 152, 206, 96, 23, 163, 61, 27, 196, 247, 151, 154, 202, 207, 20, 61, 10},
	{58, 0, 82, 116, 26, 247, 66, 27, 62, 107, 252, 182, 200, 185, 235, 55, 251, 242, 210, 144, 154, 237, 176, 141, 192, 248, 152, 249, 206, 85, 253, 142, 65, 165, 125, 23, 24, 30, 122, 240, 214, 6, 129, 218, 29, 145, 127, 134, 206, 245, 117, 29, 41, 63, 159, 142, 233, 125, 148, 123},
	{60, 0, 107, 140, 26, 12, 9, 141, 243, 197, 226, 197, 219, 45, 211, 101, 219, 120, 28, 181, 127, 6, 100, 247, 2, 205, 198, 57, 115, 219, 101, 109, 160, 82, 37, 38, 238, 49, 160, 209, 121, 86, 11, 124, 30, 181, 84, 25, 194, 87, 65, 102, 190, 220, 70, 27, 209, 16, 89, 7, 33, 240},
	{62, 0, 65, 202, 113, 98, 71, 223, 248, 118, 214, 94, 0, 122, 37, 23, 2, 228, 58, 121, 7, 105, 135, 78, 243, 118, 70, 76, 223, 89, 72, 50, 70, 111, 194, 17, 212, 126, 181, 35, 221, 117, 235, 11, 229, 149, 147, 123, 213, 40, 115, 6, 200, 100, 26, 246, 182, 218, 127, 215, 36, 186, 110, 106},
	{64, 0, 45, 51, 175, 9, 7, 158, 159, 49, 68, 119, 92, 123, 177, 204, 187, 254, 200, 78, 141, 149, 119, 26, 127, 53, 160, 93, 199, 212, 29, 24, 145, 156, 208, 150, 218, 209, 4, 216, 91, 47, 184, 146, 47, 140, 195, 195, 125, 242, 238, 63, 99, 108, 140, 230, 242, 31, 204, 11, 178, 243, 217, 156, 213, 231},
	{66, 0, 5, 118, 222, 180, 136, 136, 162, 51, 46, 117, 13, 215, 81, 17, 139, 247, 197, 171, 95, 173, 65, 137, 178, 68, 111, 95, 101, 41, 72, 214, 169, 197, 95, 7, 44, 154, 77, 111, 236, 40, 121, 143, 63, 87, 80, 253, 240, 126, 217, 77, 34, 232, 106, 50, 168, 82, 76, 146, 67, 106, 171, 25, 132, 93, 45, 105},
	{68, 0, 247, 159, 223, 33, 224, 93, 77, 70, 90, 160, 32, 254, 43, 150, 84, 101, 190, 205, 133, 52, 60, 202, 165, 220, 203, 151, 93, 84, 15, 84, 253, 173, 160, 89, 227, 52, 199, 97, 95, 231, 52, 177, 41, 125, 137, 241, 166, 225, 118, 2, 54, 32, 82, 215, 175, 198, 43, 238, 235, 27, 101, 184, 127, 3, 5, 8, 163, 238}
};

uint16_t format_patterns[4][8] = {
	{
		0x77c4,
		0x72f3,
		0x7daa,
		0x789d,
		0x662f,
		0x6318,
		0x6c41,
		0x6976
	}, {
		0x5412,
		0x5125,
		0x5e7c,
		0x5b4b,
		0x45f9,
		0x40ce,
		0x4f97,
		0x4aa0
	}, {
		0x355f,
		0x3068,
		0x3f31,
		0x3a06,
		0x24b4,
		0x2183,
		0x2eda,
		0x2bed
	}, {
		0x1689,
		0x13be,
		0x1ce7,
		0x19d0,
		0x0762,
		0x0255,
		0x0d0c,
		0x083b
	}
};

const uint32_t version_patterns[34] = {
	0x07C94,
	0x085BC,
	0x09A99,
	0x0A4D3,
	0x0BBF6,
	0x0C762,
	0x0D847,
	0x0E60D,
	0x0F928,
	0x10B78,
	0x1145D,
	0x12A17,
	0x13532,
	0x149A6,
	0x15683,
	0x168C9,
	0x177EC,
	0x18EC4,
	0x191E1,
	0x1AFAB,
	0x1B08E,
	0x1CC1A,
	0x1D33F,
	0x1ED75,
	0x1F250,
	0x209D5,
	0x216F0,
	0x228BA,
	0x2379F,
	0x24B0B,
	0x2542E,
	0x26A64,
	0x27541,
	0x28C69
};

unsigned char format_position1[15][2] = {
	{8, 0},
	{8, 1},
	{8, 2},
	{8, 3},
	{8, 4},
	{8, 5},
	{8, 7},
	{8, 8},
	{7, 8},
	{5, 8},
	{4, 8},
	{3, 8},
	{2, 8},
	{1, 8},
	{0, 8}
};

unsigned char format_position2[15][2] = {
	{1, 8},
	{2, 8},
	{3, 8},
	{4, 8},
	{5, 8},
	{6, 8},
	{7, 8},
	{8, 8},
	{8, 7},//Change in coordinates here
	{8, 6},
	{8, 5},
	{8, 4},
	{8, 3},
	{8, 2},
	{8, 1}
};

//Transform the coords by y = height - y
//and again by y = x, x = height - y for the second version information position
const unsigned char version_position[18][2] = {
	{0, 11},
	{0, 10},
	{0, 9},
	{1, 11},
	{1, 10},
	{1, 9},
	{2, 11},
	{2, 10},
	{2, 9},
	{3, 11},
	{3, 10},
	{3, 9},
	{4, 11},
	{4, 10},
	{4, 9},
	{5, 11},
	{5, 10},
	{5, 9}
};

