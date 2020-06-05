#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define L_CORRECTION 0
#define M_CORRECTION 1
#define Q_CORRECTION 2
#define H_CORRECTION 3

#define MAX_VERSION 40
#define MAX_POLY 30

extern unsigned char qr_polynomials[31][70];

extern unsigned char error_correction_table[][4][2][3];

extern unsigned char alignment_coords[41][7];

extern uint8_t GF256_log2[256];

extern uint8_t GF256_2exp[256];

extern uint16_t format_patterns[4][8];

extern unsigned char format_position1[15][2];

extern unsigned char format_position2[15][2];

extern const unsigned int capacities[][4][2][3];

extern const uint32_t version_patterns[34];

extern const unsigned char version_position[18][2];

void write_bmp_header(FILE *output_file, unsigned int width, unsigned int height);

void place_bmp_pixels(FILE *output_file, unsigned int width, unsigned int height, unsigned char pixels);

struct qr_code{
	unsigned char modules[23][177];
	unsigned char written_mask[23][177];
	unsigned char version;
	int x;
	int y;
	unsigned char up;
	unsigned char next;
};

struct qr_block{
	unsigned char *data;
	unsigned char *poly;
	unsigned int data_size;
	unsigned int error_size;
	unsigned int data_left;
	unsigned int error_left;
};

void write_ppm_header(FILE *output_file, struct qr_code *qr){
	fprintf(output_file, "P6 %d %d 255 ", qr->version*4 + 17, qr->version*4 + 17);
}

void write_ppm_file(FILE *output_file, struct qr_code *qr){
	unsigned int x;
	unsigned int y;
	unsigned char bit;

	for(y = 0; y < qr->version*4 + 17; y++){
		bit = 0x80;
		for(x = 0; x < qr->version*4 + 17; x++){
			if(bit&qr->modules[x/8][y]){
				fputc(0x00, output_file);
				fputc(0x00, output_file);
				fputc(0x00, output_file);
			} else {
				fputc(0xFF, output_file);
				fputc(0xFF, output_file);
				fputc(0xFF, output_file);
			}
			bit >>= 1;
			if(!bit)
				bit = 0x80;
		}
	}
}

void write_bmp_file(FILE *output_file, struct qr_code *qr){
	unsigned int y;
	unsigned int byte_end;
	unsigned int current_byte;

	byte_end = qr->version/2 + 2;

	for(y = qr->version*4 + 17; y; y--)
		for(current_byte = 0; current_byte <= byte_end; current_byte++)
			place_bmp_pixels(output_file, qr->version*4 + 17, qr->version*4 + 17, qr->modules[current_byte][y - 1]);
}

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
	printf("%d %d 0x%02x\n", blocks[0].data_left, blocks[1].data_left, byte);
}

void initialize(unsigned char *version, unsigned char *correction_level, struct qr_block **blocks, unsigned char *data, unsigned int data_size){
	unsigned int i;
	unsigned int j;
	unsigned char header_size;

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
	//*version = 1;
	//header_size = 2;
	//*correction_level = 0;
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

int generate_qr_code(FILE *output_file, unsigned char *data, unsigned int data_size, unsigned char mask){
	struct qr_code qr;
	struct qr_block *blocks;
	unsigned char correction_level;

	initialize(&(qr.version), &correction_level, &blocks, data, data_size);
	if(!blocks)
		return 1;
	memset(qr.modules, 0, sizeof(qr.modules));
	memset(qr.written_mask, 0, sizeof(qr.written_mask));
	//write_bmp_header(output_file, qr.version*4 + 17, qr.version*4 + 17);
	write_ppm_header(output_file, &qr);
	create_finder_patterns(&qr);
	apply_mask_pattern(&qr, mask);
	if(qr.version > 1)
		create_alignment_patterns(&qr);
	create_version_information(&qr);
	create_timing_patterns(&qr);
	create_format_information(&qr, correction_level, mask);
	qr.x = qr.version*4 + 16;
	qr.y = qr.version*4 + 16;
	qr.up = 1;
	qr.next = 0;
	write_data(&qr, blocks, num_blocks(qr.version - 1, correction_level));
	create_error_data(blocks, num_blocks(qr.version - 1, correction_level));
	write_error_data(&qr, blocks, num_blocks(qr.version - 1, correction_level));
	write_ppm_file(output_file, &qr);
	//write_bmp_file(output_file, &qr);
	free_blocks(blocks, num_blocks(qr.version - 1, correction_level));
	return 0;
}

int main(int argc, char **argv){
	FILE *output_file;

	if(argc != 4){
		fprintf(stderr, "Error: expected exactly three arguments\n");
		return 1;
	}

	output_file = fopen(argv[3], "wb");
	if(!output_file){
		fprintf(stderr, "Error: could not open output file\n");
		return 1;
	}

	if(generate_qr_code(output_file, (unsigned char *) argv[2], strlen(argv[2]), atoi(argv[1]))){
		fclose(output_file);
		fprintf(stderr, "Error: too much data to store\n");
		return 1;
	}

	fclose(output_file);

	return 0;
}

