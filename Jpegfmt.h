//---------------------------------------------------------------------------
#ifndef JpegH
#define JpegH

class Graphics::TBitmap;

int ReadJPEG(FILE *fp,Graphics::TBitmap *bmp);
void WriteJPEG(FILE *fp,Graphics::TBitmap *bmp,int xSize,
    int ySize,int highQuality);
//---------------------------------------------------------------------------
#endif
