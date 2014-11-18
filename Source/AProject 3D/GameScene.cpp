#include "Include.h"

#if defined(DEBUG) || defined(_DEBUG)

#define DEBUG_BALL				0
#define DEBUG_PHYSICS_LINE		0
#define DEBUG_SHADOW_OFF		0
#define DEBUG_BILLIARDS_OFF		0
#define DEBUG_FLOOR_OFF			0
#define DEBUG_BALL_OFF			0
#define DEBUG_SKYMAP_OFF		0

#endif

/// 기본 설정
#define RED_BALL_COUNT			2

#define BALL_FACTOR				1.2f
#define BALL_SIZE				(0.037f*BALL_FACTOR)

#define BALL_MAX_SPEED			0.14f
#define BALL_SPIN_MAX_SPEED		0.24f
#define BALL_MIN_SPEED			0.00018f
#define BALL_SPIN_RADIUS_DIVIDE	4.0f

#define FRICTIONAL_FORCE		0.0023f
#define FRICTIONAL_SPIN_FORCE	0.8f
#define REDUCE_START			0.105f
#define REDUCE_MINIMUM			0.13f
#define REDUCE_SQUARED			5

#define BALL_COLLISION_DIST		0.001f
#define BALL_COLLISION_MIN		0.005f

#define SCALE_X					(APROJECT_WINDOW->GetWidth() / 1440.0f * 0.65f)
#define SCALE_Y					(APROJECT_WINDOW->GetHeight() / 900.0f * 0.7f)

CGameScene::CGameScene (void)
	: CARootScene ()
{
	Initialize ();

	m_vCameraPos = NULL;
	m_vCameraPosOld = NULL;
	m_vCameraLookat = NULL;
	m_vCameraLookatOld = NULL;

	m_vWallList = new AVector3*[4];
	for (int i = 0; i < 4; i++)
		m_vWallList[i] = new AVector3[3];

	m_vWallList[0][0] = AVector3(-1.68f, 0.0f, 0.824f);
	m_vWallList[0][1] = AVector3(1.68f, 0.0f, 0.824f);
	m_vWallList[0][2] = AVector3(0.0f, 0.0f, 1.0f);
	m_vWallList[1][0] = AVector3(-1.68f, 0.0f, -0.824f);
	m_vWallList[1][1] = AVector3(1.68f, 0.0f, -0.824f);
	m_vWallList[1][2] = AVector3(0.0f, 0.0f, -1.0f);
	m_vWallList[2][0] = AVector3(1.67f, 0.0f, -0.834f);
	m_vWallList[2][1] = AVector3(1.67f, 0.0f, 0.834f);
	m_vWallList[2][2] = AVector3(1.0f, 0.0f, 0.0f);
	m_vWallList[3][0] = AVector3(-1.67f, 0.0f, -0.834f);
	m_vWallList[3][1] = AVector3(-1.67f, 0.0f, 0.834f);
	m_vWallList[3][2] = AVector3(-1.0f, 0.0f, 0.0f);

	m_pBall = new CAXModel*[RED_BALL_COUNT + 2];
	m_vBallPos = new AVector3[RED_BALL_COUNT + 2];
	m_pBallShadow = new CAGeometry*[RED_BALL_COUNT + 2];
	m_vRotateLine = new AVector3[RED_BALL_COUNT + 2];
	m_fRotateSum = new AReal32[RED_BALL_COUNT + 2];
	m_fOldRotate = new AReal32[RED_BALL_COUNT + 2];
	m_vOldRotateLine = new AVector3[RED_BALL_COUNT + 2];
	m_vBallSpeed = new AVector3[RED_BALL_COUNT + 2];
	m_fBallRotateSpeed = new AReal32[RED_BALL_COUNT + 2];
	m_fBallDelta = new AReal32[RED_BALL_COUNT + 2];

	m_pNumberList = new CATexture*[11];
}

CGameScene::~CGameScene (void)
{
	SAFE_DELETE_ARRAY(m_pNumberList);

	SAFE_DELETE_ARRAY(m_pBall);
	SAFE_DELETE(m_vBallPos);
	SAFE_DELETE(m_pBallShadow);
	SAFE_DELETE(m_vRotateLine);
	SAFE_DELETE(m_fRotateSum);
	SAFE_DELETE(m_fOldRotate);
	SAFE_DELETE(m_vOldRotateLine);
	SAFE_DELETE(m_vBallSpeed);
	SAFE_DELETE(m_fBallRotateSpeed);
	SAFE_DELETE(m_fBallDelta);

	for (int i = 0; i < RED_BALL_COUNT + 2; i++)
		SAFE_DELETE(m_vWallList[i]);
	SAFE_DELETE(m_vWallList);

	SAFE_DELETE(m_vCameraPos);
	SAFE_DELETE(m_vCameraPosOld);
	SAFE_DELETE(m_vCameraLookat);
	SAFE_DELETE(m_vCameraLookatOld);
}

