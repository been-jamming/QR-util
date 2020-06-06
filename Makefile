default:
	gcc qr_generate.c bmp_generate.c tables.c -Wall -pedantic -Ofast -o qrutil
