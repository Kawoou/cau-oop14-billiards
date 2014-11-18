#ifndef __APROJECT_SECUFS_DEFINE__
#define __APROJECT_SECUFS_DEFINE__

#pragma once

#include "Op128bit.h"

typedef char				A1byte;
typedef unsigned char		AU1byte;
typedef short				A2byte;
typedef unsigned short		AU2byte;
typedef long				A4byte;
typedef unsigned long		AU4byte;
typedef long long			A8byte;
typedef unsigned long long	AU8byte;
typedef Op128bit			A16byte;

typedef float				AF4byte;
typedef double				AF8byte;

typedef unsigned char		AB1byte;

typedef bool (*LPCALLBACKFUNCTION)(A16byte,A16byte,bool);

typedef void *SHANDLE, *LHANDLE, *AHANDLE;
#ifdef MAX_PATH
#undef MAX_PATH
#endif
#define MAX_PATH 127

#ifdef INVALID_HANDLE_VALUE
#undef INVALID_HANDLE_VALUE
#endif
#define INVALID_HANDLE_VALUE ((SHANDLE)-1)

typedef struct
{
        int nYear;
        int nMonth;
        int nDay;

        bool bAmpm;
        int nHour;
        int nMinute;
        int nSecond;
} stDateTime;

enum _FILE_SEEK_TYPE {
        FileSeekSet,
        FileSeekNext,
        FileSeekPrev,
        FileSeekEnd
};

enum _RECOVERY_KEY_TYPE {
        KT_SEND,
        KT_TEXT
};

#ifndef DE_READONLY
#define DE_READONLY	0x01
#endif
#ifndef DE_FOLDER
#define DE_FOLDER	0x02
#endif
#ifndef DE_FILE
#define DE_FILE		0x04
#endif
#ifndef DE_LONGNAME
#define DE_LONGNAME	0x08
#endif

#ifndef IsReadOnly
#define IsReadOnly(Attribute)		(Attribute & DE_READONLY)
#endif
#ifndef IsPersenceCheck
#define IsPersenceCheck(Attribute)	(Attribute != 0)
#endif
#ifndef IsFile
#define IsFile(Attribute)		((Attribute & DE_FILE) && (~Attribute & DE_FOLDER))
#endif
#ifndef IsFolder
#define IsFolder(Attribute)		((~Attribute & DE_FILE) && (Attribute & DE_FOLDER))
#endif
#ifndef IsLongFilename
#define IsLongFilename(Attribute)	(Attribute & DE_LONGNAME)
#endif

// Compatibility struct
typedef struct _FIND_DATA {
    AU1byte		dwFileAttributes;
    stDateTime  	ftCreationTime;
    stDateTime  	ftLastAccessTime;
    A16byte		nFileSize;
    AU1byte		szReservedBytes [76];
    A1byte		cFileName [MAX_PATH];
} FIND_DATA, *PFIND_DATA, *LPFIND_DATA;

typedef SHANDLE (*CREATE_VOLUME)            (char*,char*,A16byte,unsigned int);
typedef SHANDLE (*OPEN_VOLUME)              (char*,char*);
typedef bool (*CLOSE_VOLUME)                (SHANDLE*);
typedef bool (*DESTORY_VOLUME)              (char*);
typedef bool (*GET_VOLUME_INFORMATION)      (SHANDLE,char*,char*);
typedef A16byte (*GET_VOLUME_TOTAL_SPACE)   (SHANDLE);
typedef A16byte (*GET_VOLUME_USING_SPACE)   (SHANDLE);
typedef A16byte (*GET_VOLUME_FREE_SPACE)    (SHANDLE);
typedef bool (*CREATE_RECOVERY_KEY)         (SHANDLE,char*);
typedef bool (*CREATE_RECOVERY_KEY_EX)      (SHANDLE,char*,_RECOVERY_KEY_TYPE);
typedef bool (*CHANGE_PASSWORD)             (SHANDLE,char*,char*);
typedef bool (*RECOVERY_PASSWORD)           (char*,char*,char*);
typedef bool (*CHECK_RECOVERY_KEY)          (char*,char*);
typedef bool (*CREATE_DIRECTORY)            (SHANDLE,char*);
typedef bool (*CREATE_DIRECTORY_EX)         (SHANDLE,char*,char*);
typedef bool (*PRESENCE_DIRECTORY)          (SHANDLE,char*);
typedef bool (*REMOVE_DIRECTORY)            (SHANDLE,char*);
typedef bool (*REMOVE_DIRECTORY_EX)         (SHANDLE,char*,char*);
typedef bool (*RENAME_DIRECTORY)			(SHANDLE,char*,char*);
typedef bool (*RENAME_DIRECTORY_EX)			(SHANDLE,char*,char*,char*);
typedef bool (*IMPORT_FILE)                 (SHANDLE,char*,char*,LPCALLBACKFUNCTION);
typedef bool (*IMPORT_FILE_EX)              (SHANDLE,char*,char*,char*,LPCALLBACKFUNCTION);
typedef bool (*EXPORT_FILE)                 (SHANDLE,char*,char*,LPCALLBACKFUNCTION);
typedef bool (*EXPORT_FILE_EX)              (SHANDLE,char*,char*,char*,LPCALLBACKFUNCTION);
typedef bool (*DELETE_FILE)                 (SHANDLE,char*);
typedef bool (*DELETE_FILE_EX)              (SHANDLE,char*,char*);
typedef bool (*RENAME_FILE)					(SHANDLE,char*,char*);
typedef bool (*RENAME_FILE_EX)				(SHANDLE,char*,char*,char*);
typedef LHANDLE (*FIND_FIRST_FILE)          (SHANDLE,char*,LPFIND_DATA);
typedef bool (*FIND_NEXT_FILE)              (LHANDLE,LPFIND_DATA);
typedef bool (*FIND_CLOSE)                  (LHANDLE*);
typedef AHANDLE (*OPEN_FILE)                (SHANDLE,char*);
typedef AHANDLE (*OPEN_FILE_EX)             (SHANDLE,char*,char*);
typedef bool (*READ_FILE)                   (void*,AU8byte,AHANDLE);
typedef bool (*WRITE_FILE)                  (void*,AU8byte,AHANDLE);
typedef bool (*SEEK_FILE)                   (AHANDLE,A16byte,_FILE_SEEK_TYPE);
typedef A16byte (*GET_FILE_OFFSET)          (AHANDLE);
typedef bool (*GET_FILE_EOF)                (AHANDLE);
typedef bool (*CLOSE_FILE)                  (AHANDLE*);

#endif // DEFINE_H