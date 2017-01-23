#include <windows.h>
#include "wcxhead.h"

//============================================

typedef struct
{
char file[MAX_PATH];
char name[32];
char type[5];
char id[5];
char pc[5];
int pc_type;
short int num_records;
int offset[65535];
unsigned int len;
int time;
} PDB_data;


//============================================


PDB_data data;
int nfile;
char head[78];

HINSTANCE hinst;
char inifile[MAX_PATH];

//============================================

#define nTYPES 4

char id[nTYPES][5] = 
{
"REAd",
"Foto",
"MNBi",
"GPlm"
};

char type[nTYPES][5] = 
{
"TEXt",
"Foto", 
"DATA",
"zTXT"
};

char pc[nTYPES][5] = 
{
".txt",
"",
".txt",
".txt"
};

//============================================

DWORD Swap32(DWORD x)
{

return ( (x&0xff) << 24 ) | ( (x&0xff00) << 8 ) | ( (x&0xff0000) >> 8 ) | ( (x&0xff000000) >> 24 );

}

//============================================

WORD Swap16(WORD x)
{

return ( (x&0xff) << 8 ) | ( (x&0xff00) >> 8 ) ;

}


//============================================

int _atoi ( char *s)
{

int i, n;
 
n = 0;

for ( i = 0; s[i]>='0' && s[i]<='9'; ++i)
             n = 10 * n + s[i] - '0';

return n;
}

//============================================

int digit(char c)
{

return ( ( '0' <= c ) && ( c <= '9') );

} 


//============================================


int number(char *s)
{

int i, l;

l = lstrlen(s);
for (i = 0; i < l; i++)
	if ( ! digit( *(s+i) )  )
		return 0;

return 1;
}


//============================================

int GetNullTime()
{
return 0<<25 | 1<<21 | 1<<16 |
			0<<11 | 0<<5 | 0;

}


//============================================


int isLeap(int y)
{

if (y%4 != 0)
	return FALSE;
	
if ((y%100 == 0)&&(y%400 != 0))
	return FALSE;

return TRUE;

}

//============================================

