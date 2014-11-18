#ifndef __ADATA_TYPE__
#define __ADATA_TYPE__

// Default Using Datatypes Define
typedef char				AInt8;
typedef unsigned char		AUInt8;
typedef short				AInt16;
typedef unsigned short		AUInt16;
typedef long				AInt32;
typedef unsigned long		AUInt32;
typedef __int64				AInt64;
typedef unsigned __int64	AUInt64;
typedef int					AInt;
typedef unsigned int		AUInt;

typedef float				AReal32;
typedef double				AReal64;

enum ABool32 { AFalse = 0, ATrue = 1, ABool32_Force32 = 0xFFFFFFFF };

enum APROJECT_MESSAGE
{
	AM_FIRST = 0xF001,
	AM_UIBUTTONUP = AM_FIRST,
	AM_UIBUTTONDOWN,

	AM_MOUSEMOVE,
	AM_LBUTTONUP,
	AM_LBUTTONDOWN,
	AM_RBUTTONUP,
	AM_RBUTTONDOWN,
	AM_MBUTTONUP,
	AM_MBUTTONDOWN,
	AM_MOUSEWHEEL,
};

enum AError {
	// Success
	AE_SUCCESS						= 0x00000000,

	AE_ERROR_ALREADY_INITIALIZE,
	AE_ERROR_NOT_INITIALIZE,
	AE_ERROR_ALREADY_HWND,
	AE_ERROR_NOTSET_DEVICECALLBACKS,

	AE_ERROR_CREATEWINDOW,
	AE_ERROR_REGISTER_WINDOW,

	AE_ERROR_ALREADY_DIRECT3D,
	AE_ERROR_DIRECT3D_CREATE,
	AE_ERROR_GET_DISPLAYMODE,
	AE_ERROR_FAILED_CREATEDEVICE,

	AE_ERROR_EQUAL_NAME,
	AE_ERROR_NOT_FIND_NAME,

	AE_ERROR_GET_MOUSEDATA,
	AE_ERROR_KEYBOARD_ACQUIRE,

	AE_ERROR_CREATE_VERTEXBUFFER,
	AE_ERROR_LOCK_VERTEXBUFFER,
	AE_ERROR_CREATE_INDEXBUFFER,
	AE_ERROR_LOCK_INDEX_BUFFER,

	AE_ERROR_FIND_TEXTUREFILE,
};

enum AKeyState
{
	AK_KEYNONE = 0,
	AK_KEYUP,
	AK_KEYDOWN,
	AK_KEYLONG,
	AK_KEYDOUBLE,
};

typedef struct
{
	D3DXVECTOR3 Dir;
	D3DXVECTOR3 Origin;
} RAY;

// Location of the line structure
typedef struct
{
	AReal64 _x1, _y1;
	AReal64 _x2, _y2;
} ALine;

typedef struct
{
	float tU;
	float tV;
} AUVPosition;

// 3D structure of the line location information
typedef struct
{
	AReal64 _x1, _y1, _z1;
	AReal64 _x2, _y2, _z2;
} ALineZ;

// Coordinate the use of UV
typedef struct
{
	AReal32		x, y, z;
#ifdef AUsingTangent
	AReal32		texCoord [2];
	AReal32		nx, ny, nz;
	AReal32		tx, ty, tz, tw;
#else
	#if AUsingRHW == 1
		AReal32		rhw;
	#elif AUsingRHW == 2
		AReal32		w;
	#endif
	#ifdef AUsingVectorNormal
		AReal32		nx, ny, nz;
	#endif
	#if AUsingColor == 1
		D3DCOLOR	diffColor;
	#elif AUsingColor == 2
		D3DCOLOR	specColor;
	#endif

	#if ATextureCount != 0
		// UV coordinates that contains structures
		AUVPosition	tUV [ATextureCount];
	#endif
#endif
} CustomVertex;

// The structure of the index buffer
typedef struct
{
	AUInt16 _0, _1, _2;
} CustomIndex;

