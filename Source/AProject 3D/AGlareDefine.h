#ifndef __APROJECT_GLARE_DEFINE__
#define __APROJECT_GLARE_DEFINE__

#pragma once

// Define each line of the star.
typedef struct
{
    AInt32		nPasses;
    AReal32		fSampleLength;
    AReal32		fAttenuation;
    AReal32		fInclination;
} STARLINE, *LPSTARLINE;

// Simple definition of the star.
typedef struct
{
    WCHAR*		szStarName;
	AInt32		nStarLines;
    AInt32		nPasses;
    AReal32		fSampleLength;
    AReal32		fAttenuation;
    AReal32		fInclination;
    bool		bRotation;
} STARDEF, *LPSTARDEF;

// Simple definition of the sunny cross filter
typedef struct
{
    WCHAR*		szStarName;
    AReal32		fSampleLength;
    AReal32		fAttenuation;
    AReal32		fInclination;
} STARDEF_SUNNYCROSS, *LPSTARDEF_SUNNYCROSS;

// Star form library
enum ESTARLIBTYPE
{
    STLT_DISABLE = 0,

    STLT_CROSS,
    STLT_CROSSFILTER,
    STLT_SNOWCROSS,
    STLT_VERTICAL,
    NUM_BASESTARLIBTYPES,

    STLT_SUNNYCROSS = NUM_BASESTARLIBTYPES,

    NUM_STARLIBTYPES,
};

// Star generation object
class CAStarDefine
{
public:
	WCHAR					m_strStarName [256];
	AInt32					m_nStarLines;
	LPSTARLINE				m_pStarLine;	// [m_nStarLines]
	AReal32					m_fInclination;
	bool					m_bRotation;	// Rotation is available from outside ?

public:
	static CAStarDefine*	ms_pStarLib;
	static D3DXCOLOR		ms_avChromaticAberrationColor [8];

public:
	CAStarDefine ();
	CAStarDefine (const CAStarDefine& src);
	~CAStarDefine ();

public:
	CAStarDefine& operator= (const CAStarDefine& src)
	{
		Initialize (src);
		return *this;
	}

public:
	HRESULT Construct ();
	void Destruct ();
	void Release ();

	HRESULT Initialize (const CAStarDefine& src);
	HRESULT Initialize (const TCHAR* szStarName, AInt32 nStarLines, AInt32 nPasses, AReal32 fSampleLength, AReal32 fAttenuation, AReal32 fInclination, bool bRotation);
	HRESULT Initialize (ESTARLIBTYPE eType)
	{
		return Initialize (ms_pStarLib [eType]);
	}
	HRESULT Initialize (const STARDEF& starDef)
	{
		return Initialize (starDef.szStarName, starDef.nStarLines, starDef.nPasses, starDef.fSampleLength, starDef.fAttenuation, starDef.fInclination, starDef.bRotation);
	}

	/// Specific star generation
	//  Sunny cross filter
	HRESULT Initialize_SunnyCrossFilter (const TCHAR* szStarName = TEXT ("SunnyCross"), float fSampleLength = 1.0f, float fAttenuation = 0.88f, float fLongAttenuation = 0.95f, float fInclination = D3DXToRadian (0.0f) );

public:
	/// Create star library
	static HRESULT InitializeStaticStarLibs ();
	static HRESULT DeleteStaticStarLibs ();

	/// Access to the star library
	static const CAStarDefine& GetLib (DWORD dwType)
	{
		return ms_pStarLib [dwType];
	}

	static const D3DXCOLOR& GetChromaticAberrationColor (DWORD dwID)
	{
		return ms_avChromaticAberrationColor [dwID];
	}
};

// Clare definition
// Glare form library
enum EGLARELIBTYPE
{
    GLT_DISABLE = 0,

