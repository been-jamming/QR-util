#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "qrutil.h"

#define NONE 0
#define VERSION 1
#define ERROR_CORRECTION 2
#define UPSCALING 3

#define INPUT_BUFFER_INCREASE 32

char *argument_errors[11] = {NULL};
char *option_string;

void write_module(struct qr_code *qr, unsigned char x, unsigned char y, unsigned char value, unsigned char do_update_mask){
	if(value)
		qr->modules[x/8][y] |= 0x80>>(x&7);
	else
		qr->modules[x/8][y] &= ~(0x80>>(x&7));
	if(do_update_mask)
		qr->written_mask[x/8][y] |= 0x80>>(x&7);
}

void change_module(struct qr_code *qr, unsigned char x, unsigned char y){
	qr->modules[x/8][y] ^= 0x80>>(x&7);
}

unsigned char change_module_unwritten(struct qr_code *qr, unsigned char x, unsigned char y){
	if(qr->written_mask[x/8][y]&(0x80>>(x&7)))
		return 1;
	change_module(qr, x, y);
	return 0;
}

unsigned char write_module_unwritten(struct qr_code *qr, unsigned char x, unsigned char y, unsigned char value, unsigned char do_update_mask){
	if(qr->written_mask[x/8][y]&(0x80>>(x&7)))
		return 1;
	write_module(qr, x, y, value, do_update_mask);
	return 0;
}

unsigned char read_module(struct qr_code *qr, unsigned char x, unsigned char y){
	return (qr->modules[x/8][y]&(0x80>>(x&7))) != 0;
}

unsigned char module_written(struct qr_code *qr, unsigned char x, unsigned char y){
	return (qr->written_mask[x/8][y]&(0x80>>(x&7))) != 0;
}

unsigned char read_mask(unsigned char modules[23][177], unsigned char x, unsigned char y){
	return (modules[x/8][y]&(0x80>>(x&7))) != 0;
}

void write_mask(unsigned char modules[23][177], unsigned char x, unsigned char y, unsigned char value){
	if(value)
		modules[x/8][y] |= 0x80>>(x&7);
	else
		modules[x/8][y] &= ~(0x80>>(x&7));
}

void create_finder_patterns(struct qr_code *qr){
	unsigned char byte_start;
	unsigned char i;

	//Create the top-left finder pattern
	//This always involves setting the same bits
	qr->modules[0][0] = 0xFE;
	qr->modules[0][1] = 0x82;
	qr->modules[0][2] = 0xBA;
	qr->modules[0][3] = 0xBA;
	qr->modules[0][4] = 0xBA;
	qr->modules[0][5] = 0x82;
	qr->modules[0][6] = 0xFE;
	qr->modules[0][7] = 0x00;
	for(i = 0; i < 8; i++)
		qr->written_mask[0][i] = 0xFF;

	//Create the top-right finder pattern
	//Which bits must be set depends on whether the version is odd or even
	//If it is odd, then the bits start at the 4 bit of the second-to-last byte from the right
	//Otherwise, the bits start at the 64 bit of the second-to-last byte from the right
	byte_start = qr->version/2 + 1;
	if(qr->version&1){
		qr->modules[byte_start][0] = 0x03;
		qr->written_mask[byte_start][0] = 0x07;
		for(i = 1; i < 6; i++){
			qr->modules[byte_start][i] = 0x02;
			qr->written_mask[byte_start][i] = 0x07;
		}
		qr->modules[byte_start][6] = 0x03;
		qr->modules[byte_start][7] = 0x00;
		qr->written_mask[byte_start][6] = 0x07;
		qr->written_mask[byte_start][7] = 0x07;
		qr->modules[byte_start + 1][0] = 0xF8;
		qr->modules[byte_start + 1][1] = 0x08;
		qr->modules[byte_start + 1][2] = 0xE8;
		qr->modules[byte_start + 1][3] = 0xE8;
		qr->modules[byte_start + 1][4] = 0xE8;
		qr->modules[byte_start + 1][5] = 0x08;
		qr->modules[byte_start + 1][6] = 0xF8;
		qr->modules[byte_start + 1][7] = 0x00;
		for(i = 0; i < 8; i++)
			qr->written_mask[byte_start + 1][i] = 0xF8;
	} else {
		qr->modules[byte_start][0] = 0x3F;
		qr->modules[byte_start][1] = 0x20;
		qr->modules[byte_start][2] = 0x2E;
		qr->modules[byte_start][3] = 0x2E;
		qr->modules[byte_start][4] = 0x2E;
		qr->modules[byte_start][5] = 0x20;
		qr->modules[byte_start][6] = 0x3F;
		qr->modules[byte_start][7] = 0x00;
		for(i = 0; i < 8; i++){
			qr->written_mask[byte_start][i] = 0x7F;
			qr->written_mask[byte_start + 1][i] = 0x80;
		}
		for(i = 0; i < 7; i++)
			qr->modules[byte_start + 1][i] = 0x80;
		qr->modules[byte_start + 1][7] = 0x00;
	}

	//Create the bottom-left finder pattern
	byte_start = qr->version*4 + 9;
	qr->modules[0][byte_start] = 0x00;
	qr->modules[0][byte_start + 1] = 0xFE;
	qr->modules[0][byte_start + 2] = 0x82;
	qr->modules[0][byte_start + 3] = 0xBA;
	qr->modules[0][byte_start + 4] = 0xBA;
	qr->modules[0][byte_start + 5] = 0xBA;
	qr->modules[0][byte_start + 6] = 0x82;
	qr->modules[0][byte_start + 7] = 0xFE;
	for(i = 0; i < 8; i++)
		qr->written_mask[0][byte_start + i] = 0xFF;
}