// Callback Functions
typedef HRESULT	(CALLBACK *LPCALLBACKD3D9DEVICECREATED)(LPDIRECT3DDEVICE9 pD3DDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
typedef HRESULT (CALLBACK *LPCALLBACKD3D9DEVICERESETED)(LPDIRECT3DDEVICE9 pD3DDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
typedef void    (CALLBACK *LPCALLBACKD3D9DEVICEDESTROYED)();
typedef void	(CALLBACK *LPCALLBACKMOUSESEVENTED)(AUInt32 unType, AInt nX, AInt nY, AInt nMouseWheelDelta, bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown);
typedef void	(CALLBACK *LPCALLBACKKEYBOARDSEVENTED)(AUInt vkKey, AKeyState kState, bool bCtrlDown, bool bAltDown, bool bShiftDown);
typedef void	(CALLBACK *LPCALLBACKOBJECTSEVENTED)(const WCHAR* pTitle, UINT msg, WPARAM wParam, LPARAM lParam);

#define ROUND(a) ((AInt)(a+0.5f))

#define RAD2DEG(x) ((x) * 57.29577951308232087684)
#define DEG2RAD(x) ((x) * 0.017453292519943295762)

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p); (p) = NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete [] (p); (p) = NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release (); (p) = NULL; } }
#endif

// Kind of debugging message
enum ADebug
{
	DebugError		= 0,
	DebugClear, DebugChange, DebugSelect, DebugMsg,

	AE_MSG
};

// Color of the debugging messages
enum AColorList
{
	AE_BLACK = 0,			//°ËÁ¤
	AE_BLUE,				//ÆÄ¶û
	AE_GREEN,				//ÃÊ·Ï
	AE_CYAN,				//½Ã¾È   (ÆÄ¶û+ÃÊ·Ï)
	AE_RED,					//»¡°­
	AE_MAGENTA,				//¸¶Á¨Å¸ (ÆÄ¶û+»¡°­)
	AE_BROWN,				//°¥»ö   (ÃÊ·Ï+»¡°­)
	AE_LIGHTGRAY,			//Èò»ö   (ÆÄ¶û+ÃÊ·Ï+»¡°­)
	AE_DARKGRAY,			//È¸»ö   (LIGHTBLACK) 
	AE_LIGHTBLUE,			//¹àÀº ÆÄ¶û
	AE_LIGHTGREEN,			//¹àÀº ³ì»ö
	AE_LIGHTCYAN,			//¹àÀº ½Ã¾È
	AE_LIGHTRED,			//¹àÀº »¡°­
	AE_LIGHTMAGENTA,		//¹àÀº ¸¶Á¨Å¸
	AE_YELLOW,				//¹àÀº ³ë¶û
	AE_WHITE,				//¹àÀº ÇÏ¾á»ö

	AE_COLOR_ALL
};

#if defined (DEBUG) | defined (_DEBUG)
	// Debug Log window displays
	inline void __cdecl DebugLog (const char* _Format,...)
	{
		char acTemp1 [2048] = {0,};
		char acTemp2 [2048] = {0,};

		va_list argList;
		va_start (argList, _Format);
		vsprintf (acTemp1, _Format, argList);
		va_end (argList);

		sprintf (acTemp2, "[L%d]:%s - %s\n", __LINE__, __FUNCTION__, acTemp1);
		OutputDebugStringA (acTemp2);
	}

	// Drawn to the console color setting
	inline void SetColorConsole (AColorList cColor)
	{
		SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), cColor);
	}

	// Show console log
	inline void __cdecl DebugConsoleLog (ADebug _Type, const char* _Format,...)
	{
		switch (_Type)
		{
			case DebugError:
				SetColorConsole (AE_LIGHTRED);
				printf ("[Error]   ");
				break;
			case DebugClear:
				SetColorConsole (AE_WHITE);
				printf ("[Clear]   ");
				break;
			case DebugChange:
				SetColorConsole (AE_YELLOW);
				printf ("[Change]  ");
				break;
			case DebugSelect:
				SetColorConsole (AE_YELLOW);
				printf ("[Select]  ");
				break;
			case DebugMsg:
				SetColorConsole (AE_LIGHTMAGENTA);
				printf ("[Message] ");
				break;
			default:
				SetColorConsole (AE_LIGHTGREEN);
		}

		char acTemp [2048] = {0,};
		va_list argList;
		va_start (argList, _Format);
		vsprintf (acTemp, _Format, argList);
		va_end (argList);

		printf ("%s\n", acTemp);
		SetColorConsole (AE_LIGHTGRAY);
	}
#else
	inline void __cdecl DebugLog (const char* _Format,...) {}
	inline void SetColorConsole (AColorList cColor) {}
	inline void __cdecl DebugConsoleLog (ADebug _Type, const char* _Format,...) { }
#endif

#endif