    GLT_CAMERA,
    GLT_NATURAL,
    GLT_CHEAPLENS,
    //GLT_AFTERIMAGE,
    GLT_FILTER_CROSSSCREEN,
    GLT_FILTER_CROSSSCREEN_SPECTRAL,
    GLT_FILTER_SNOWCROSS,
    GLT_FILTER_SNOWCROSS_SPECTRAL,
    GLT_FILTER_SUNNYCROSS,
    GLT_FILTER_SUNNYCROSS_SPECTRAL,
    GLT_CINECAM_VERTICALSLITS,
    GLT_CINECAM_HORIZONTALSLITS,

    NUM_GLARELIBTYPES,
    GLT_USERDEF = -1,
    GLT_DEFAULT = GLT_FILTER_CROSSSCREEN,
};

// Simple glare definition
typedef struct
{
    TCHAR*			szGlareName;
    float			fGlareLuminance;

    float			fBloomLuminance;
    float			fGhostLuminance;
    float			fGhostDistortion;
    float			fStarLuminance;
    ESTARLIBTYPE	eStarType;
    float			fStarInclination;

    float			fChromaticAberration;

    float			fAfterimageSensitivity;    // Current weight
    float			fAfterimageRatio;          // Afterimage weight
    float			fAfterimageLuminance;
} GLAREDEF, * LPGLAREDEF;

// Glare definition
class CAGlareDefine
{
public:
    
    WCHAR					m_strGlareName [256];

    float					m_fGlareLuminance;     // Total glare intensity (not effect to "after image")
    float					m_fBloomLuminance;
    float					m_fGhostLuminance;
    float					m_fGhostDistortion;
    float					m_fStarLuminance;
    float					m_fStarInclination;

    float					m_fChromaticAberration;

    float					m_fAfterimageSensitivity;  // Current weight
    float					m_fAfterimageRatio;        // Afterimage weight
    float					m_fAfterimageLuminance;

    CAStarDefine			m_starDef;

    // Static library
public:
    static CAGlareDefine*	ms_pGlareLib;

    // Public method
public:
    CAGlareDefine ();
    CAGlareDefine (const CAGlareDefine& src);
    ~CAGlareDefine ();

public:
    CAGlareDefine& operator= (const CAGlareDefine& src)
    {
        Initialize (src);
        return *this;
    }

    HRESULT Construct ();
    void Destruct ();
    void Release ();

public:
    HRESULT Initialize (const CAGlareDefine& src);
    HRESULT Initialize (const TCHAR* szStarName, float fGlareLuminance, float fBloomLuminance, float fGhostLuminance, float fGhostDistortion, float fStarLuminance, ESTARLIBTYPE eStarType, float fStarInclination, float fChromaticAberration, float fAfterimageSensitivity, float fAfterimageRatio, float fAfterimageLuminance);
    HRESULT Initialize (const GLAREDEF& glareDef)
    {
        return Initialize (glareDef.szGlareName, glareDef.fGlareLuminance, glareDef.fBloomLuminance, glareDef.fGhostLuminance, glareDef.fGhostDistortion, glareDef.fStarLuminance, glareDef.eStarType, glareDef.fStarInclination, glareDef.fChromaticAberration, glareDef.fAfterimageSensitivity, glareDef.fAfterimageRatio, glareDef.fAfterimageLuminance);
    }
    HRESULT Initialize (EGLARELIBTYPE eType)
    {
        return Initialize (ms_pGlareLib [eType]);
    }

    // Public static method
public:
    /// Create glare library
    static HRESULT InitializeStaticGlareLibs ();
    static HRESULT DeleteStaticGlareLibs ();

    /// Access to the glare library
    static const CAGlareDefine& GetLib (DWORD dwType)
    {
        return ms_pGlareLib [dwType];
    }
};

// Dummy to generate static object of glare
class __CAGlare_GenerateStaticObjects
{
public:
	__CAGlare_GenerateStaticObjects ()
	{
		CAStarDefine::InitializeStaticStarLibs ();
		CAGlareDefine::InitializeStaticGlareLibs ();
	}

	~__CAGlare_GenerateStaticObjects ()
	{
		CAGlareDefine::DeleteStaticGlareLibs ();
		CAStarDefine::DeleteStaticStarLibs ();
	}

    //static __CAGlare_GenerateStaticObjects ms_staticObject;
};

#endif