void create_alignment_pattern(struct qr_code *qr, unsigned char x, unsigned char y){
	unsigned char i;
	unsigned char j;

	for(i = 0; i < 5; i++){
		write_module(qr, x - 2 + i, y - 2, 1, 1);
		write_module(qr, x - 2 + i, y + 2, 1, 1);
	}
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			write_module(qr, x - 1 + i, y - 1 + j, 0, 1);
	write_module(qr, x - 2, y - 1, 1, 1);
	write_module(qr, x - 2, y, 1, 1);
	write_module(qr, x - 2, y + 1, 1, 1);
	write_module(qr, x + 2, y - 1, 1, 1);
	write_module(qr, x + 2, y, 1, 1);
	write_module(qr, x + 2, y + 1, 1, 1);
	write_module(qr, x, y, 1, 1);
}

void create_alignment_patterns(struct qr_code *qr){
	unsigned char i;
	unsigned char j;
	unsigned char x;
	unsigned char y;

	for(i = 0; i < 7; i++){
		for(j = 0; j < 7; j++){
			x = alignment_coords[qr->version][i];
			y = alignment_coords[qr->version][j];
			if(x && y && !module_written(qr, x, y))
				create_alignment_pattern(qr, x, y);
		}
	}
}

void apply_mask_pattern(struct qr_code *qr, unsigned char mask_pattern){
	unsigned int i;
	unsigned int j;

	switch(mask_pattern){
		case 0:
			for(i = 0; i < qr->version*4 + 17; i++)
				for(j = i&1; j < qr->version*4 + 17; j += 2)
					change_module_unwritten(qr, i, j);
			return;
		case 1:
			for(i = 0; i < qr->version*4 + 17; i++)
				for(j = 0; j < qr->version*4 + 17; j += 2)
					change_module_unwritten(qr, i, j);
			return;
		case 2:
			for(i = 0; i < qr->version*4 + 17; i++)
				for(j = 0; j < qr->version*4 + 17; j++)
					if(!(i%3))
						change_module_unwritten(qr, i, j);
			return;
		case 3:
			for(i = 0; i < qr->version*4 + 17; i++)
				for(j = 0; j < qr->version*4 + 17; j++)
					if(!((i + j)%3))
						change_module_unwritten(qr, i, j);
			return;
		case 4:
			for(i = 0; i < qr->version*4 + 17; i++)
				for(j = 0; j < qr->version*4 + 17; j++)
					if(!((j/2 + i/3)&1))
						change_module_unwritten(qr, i, j);
			return;
		case 5:
			for(i = 0; i < qr->version*4 + 17; i++)
				for(j = 0; j < qr->version*4 + 17; j++)
					if(!((i*j)%2 + (i*j)%3))
						change_module_unwritten(qr, i, j);
			return;
		case 6:
			for(i = 0; i < qr->version*4 + 17; i++)
				for(j = 0; j < qr->version*4 + 17; j++)
					if(!(((i*j)%2 + (i*j)%3)&1))
						change_module_unwritten(qr, i, j);
			return;
		case 7:
			for(i = 0; i < qr->version*4 + 17; i++)
				for(j = 0; j < qr->version*4 + 17; j++)
					if(!(((i + j)%2 + (i*j)%3)&1))
						change_module_unwritten(qr, i, j);
	}
}

