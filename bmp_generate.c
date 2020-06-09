//Used for generating black and white bmp images only!

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "qrutil.h"

static void write_uint16_t(FILE *output_file, uint16_t n){
	fputc(n&0xFF, output_file);
	fputc((n&0xFF00)>>8, output_file);
}

static void write_uint32_t(FILE *output_file, uint32_t n){
	fputc(n&0xFF, output_file);
	fputc((n&0xFF00)>>8, output_file);
	fputc((n&0xFF0000)>>16, output_file);
	fputc((n&0xFF000000)>>24, output_file);
}

static void write_bytes_upscaling(FILE *output_file, unsigned char byte, unsigned int upscaling){
	unsigned char write_byte = 0x00;
	unsigned char write_bit = 0x80;
	unsigned char current_bit = 0x80;
	int i;

	while(current_bit){
		for(i = 0; i < upscaling; i++){
			if(byte&current_bit)
				write_byte |= write_bit;
			write_bit >>= 1;
			if(!write_bit){
				fputc(write_byte, output_file);
				write_bit = 0x80;
				write_byte = 0x00;
			}
		}
		current_bit >>= 1;
	}
}

void generate_bmp(FILE *output_file, struct qr_code *qr, unsigned int upscaling){
	unsigned int width;
	unsigned int row_bytes;
	unsigned int width_bytes;
	int y;
	int current_byte;
	int i;
	int j;

	if(!upscaling)
		return;

	width = qr->version*4 + 17;
	if(((width + 16)*upscaling)%32)
		row_bytes = ((width + 16)*upscaling + 32 - ((width + 16)*upscaling)%32)/8;
	else
		row_bytes = (width + 16)*upscaling/8;
	if(width%8)
		width_bytes = (width + 8 - width%8)/8;
	else
		width_bytes = width/8;
	fprintf(output_file, "BM");
	write_uint32_t(output_file, 62 + row_bytes*(width + 16)*upscaling);
	write_uint32_t(output_file, 0);
	write_uint32_t(output_file, 62);
	write_uint32_t(output_file, 40);
	write_uint32_t(output_file, (width + 16)*upscaling);
	write_uint32_t(output_file, (width + 16)*upscaling);
	write_uint16_t(output_file, 1);
	write_uint16_t(output_file, 1);
	write_uint32_t(output_file, 0);
	write_uint32_t(output_file, 0);
	write_uint32_t(output_file, 0);
	write_uint32_t(output_file, 0);
	write_uint32_t(output_file, 0);
	write_uint32_t(output_file, 0);
	write_uint32_t(output_file, 0x00FFFFFF);
	write_uint32_t(output_file, 0x00000000);
	for(y = 0; y < 8; y++)
		for(i = 0; i < upscaling; i++)
			for(current_byte = 0; current_byte < row_bytes; current_byte++)
				fputc(0, output_file);
	for(y = width - 1; y >= 0; y--)
		for(i = 0; i < upscaling; i++){
			for(j = 0; j < upscaling; j++)
				fputc(0, output_file);
			current_byte = upscaling;
			for(j = 0; j < width_bytes; j++){
				write_bytes_upscaling(output_file, qr->modules[j][y], upscaling);
				current_byte += upscaling;
			}
			while(current_byte < row_bytes){
				fputc(0, output_file);
				current_byte++;
			}
		}
	for(y = 0; y < 8; y++)
		for(i = 0; i < upscaling; i++)
			for(current_byte = 0; current_byte < row_bytes; current_byte++)
				fputc(0, output_file);
}
