#ifndef __VIDEO_H__
#define __VIDEO_H__

struct PaletteEntry
{
	char r;
	char g;
	char b;
};

void InitGraph(void);
void CloseGraph(void);
void SetPalette(struct PaletteEntry *pal);
extern unsigned char *ScreenBuf;

#endif