void create_timing_patterns(struct qr_code *qr){
	unsigned int i;

	for(i = 8; i < qr->version*4 + 9; i++){
		write_module(qr, i, 6, !(i&1), 1);
		write_module(qr, 6, i, !(i&1), 1);
	}
}

void create_format_information(struct qr_code *qr, unsigned char correction_level, unsigned char mask_pattern){
	uint16_t format_pattern;
	unsigned char i;

	format_pattern = format_patterns[correction_level][mask_pattern];
	for(i = 0; i < 15; i++){
		write_module(qr, format_position1[i][0], format_position1[i][1], format_pattern&1, 1);
		if(i < 8)
			write_module(qr, qr->version*4 + 17 - format_position2[i][0], format_position2[i][1], format_pattern&1, 1);
		else
			write_module(qr, format_position2[i][0], qr->version*4 + 17 - format_position2[i][1], format_pattern&1, 1);
		format_pattern >>= 1;
	}

	//This module is always black!
	write_module(qr, 8, qr->version*4 + 9, 1, 1);
}

void create_version_information(struct qr_code *qr){
	uint32_t version_pattern;
	unsigned char i;

	if(qr->version >= 7){
		version_pattern = version_patterns[qr->version - 7];
		for(i = 0; i < 18; i++){
			write_module(qr, version_position[i][0], qr->version*4 + 17 - version_position[i][1], version_pattern&1, 1);
			write_module(qr, qr->version*4 + 17 - version_position[i][1], version_position[i][0], version_pattern&1, 1);
			version_pattern >>= 1;
		}
	}
}

unsigned char write_data_bit(struct qr_code *qr, unsigned char bit){
	unsigned char output;

	if(qr->x >= 0)
		output = !write_module_unwritten(qr, qr->x, qr->y, read_module(qr, qr->x, qr->y)^bit, 0);
	else
		output = 0;
	if(qr->up){
		if(qr->next){
			if(qr->y){
				qr->x += 1;
				qr->y -= 1;
			} else {
				qr->x -= 1;
				if(qr->x == 6)
					qr->x--;
				qr->up = 0;
			}
		} else
			qr->x -= 1;
	} else {
		if(qr->next){
			if(qr->y < qr->version*4 + 16){
				qr->x += 1;
				qr->y += 1;
			} else {
				qr->x -= 1;
				if(qr->x == 6)
					qr->x--;
				qr->up = 1;
			}
		} else
			qr->x -= 1;
	}
	qr->next = !qr->next;

	return output;
}

void write_byte(struct qr_code *qr, unsigned char byte){
	unsigned char count;

	count = 8;
	while(count){
		if(write_data_bit(qr, (byte&0x80)>>7)){
			count--;
			byte <<= 1;
		}
	}
}

void reduce_poly_GF256(unsigned char *data, unsigned int data_size, unsigned char *poly, unsigned int poly_size){
	int i;

	while(data_size >= poly_size){
		if(*data){
			for(i = poly_size - 1; i >= 0; i--){
				data[i] ^= GF256_2exp[(poly[i] + GF256_log2[data[0]])%255];
			}
		}
		data++;
		data_size--;
	}
}