int GetPalmTime(unsigned v)
{

int y, m, d;
int n_d, v_t, h, min, s;

const SECS_IN_DAY = 86400;
const SECS_IN_HOUR = 3600;
const SECS_IN_MIN = 60;

n_d = v / SECS_IN_DAY;
v_t = v % SECS_IN_DAY;
h = v_t / SECS_IN_HOUR;
v_t = v_t % SECS_IN_HOUR;
min = v_t / SECS_IN_MIN;
s = v_t % SECS_IN_MIN;


int ms[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

m = 0;
y = 1970;

d = v / SECS_IN_DAY;

for (;;)
{

if ( d > ms[m] )
	{
	if (m == 1)
		{
		if ( isLeap(y) )
			d-=29;
		else d-=28;
		}
	else 
		d-=ms[m];

	if (m == 11)
		{
		m = 0;
		y++;
		}
	else m++;

	}
else break;

}

if (y < 1980)
	y = 1980;

if (y > 2020)
	y += 1904 - 1970;


return ((y-1980)<<25) | ( (m+1)<<21 )  | ( (d+1)<<16 ) |
			h<<11 | min<<5 | s/2;

}


//============================================

int TypeCheck()
{

int i;

for ( i = 0; i < nTYPES; i++)
if ( (!lstrcmp(id[i], data.id) ) && (!lstrcmp(type[i], data.type)) )
	{
	lstrcpy (data.pc, pc[i]);
	data.pc_type = i;
	return 1;
	}

return 0;
}


//============================================


void RepairName(char *s)
{

int i, l;

l = lstrlen(s);
for (i = 0; i < l; i++)
	switch ( *(s+i) )
		{
		case '\\':
		case '/':
		case ':':
		case '*':
		case '?':
		case '\"':
		case '<':
		case '>':
		case '|':
				*(s+i) = '_';
				break;
		
		default:
			break;
		};

}


//============================================

typedef int (*uncompress) (unsigned char *dest, DWORD *destLen,
                                   const unsigned char *source, DWORD sourceLen);

//============================================

int TypeProcess(HANDLE fr, HANDLE fw)
{

unsigned char *buf;
unsigned char *buf2;
DWORD len, r, d;
int i;
int pos;
short int _short;
HINSTANCE hLib;
uncompress unk;

switch (data.pc_type)
{

case 0:
	len = data.len - data.offset[1];
	buf = (unsigned char *) LocalAlloc(LPTR, len);
	buf2 = (unsigned char *) LocalAlloc(LPTR, len*2);
	SetFilePointer(fr, data.offset[1], NULL, FILE_BEGIN);
	ReadFile (fr, buf, len, &r, NULL);

	pos = 0;
	for ( i = 0 ; i < len; i++ )
		{
		if (buf[i] == 10)
			{
			memcpy(buf2+pos, "\r\n", 2);
			pos += 2;
			continue;
			}

		if ( (buf[i] > (char) 31) || (buf[i] == 9) )
			{
			memcpy(buf2+pos, &buf[i], 1);
			pos++;
			}

		}

	WriteFile (fw, buf2, pos, &r, NULL);
	LocalFree(buf2);
	LocalFree(buf);
	break;


case 1:
	for (i = 0; i < data.num_records; i++)
		{
		if (i+1 < data.num_records)
			len = data.offset[i+1] - data.offset[i];
		else
			len = data.len - data.offset[i];

		buf = (unsigned char *) LocalAlloc(LPTR, len);
		SetFilePointer(fr, data.offset[i]+8, NULL, FILE_BEGIN);
		ReadFile (fr, buf, len-8, &r, NULL);
		WriteFile (fw, buf, len-8, &r, NULL);
		LocalFree(buf);
		}
	break;


case 2:
	for (i = 0; i < data.num_records; i++)
		{
		if (i+1 < data.num_records)
			len = data.offset[i+1] - data.offset[i];
		else
			len = data.len - data.offset[i];

		buf = (unsigned char *) LocalAlloc(LPTR, len);
		buf2 = (unsigned char *) LocalAlloc(LPTR, len*20);

		SetFilePointer(fr, data.offset[i], NULL, FILE_BEGIN);
		ReadFile (fr, &_short, 2, &r, NULL);
		_short = Swap16 (_short);

		wsprintf ((char *) buf2, "%2d.%2d.%d   ", _short & 31, (_short & 480)>>5, 
					( (_short & 65024)>>9 ) - 66 + 1970 );

		WriteFile (fw, buf2, lstrlen((char *)buf2), &r, NULL);
		ReadFile (fr, &_short, 2, &r, NULL);
		ReadFile (fr, buf, len-4, &r, NULL);
		WriteFile (fw, buf, len-5, &r, NULL);
		WriteFile (fw, "\r\n", 2, &r, NULL);

		LocalFree(buf2);
		LocalFree(buf);

		}
	break;

case 3:
	hLib = LoadLibrary ("zlib1.dll");
	if (hLib == NULL)
		break;

	unk = (uncompress) GetProcAddress(hLib,"uncompress");
	if (unk == NULL)
		break;

	len = data.len - data.offset[1];
	d = len*5;

	buf = (unsigned char *) LocalAlloc(LPTR, len);
	buf2 = (unsigned char *) LocalAlloc(LPTR, d);

	SetFilePointer(fr, data.offset[1], NULL, FILE_BEGIN);
	ReadFile (fr, buf, len, &r, NULL);
	(*unk)(buf2, &d, buf, len);

	LocalFree(buf);
	d = len*5;
	buf = (unsigned char *) LocalAlloc(LPTR, d);

	pos = 0;
	for ( i = 0 ; i < d; i++ )
		{
		if (buf2[i] == 10)
			{
			memcpy(buf+pos, "\r\n", 2);
			pos += 2;
			continue;
			}

		if ( (buf2[i] > (char) 31) || (buf2[i] == 9) )
			{
			memcpy(buf+pos, &buf2[i], 1);
			pos++;
			}

		}

	WriteFile (fw, buf, pos, &r, NULL);

	LocalFree(buf2);
	LocalFree(buf);
	FreeLibrary(hLib);

	break;

};



CloseHandle(fr);

return 0;
}



//============================================

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )

{

char LangName[3];
char path[MAX_PATH];
int c, i;

hinst = (HINSTANCE) hModule;

GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, LangName, sizeof(LangName));


GetModuleFileName(hinst, path, MAX_PATH-1);

c = 0;
for (i = 0; i < lstrlen (path); i++ )
	if (path[i] == '\\')
		c = i;

path[c+1] = '\0';
wsprintf (inifile, "%slang\\%s\\wcx_pdb.ini", path, LangName);

return TRUE;
}


//============================================


