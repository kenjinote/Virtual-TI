//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "Jpegfmt.h"
#include "bitimage.h"
#include "jpegdeco.h"
#include "jpegenco.h"

int ReadJPEG(FILE *fp,Graphics::TBitmap *bmp)
{
    if (!fp) return 0;
    BitmapImageDecoder *dec=new JpegDecoder;
    BitmapImage bi;
    dec->ReadImage(fp,bi);
    bmp->Width=bi.Width();
    bmp->Height=bi.Height();
    bmp->PixelFormat=pf24bit;
    for (int y=0;y<bi.Height();y++)
        memcpy(bmp->ScanLine[y],bi[y],bi.Width()*3);
    delete dec;
    return 1;
}

void WriteJPEG(FILE *fp,Graphics::TBitmap *bmp,int xSize,int ySize,int highQuality)
{
    if (!fp) return;
    JpegEncoder *enc=new JpegEncoder;
    BitmapImage bi;
    bi.SetSize(0,24,xSize,ySize);
    for (int y=0;y<ySize;y++)
        memcpy(bi[y],bmp->ScanLine[y],xSize*3);
    enc->SetGrayscale(false);
    enc->SetQuality(highQuality?66:56);
    enc->SetProgressive(false);
    enc->SetRowsPerRestart(0);
    enc->WriteImage(fp,bi);
    delete enc;
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