unsigned int data_capacity(unsigned char version, unsigned char correction_level){
	return capacities[version][correction_level][0][2]*capacities[version][correction_level][0][0] + capacities[version][correction_level][1][2]*capacities[version][correction_level][1][0];
}

unsigned int num_blocks(unsigned char version, unsigned char correction_level){
	return capacities[version][correction_level][0][0] + capacities[version][correction_level][1][0];
}

void write_byte_blocks(struct qr_block *blocks, unsigned int num_blocks, unsigned char byte){
	unsigned int i;

	for(i = 0; i < num_blocks; i++){
		if(blocks[i].data_left){
			blocks[i].data[blocks[i].data_size - blocks[i].data_left] = byte;
			blocks[i].data_left--;
			return;
		}
	}
}

void initialize(unsigned char *version, unsigned char *correction_level, struct qr_block **blocks, unsigned char *data, unsigned int data_size){
	int i;
	int j = 1;
	unsigned char header_size;

	if(*version){
		if(*version >= 10)
			header_size = 4;
		else
			header_size = 2;
		if(*correction_level && data_capacity(*version - 1, *correction_level - 1) - header_size < data_size){
			*blocks = NULL;
			return;
		} else if(!*correction_level && data_capacity(*version - 1, L_CORRECTION) - header_size < data_size){
			*blocks = NULL;
			return;
		}
		if(*correction_level)
			(*correction_level)--;
		else{
			for(i = 3; i >= 0; i--)
				if(data_capacity(*version - 1, i) - header_size >= data_size)
					break;
			*correction_level = i;
		}
	} else {
		if(*correction_level){
			for(i = 0; i < MAX_VERSION; i++){
				if(i < 9)
					header_size = 2;
				else
					header_size = 4;
				if(data_capacity(i, *correction_level - 1) - header_size >= data_size)
					break;
			}
			if(i == MAX_VERSION){
				*blocks = NULL;
				return;
			}
			*version = i + 1;
			(*correction_level)--;
		} else {
			for(i = 0; i < MAX_VERSION; i++){
				if(i < 9)
					header_size = 2;
				else
					header_size = 4;
				if(data_size <= data_capacity(i, L_CORRECTION) - header_size){
					for(j = 1; j < 4; j++)
						if(data_capacity(i, j) - header_size < data_size)
							break;
					break;
				}
			}

			if(i == MAX_VERSION){
				*blocks = NULL;
				return;
			}

			*version = i + 1;
			*correction_level = j - 1;
		}
	}

	*blocks = calloc(num_blocks(*version - 1, *correction_level), sizeof(struct qr_block));
	(*blocks)[0].data_size = capacities[*version - 1][*correction_level][0][2];
	(*blocks)[0].error_size = capacities[*version - 1][*correction_level][0][1] - (*blocks)[0].data_size;
	(*blocks)[0].data_left = (*blocks)[0].data_size;
	(*blocks)[0].error_left = (*blocks)[0].error_size;
	(*blocks)[0].data = calloc((*blocks)[0].data_size + (*blocks)[0].error_size, sizeof(unsigned char));
	for(i = 0; i <= MAX_POLY; i++)
		if(qr_polynomials[i][0] == (*blocks)[0].error_size){
			(*blocks)[0].poly = qr_polynomials[i] + 1;
			break;
		}
	if(i > MAX_POLY){
		free((*blocks)[0].data);
		free(*blocks);
		*blocks = NULL;
		return;
	}
		
	for(i = 1; i < capacities[*version - 1][*correction_level][0][0]; i++){
		(*blocks)[i].data_size = (*blocks)[0].data_size;
		(*blocks)[i].error_size = (*blocks)[0].error_size;
		(*blocks)[i].data_left = (*blocks)[i].data_size;
		(*blocks)[i].error_left = (*blocks)[i].error_size;
		(*blocks)[i].data = calloc((*blocks)[i].data_size + (*blocks)[i].error_size, sizeof(unsigned char));
		(*blocks)[i].poly = (*blocks)[0].poly;
	}
	if(i < num_blocks(*version - 1, *correction_level)){
		(*blocks)[i].data_size = capacities[*version - 1][*correction_level][1][2];
		(*blocks)[i].error_size = capacities[*version - 1][*correction_level][1][1] - (*blocks)[i].data_size;
		(*blocks)[i].data_left = (*blocks)[i].data_size;
		(*blocks)[i].error_left = (*blocks)[i].error_size;
		(*blocks)[i].data = calloc((*blocks)[i].data_size + (*blocks)[i].error_size, sizeof(unsigned char));
		for(j = 0; j <= MAX_POLY; j++)
			if(qr_polynomials[j][0] == (*blocks)[i].error_size){
				(*blocks)[i].poly = qr_polynomials[j] + 1;
				break;
			}
		for(j = i + 1; j < num_blocks(*version - 1, *correction_level); j++){
			(*blocks)[j].data_size = (*blocks)[i].data_size;
			(*blocks)[j].error_size = (*blocks)[i].error_size;
			(*blocks)[j].data_left = (*blocks)[j].data_size;
			(*blocks)[j].error_left = (*blocks)[j].error_size;
			(*blocks)[j].data = calloc((*blocks)[j].data_size + (*blocks)[j].error_size, sizeof(unsigned char));
			(*blocks)[j].poly = (*blocks)[i].poly;
		}
	}
	if(header_size == 4){
		write_byte_blocks(*blocks, num_blocks(*version - 1, *correction_level), 0x40 | ((data_size&0xF000)>>12));
		write_byte_blocks(*blocks, num_blocks(*version - 1, *correction_level), ((data_size&0x0F00)>>4) | ((data_size&0x00F0)>>4));
		write_byte_blocks(*blocks, num_blocks(*version - 1, *correction_level), ((data_size&0x000F)<<4) | ((data[0]&0xF0)>>4));
	} else {
		write_byte_blocks(*blocks, num_blocks(*version - 1, *correction_level), 0x40 | ((data_size&0xF0)>>4));
		write_byte_blocks(*blocks, num_blocks(*version - 1, *correction_level), ((data_size&0x0F)<<4) | ((data[0]&0xF0)>>4));
	}
	for(i = 1; i < data_size; i++)
		write_byte_blocks(*blocks, num_blocks(*version - 1, *correction_level), ((data[i - 1]&0x0F)<<4) | ((data[i]&0xF0)>>4));
	write_byte_blocks(*blocks, num_blocks(*version - 1, *correction_level), (data[i - 1]&0x0F)<<4);
	for(j = 0; j < data_capacity(*version - 1, *correction_level) - header_size - data_size + (header_size == 4); j++)
		if(j&1)
			write_byte_blocks(*blocks, num_blocks(*version - 1, *correction_level), 0x11);
		else
			write_byte_blocks(*blocks, num_blocks(*version - 1, *correction_level), 0xEC);
	for(i = 0; i < num_blocks(*version - 1, *correction_level); i++)
		(*blocks)[i].data_left = (*blocks)[i].data_size;
}