int __stdcall GetPackerCaps()
{
return PK_CAPS_MULTIPLE	| PK_CAPS_HIDE | PK_CAPS_NEW;
}


//============================================


HANDLE __stdcall OpenArchive(tOpenArchiveData *ArchiveData)
{

HANDLE f;
DWORD r;
int i;

char a[4];


memset(&data, 0, sizeof(data));

lstrcpy (data.file, ArchiveData -> ArcName);

f = CreateFile(  ArchiveData -> ArcName, GENERIC_READ, 0, NULL, 
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 

ReadFile (f, &head, 78, &r, NULL);

memcpy ((char *) &data.num_records, head+76, 2);
data.num_records = Swap16 (data.num_records);

memcpy (data.name, head, 32);
RepairName(data.name);

memcpy ( data.type, head+60, 4);
data.type[4] = 0;

memcpy ( data.id, head+64, 4);
data.id[4] = 0;


memcpy ((char *) &data.time, head+36, 4);
data.time = GetPalmTime( (unsigned) Swap32 (data.time) );

for (i = 0; i < data.num_records; i++)
	{
	ReadFile (f, a, 4, &r, NULL);

	memcpy ((char *) &data.offset[i], a, 4);
	data.offset[i] = Swap32(data.offset[i]);

	ReadFile (f, a, 4, &r, NULL);
	}

data.len = SetFilePointer(f, 0, NULL, FILE_END);

CloseHandle(f);
ArchiveData -> OpenResult = 0;

if ( TypeCheck() )
	nfile = -2;
else 
	nfile = -1;

return (char *) &data;
}


//============================================


int __stdcall ReadHeader(HANDLE hArcData, tHeaderData *HeaderData)
{


char name[260];
char str_records[64]; 
char str_converted[64];
char _RECORDS_[] = {"Records"};
char _CONVERTED_[] = {"Converted"};
char _WCX_PDB_[] = {"WCX_PDB"};

GetPrivateProfileString( _WCX_PDB_, _RECORDS_,  _RECORDS_, str_records, 63, inifile );
GetPrivateProfileString( _WCX_PDB_, _CONVERTED_,  _CONVERTED_, str_converted, 63, inifile );

if (nfile == data.num_records)
	return 1;

memset(HeaderData, 0, sizeof (tHeaderData));

HeaderData -> FileTime = data.time;

if (nfile == -2)
	{
	wsprintf (name, "%s\\WCX_PDB_%s%s", str_converted, data.name, data.pc);
	lstrcpy (HeaderData -> FileName, name);
	HeaderData -> PackSize = HeaderData -> UnpSize = data.len-data.offset[0];
	nfile++;
	return 0;
	}


if (nfile == -1)
	{
	wsprintf (name, "%s.%s", data.name, data.type);
	lstrcpy (HeaderData -> FileName, name);
	HeaderData -> PackSize = HeaderData -> UnpSize = data.len-data.offset[0];
	nfile++;
	return 0;
	}


if (nfile < 10)		
	wsprintf (name, "%s\\0000%d", str_records, nfile);
else if (nfile < 100)		
		wsprintf (name, "%s\\000%d", str_records, nfile);
	else if (nfile < 1000)		
			wsprintf (name, "%s\\00%d", str_records, nfile);
		else if (nfile < 10000)		 
				wsprintf (name, "%s\\0%d", str_records, nfile);
			else wsprintf (name, "%s\\%d", str_records, nfile);


lstrcpy (HeaderData -> FileName, name);

if (nfile+1 < data.num_records)
	HeaderData -> PackSize = HeaderData -> UnpSize = data.offset[nfile+1]-data.offset[nfile];
else
	HeaderData -> PackSize = HeaderData -> UnpSize = data.len-data.offset[nfile];

nfile++ ;
return 0;
}


//============================================


int __stdcall ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName)
{

char w[]={"WCX_PDB_"};
char wcx[sizeof (w)];
char fn[260];
char *buf;
int num;
int l;
HANDLE fr, fw;
DWORD r;

if(Operation == PK_SKIP || Operation == PK_TEST)
		return 0;


GetFileTitle(DestName, fn, 260);

lstrcpyn(wcx, fn, sizeof(w) );
if ( !lstrcmp(w, wcx) )
	{
	fr = CreateFile(  data.file, GENERIC_READ, 0, NULL, 
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 

	fw = CreateFile( DestName, GENERIC_WRITE, 0, NULL, 
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	TypeProcess(fr, fw);

	CloseHandle(fw);
	CloseHandle(fr);

	return 0;
	}

if ( number (fn) )
	{
	num = _atoi (fn);
	if (num+1 < data.num_records)
		l = data.offset[num+1] - data.offset[num];
	else
		l = data.len - data.offset[num];
	}
else
	{
	l = data.len - data.offset[0];
	num = 0;
	}

fr = CreateFile(  data.file, GENERIC_READ, 0, NULL, 
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 

fw = CreateFile( DestName, GENERIC_WRITE, 0, NULL, 
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


SetFilePointer(fr, data.offset[num], NULL, FILE_BEGIN);

buf = (char *) LocalAlloc(LPTR, l);

ReadFile (fr, buf, l, &r, NULL);
WriteFile (fw, buf, l, &r, NULL);

LocalFree(buf);
CloseHandle(fw);
CloseHandle(fr);

return 0;
}


//============================================


int __stdcall CloseArchive(HANDLE *hArcData)
{
memset (&data, 0, sizeof(data));
return 0;
}


//============================================


int __stdcall PackFiles (char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags)
{

WIN32_FIND_DATA FindFileData;
char infname[MAX_PATH];
unsigned char *buf;
unsigned char *buf2;
DWORD i, pos, len, r, t;
HANDLE fr, fw;
WORD j;

if ( INVALID_HANDLE_VALUE != FindFirstFile(PackedFile, &FindFileData) )
	return E_NOT_SUPPORTED;

wsprintf (infname, "%s%s", SrcPath, AddList);


fr = CreateFile(  infname, GENERIC_READ, 0, NULL, 
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 


len = SetFilePointer(fr, 0, NULL, FILE_END);
SetFilePointer(fr, 0, NULL, FILE_BEGIN);

buf = (unsigned char *) LocalAlloc(LPTR, len);
buf2 = (unsigned char *) LocalAlloc(LPTR, len*2);


ReadFile (fr, (char *) buf, len, &r, NULL);


fw = CreateFile( PackedFile, GENERIC_WRITE, 0, NULL, 
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

pos = 0;
for ( i = 0 ; i < len; i++ )
	{

	if ( (buf[i] > (unsigned char) 31) || (buf[i] == 9) || (buf[i] == 10))
		{
		memcpy(buf2+pos, buf+i, 1);
		pos++;
		}

	}

WriteFile (fw, AddList, 31, &r, NULL);
WriteFile (fw, "\0", 1, &r, NULL);
j = 0;
WriteFile (fw, &j, 2, &r, NULL);
j = Swap16(3);
WriteFile (fw, &j, 2, &r, NULL);
memset (buf, 0, len);
WriteFile (fw, buf, 24, &r, NULL);
WriteFile (fw, "TEXt", 4, &r, NULL);
WriteFile (fw, "REAd", 4, &r, NULL);
WriteFile (fw, buf, 8, &r, NULL);
j = Swap16(pos/4096+2);
WriteFile (fw, &j, 2, &r, NULL);

j = pos/4096+1;

t = Swap32 ( 78 + (j+1)*8 );

WriteFile (fw, &t, 4, &r, NULL);
WriteFile (fw, buf, 1, &r, NULL);
WriteFile (fw, buf, 3, &r, NULL);

for ( i = 0 ; i < j; i++)
	{
	t = Swap32 ( 78 + (j+1)*8 + i*4096 + 16);

	WriteFile (fw, &t, 4, &r, NULL);
	WriteFile (fw, buf, 1, &r, NULL);
	t = i << 8;
	WriteFile (fw, &t, 3, &r, NULL);
	}

//WriteFile (fw, buf, 16, &r, NULL);

t = Swap32 ( 0x10000 );
WriteFile (fw, &t, 4, &r, NULL);

t = Swap32 ( pos );
WriteFile (fw, &t, 4, &r, NULL);

j = Swap16 (pos/4096+1);
WriteFile (fw, &j, 2, &r, NULL);

t = Swap32 ( 0x10000000 );
WriteFile (fw, &t, 4, &r, NULL);

j = 0;
WriteFile (fw, &j, 2, &r, NULL);


WriteFile (fw, buf2, pos, &r, NULL);

CloseHandle(fw);
CloseHandle(fr);

LocalFree(buf2);
LocalFree(buf);


return 0;
}

//============================================


void __stdcall SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1)
{
}

//============================================

void __stdcall SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc)
{
}

//============================================
