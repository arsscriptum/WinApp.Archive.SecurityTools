/* base64dec.c - a simple Base64 decoder
 * Copyright (c) Ragestorm 2003 (see http://www.ragestorm.net) 
 * Can be freely redistributed under the terms of the GNU Lesser Public
 * License
 */
#include <stdio.h>

void base64_decode(char *, long);

// 128-value reverse lookup table
char rtable[] = {0,	0,	0,	0,	0,	0,	0,	0,
		0,	0,	0,	0,	0,	0,	0,	0,
		0,	0,	0,	0,	0,	0,	0,	0,
		0,	0,	0,	0,	0,	0,	0,	0,
		0,	0,	0,	0,	0,	0,	0,	0,
		0,	0,	0,	62,	0,	0,	0,	63,
		52,	53,	54,	55,	56,	57,	58,	59,
		60,	61,	0,	0,	0,	0,	0,	0,
		0,	0,	1,	2,	3,	4,	5,	6,
		7,	8,	9,	10,	11,	12,	13,	14,
		15,	16,	17,	18,	19,	20,	21,	22,
		23,	24,	25,	0,	0,	0,	0,	0,
		0,	26,	27,	28,	29,	30,	31,	32,
		33,	34,	35,	36,	37,	38,	39,	40,
		41,	42,	43,	44,	45,	46,	47,	48,
		49,	50,	51,	0,	0,	0,	0,	0};

int main()
{
	char p[1024];
	fgets(p, 1024, stdin);
	base64_decode(p, strlen(p) - 1); // 1 for the extra LF
	printf("\n");
}

void base64_decode(char * data, long len)
{
	unsigned short t = 0;	// the "buffer" of this story
// c has the number of the highest currently
	char c = 0;
// Valid base64 input length is always divisible by 4!
	if(len == 0 || (len & 0x3 != 0))
		return;

	while(len-- && *data != '=')
	{
// We take only the low 7 bits in order to stay within the table
		t = (t << 6) + rtable[*data++ & 0x7f];
		if(c >= 2)
		{
// We should have done c -= 8, c += 6, but we optimize (of course)
			c -= 2;
// The 8 bits starting at c are the next meaningful output byte
			printf("%c", t >> c);
// And we get rid of what we printed, so we won't have to deal with it
// at the next cycle
			t &= 0xffff >> (16 - c);
		} 
		else
			c += 6; 
	}
}

