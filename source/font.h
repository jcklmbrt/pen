
#ifndef _INCLUDE_FONT_H
#define _INCLUDE_FONT_H


#include <stdbool.h>


enum fontrange {
	FONTRANGE_ASCII        = 128,
	FONTRANGE_ASCII_EX     = 256,
	FONTRANGE_MULTILINGUAL = 65536,
	FONTRANGE_MASK = FONTRANGE_ASCII    | 
	                 FONTRANGE_ASCII_EX | 
	                 FONTRANGE_MULTILINGUAL
};


enum fontstyle {
	FONTSTYLE_NONE    = 0,
	FONTSTYLE_SHADOW  = 1 << 4,
	FONTSTYLE_OUTLINE = 1 << 5,
	FONTSTYLE_MASK = FONTSTYLE_SHADOW | 
	                 FONTSTYLE_OUTLINE
};


struct font {
	unsigned char *data;
	int            size;
	unsigned int   flags;
	unsigned char  width;
	unsigned char  height;
};


enum fontid {
	TERMINUS_8x16,
	TERMINUS_6x12,
	FONTID_NUM
};


extern struct font fontdata[FONTID_NUM];


#endif
