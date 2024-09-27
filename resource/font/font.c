#include <stdbool.h>
#include <font.h>


extern unsigned char terminus8x16_data[4096];
extern unsigned char terminus6x12_data[3072];


struct font fontdata[FONTID_NUM] = {
	{ terminus8x16_data, sizeof(terminus8x16_data), FONTRANGE_ASCII | FONTSTYLE_SHADOW,  8, 16 },
	{ terminus6x12_data, sizeof(terminus6x12_data), FONTRANGE_ASCII | FONTSTYLE_OUTLINE, 6, 12 },
};
