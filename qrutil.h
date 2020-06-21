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

extern void generate_bmp(FILE *output_file, struct qr_code *qr, unsigned int upscaling, unsigned char invert);
