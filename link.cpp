#include <vcl.h>
#include <stdio.h>
#include "dumprom.h"

static HANDLE hCom=NULL;
static int error=0;
static int graphLink;
static unsigned char* ptr;

static int cl_getport()
{
    DWORD s;
    int b;

    GetCommModemStatus(hCom,&s);
    b=(s&MS_CTS_ON?1:0)|(s&MS_DSR_ON?2:0);
    return b;
}

static void cl_setport(int b)
{
    EscapeCommFunction(hCom,(b&2)?SETRTS:CLRRTS);
    EscapeCommFunction(hCom,(b&1)?SETDTR:CLRDTR);
}

static void InitCom(int n)
{
    char name[8],str[256];
    DCB dcb;

    if (n>4)
    {
        n-=4;
        graphLink=0;
    }
    else
        graphLink=1;
    if (n>4)
        sprintf(name,"LPT%d:",n);
    else
        sprintf(name,"COM%d:",n);
    hCom=CreateFile(name,GENERIC_READ|GENERIC_WRITE,0,NULL,
        OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (!hCom)
    {
        MessageBox(NULL,"Error opening COM port","Error",MB_OK);
        return;
    }
    dcb.DCBlength=sizeof(DCB);
    dcb.BaudRate=CBR_9600; dcb.ByteSize=8; dcb.Parity=0;
    dcb.StopBits=graphLink?0:1;
    dcb.fNull=0; dcb.fBinary=1; dcb.fParity=0;
    dcb.fOutxCtsFlow=1; dcb.fOutxDsrFlow=1;
    dcb.fDtrControl=0;
    dcb.fOutX=0; dcb.fInX=0;
    dcb.fErrorChar=0; dcb.fRtsControl=RTS_CONTROL_ENABLE;
    SetupComm(hCom,1024,1024);
    GetCommState(hCom,&dcb);
    sprintf(str,"COM%d: baud=9600 parity=N data=8 stop=1",n);
    BuildCommDCB(str,&dcb);
    dcb.fNull=0;
    SetCommState(hCom,&dcb);
    COMMTIMEOUTS to;
    to.ReadIntervalTimeout=MAXDWORD;
    to.ReadTotalTimeoutMultiplier=0;
    to.ReadTotalTimeoutConstant=1000;
    to.WriteTotalTimeoutMultiplier=0;
    to.WriteTotalTimeoutConstant=0;
    SetCommTimeouts(hCom,&to);
    error=0;
}

static void CloseCom()
{
	if (hCom) CloseHandle(hCom);
}

static void SendCom(int ch)
{
    if (error) return;
    DWORD i;
    if (graphLink)
        WriteFile(hCom,&ch,1,&i,NULL);
    else
    {
        DWORD startTime=GetTickCount();
        for (i=0;i<8;i++)
        {
            if (ch&1)
                cl_setport(2);
            else
                cl_setport(1);
            while (cl_getport()!=0)
            {
                if ((GetTickCount()-startTime)>250)
                    return;
            }
            cl_setport(3);
            while (cl_getport()!=3)
            {
                if ((GetTickCount()-startTime)>250)
                    return;
            }
            ch>>=1;
        }
    }
}

static int GetCom(int *ch)
{
    DWORD i,j,bit;
    unsigned char c;
    if (graphLink)
    {
        if (!ReadFile(hCom,&c,1,&i,NULL))
            error=1;
        if (!i) error=1;
        *ch=c;
        return i;
    }
    DWORD startTime=GetTickCount();
    int timeout=1000;
    for (i=0,bit=1,*ch=0;i<8;i++)
    {
        while ((j=cl_getport())==3)
        {
            if ((GetTickCount()-startTime)>timeout)
            {
                error=1;
                return 0;
            }
        }
        if (j==1)
        {
            *ch|=bit;
            cl_setport(1);
            j=2;
        }
        else
        {
            cl_setport(2);
            j=1;
        }
        while ((cl_getport()&j)==0)
        {
            if ((GetTickCount()-startTime)>timeout)
            {
                error=1;
                return 0;
            }
        }
        cl_setport(3);
        bit<<=1;
    }
    return 1;
}

static int ComError()
{
    return error;
}

static int WaitForAck()
{
    int i;
    GetCom(&i); GetCom(&i); GetCom(&i); GetCom(&i);
    return i;
}

#define seek(a,b,c) ptr=&(dumpRom[b])
#define get(a) *(ptr++)
#define read(a,b,c,d) memcpy(a,ptr,b*c)

int SendFile(int port)
{
    int csum,type,size,id,i,j;
    char calcName[32],buf[16];

    InitCom(port);
    ptr=dumpRom;
    seek(fp,0x48,SEEK_SET);
    type=get(fp);
    seek(fp,0xa,SEEK_SET);
    read(buf,8,1,fp);
    buf[8]=0; strcpy(calcName,buf);
    seek(fp,0x40,SEEK_SET);
    read(buf,8,1,fp);
    seek(fp,0x4c,SEEK_SET);
    size=get(fp);
    size|=get(fp)<<8;
    size-=0x5a;
    buf[8]=0; strcat(calcName,"\\"); strcat(calcName,buf);
    id=0x89;
    SendCom(id); SendCom(6);
    SendCom(6+strlen(calcName));
    SendCom(0); SendCom((size+2)&0xff);
    SendCom(((size+2)>>8)&0xff);
    SendCom(0); SendCom(0);
    csum=(size+2)&0xff; csum+=((size+2)>>8)&0xff;
    SendCom(type);
    csum+=type;
    SendCom(strlen(calcName));
    csum+=strlen(calcName);
    for (i=0;i<strlen(calcName);i++)
    {
	SendCom(calcName[i]);
        csum+=calcName[i];
    }
    SendCom(csum&0xff); SendCom((csum>>8)&0xff);
    WaitForAck(); WaitForAck();
    SendCom(id); SendCom(0x56); SendCom(0); SendCom(0);
    SendCom(id); SendCom(0x15);
    SendCom((size+6)&0xff); SendCom(((size+6)>>8)&0xff);
    SendCom(0); SendCom(0); SendCom(0); SendCom(0);
    if (ComError())
    {
        CloseCom();
//        printf("Error in transmission\n");
        return 1;
    }
    seek(fp,0x56,SEEK_SET);
    csum=0;
    for (j=0;j<(size+2);j++)
    {
        i=get(fp);
        SendCom(i);
        csum+=i;
    }
    csum&=0xffff;
    SendCom(csum&0xff); SendCom((csum>>8)&0xff);
    WaitForAck();
    SendCom(id); SendCom(0x92); SendCom(0); SendCom(0);
    WaitForAck();
    CloseCom();
    if (ComError())
    {
//        printf("Error in transmission\n");
        return 1;
    }
    return 0;
}