void write_data(struct qr_code *qr, struct qr_block *blocks, unsigned int num_blocks){
	unsigned int i;
	unsigned char data_left = 1;

	while(data_left){
		data_left = 0;
		for(i = 0; i < num_blocks; i++)
			if(blocks[i].data_left){
				write_byte(qr, blocks[i].data[blocks[i].data_size - blocks[i].data_left]);
				blocks[i].data_left--;
				data_left = 1;
			}
	}
}

void write_error_data(struct qr_code *qr, struct qr_block *blocks, unsigned int num_blocks){
	unsigned int i;
	unsigned char error_left = 1;

	while(error_left){
		error_left = 0;
		for(i = 0; i < num_blocks; i++)
			if(blocks[i].error_left){
				write_byte(qr, blocks[i].data[blocks[i].data_size + blocks[i].error_size - blocks[i].error_left]);
				blocks[i].error_left--;
				error_left = 1;
			}
	}
}

void create_error_data(struct qr_block *blocks, unsigned int num_blocks){
	unsigned int i;

	for(i = 0; i < num_blocks; i++)
		reduce_poly_GF256(blocks[i].data, blocks[i].data_size + blocks[i].error_size, blocks[i].poly, blocks[i].error_size + 1);
}

void free_blocks(struct qr_block *blocks, unsigned int num_blocks){
	unsigned int i;

	for(i = 0; i < num_blocks; i++)
		free(blocks[i].data);
	free(blocks);
}