void CGameScene::DefaultSetting (LPDIRECT3DDEVICE9 pD3DDevice)
{
	//this->SetColor(0x000000);
	GetCamera()->SetView(&D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(1.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
}

void CGameScene::Create (LPDIRECT3DDEVICE9 pD3DDevice)
{
	CARootScene::Create(pD3DDevice);

	DefaultSetting(pD3DDevice);

	//CAGrid *grid = new CAGrid(20, 1);
	//this->Commit(0, L"01Grid", grid);

#if !DEBUG_SKYMAP_OFF
	/*
	{
		const unsigned int iVertices = 20;

		// Bottom
		{
			CustomVertex vertices[2 * iVertices + 2];
			float anglestep = D3DX_PI / (float)iVertices;

			float cospitch = cos(-D3DX_PI / 2.0f);
			float sinpitch = sin(-D3DX_PI / 2.0f);
			float cosyaw = cos(0.0f);
			float sinyaw = sin(0.0f);
			vertices[0].z = sinpitch;
			vertices[0].x = cospitch * cosyaw;
			vertices[0].y = cospitch * sinyaw;
			vertices[0].nx = vertices[0].x;
			vertices[0].ny = vertices[0].y;
			vertices[0].nz = vertices[0].z;
			vertices[0].tUV[0].tV = (-D3DX_PI / 2.0f + D3DXToRadian(90)) / D3DXToRadian(180);
			vertices[0].tUV[0].tU = 0.0f / D3DXToRadian(360);

			for (int i = iVertices * 2; i >= 0; i--)
			{
				int p = iVertices * 2 - i + 1;
				float cospitch = cos(-D3DX_PI / 2.0f + anglestep);
				float sinpitch = sin(-D3DX_PI / 2.0f + anglestep);
				float cosyaw = cos(anglestep * i);
				float sinyaw = sin(anglestep * i);
				vertices[p].z = sinpitch;
				vertices[p].x = cospitch * cosyaw;
				vertices[p].y = cospitch * sinyaw;
				vertices[p].nx = vertices[p].x;
				vertices[p].ny = vertices[p].y;
				vertices[p].nz = vertices[p].z;
				vertices[p].tUV[0].tV = (-D3DX_PI / 2.0f + anglestep + D3DXToRadian(90)) / D3DXToRadian(180);
				vertices[p].tUV[0].tU = anglestep * i / D3DXToRadian(360);
			}

			CAGeometry *pGeometry = new CAGeometry(NULL, vertices, NULL, 2 * iVertices + 2, 0);
			pGeometry->Create();
			pGeometry->SetSize(1.0f);
			//pGeometry->SetPrimitiveType(D3DPT_TRIANGLEFAN);
			pGeometry->SetPrimitiveType(D3DPT_TRIANGLEFAN);
			pGeometry->SetTexture(new CATexture(D3DPOOL_MANAGED));
			pGeometry->GetTexture()->SetTexture("./Data/SkyNight.png");
			pGeometry->GetTexture()->SetEnable(true);
			pGeometry->GetTexture()->SetBlend(true);
			//pGeometry->GetTexture()->SetColor(AColor(1.0f, 8.0f / 255.0f, 27.0f / 255.0f, 53.0f / 255.0f));
			//pGeometry->GetTexture()->SetColor(AColor(1.0f, 0.0f, 0.0f, 0.0f));
			this->Commit(0, L"Skymap", pGeometry);
		}
	}
	*/
	/*
	{
		const unsigned int iVertices = 20;
		CustomVertex *vertices = new CustomVertex[iVertices * iVertices + 2];

		vertices[0].x = 0.0f;
		vertices[0].y = 0.0f;
		vertices[0].z = 1.0f;
		vertices[0].nx = vertices[0].x;
		vertices[0].ny = vertices[0].y;
		vertices[0].nz = vertices[0].z;
		vertices[0].tUV[0].tU = 0.5f + atan2(vertices[0].z, vertices[0].x) / (2.0f * D3DX_PI);
		vertices[0].tUV[0].tV = 0.5f - asin(vertices[0].y) / D3DX_PI;

		for (int i = 0; i < iVertices; i++)
		{
			float theta = (i + 1) * D3DX_PI / (iVertices + 2);
			for (int j = 0; j < iVertices; j++)
			{
				int p = i * iVertices + j + 1;
				float phi = p * 2 * D3DX_PI / iVertices;
				
				vertices[p].x = sin(theta) * cos(phi);
				vertices[p].y = sin(theta) * sin(phi);
				vertices[p].z = cos(theta);
				vertices[p].nx = vertices[p].x;
				vertices[p].ny = vertices[p].y;
				vertices[p].nz = vertices[p].z;
				vertices[p].tUV[0].tU = 0.5f + atan2(vertices[p].z, vertices[p].x) / (2.0f * D3DX_PI);
				vertices[p].tUV[0].tV = 0.5f - asin(vertices[p].y) / D3DX_PI;
			}
		}
		
		int p = iVertices * iVertices + 1;
		vertices[p].x = 0.0f;
		vertices[p].y = 0.0f;
		vertices[p].z = -1.0f;

		vertices[p].nx = vertices[p].x;
		vertices[p].ny = vertices[p].y;
		vertices[p].nz = vertices[p].z;
		vertices[p].tUV[0].tU = 0.5f + atan2(vertices[p].z, vertices[p].x) / (2.0f * D3DX_PI);
		vertices[p].tUV[0].tV = 0.5f - asin(vertices[p].y) / D3DX_PI;

		CAGeometry *pGeometry = new CAGeometry(NULL, vertices, NULL, iVertices * iVertices + 2, 0);
		pGeometry->Create();
		pGeometry->SetSize(1.0f);
		//pGeometry->SetPrimitiveType(D3DPT_TRIANGLEFAN);
		pGeometry->SetPrimitiveType(D3DPT_TRIANGLELIST);
		pGeometry->SetTexture(new CATexture(D3DPOOL_MANAGED));
		pGeometry->GetTexture()->SetTexture("./Data/SkyNight.png");
		pGeometry->GetTexture()->SetEnable(true);
		pGeometry->GetTexture()->SetBlend(true);
		//pGeometry->GetTexture()->SetColor(AColor(1.0f, 8.0f / 255.0f, 27.0f / 255.0f, 53.0f / 255.0f));
		//pGeometry->GetTexture()->SetColor(AColor(1.0f, 0.0f, 0.0f, 0.0f));
		this->Commit(0, L"SkyMap", pGeometry);

		SAFE_DELETE(vertices);
	}
	*/
#endif

#if !DEBUG_FLOOR_OFF
	{
		CustomVertex vert[1600 * 4];
		CustomIndex inde[1600 * 2];
		for (int i = 0; i < 20; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				int pos = (i * 20 + j) * 4;
				vert[pos + 0].x = (0.0f - 10 + i) * 2.0f;
				vert[pos + 0].y = 0.0f;
				vert[pos + 0].z = (1.0f - 10 + j) * 2.0f + (i % 2) * 0.5f;
				vert[pos + 0].nx = 0.0f;
				vert[pos + 0].ny = 1.0f;
				vert[pos + 0].nz = 0.0f;
				vert[pos + 0].tUV[0].tU = 0.000f;
				vert[pos + 0].tUV[0].tV = 1.000f;

				vert[pos + 1].x = (1.0f - 10 + i) * 2.0f;
				vert[pos + 1].y = 0.0f;
				vert[pos + 1].z = (1.0f - 10 + j) * 2.0f + (i % 2) * 0.5f;
				vert[pos + 1].nx = 0.0f;
				vert[pos + 1].ny = 1.0f;
				vert[pos + 1].nz = 0.0f;
				vert[pos + 1].tUV[0].tU = 1.000f;
				vert[pos + 1].tUV[0].tV = 1.000f;

				vert[pos + 2].x = (1.0f - 10 + i) * 2.0f;
				vert[pos + 2].y = 0.0f;
				vert[pos + 2].z = (0.0f - 10 + j) * 2.0f + (i % 2) * 0.5f;
				vert[pos + 2].nx = 0.0f;
				vert[pos + 2].ny = 1.0f;
				vert[pos + 2].nz = 0.0f;
				vert[pos + 2].tUV[0].tU = 1.000f;
				vert[pos + 2].tUV[0].tV = 0.000f;

				vert[pos + 3].x = (0.0f - 10 + i) * 2.0f;
				vert[pos + 3].y = 0.0f;
				vert[pos + 3].z = (0.0f - 10 + j) * 2.0f + (i % 2) * 0.5f;
				vert[pos + 3].nx = 0.0f;
				vert[pos + 3].ny = 1.0f;
				vert[pos + 3].nz = 0.0f;
				vert[pos + 3].tUV[0].tU = 0.000f;
				vert[pos + 3].tUV[0].tV = 0.000f;


				int in_pos = (i * 20 + j) * 2;

				inde[in_pos + 0]._0 = pos + 0;
				inde[in_pos + 0]._1 = pos + 1;
				inde[in_pos + 0]._2 = pos + 2;

				inde[in_pos + 1]._0 = pos + 0;
				inde[in_pos + 1]._1 = pos + 2;
				inde[in_pos + 1]._2 = pos + 3;
			}
		}
		CAGeometry *pGeometry = new CAGeometry(NULL, vert, inde, 400 * 4, 400 * 2);
		pGeometry->Create();

		pGeometry->SetTexture(new CATexture(D3DPOOL_MANAGED));
		pGeometry->GetTexture()->SetLayer(0);
		pGeometry->GetTexture()->SetEnable(true);
		pGeometry->GetTexture()->SetBlend(true);
		pGeometry->GetTexture()->SetTexture("./Data/floor.dds");
		pGeometry->SetPos(0, 0, 0);
		this->Commit(0, L"Floor", pGeometry);
	}
#endif

#if !DEBUG_BILLIARDS_OFF
	CAXModel *pXModel = new CAXModel();
	pXModel->Create("./Data/billiards.x");
	pXModel->SetSize(0.03f);
	this->Commit(0, L"Billiards", pXModel);
#endif

#if !DEBUG_BALL_OFF
	m_pBall[0] = new CAXModel();
	m_pBall[0]->Create("./Data/ball-white.x");
	m_vBallPos[0] = AVector3(0.0f, 0.958f, 0.0f);
	m_pBall[0]->SetSize(AVector3(0.03f * BALL_FACTOR));
	this->Commit(0, L"BallWhite", m_pBall[0]);

	m_pBall[1] = new CAXModel();
	m_pBall[1]->Create("./Data/ball-yellow.x");
	m_vBallPos[1] = AVector3(1.0f, 0.958f, 0.5f);
	m_pBall[1]->SetSize(AVector3(0.03f * BALL_FACTOR));
	this->Commit(0, L"BallYellow", m_pBall[1]);

	m_pBall[2] = new CAXModel();
	m_pBall[2]->Create("./Data/ball-red.x");
	m_vBallPos[2] = AVector3(0.0f, 0.958f, -0.5f);
	m_pBall[2]->SetSize(AVector3(0.03f * BALL_FACTOR));
	this->Commit(0, L"BallRed1", m_pBall[2]);

	m_pBall[3] = new CAXModel();
	m_pBall[3]->Create("./Data/ball-red.x");
	m_vBallPos[3] = AVector3(0.0f, 0.958f, 0.5f);
	m_pBall[3]->SetSize(AVector3(0.03f * BALL_FACTOR));
	this->Commit(0, L"BallRed2", m_pBall[3]);

	for (unsigned int i = 4; i < RED_BALL_COUNT + 2; i++)
	{
		m_pBall[i] = new CAXModel();
		m_pBall[i]->Create("./Data/ball-red.x");
		m_vBallPos[i] = AVector3(-1.48f + 2.96f * (rand() % 100) / 100.0f, 0.958f, -0.804f + 1.608f * (rand() % 100) / 100.0f);
		m_pBall[i]->SetSize(AVector3(0.03f * BALL_FACTOR));
		this->Commit(0, L"BallRed" + (i - 2), m_pBall[i]);
	}
#endif

#if !DEBUG_SHADOW_OFF
	{
		CustomVertex vertices[4] = {
			{-1.0f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
			{ 0.0f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},
			{ 0.0f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
			{-1.0f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f}
		};
		CustomIndex indices[2] = {
			{ 0, 1, 2 }, { 0, 2, 3 }
		};

		CAGeometry *pGeometry = new CAGeometry(NULL, vertices, indices, 4, 2);
		pGeometry->Create();
		pGeometry->SetPos(AVector3(1.5f, 0.001f, 0.0f));
		pGeometry->SetSize(4.5f, 0.0f, 2.0f);
		pGeometry->SetTexture(new CATexture(D3DPOOL_MANAGED));
		pGeometry->GetTexture()->SetEnable(true);
		pGeometry->GetTexture()->SetBlend(true);
		pGeometry->GetTexture()->SetColor(AColor(0.4f, 0.0f, 0.0f, 0.0f));
		this->Commit(0, L"TableShadow", pGeometry);
	}

	// Table Shadow Mini
	{
		// Right
		{
			CustomVertex vertices[4] = {
				{ -1.0f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f },
				{ 0.0f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f },
				{ -1.0f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f }
			};
			CustomIndex indices[2] = {
				{ 0, 1, 2 }, { 0, 2, 3 }
			};

			CAGeometry *pGeometry = new CAGeometry(NULL, vertices, indices, 4, 2);
			pGeometry->Create();
			pGeometry->SetPos(AVector3(1.68f + 0.022f, 0.925f, 0.0f));
			pGeometry->SetSize(0.066f, 0.0f, 1.718f);
			pGeometry->SetTexture(new CATexture(D3DPOOL_MANAGED));
			pGeometry->GetTexture()->SetEnable(true);
			pGeometry->GetTexture()->SetBlend(true);
			pGeometry->GetTexture()->SetColor(AColor(0.15f, 0.0f, 0.0f, 0.0f));
			this->Commit(0, L"TableShadowRight", pGeometry);
		}

		// Top
		{
			CustomVertex vertices[4] = {
				{  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
				{  0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f },
				{ -0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f },
				{ -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f }
			};
			CustomIndex indices[2] = {
				{ 0, 2, 1 }, { 0, 3, 2 }
			};

			CAGeometry *pGeometry = new CAGeometry(NULL, vertices, indices, 4, 2);
			pGeometry->Create();
			pGeometry->SetPos(AVector3(-0.01f, 0.925f, 0.837f + 0.022f));
			pGeometry->SetSize(3.36f + 0.03f, 0.0f, 0.022f);
			pGeometry->SetTexture(new CATexture(D3DPOOL_MANAGED));
			pGeometry->GetTexture()->SetEnable(true);
			pGeometry->GetTexture()->SetBlend(true);
			pGeometry->GetTexture()->SetColor(AColor(0.15f, 0.0f, 0.0f, 0.0f));
			this->Commit(0, L"TableShadowTop", pGeometry);
		}

		// Bottom
		{
			CustomVertex vertices[4] = {
				{  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
				{  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f },
				{ -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f },
				{ -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f }
			};
			CustomIndex indices[2] = {
				{ 0, 1, 2 }, { 0, 2, 3 }
			};

			CAGeometry *pGeometry = new CAGeometry(NULL, vertices, indices, 4, 2);
			pGeometry->Create();
			pGeometry->SetPos(AVector3(-0.01f, 0.925f, -0.837f - 0.022f));
			pGeometry->SetSize(3.36f + 0.03f, 0.0f, 0.022f);
			pGeometry->SetTexture(new CATexture(D3DPOOL_MANAGED));
			pGeometry->GetTexture()->SetEnable(true);
			pGeometry->GetTexture()->SetBlend(true);
			pGeometry->GetTexture()->SetColor(AColor(0.15f, 0.0f, 0.0f, 0.0f));
			this->Commit(0, L"TableShadowBottom", pGeometry);
		}
	}

	{
		const unsigned int iVertices = 20;
		CustomVertex *vertices = new CustomVertex[iVertices + 3];

		vertices[0].x = 0.0f;
		vertices[0].y = 0.0f;
		vertices[0].z = 0.0f;
		vertices[0].nx = 0.0f;
		vertices[0].ny = 1.0f;
		vertices[0].nz = 0.0f;
		vertices[0].tUV[0].tU = 0.0f;
		vertices[0].tUV[0].tV = 0.0f;

		for (int i = 1; i <= iVertices + 1; i++)
		{
			float fComplete = (float)i / (float)iVertices;
			vertices[i].x = (float)sin(6.2831f * fComplete);
			vertices[i].y = 0.0f;
			vertices[i].z = (float)cos(6.2831f * fComplete);

			vertices[i].nx = 0.0f;
			vertices[i].ny = 1.0f;
			vertices[i].nz = 0.0f;

			vertices[i].tUV[0].tU = 0.0f;
			vertices[i].tUV[0].tV = 0.0f;
		}

		for (int i = 0; i < RED_BALL_COUNT + 2; i++)
		{
			m_pBallShadow[i] = new CAGeometry(NULL, vertices, NULL, iVertices + 2, 0);
			m_pBallShadow[i]->Create();
			m_pBallShadow[i]->SetSize(0.04f * BALL_FACTOR);
			m_pBallShadow[i]->SetPrimitiveType(D3DPT_TRIANGLEFAN);
			m_pBallShadow[i]->SetTexture(new CATexture(D3DPOOL_MANAGED));
			m_pBallShadow[i]->GetTexture()->SetEnable(true);
			m_pBallShadow[i]->GetTexture()->SetBlend(true);
			//m_pBallShadow->GetTexture()->SetColor(AColor(1.0f, 8.0f / 255.0f, 27.0f / 255.0f, 53.0f / 255.0f));
			m_pBallShadow[i]->GetTexture()->SetColor(AColor(0.5f, 0.0f, 0.0f, 0.0f));
			this->Commit(0, (wstring(L"BallShadow") + IntToWstring(i)).c_str(), m_pBallShadow[i]);
		}

		SAFE_DELETE(vertices);
	}
#endif

	m_pInterface = new CATexture(D3DPOOL_MANAGED);
	m_pInterface->SetTexture("./Data/Interface.png");
	m_pInterface->SetSize(SCALE_X, SCALE_Y);
	m_pInterface->SetPos(1105 * SCALE_X, 1084 * SCALE_Y);
	m_pInterface->SetOrtho2D(true);
	this->Commit(0, L"00Interface", m_pInterface);

	m_pTarget = new CATexture(D3DPOOL_MANAGED);
	m_pTarget->SetTexture("./Data/target.png");
	m_pTarget->SetSize(SCALE_X, SCALE_Y);
	m_pTarget->SetPos(293 * SCALE_X, 1084 * SCALE_Y);
	m_pTarget->SetOrtho2D(true);
	this->Commit(0, L"01Target", m_pTarget);

	m_pGaugeBlue = new CATexture(D3DPOOL_MANAGED);
	m_pGaugeBlue->SetTexture("./Data/blue-bar.png");
	m_pGaugeBlue->SetSize(1.56f * SCALE_X, 0.5f * SCALE_Y);
	m_pGaugeBlue->SetPos((500 + 798) * SCALE_X, 1144 * SCALE_Y);
	m_pGaugeBlue->SetOrtho2D(true);
	this->Commit(0, L"02GaugeBlue", m_pGaugeBlue);

	m_pGaugeRed = new CATexture(D3DPOOL_MANAGED);
	m_pGaugeRed->SetTexture("./Data/red-bar.png");
	m_pGaugeRed->SetSize(1.56f * SCALE_X * 0.0f, 0.5f * SCALE_Y);
	m_pGaugeRed->SetPos((500 + 798 * 0.0f) * SCALE_X, 1144 * SCALE_Y);
	m_pGaugeRed->SetOrtho2D(true);
	this->Commit(0, L"03GaugeRed", m_pGaugeRed);

	m_pGaugeButton = new CATexture(D3DPOOL_MANAGED);
	m_pGaugeButton->SetTexture("./Data/button-bar.png");
	m_pGaugeButton->SetSize(0.7f * SCALE_X, 0.7f * SCALE_Y);
	m_pGaugeButton->SetPos((510 + 1575 * 1.0f) * SCALE_X, 1144 * SCALE_Y);
	m_pGaugeButton->SetOrtho2D(true);
	this->Commit(0, L"04GaugeButton", m_pGaugeButton);

	m_pTimeInterface = new CATexture(D3DPOOL_MANAGED);
	m_pTimeInterface->SetTexture("./Data/time-interface.png");
	m_pTimeInterface->SetSize(SCALE_X, 0.66667f * SCALE_Y);
	m_pTimeInterface->SetPos(1110 * SCALE_X, 150 * SCALE_Y);
	m_pTimeInterface->SetOrtho2D(true);
	this->Commit(0, L"05TimeInterface", m_pTimeInterface);

	for (int i = 0; i < 10; i++)
	{
		m_pNumberList[i] = new CATexture(D3DPOOL_MANAGED);
		m_pNumberList[i]->SetTexture((string("./Data/num") + IntToString(i) + ".png").c_str());
		m_pNumberList[i]->SetSize(0.5f * SCALE_X, 0.5f * SCALE_Y);
		m_pNumberList[i]->SetOrtho2D(true);
		m_pNumberList[i]->SetLayer(0);
	}
	m_pNumberList[10] = new CATexture(D3DPOOL_MANAGED);
	m_pNumberList[10]->SetTexture("./Data/numx.png");
	m_pNumberList[10]->SetSize(0.5f * SCALE_X, 0.5f * SCALE_Y);
	m_pNumberList[10]->SetOrtho2D(true);
	m_pNumberList[10]->SetLayer(0);

	m_pScoreBoard = new CATexture(D3DPOOL_MANAGED);
	m_pScoreBoard->SetTexture("./Data/score-board.png");
	m_pScoreBoard->SetSize(SCALE_X, SCALE_Y);
	m_pScoreBoard->SetPos(1110 * SCALE_X, 680 * SCALE_Y);
	m_pScoreBoard->SetOrtho2D(true);
	m_pScoreBoard->SetEnable(false);
	this->Commit(0, L"08ScoreBoard", m_pScoreBoard);

	m_pChipRed = new CATexture(D3DPOOL_MANAGED);
	m_pChipRed->SetTexture("./Data/red-chip.png");
	m_pChipRed->SetSize(SCALE_X, SCALE_Y);
	m_pChipRed->SetPos(380 * SCALE_X, 620 * SCALE_Y);
	m_pChipRed->SetOrtho2D(true);
	m_pChipRed->SetLayer(0);

	m_pChipWhite = new CATexture(D3DPOOL_MANAGED);
	m_pChipWhite->SetTexture("./Data/white-chip.png");
	m_pChipWhite->SetSize(SCALE_X, SCALE_Y);
	m_pChipWhite->SetPos(380 * SCALE_X, 620 * SCALE_Y);
	m_pChipWhite->SetOrtho2D(true);
	m_pChipWhite->SetLayer(0);

	m_pPlayer1Win = new CATexture(D3DPOOL_MANAGED);
	m_pPlayer1Win->SetTexture("./Data/1-players-win.png");
	m_pPlayer1Win->SetSize(SCALE_X, SCALE_Y);
	m_pPlayer1Win->SetPos(1108 * SCALE_X, 642 * SCALE_Y);
	m_pPlayer1Win->SetOrtho2D(true);
	m_pPlayer1Win->SetEnable(false);
	this->Commit(0, L"Player1sWin", m_pPlayer1Win);

	m_pPlayer2Win = new CATexture(D3DPOOL_MANAGED);
	m_pPlayer2Win->SetTexture("./Data/2-players-win.png");
	m_pPlayer2Win->SetSize(SCALE_X, SCALE_Y);
	m_pPlayer2Win->SetPos(1108 * SCALE_X, 642 * SCALE_Y);
	m_pPlayer2Win->SetOrtho2D(true);
	m_pPlayer2Win->SetEnable(false);
	this->Commit(0, L"Player2sWin", m_pPlayer2Win);

#if DEBUG_BALL
	m_pBallVector = new CAGrid(0, 0.1f);
	m_pBallVector->SetPos(0.0f, 0.958f, 0.0f);
	this->Commit(0, L"BallVector", m_pBallVector);
#endif

#if DEBUG_PHYSICS_LINE
	CAGrid *line = new CAGrid(0, 3);
	line->SetPos(1.68f, 0.925f);
	this->Commit(0, L"LineX1", line);

	line = new CAGrid(0, 3);
	line->SetPos(-1.68f, 0.925f);
	this->Commit(0, L"LineX2", line);

	line = new CAGrid(0, 3);
	line->SetPos(0.0f, 0.925f, 0.837f);
	this->Commit(0, L"LineZ1", line);

	line = new CAGrid(0, 3);
	line->SetPos(0.0f, 0.925f, -0.837f);
	this->Commit(0, L"LineZ2", line);
#endif

	m_vCameraPos = new AVector3(*GetCamera()->GetEye());
	m_vCameraPosOld = new AVector3(*GetCamera()->GetEye());
	m_vCameraLookat = new AVector3(*GetCamera()->GetLookat());
	m_vCameraLookatOld = new AVector3(*GetCamera()->GetLookat());
	m_nCameraCurFrame = m_nCameraMaxFrame = 1;

	for (int i = 0; i < RED_BALL_COUNT + 2; i++)
	{
		m_vRotateLine[i] = AVector3(0.0f);
		m_fRotateSum[i] = 0.0f;
		m_fOldRotate[i] = 0.0f;
		m_vOldRotateLine[i] = AVector3(0.0f);

		m_vBallSpeed[i] = AVector3(0.0f);
		m_fBallRotateSpeed[i] = 0.0f;
		m_fBallDelta[i] = 1.0f;

		BallSpin(0.0f, 0.0f, i);
	}

	m_bTurnPlayer = 0;
	m_iCollisionBall = 0;
	m_bShootingBall = false;

	m_iMaxScore = 200;
	m_iScore[0] = 0;
	m_iScore[1] = 0;

	m_fGaugePower = 0.0f;
	m_fSpinDelta = 0.0f;
	m_fShootingRotate = 0.0f;
	m_nRemainTime = timeGetTime() + 90000;

	m_nState = 0;
}

void CGameScene::Reset (LPDIRECT3DDEVICE9 pD3DDevice)
{
	DefaultSetting (pD3DDevice);

	CARootScene::Reset (pD3DDevice);

	for (int i = 0; i < 10; i++)
		m_pNumberList[i]->Reset();

	m_pChipRed->Reset();
	m_pChipWhite->Reset();
}

void CGameScene::AnimateCamera(AVector3& vEye, AVector3& vLookAt, AReal32 frame)
{
	AReal32 delta = 1.0f;
	if (m_nCameraCurFrame <= m_nCameraMaxFrame)
		delta = expoEasingOut(m_nCameraCurFrame, 0.0f, 1.0f, m_nCameraMaxFrame);

	if (*m_vCameraPos != vEye && *m_vCameraLookat != vLookAt)
	{
		m_nCameraCurFrame = 0;
		m_nCameraMaxFrame = frame - 1;
	}

	if (*m_vCameraPos != vEye)
	{
		*m_vCameraPosOld = *m_vCameraPosOld * (1.0f - delta) + *m_vCameraPos * delta;
		*m_vCameraPos = vEye;
	}
	if (*m_vCameraLookat != vLookAt)
	{
		*m_vCameraLookatOld = *m_vCameraLookatOld * (1.0f - delta) + *m_vCameraLookat * delta;
		*m_vCameraLookat = vLookAt;
	}
}

void CGameScene::MoveCamera(void)
{
	if (m_nCameraCurFrame > m_nCameraMaxFrame)
		return;

	AReal32 delta = expoEasingOut(m_nCameraCurFrame ++, 0.0f, 1.0f, m_nCameraMaxFrame);

	GetCamera()->SetView(&(*m_vCameraPosOld * (1.0f - delta) + *m_vCameraPos * delta).D3DXVector(),
						 &(*m_vCameraLookatOld * (1.0f - delta) + *m_vCameraLookat * delta).D3DXVector(),
						 GetCamera()->GetUp());
}

void CGameScene::BallSpin(AReal32 fRotateXZ, AReal32 fRotateY, int index)
{
	D3DXMATRIX matPosition;
	D3DXMATRIX matSize;
	D3DXMATRIX matRotate;
	D3DXMATRIX matAnchorPoint;
	D3DXMATRIX matComplate;
	
	m_vBallPos[index] += m_vBallSpeed[index] * m_fBallDelta[index];
	if (fRotateXZ > 0 || fRotateY != 0)
	{
		D3DXMATRIX matRot;

		AVector3 vUp(0.0f, 1.0f, 0.0f);
		AVector3 vSpeed = m_vBallSpeed[index] * m_fBallDelta[index];
		if (vSpeed.Length() > 0)
		{
			m_vRotateLine[index] = (vUp * vSpeed).Normalize();
			if (fRotateY != 0.0f)
				m_vRotateLine[index].y = fRotateY / vSpeed.Length();
		}
		else
		{
			if (fRotateY != 0.0f)
				m_vRotateLine[index].y = ((fRotateY > 0.0f) ? 1 : -1) * (m_vRotateLine[index].Length() > 1.0f) ? (m_vRotateLine[index].Length()) : 1.0f;
			else
			{
				if (m_vOldRotateLine[index].Length() > 0 && m_vRotateLine[index].y != 0.0f)
				{
					m_vRotateLine[index] = m_vOldRotateLine[index];
					m_vRotateLine[index].y *= 1.0f - min(fabs(m_fOldRotate[index] * 100.0f), 1.0f);
				}
			}
		}
		m_vRotateLine[index] = m_vRotateLine[index].Normalize();

		if (m_vRotateLine[index].y > 0)
		{
			m_fOldRotate[index] = vSpeed.Length();
			m_vOldRotateLine[index] = m_vRotateLine[index];
		}
		
		fRotateY *= 31.0f;
		m_fRotateSum[index] += m_vBallSpeed[index].Length() * m_fBallDelta[index] * 31.0f * fRotateXZ;
		if (fRotateY != 0.0f)
			m_fRotateSum[index] += m_vRotateLine[index].y * m_fBallDelta[index] * fabs(fRotateY);
	}
	
#if !DEBUG_BALL_OFF
	D3DXMatrixTranslation(&matAnchorPoint, m_pBall[index]->GetAnchorPoint().x, m_pBall[index]->GetAnchorPoint().y, m_pBall[index]->GetAnchorPoint().z);
	D3DXMatrixScaling(&matSize, m_pBall[index]->GetSize().x, m_pBall[index]->GetSize().y, m_pBall[index]->GetSize().z);

	D3DXMatrixRotationAxis(&matRotate, &m_vRotateLine[index].D3DXVector(), m_fRotateSum[index]);
	D3DXMatrixTranslation(&matPosition, m_vBallPos[index].x, m_vBallPos[index].y, m_vBallPos[index].z);
	matComplate = matAnchorPoint * matSize * matRotate * matPosition;

	m_pBall[index]->SetMatrix(matComplate);
#endif

#if DEBUG_BALL
	D3DXMatrixScaling(&matSize, m_pBallVector->GetSize().x, m_pBallVector->GetSize().y, m_pBallVector->GetSize().z);
	matComplate = matAnchorPoint * matSize * matRotate * matPosition;
	m_pBallVector->SetMatrix(matComplate);
#endif
}

AInt32 CGameScene::WallCollisionCheck(AVector3& ballPos, AVector3& ballLastPos)
{
	int isCollision = 0;
	for (int i = 0; i < 4; i++)
		if (LineToLineCollision(ballPos, ballLastPos, m_vWallList[i][0], m_vWallList[i][1]))
		{
			isCollision = i + 1;
			break;
		}

	return isCollision;
}

void CGameScene::BallUpdate(int index)
{
	BOOL bWallCollision;
	AUInt32 minDiagonalNumber;
	AReal32 minDiagonalDist;
	AUInt32 lastDiagonalNumber = -1;
	do
	{
		/* 벡터 충돌 체크 */
		// 가장 가까운 대각선 거리 계산
		bWallCollision = false;
		minDiagonalNumber = -1;
		minDiagonalDist = 10000.0f;

		// 공이 이동하는 방향으로의 반지름 계산
		AVector3 moveRadius = m_vBallSpeed[index].Normalize() * BALL_SIZE;
		if (moveRadius.Length() == 0.0f)
			moveRadius = AVector3(BALL_SIZE, 0.0f, 0.0f);
		// 반지름 벡터와 직각을 이루는 벡터 계산
		AVector3 moveNRadius = AVector3(-moveRadius.z, moveRadius.y, moveRadius.x);

		// 현재 위치로부터 이동될 위치까지의 사각형
		AVector3 curBallRect[4];
		curBallRect[0] = m_vBallPos[index] - moveNRadius - moveRadius;
		curBallRect[1] = m_vBallPos[index] + moveNRadius - moveRadius;
		curBallRect[2] = curBallRect[1] + m_vBallSpeed[index] + moveRadius * 2;
		curBallRect[3] = curBallRect[0] + m_vBallSpeed[index] + moveRadius * 2;

		// 다른 공과의 충돌 여부 판단 (Rectangle Collision)
		for (int i = 0; i < RED_BALL_COUNT + 2; i++)
		{
			if (i == index) continue;
			if (i == lastDiagonalNumber) continue;
			if (m_vBallSpeed[index].Length() == 0.0f && m_vBallSpeed[i].Length() == 0.0f) continue;

			// 공이 이동하는 방향으로의 반지름 계산
			moveRadius = m_vBallSpeed[i].Normalize() * BALL_SIZE;
			if (moveRadius.Length() == 0.0f)
				moveRadius = AVector3(BALL_SIZE, 0.0f, 0.0f);
			// 반지름 벡터와 직각을 이루는 벡터 계산
			moveNRadius = AVector3(-moveRadius.z, moveRadius.y, moveRadius.x);

			// 현재 위치부터 이동될 위치까지를 사각형으로 묶는다
			AVector3 otherBallRect[4];
			otherBallRect[0] = m_vBallPos[i] - moveNRadius - moveRadius;
			otherBallRect[1] = m_vBallPos[i] + moveNRadius - moveRadius;
			otherBallRect[2] = otherBallRect[1] + m_vBallSpeed[i] + moveRadius * 2;
			otherBallRect[3] = otherBallRect[0] + m_vBallSpeed[i] + moveRadius * 2;

			// 다른 공의 경로와 현재 공의 경로가 충돌할 시
			if (PolyToPolyCollision(curBallRect, otherBallRect, 4, 4))
			{
				BOOL bCollision = false;

				// 방향 벡터로 표현되는 선의 교점을 이용하여 최대치 계산
				AReal32 fBinaryDeltaMin = 0.0f;
				AReal32 fBinaryDeltaMax = 1.0f;
				if (LineToLineCollision(m_vBallPos[index], m_vBallPos[index] + m_vBallSpeed[index] * m_fBallDelta[index],
					m_vBallPos[i], m_vBallPos[i] + m_vBallSpeed[i] * m_fBallDelta[i]))
				{
					// 벡터간 충돌 비율 계산
					AVector3 v1 = m_vBallPos[i];
					AVector3 v2 = m_vBallPos[i] + m_vBallSpeed[i] * m_fBallDelta[i];
					AVector3 w1 = m_vBallPos[index];
					AVector3 w2 = m_vBallPos[index] + m_vBallSpeed[index] * m_fBallDelta[index];
					float denominator = ((v2.x - v1.x) * (w2.z - w1.z)) - ((v2.z - v1.z) * (w2.x - w1.x));
					float numerator1 = ((v1.z - w1.z) * (w2.x - w1.x)) - ((v1.x - w1.x) * (w2.z - w1.z));
					if (denominator == 0)
						fBinaryDeltaMax = 1.0f;
					else
						fBinaryDeltaMax = numerator1 / denominator;
				}

				// 처음 위치에서 공끼리의 거리 계산
				AReal32 fMinDist = AVector3::Distance(m_vBallPos[index], m_vBallPos[i]) - BALL_SIZE * 2;
				AReal32 fMaxDist;
				
				// 처음 위치에서 나아가는 방향에 대해서 공끼리의 각도 계산
				AReal32 fMaxRadian = acos(AVector3::Dot((m_vBallPos[index] - m_vBallPos[i]).Normalize(), m_vBallSpeed[i].Normalize()));

				// 이진 탐색을 통해 최단 충돌 위치 계산
				while (fBinaryDeltaMax - fBinaryDeltaMin > BALL_COLLISION_MIN)
				{
					AReal32 center = (fBinaryDeltaMax + fBinaryDeltaMin) / 2.0f;

					AVector3 vA = m_vBallPos[index] + m_vBallSpeed[index] * (m_fBallDelta[index] * center);
					AVector3 vB = m_vBallPos[i]     + m_vBallSpeed[i]     * (m_fBallDelta[i]     * center);
					AReal32 fRadian = acos(AVector3::Dot((m_vBallPos[index] - vB).Normalize(), (vA - vB).Normalize()));
					AReal32 fDist = AVector3::Distance(vA, vB) - BALL_SIZE * 2;
					if (fDist < 0)
						bCollision = true;

					if (fMaxRadian < fRadian)
					{
						fBinaryDeltaMax = center;
					}
					else
					{
						if (fDist > 0)
						{
							fMinDist = fDist;
							fBinaryDeltaMin = center;
						}
						else
						{
							fMaxDist = fDist;
							fBinaryDeltaMax = center;
						}
					}
				}
				
				// 만약 공끼리 충돌한게 사실이라면
				if (bCollision)
				{
					if (fBinaryDeltaMin < fBinaryDeltaMax && fBinaryDeltaMin != 0.0f)
					{
						// 다른 공보다 더 빠른 시간 내에 충돌한다면 먼저 충돌 계산
						if (minDiagonalDist > fBinaryDeltaMin)
						{
							minDiagonalNumber = i;
							minDiagonalDist = fBinaryDeltaMin;
						}
					}
					else
					{
						if (minDiagonalDist > fBinaryDeltaMax && fBinaryDeltaMax != 0.0f)
						{
							minDiagonalNumber = i;
							minDiagonalDist = fBinaryDeltaMax;
						}
					}
				}
			} // End Collision
		} // End For

		/* 벽면 충돌 체크 */
		AVector3 ballPos = m_vBallPos[index];
		AVector3 ballLastPos = ballPos + m_vBallSpeed[index] * m_fBallDelta[index] + m_vBallSpeed[index].Normalize() * BALL_SIZE;
		int isCollision = WallCollisionCheck(ballPos, ballLastPos);
		if (isCollision > 0)
		{
			AReal32 delta = 1.0f;
			AVector3 newVector = m_vBallSpeed[index] * m_fBallDelta[index];

			// 벡터간 충돌 비율 계산
			AVector3 v1 = ballPos + m_vBallSpeed[index].Normalize() * BALL_SIZE;
			AVector3 v2 = ballLastPos;
			AVector3 w1 = m_vWallList[isCollision - 1][0];
			AVector3 w2 = m_vWallList[isCollision - 1][1];
			float denominator = ((v2.x - v1.x) * (w2.z - w1.z)) - ((v2.z - v1.z) * (w2.x - w1.x));
			float numerator1 = ((v1.z - w1.z) * (w2.x - w1.x)) - ((v1.x - w1.x) * (w2.z - w1.z));
			float r = numerator1 / denominator;
			if (denominator == 0)
				r = 0.0f;

			if (minDiagonalDist > r)
			{
				bWallCollision = true;
				minDiagonalNumber = isCollision - 1;
				minDiagonalDist = r - (BALL_SIZE / m_vBallSpeed[index].Length());
				if (minDiagonalDist < 0.0f)
					minDiagonalDist = 0.0f;
			}
		}
		
		if (minDiagonalNumber != -1)
		{
			if (!bWallCollision)
			{
				DebugConsoleLog(DebugMsg, "Ball(%d) to Ball(%d) Collision: %lf", index, minDiagonalNumber, minDiagonalDist);

				// 노란공과 흰공이 부딪힌적이 없다면
				if (m_iCollisionBall != -1)
				{
					// 다른 공의 번호
					AUInt32 iOtherIndex = (index == m_bTurnPlayer) ? minDiagonalNumber : index;

					// 만약 두 공중 하나가 현재 턴의 공이라면
					if (index == m_bTurnPlayer || minDiagonalNumber == m_bTurnPlayer)
					{
						// 다른 공이 노란공이거나 흰공일 경우
						if (iOtherIndex == !m_bTurnPlayer)
							m_iCollisionBall = -1;

						// 빨간 공이면 공의 번호를 OR처리
						else if (2 <= iOtherIndex)
							m_iCollisionBall |= iOtherIndex - 1;
					}
				}

				// 최소 지점까지의 비율을 구함
				m_fBallDelta[index] *= minDiagonalDist;
				m_fBallDelta[minDiagonalNumber] *= minDiagonalDist;

				// 최소 지점까지 이동
				BallSpin(1.0f, m_fBallRotateSpeed[index], index);
				BallSpin(1.0f, m_fBallRotateSpeed[minDiagonalNumber], minDiagonalNumber);

				// 최소 지점으로부터의 벡터 변환
				AReal32 fCurve = -D3DX_PI * (m_fBallRotateSpeed[index] + m_fBallRotateSpeed[minDiagonalNumber]) / (BALL_SPIN_MAX_SPEED * BALL_SPIN_RADIUS_DIVIDE);
				AVector3 v1 = m_vBallSpeed[index];
				AVector3 v2 = m_vBallSpeed[minDiagonalNumber];
				AVector3 vN = (m_vBallPos[index] - m_vBallPos[minDiagonalNumber]).Normalize();
				vN = AVector3(cos(fCurve) * vN.x + sin(fCurve) * vN.z, vN.y, -sin(fCurve) * vN.x + cos(fCurve) * vN.z).Normalize();
				AVector3 vD = AVector3(-vN.z, vN.y, vN.x);
				AVector3 v1P = vN * AVector3::Dot(vN, v1);
				AVector3 v1O = vD * AVector3::Dot(vD, v1);
				AVector3 v2P = vN * AVector3::Dot(vN, v2);
				AVector3 v2O = vD * AVector3::Dot(vD, v2);
				m_vBallSpeed[index] = v1O + v2P;
				m_vBallSpeed[minDiagonalNumber] = v2O + v1P;

				// 남은 이동거리 계산
				m_fBallDelta[index] = m_fBallDelta[index] / minDiagonalDist * (1.0f - minDiagonalDist);
				m_fBallDelta[minDiagonalNumber] = m_fBallDelta[minDiagonalNumber] / minDiagonalDist * (1.0f - minDiagonalDist);

				lastDiagonalNumber = minDiagonalNumber;
			}
			else
			{
				DebugConsoleLog(DebugMsg, "Ball(%d) to Wall Collision: %lf", index, minDiagonalDist);

				AReal32 fCurve = -D3DX_PI * m_fBallRotateSpeed[index] / (BALL_SPIN_MAX_SPEED * BALL_SPIN_RADIUS_DIVIDE);
				if (minDiagonalDist == 0.0f)
				{
					// Reflection 벡터 계산
					AVector3 normal = m_vWallList[minDiagonalNumber][2];
					m_vBallSpeed[index] = m_vBallSpeed[index] + normal * AVector3::Dot(m_vBallSpeed[index], normal) * -2.0f;
				}
				else
				{
					// 최소 지점까지의 비율을 구함
					m_fBallDelta[index] *= minDiagonalDist;

					// 최소 지점까지 이동
					BallSpin(1.0f, m_fBallRotateSpeed[index], index);

					// Reflection 벡터 계산
					AVector3 normal = m_vWallList[minDiagonalNumber][2];
					m_vBallSpeed[index] = m_vBallSpeed[index] + normal * AVector3::Dot(m_vBallSpeed[index], normal) * -2.0f;

					// 남은 이동거리 계산
					m_fBallDelta[index] = m_fBallDelta[index] / minDiagonalDist * (1.0f - minDiagonalDist);
				}
				m_vBallSpeed[index] = AVector3(cos(fCurve) * m_vBallSpeed[index].x + sin(fCurve) * m_vBallSpeed[index].z, m_vBallSpeed[index].y, -sin(fCurve) * m_vBallSpeed[index].x + cos(fCurve) * m_vBallSpeed[index].z);

				// 토탈 회전 각이 벽면을 넘어설 경우 벡터 반사
				AVector3 ballPos = m_vBallPos[index];
				AVector3 ballLastPos = ballPos + m_vBallSpeed[index] * m_fBallDelta[index] + m_vBallSpeed[index].Normalize() * BALL_SIZE;
				int isCollision = WallCollisionCheck(ballPos, ballLastPos);
				if (isCollision > 0)
				{
					AVector3 normal = m_vWallList[isCollision - 1][2];
					m_vBallSpeed[index] = m_vBallSpeed[index] + normal * AVector3::Dot(m_vBallSpeed[index], normal) * -2.0f;
				}

				lastDiagonalNumber = -1;
			}

			m_fBallRotateSpeed[index] *= FRICTIONAL_SPIN_FORCE;
		}
	} while (minDiagonalNumber != -1 && m_fBallDelta[index] > 0.0f);

	// 나머지 이동에 따른 회전 효과
	BallSpin(1.0f, m_fBallRotateSpeed[index], index);
	// 이동이 완료되었으므로 델타를 초기화
	m_fBallDelta[index] = 1.0f;

	/* 마찰력 계산 */
	if (m_vBallSpeed[index].Length() > REDUCE_START)
		m_vBallSpeed[index] += (AVector3(0) - m_vBallSpeed[index]).Normalize() * FRICTIONAL_FORCE;
	else
		m_vBallSpeed[index] += (AVector3(0) - m_vBallSpeed[index]).Normalize() * FRICTIONAL_FORCE * max(pow(m_vBallSpeed[index].Length() / REDUCE_START, REDUCE_SQUARED), REDUCE_MINIMUM);

	if (fabs(m_fBallRotateSpeed[index]) > REDUCE_START)
		m_fBallRotateSpeed[index] += (m_fBallRotateSpeed[index] > 0.0f ? -1.0f : (m_fBallRotateSpeed[index] == 0.0f ? 0.0f : 1.0f)) * FRICTIONAL_FORCE * 0.5f;
	else
		m_fBallRotateSpeed[index] += (m_fBallRotateSpeed[index] > 0.0f ? -1.0f : (m_fBallRotateSpeed[index] == 0.0f ? 0.0f : 1.0f)) * FRICTIONAL_FORCE * 0.5f * max(pow(fabs(m_fBallRotateSpeed[index]) / REDUCE_START, REDUCE_SQUARED), REDUCE_MINIMUM);

	if (m_vBallSpeed[index].Length() < BALL_MIN_SPEED)
		m_vBallSpeed[index] = AVector3(0.0f);

	if (fabs(m_fBallRotateSpeed[index]) < BALL_MIN_SPEED)
		m_fBallRotateSpeed[index] = 0.0f;
}

void CGameScene::Update (LPDIRECT3DDEVICE9 pD3DDevice, AReal32 dt)
{
	// 공 정보 업데이트 (물리 연산)
	for (int i = 0; i < RED_BALL_COUNT + 2; i++)
	{
		BallUpdate(i);

#if !DEBUG_SHADOW_OFF
		m_pBallShadow[i]->SetPos(AVector3(-0.03f, 0.925f - 0.958f, 0.0f) + m_vBallPos[i]);
#endif
	}

	// 기본적으로 점수판 숨기기
	m_pScoreBoard->SetEnable(false);

	// 공에 대해서 조정중일때
	if (m_nState <= 2)
	{
		// 시간이 넘어갈 경우 턴을 넘김
		if (m_nRemainTime <= timeGetTime())
			ChangeTurn();

		m_pTimeInterface->SetEnable(true);

		// 탭이 눌릴 경우 점수판 표시
		if (AINPUT_KEYBOARD->IsButtonDown(DIK_TAB))
			m_pScoreBoard->SetEnable(true);
	}
	else
		m_pTimeInterface->SetEnable(false);

	// 각도 선택
	if (m_nState == 0)
	{
		float scale = 1.0f;
		if (AINPUT_KEYBOARD->IsButtonDown(DIK_LSHIFT))
			scale = 6.0f;

		if (AINPUT_KEYBOARD->IsButtonDown(DIK_LEFT))
			m_fShootingRotate -= 0.005f * scale;
		if (AINPUT_KEYBOARD->IsButtonDown(DIK_RIGHT))
			m_fShootingRotate += 0.005f * scale;

		if (AINPUT_KEYBOARD->IsButtonDown(DIK_SPACE))
			m_nState = 1;

		// 인터페이스 숨김
		m_pInterface->SetEnable(false);
		m_pGaugeRed->SetEnable(false);
		m_pGaugeBlue->SetEnable(false);
		m_pGaugeButton->SetEnable(false);
		m_pTarget->SetEnable(false);

		// 카메라 이동
		AnimateCamera(AVector3(0.0f, 4.0f, 0.1f), AVector3(0.0f, 0.0f, 0.0f), 120);
	}
	// 스페이스바 관련 처리
	else if (m_nState == 1)
	{
		if (AINPUT_KEYBOARD->IsButtonUp(DIK_SPACE))
			m_nState = 2;
	}
	// 회전 선택
	else if (m_nState == 2)
	{
		if (AINPUT_KEYBOARD->IsButtonDown(DIK_LEFT))
			m_nState = 0;
		if (AINPUT_KEYBOARD->IsButtonDown(DIK_RIGHT))
			m_nState = 0;

		// 공이 발사되지 않은 상태에서 스페이스를 누르면 파워를 늘려줌
		if (AINPUT_KEYBOARD->IsButtonDown(DIK_SPACE))
		{
			m_fGaugePower += 0.008f;
			if (m_fGaugePower > 1.0f)
				m_fGaugePower = 1.0f;
		}
		else
		{
			// 발사하지 않은 상태에서 게이지가 차있다면 공을 발사
			if (m_fGaugePower != 0.0f)
			{
				m_vBallSpeed[m_bTurnPlayer] = AVector3(BALL_MAX_SPEED * cos(m_fShootingRotate) * m_fGaugePower, 0.0f, -BALL_MAX_SPEED * sin(m_fShootingRotate) * m_fGaugePower);
				m_fBallRotateSpeed[m_bTurnPlayer] = BALL_SPIN_MAX_SPEED * m_fSpinDelta * m_fGaugePower;

				m_fSpinDelta = 0.0f;
				m_fGaugePower = 0.0f;
				//m_fShootingRotate = 0.0f;
				m_bShootingBall = true;

				m_nState = 3;
			}
		}

		// 인터페이스 보임
		m_pInterface->SetEnable(true);
		m_pGaugeRed->SetEnable(true);
		m_pGaugeBlue->SetEnable(true);
		m_pGaugeButton->SetEnable(true);
		m_pTarget->SetEnable(true);

		// 게이지와 타겟 좌표 크기 변경
		m_pGaugeRed->SetSize(1.56f * SCALE_X * m_fGaugePower, 0.5f * SCALE_Y);
		m_pGaugeRed->SetPos((500 + 798 * m_fGaugePower) * SCALE_X, 1144 * SCALE_Y);
		m_pGaugeButton->SetPos((510 + 1575 * m_fGaugePower) * SCALE_X, 1144 * SCALE_Y);
		m_pTarget->SetPos((110 * m_fSpinDelta + 293) * SCALE_X, 1084 * SCALE_Y);

		// 카메라 이동
		if (m_fGaugePower == 0.0f)
			AnimateCamera(m_vBallPos[m_bTurnPlayer] + AVector3(-cos(m_fShootingRotate) * 2.3f, 0.7f, sin(m_fShootingRotate) * 2.3f), m_vBallPos[m_bTurnPlayer] + AVector3(0.0f, 0.3f, 0.0f), 120);
		else
			AnimateCamera(m_vBallPos[m_bTurnPlayer] + AVector3(0.0f, 2.0f, 0.1f), m_vBallPos[m_bTurnPlayer], 120);
	}
	// 공이 발사중인 상태
	else if (m_nState == 3)
	{
		// 공이 발사된 상태라면
		if (m_bShootingBall)
		{
			int i;
			for (i = 0; i < RED_BALL_COUNT + 2; i++)
			{
				if (m_vBallSpeed[i].Length() != 0 || m_fBallRotateSpeed[i] != 0)
					break;
			}

			// 모든 공이 멈춘 상태라면
			if (i == RED_BALL_COUNT + 2)
				ChangeTurn();
		}
		
		// 인터페이스 숨김
		m_pInterface->SetEnable(false);
		m_pGaugeRed->SetEnable(false);
		m_pGaugeBlue->SetEnable(false);
		m_pGaugeButton->SetEnable(false);
		m_pTarget->SetEnable(false);

		// 카메라 이동
		if (m_vBallSpeed[m_bTurnPlayer].Length() == 0.0f)
		{
			int i;
			for (i = 0; i < RED_BALL_COUNT + 2; i++)
			{
				if (m_vBallSpeed[i].Length() > 0)
					break;
			}
			if (i != RED_BALL_COUNT + 2)
				AnimateCamera(m_vBallPos[i] + AVector3(0.0f, 1.6f, 2.6f), m_vBallPos[i], 240);
			else
				AnimateCamera(m_vBallPos[m_bTurnPlayer] + AVector3(0.0f, 0.2f, 0.8f), m_vBallPos[m_bTurnPlayer], 240);
		}
		else
			AnimateCamera(m_vBallPos[m_bTurnPlayer] + AVector3(0.0f, 1.3f, 2.0f), m_vBallPos[m_bTurnPlayer], 120);
	}
	// 게임 종료
	else if (m_nState == 4)
	{
		AnimateCamera(m_vBallPos[m_bTurnPlayer] + AVector3(0.0f, 0.2f, 0.8f), m_vBallPos[m_bTurnPlayer], 240);
	}
	MoveCamera();

	CARootScene::Update (pD3DDevice, dt);
}

void CGameScene::Render (LPDIRECT3DDEVICE9 pD3DDevice, BOOL bRenderOrtho2D)
{
	CARootScene::Render (pD3DDevice, bRenderOrtho2D);

	if (bRenderOrtho2D && m_nState <= 2)
	{
		// 남은 시간초 표시
		{
			char buffer[10];
			int lastTime = (m_nRemainTime - timeGetTime()) / 1000.0f;
			int minuteTime = (lastTime / 60) % 60;
			int secondTime = lastTime % 60;
			sprintf(buffer, "%02d:%02d", minuteTime, secondTime);
			for (AUInt16 i = 0; i < strlen(buffer); i++)
			{
				int j = buffer[i] - '0';
				if (0 <= j && j < 10)
				{
					m_pNumberList[j]->SetSize(SCALE_X * 0.5f, SCALE_Y * 0.5f);
					m_pNumberList[j]->SetPos((960 + 30 + i * 60) * SCALE_X, (145 + 35) * SCALE_Y);
					m_pNumberList[j]->Update();
					m_pNumberList[j]->Render();
				}
				else
				{
					m_pNumberList[10]->SetSize(SCALE_X * 0.5f, SCALE_Y * 0.5f);
					m_pNumberList[10]->SetPos((960 + 30 + i * 60) * SCALE_X, (145 + 35) * SCALE_Y);
					m_pNumberList[10]->Update();
					m_pNumberList[10]->Render();
				}
			}
		}

		// 탭이 눌릴 경우 점수판 표시
		if (AINPUT_KEYBOARD->IsButtonDown(DIK_TAB))
		{
			// Player 1의 칩
			for (int i = 0; i < m_iMaxScore / 10; i++)
			{
				int delta = ((m_iMaxScore - m_iScore[0]) / 10) > i ? 0 : 660;
				if ((i / 5) % 2 == 0)
				{
					m_pChipRed->SetPos((380 + i * 30 + delta) * SCALE_X, 625 * SCALE_Y);
					m_pChipRed->Update();
					m_pChipRed->Render();
				}
				else
				{
					m_pChipWhite->SetPos((380 + i * 30 + delta) * SCALE_X, 625 * SCALE_Y);
					m_pChipWhite->Update();
					m_pChipWhite->Render();
				}
			}

			// Player 1의 점수
			{
				char buffer[10];
				sprintf(buffer, "%3d", m_iScore[0]);
				for (AUInt16 i = 0; i < strlen(buffer); i++)
				{
					int j = buffer[i] - '0';
					if (0 <= j && j < 10)
					{
						m_pNumberList[j]->SetSize(SCALE_X * 0.35f, SCALE_Y * 0.35f);
						m_pNumberList[j]->SetPos((1730 + 60 * 0.35f + i * 120 * 0.35f) * SCALE_X, (595 + 70 * 0.35f) * SCALE_Y);
						m_pNumberList[j]->Update();
						m_pNumberList[j]->Render();
					}
				}
			}

			// Player 2의 칩
			for (int i = 0; i < m_iMaxScore / 10; i++)
			{
				int delta = ((m_iMaxScore - m_iScore[1]) / 10) > i ? 0 : 660;
				if ((i / 5) % 2 == 1)
				{
					m_pChipRed->SetPos((380 + i * 30 + delta) * SCALE_X, 865 * SCALE_Y);
					m_pChipRed->Update();
					m_pChipRed->Render();
				}
				else
				{
					m_pChipWhite->SetPos((380 + i * 30 + delta) * SCALE_X, 865 * SCALE_Y);
					m_pChipWhite->Update();
					m_pChipWhite->Render();
				}
			}

			// Player 2의 점수
			{
				char buffer[10];
				sprintf(buffer, "%3d", m_iScore[1]);
				for (AUInt16 i = 0; i < strlen(buffer); i++)
				{
					int j = buffer[i] - '0';
					if (0 <= j && j < 10)
					{
						m_pNumberList[j]->SetSize(SCALE_X * 0.35f, SCALE_Y * 0.35f);
						m_pNumberList[j]->SetPos((1730 + 60 * 0.35f + i * 120 * 0.35f) * SCALE_X, (835 + 70 * 0.35f) * SCALE_Y);
						m_pNumberList[j]->Update();
						m_pNumberList[j]->Render();
					}
				}
			}
		}
	}

	if (!bRenderOrtho2D && m_nState <= 2)
	{
		GridVertex line[] = {
			{ 0.0f, m_vBallPos[m_bTurnPlayer].y, 0.0f, 0xff00ff00 },
			{ 5.0f, m_vBallPos[m_bTurnPlayer].y, 0.0f, 0xff00ff00 },
		};
		AReal32 a = cos(m_fShootingRotate) * line[1].x + sin(m_fShootingRotate) * line[1].z;
		AReal32 b = -sin(m_fShootingRotate) * line[1].x + cos(m_fShootingRotate) * line[1].z;

		line[0].x += m_vBallPos[m_bTurnPlayer].x;
		line[0].z += m_vBallPos[m_bTurnPlayer].z;
		line[1].x = a + m_vBallPos[m_bTurnPlayer].x;
		line[1].z = b + m_vBallPos[m_bTurnPlayer].z;

		AVector3 p1 = AVector3(line[0].x, line[0].y, line[0].z);
		AVector3 p2 = AVector3(line[1].x, line[1].y, line[1].z);
		int isCollision = WallCollisionCheck(p1, p2);
		if (isCollision > 0)
		{
			AVector3 w1 = m_vWallList[isCollision - 1][0];
			AVector3 w2 = m_vWallList[isCollision - 1][1];
			float denominator = ((p2.x - p1.x) * (w2.z - w1.z)) - ((p2.z - p1.z) * (w2.x - w1.x));
			float numerator1 = ((p1.z - w1.z) * (w2.x - w1.x)) - ((p1.x - w1.x) * (w2.z - w1.z));
			float r = numerator1 / denominator;
			if (denominator == 0)
				r = 0.0f;

			p2 -= (p2 - p1) * (1.0f - r);
		}
		line[1].x = p2.x;
		line[1].y = p2.y;
		line[1].z = p2.z;
		
		LPDIRECT3DVERTEXBUFFER9 pVertexBuffer;
		D3DXMATRIX matWorld;
		D3DXMatrixIdentity(&matWorld);

		if (FAILED(APROJECT_WINDOW->GetD3DDevice()->CreateVertexBuffer(sizeof (line), 0,
			D3DFVF_XYZ | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &pVertexBuffer, NULL)))
		{
			DebugLog("CAGrid(Create vertex buffer Error) 실패!");
			DebugConsoleLog(DebugError, "CAGrid: CAGrid(Create vertex buffer Error) 실패!");

			return;
		}
		VOID *pVertices;
		if (FAILED(pVertexBuffer->Lock(0, sizeof (line), (void**)&pVertices, 0)))
		{
			DebugLog("Reset(Lock vertex buffer Error) 실패!");
			DebugConsoleLog(DebugError, "CAGrid: Reset(Lock vertex buffer Error) 실패!");

			return;
		}
		memcpy(pVertices, line, sizeof (line));
		pVertexBuffer->Unlock();

		APROJECT_WINDOW->GetD3DDevice()->SetStreamSource(0, pVertexBuffer, 0, sizeof (GridVertex));
		APROJECT_WINDOW->GetD3DDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		APROJECT_WINDOW->GetD3DDevice()->SetTransform(D3DTS_WORLD, &matWorld);
		APROJECT_WINDOW->GetD3DDevice()->DrawPrimitive(D3DPT_LINELIST, 0, 2);

		pVertexBuffer->Release();
		pVertexBuffer = NULL;
	}
}
	
void CGameScene::Destroy (void)
{
	CARootScene::Destroy ();

	for (int i = 0; i <= 10; i++)
		SAFE_DELETE(m_pNumberList[i]);

	SAFE_DELETE(m_pChipRed);
	SAFE_DELETE(m_pChipWhite);
}

void CALLBACK CGameScene::MouseEvent (AUInt32 unType, AInt nX, AInt nY, AInt nMouseWheelDelta, bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown)
{
	nX /= SCALE_X;
	nY /= SCALE_Y;
	if (bLeftButtonDown && m_nState == 2 && m_fGaugePower == 0.0f)
	{
		if (296 - 180 <= nX && nX <= 296 + 180 && 1082 - 180 <= nY && nY <= 1082 + 180)
			m_fSpinDelta = min(max((nX - 296) / 110.0f, -1.0f), 1.0f);
	}
}

void CALLBACK CGameScene::KeyboardEvent (AUInt vkKey, AKeyState kState, bool bCtrlDown, bool bAltDown, bool bShiftDown)
{
	if (kState == AK_KEYDOWN)
	{
		if (AINPUT_KEYBOARD->IsButtonDown(DIK_RETURN))
		{
			char smTemp[100];
			wchar_t sTemp[100];
			time_t timer = time(NULL);
			struct tm* t = localtime(&timer);
			wsprintf(sTemp, L".\\%04d-%02d-%02d %02d-%02d-%02d.%03d.png", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, timeGetTime() % 1000);
			ConvertToChar(sTemp, smTemp);
			if (!isFileExists(smTemp))
				SaveScreenShot(sTemp, APROJECT_WINDOW->GetWidth(), APROJECT_WINDOW->GetHeight());
			wprintf(L"%s\n", sTemp);
		}
		/*
		if (vkKey == DIK_2)
		{
			m_vBallSpeed[m_bTurnPlayer] = AVector3(BALL_MAX_SPEED * sqrt(2) / 2, 0.0f, BALL_MAX_SPEED * sqrt(2) / 2);
			m_fBallRotateSpeed[m_bTurnPlayer] = BALL_MAX_SPEED;
			m_bShootingBall = true;
		}
		if (vkKey == DIK_3)
		{
			m_vBallSpeed[m_bTurnPlayer] = AVector3(BALL_MAX_SPEED, 0.0f, 0.0f);
			m_fBallRotateSpeed[m_bTurnPlayer] = BALL_MAX_SPEED;
			m_bShootingBall = true;
		}
		if (vkKey == DIK_4)
		{
			m_vBallSpeed[m_bTurnPlayer] = AVector3(0.0f, 0.0f, BALL_MAX_SPEED);
			m_fBallRotateSpeed[m_bTurnPlayer] = BALL_SPIN_MAX_SPEED;
			m_bShootingBall = true;
		}
		*/
		if (vkKey == DIK_1)
		{
			DWORD state;
			APROJECT_WINDOW->GetD3DDevice()->GetRenderState(D3DRS_FILLMODE, &state);
			if (state == D3DFILL_WIREFRAME)
				APROJECT_WINDOW->GetD3DDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
			else if (state == D3DFILL_POINT)
				APROJECT_WINDOW->GetD3DDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			else
				APROJECT_WINDOW->GetD3DDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}
	}
}

void CALLBACK CGameScene::ObjectEvent (const WCHAR* pTitle, UINT msg, WPARAM wParam, LPARAM lParam)
{
}

void CGameScene::ChangeTurn(void)
{
	printf("m_iCollisionBall: %d\n", m_iCollisionBall);
	if (m_iCollisionBall <= 0)
	{
		MinusScore(10);
		printf("%d = %d(-10)\n", m_bTurnPlayer, m_iScore[m_bTurnPlayer]);
		m_bTurnPlayer = !m_bTurnPlayer;
	}
	else if (m_iCollisionBall & 0x01 && m_iCollisionBall & 0x2)
	{
		AddScore(10);
		printf("%d = %d(+10)\n", m_bTurnPlayer, m_iScore[m_bTurnPlayer]);
	}
	else
	{
		printf("%d = %d(None)\n", m_bTurnPlayer, m_iScore[m_bTurnPlayer]);
		m_bTurnPlayer = !m_bTurnPlayer;
	}
	m_bShootingBall = false;
	m_iCollisionBall = 0;
	m_nRemainTime = timeGetTime() + 90000;

	if (m_nState != 4)
		m_nState = 0;
}

void CGameScene::AddScore(int score)
{
	m_iScore[m_bTurnPlayer] += score;

	if (m_iScore[m_bTurnPlayer] == m_iMaxScore)
	{
		if (!m_bTurnPlayer)
			m_pPlayer1Win->SetEnable(true);
		else
			m_pPlayer2Win->SetEnable(true);

		m_nState = 4;
	}
}
void CGameScene::MinusScore(int score)
{
	m_iScore[m_bTurnPlayer] -= score;

	if (m_iScore[m_bTurnPlayer] <= 0)
		m_iScore[m_bTurnPlayer] = 0;
}
