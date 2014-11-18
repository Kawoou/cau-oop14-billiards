#pragma once

using namespace std;

enum INTERFACE_TYPE {
	INTERFACE_TYPE_INTERFACE = 0,
	INTERFACE_TYPE_TARGET,

	INTERFACE_TYPE_COUNT
};

class CGameScene : public CARootScene
{
private:
	CAGrid*				m_pBallVector;

private:
	CAXModel**			m_pBall;					// 공 모델
	CAGeometry**		m_pBallShadow;				// 공 그림자 모델

	CATexture*			m_pInterface;				// 인터페이스 이미지
	CATexture*			m_pTarget;					// 인터페이스 타깃 이미지
	CATexture*			m_pGaugeRed;				// 인터페이스 게이지 빨강
	CATexture*			m_pGaugeBlue;				// 인터페이스 게이지 블루
	CATexture*			m_pGaugeButton;				// 인터페이스 게이지 버튼

	CATexture*			m_pTimeInterface;			// 남은시간 인터페이스
	CATexture**			m_pNumberList;				// 숫자 이미지 목록

	CATexture*			m_pScoreBoard;				// 점수판
	CATexture*			m_pChipRed;					// 빨간 칩
	CATexture*			m_pChipWhite;				// 하얀 칩

	CATexture*			m_pPlayer1Win;				// 플레이어 1이 이김
	CATexture*			m_pPlayer2Win;				// 플페이어 2가 이김

private:
	AVector3**			m_vWallList;				// 벽면 벡터 정보

private:
	AVector3*			m_vCameraPos;				// 카메라 벡터
	AVector3*			m_vCameraLookat;			// 카메라 바라보는 곳
	AVector3*			m_vCameraPosOld;			// 카메라 이전 벡터
	AVector3*			m_vCameraLookatOld;			// 카메라 이전 바라보는 곳
	AInt16				m_nCameraCurFrame;			// 이동된 프레임
	AInt16				m_nCameraMaxFrame;			// 이동될 최대 프레임

private:
	AVector3*			m_vBallPos;					// 공의 위치
	AVector3*			m_vBallSpeed;				// 공의 속도
	AReal32*			m_fBallRotateSpeed;			// 공의 회전 속도
	AReal32*			m_fBallDelta;				// 공의 이동 정도

	AVector3*			m_vRotateLine;				// 공의 회전축
	AReal32*			m_fRotateSum;				// 공의 회전 정도 합계

	AReal32*			m_fOldRotate;				// 공의 마지막 회전 정도 (멈춘 상태에서 원점으로 돌아가는것 보정)
	AVector3*			m_vOldRotateLine;			// 공의 마지막 회전축

	AReal32				m_fSpinDelta;				// 공의 치는 위치
	AReal32				m_fGaugePower;				// 게이지 파워
	AReal32				m_fShootingRotate;			// 공을 치는 방향
	AUInt32				m_nRemainTime;				// 남은 시간

	AUInt8				m_nState;					// 게임이 진행되는 순서

private:
	void AnimateCamera(AVector3& vEye, AVector3& vLookAt, AReal32 frame);
	void MoveCamera(void);

private:
	AInt32 WallCollisionCheck(AVector3& ballPos, AVector3& ballLastPos);

	void BallSpin(AReal32 fRotateXZ, AReal32 fRotateY, int index);
	void BallUpdate(int index);

private://김지연
	BOOL				m_bTurnPlayer;
	BOOL				m_bShootingBall;
	AInt8				m_iCollisionBall;
	AInt8				m_iScore[2];
	AUInt8				m_iMaxScore;

	void ChangeTurn(void);
	void AddScore(int score);
	void MinusScore(int score);

public:
	CGameScene (void);
	virtual ~CGameScene (void);

public:
	void DefaultSetting (LPDIRECT3DDEVICE9 pD3DDevice);

public:
	virtual void Create (LPDIRECT3DDEVICE9 pD3DDevice);
	virtual void Reset (LPDIRECT3DDEVICE9 pD3DDevice);
	virtual void Update (LPDIRECT3DDEVICE9 pD3DDevice, AReal32 dt);
	virtual void Render (LPDIRECT3DDEVICE9 pD3DDevice, BOOL bRenderOrtho2D);
	virtual void Destroy (void);

	virtual void CALLBACK MouseEvent (AUInt32 unType, AInt nX, AInt nY, AInt nMouseWheelDelta, bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown);
	virtual void CALLBACK KeyboardEvent (AUInt vkKey, AKeyState kState, bool bCtrlDown, bool bAltDown, bool bShiftDown);
	virtual void CALLBACK ObjectEvent (const WCHAR* pTitle, UINT msg, WPARAM wParam, LPARAM lParam);
};