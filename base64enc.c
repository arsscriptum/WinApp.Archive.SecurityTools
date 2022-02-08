/* base64enc.c - a simple Base64 encoder
 * Copyright (c) Ragestorm 2003 
 * Can be freely redistributed under the terms of the GNU Lesser Public
 * License
 */
#include <stdio.h>

char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
		"0123456789+/==";

void base64_encode(char *, long);

int main()
{
	char p[1024];
	fgets(p, 1024, stdin);
	base64_encode(p, strlen(p));
}

void base64_encode(char * data, long len)
{
	char c = 0;		// counts the received bits  
	unsigned short r = 0;	// packing window

	while(len--)
	{
		c += 8;
		r = (r << 8) + *data++;
		while(c >= 6)	// could be unrolled more
		{
			c -= 6;
			printf("%c", table[r >> c]);	// MSB first
// Remove what we printed:
			r &= 0xffff >> (16 - c);
		}
	}

	if(c)	// '='s due 
		if(r) // one more digit due
			printf("%c%s\n", table[r << (8 / c)], table + 63 + c/2); 
		else
			printf("%s\n", table + 63 + c/2);
	else
		printf("\n");
}