void count_rectangle(struct qr_code *qr, unsigned char mask[23][177], unsigned char x, unsigned char y, unsigned int *output){
	unsigned char color;
	unsigned int width = 0;
	unsigned int height = 1;
	unsigned int i;
	unsigned char do_break = 0;

	color = read_module(qr, x, y);

	while(width < qr->version*4 + 17 - x && read_module(qr, x + width, y) == color && !read_mask(mask, x + width, y)){
		width++;
		write_mask(mask, x + width, y, 1);
	}
	while(height < qr->version*4 + 17 - y && !do_break){
		for(i = 0; i < width; i++){
			if(read_module(qr, x + width, y + height) != color || read_mask(mask, x + width, y + height)){
				do_break = 1;
				break;
			}
		}
		if(!do_break)
			for(i = 0; i < width; i++)
				write_mask(mask, x + width, y + height, 1);
	}

	if(width > 1 && height > 1)
		*output += 3*(width - 1)*(height - 1);
}

unsigned char check_finder_pattern_x(struct qr_code *qr, unsigned char x, unsigned char y){
	return read_module(qr, x, y) && !read_module(qr, x + 1, y) && read_module(qr, x + 2, y) && read_module(qr, x + 3, y) && read_module(qr, x + 4, y) && !read_module(qr, x + 5, y) && read_module(qr, x + 6, y);
}

unsigned char check_finder_pattern_y(struct qr_code *qr, unsigned char x, unsigned char y){
	return read_module(qr, x, y) && !read_module(qr, x, y + 1) && read_module(qr, x, y + 2) && read_module(qr, x, y + 3) && read_module(qr, x, y + 4) && !read_module(qr, x, y + 5) && read_module(qr, x, y + 6);
}

unsigned int grade_qr_code(struct qr_code *qr){
	unsigned int output = 0;
	unsigned int x;
	unsigned int y;
	unsigned long int num_light = 0;
	unsigned long int num_dark = 0;

	unsigned char *adjacent_light;
	unsigned char *adjacent_dark;

	unsigned char (*mask)[23][177];

	adjacent_light = calloc(qr->version*4 + 17, sizeof(unsigned char));
	adjacent_dark = calloc(qr->version*4 + 17, sizeof(unsigned char));

	for(x = 0; x < qr->version*4 + 17; x++){
		for(y = 0; y < qr->version*4 + 17; y++){
			if(read_module(qr, x, y)){
				num_dark++;
				if(adjacent_light[y] > 3)
					output += adjacent_light[y] - 2;
				adjacent_light[y] = 0;
				adjacent_dark[y]++;
			} else {
				num_light++;
				if(adjacent_dark[y] > 3)
					output += adjacent_dark[y] - 2;
				adjacent_dark[y] = 0;
				adjacent_light[y]++;
			}
		}
	}

	memset(adjacent_light, 0, sizeof(unsigned char)*(qr->version*4 + 17));
	memset(adjacent_dark, 0, sizeof(unsigned char)*(qr->version*4 + 17));

	for(y = 0; y < qr->version*4 + 17; y++){
		for(x = 0; x < qr->version*4 + 17; x++){
			if(read_module(qr, x, y)){
				if(adjacent_light[y] > 3)
					output += adjacent_light[y] - 2;
				adjacent_light[y] = 0;
				adjacent_dark[y]++;
			} else {
				if(adjacent_dark[y] > 3)
					output += adjacent_dark[y] - 2;
				adjacent_dark[y] = 0;
				adjacent_light[y]++;
			}
		}
	}

	free(adjacent_light);
	free(adjacent_dark);
	mask = calloc(23*177, sizeof(unsigned char));

	for(x = 0; x < qr->version*4 + 17; x++){
		for(y = 0; y < qr->version*4 + 17; y++){
			if(!read_mask(*mask, x, y))
				count_rectangle(qr, *mask, x, y, &output);
		}
	}

	free(mask);

	for(x = 0; x < qr->version*4 + 17; x++)
		for(y = 0; y < qr->version*4 + 10; y++)
			if(check_finder_pattern_y(qr, x, y))
				output += 40;
	for(y = 0; y < qr->version*4 + 17; y++)
		for(x = 0; x < qr->version*4 + 10; x++)
			if(check_finder_pattern_x(qr, x, y))
				output += 40;
	
	if(num_light > num_dark)
		output += num_light*200/(num_light + num_dark) - 100;

	return output;
}

