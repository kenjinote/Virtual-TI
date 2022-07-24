//---------------------------------------------------------------------------
#include <vcl.h>
#include <mmsystem.h>
#pragma hdrstop

#include "Sound.h"
#include "rom.h"

static CRITICAL_SECTION cs;
static HWAVEOUT dev=NULL;
static HANDLE bufs[2];
static unsigned char* bufPtr[2];
static HANDLE bufHdr[2];
static int curBlock=0;
static int ready;
static int bufPos,bufCycles,bufLVal,bufRVal;
static WAVEHDR* wh;
static int sampleCycles;

void CALLBACK WaveCallback(HWAVE hWave, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
   WAVEHDR *wh;
   HGLOBAL hg;
   MMRESULT res;

   if(uMsg == WOM_DONE)
   {
       EnterCriticalSection( &cs );

       wh = (WAVEHDR *)dwParam1;

       waveOutUnprepareHeader(dev, wh, sizeof (WAVEHDR));

       //Deallocate the buffer memory
       hg = GlobalHandle(wh->lpData);
       GlobalUnlock(hg);
       GlobalFree(hg);

       //Deallocate the header memory
       hg = GlobalHandle(wh);
       GlobalUnlock(hg);
       GlobalFree(hg);

       ready++;

       LeaveCriticalSection( &cs );
   }
}

void EnableSound()
{
    if (!waveOutGetNumDevs())
    {
        MessageBox(NULL,"No audio devices present","Error",
            MB_OK|MB_ICONSTOP);
        return;
    }
    WAVEFORMATEX outFormatex;
    outFormatex.wFormatTag=WAVE_FORMAT_PCM;
    outFormatex.wBitsPerSample=8;
    outFormatex.nChannels=2;
    outFormatex.nSamplesPerSec=11025;
    outFormatex.nAvgBytesPerSec=22050;
    outFormatex.nBlockAlign=2;
    if (waveOutOpen(&dev,WAVE_MAPPER,&outFormatex,(DWORD)
        WaveCallback,0,CALLBACK_FUNCTION)!=MMSYSERR_NOERROR)
    {
        MessageBox(NULL,"Could not open audio device","Error",
            MB_OK|MB_ICONSTOP);
        return;
    }
    waveOutReset(dev);
    InitializeCriticalSection(&cs);
    soundEnable=1;
    bufs[curBlock]=GlobalAlloc(GMEM_MOVEABLE,2048);
    bufPtr[curBlock]=(unsigned char *)GlobalLock(bufs[curBlock]);
    bufHdr[curBlock]=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(WAVEHDR));
    wh=(WAVEHDR*)GlobalLock(bufHdr[curBlock]);
    wh->dwBufferLength=2048;
    wh->lpData=bufPtr[curBlock];
    ready=2; bufPos=0; bufCycles=0; bufLVal=0; bufRVal=0;
}

void DisableSound()
{
    if (!soundEnable)
        return;
    if(dev)
    {
        while (ready<2)
            Sleep(50);
        waveOutReset(dev);      //reset the device
        waveOutClose(dev);      //close the device
        dev=NULL;
    }

    DeleteCriticalSection(&cs);
    soundEnable=0;
}

void SoundDoOut(int l,int r,int cycles)
{
    if (l) bufLVal+=cycles;
    if (r) bufRVal+=cycles;
    bufCycles+=cycles;
    if (bufCycles>=sampleCycles)
    {
        bufPtr[curBlock][bufPos]=(bufRVal*255)/sampleCycles;
        bufPtr[curBlock][bufPos+1]=(bufLVal*255)/sampleCycles;
        bufCycles=0;
        bufPos+=2;
        bufLVal=0; bufRVal=0;
        if (bufPos>=2048)
        {
            while (!ready)
                Sleep(3);
            ready--;
            waveOutPrepareHeader(dev,wh,sizeof(WAVEHDR));
            waveOutWrite(dev,wh,sizeof(WAVEHDR));
            curBlock=(curBlock+1)&1;
            bufs[curBlock]=GlobalAlloc(GMEM_MOVEABLE,2048);
            bufPtr[curBlock]=(unsigned char *)GlobalLock(bufs[curBlock]);
            bufHdr[curBlock]=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(WAVEHDR));
            wh=(WAVEHDR*)GlobalLock(bufHdr[curBlock]);
            wh->dwBufferLength=2048;
            wh->lpData=bufPtr[curBlock];
            bufPos=0;
        }
    }
}

void SoundOutBits(int l,int r,int cycles)
{
    if (!soundEnable) return;
    while ((bufCycles+cycles)>sampleCycles)
    {
        int left=sampleCycles-bufCycles;
        SoundDoOut(l,r,left);
        cycles-=left;
    }
    SoundDoOut(l,r,cycles);
}

void SoundSetCycles(int n)
{
    sampleCycles=n;
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