int generate_qr_code(FILE *output_file, unsigned char *data, unsigned int data_size, unsigned char version, unsigned char correction_level, unsigned int upscaling, unsigned char verbose){
	struct qr_code qr;
	struct qr_block *blocks;
	unsigned int score;
	unsigned int best_score;
	unsigned char best_mask;
	unsigned char mask;

	initialize(&version, &correction_level, &blocks, data, data_size);
	if(!blocks)
		return 1;
	qr.version = version;
	memset(qr.modules, 0, sizeof(qr.modules));
	memset(qr.written_mask, 0, sizeof(qr.written_mask));
	create_finder_patterns(&qr);
	if(qr.version > 1)
		create_alignment_patterns(&qr);
	create_version_information(&qr);
	create_timing_patterns(&qr);
	qr.x = qr.version*4 + 16;
	qr.y = qr.version*4 + 16;
	qr.up = 1;
	qr.next = 0;
	create_format_information(&qr, correction_level, 0);
	write_data(&qr, blocks, num_blocks(qr.version - 1, correction_level));
	create_error_data(blocks, num_blocks(qr.version - 1, correction_level));
	write_error_data(&qr, blocks, num_blocks(qr.version - 1, correction_level));
	apply_mask_pattern(&qr, 0);
	best_score = grade_qr_code(&qr);
	apply_mask_pattern(&qr, 0);
	best_mask = 0;

	for(mask = 1; mask < 8; mask++){
		create_format_information(&qr, correction_level, mask);
		apply_mask_pattern(&qr, mask);
		score = grade_qr_code(&qr);
		if(score < best_score){
			best_score = score;
			best_mask = mask;
		}
		apply_mask_pattern(&qr, mask);
	}

	create_format_information(&qr, correction_level, best_mask);
	apply_mask_pattern(&qr, best_mask);
	generate_bmp(output_file, &qr, upscaling);
	free_blocks(blocks, num_blocks(qr.version - 1, correction_level));

	if(verbose)
		printf("Version: %d\n"
		       "Error Correction: %d\n"
		       "Mask: %d\n"
		       "Upscaling: %d\n", (int) version, (int) correction_level + 1, (int) best_mask, (int) upscaling
		);

	return 0;
}

unsigned char parse_arguments(int argc, char **argv, unsigned char *version, unsigned char *error_correction, unsigned int *upscaling, char **filename, unsigned char *help, unsigned char *verbose){
	int i;
	int inp;
	unsigned char argument_state = NONE;

	*filename = NULL;
	*version = 0;
	*error_correction = 0;
	for(i = 1; i < argc; i++){
		if(!argument_state && !strcmp(argv[i], "-v"))
			argument_state = VERSION;
		else if(!argument_state && !strcmp(argv[i], "-c"))
			argument_state = ERROR_CORRECTION;
		else if(!argument_state && !strcmp(argv[i], "-u"))
			argument_state = UPSCALING;
		else if(!argument_state && (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")))
			*help = 1;
		else if(!argument_state && !strcmp(argv[i], "--verbose"))
			*verbose = 1;
		else if(!argument_state && argv[i][0] == '-'){
			option_string = argv[i];
			return 10;
		} else {
			switch(argument_state){
				case NONE:
					if(*filename)
						return 1;
					*filename = argv[i];
					break;
				case VERSION:
					if(*version)
						return 2;
					inp = atoi(argv[i]);
					if(inp <= 0 || inp >= 41)
						return 4;
					*version = inp;
					argument_state = NONE;
					break;
				case ERROR_CORRECTION:
					if(*error_correction)
						return 3;
					inp = atoi(argv[i]);
					if(inp <= 0 || inp >= 5)
						return 5;
					*error_correction = inp;
					argument_state = NONE;
					break;
				case UPSCALING:
					if(*upscaling)
						return 8;
					inp = atoi(argv[i]);
					if(inp <= 0)
						return 9;
					*upscaling = inp;
					argument_state = NONE;
					break;
			}
		}
	}
	if(argument_state)
		return 6;
	if(!*filename && !*help)
		return 7;
	return 0;
}

void get_data(unsigned char **data, unsigned int *data_size){
	unsigned int allocated_size = INPUT_BUFFER_INCREASE;
	int inp;

	*data_size = 0;
	*data = malloc(sizeof(unsigned char)*allocated_size);
	while((inp = fgetc(stdin)) != EOF){
		(*data)[*data_size] = inp;
		(*data_size)++;
		if(*data_size >= allocated_size){
			*data = realloc(*data, allocated_size + INPUT_BUFFER_INCREASE);
			allocated_size += INPUT_BUFFER_INCREASE;
		}
	}

	(*data_size)--;
}

int main(int argc, char **argv){
	FILE *output_file;
	unsigned char error;
	unsigned char version;
	unsigned char error_correction;
	char *filename;
	unsigned char *data;
	unsigned int data_size;
	unsigned int upscaling = 0;
	unsigned char help = 0;
	unsigned char verbose = 0;

	srand(time(NULL));
	argument_errors[1] = "Expected no more than one output file\n";
	argument_errors[2] = "Expected no more than one version\n";
	argument_errors[3] = "Expected no more than one error correction value\n";
	argument_errors[4] = "Version must be between 1 and 40\n";
	argument_errors[5] = "Error correction value must be between 1 and 4\n";
	argument_errors[6] = "Trailing argument\n";
	argument_errors[7] = "Expected output file\n";
	argument_errors[8] = "Expected no more than one upscaling value\n";
	argument_errors[9] = "Upscaling value must be positive\n";
	argument_errors[10] = "Unrecognized option";

	error = parse_arguments(argc, argv, &version, &error_correction, &upscaling, &filename, &help, &verbose);
	if(help){
		printf("Usage: qrutil [args] [BMP output file]\n"
		       "\t-h\n"
		       "\t--help    Display this message\n"
		       "\t-v [num]  Generate qr code of this version (1-40)\n"
		       "\t-c [num]  Generate qr code with this level of error correction (1-4)\n"
		       "\t-u [num]  Upscale output image. Each module is num x num pixels\n"
		       "\t--verbose Display information about the generated qr code\n"
		);
		return 0;
	}
	if(!upscaling)
		upscaling = 1;
	if(error == 10){
		fprintf(stderr, "Error: %s '%s'\n", argument_errors[10], option_string);
		return error;
	} else if(error){
		fprintf(stderr, "Error: %s", argument_errors[error]);
		return error;
	}

	output_file = fopen(filename, "wb");
	if(!output_file){
		fprintf(stderr, "Error: could not open output file\n");
		return 1;
	}

	get_data(&data, &data_size);
	if(generate_qr_code(output_file, data, data_size, version, error_correction, upscaling, verbose)){
		fclose(output_file);
		fprintf(stderr, "Error: too much data to store\n");
		return 1;
	}

	fclose(output_file);

	return 0;
}

