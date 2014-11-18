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
	CAXModel**			m_pBall;					// �� ��
	CAGeometry**		m_pBallShadow;				// �� �׸��� ��

	CATexture*			m_pInterface;				// �������̽� �̹���
	CATexture*			m_pTarget;					// �������̽� Ÿ�� �̹���
	CATexture*			m_pGaugeRed;				// �������̽� ������ ����
	CATexture*			m_pGaugeBlue;				// �������̽� ������ ���
	CATexture*			m_pGaugeButton;				// �������̽� ������ ��ư

	CATexture*			m_pTimeInterface;			// �����ð� �������̽�
	CATexture**			m_pNumberList;				// ���� �̹��� ���

	CATexture*			m_pScoreBoard;				// ������
	CATexture*			m_pChipRed;					// ���� Ĩ
	CATexture*			m_pChipWhite;				// �Ͼ� Ĩ

	CATexture*			m_pPlayer1Win;				// �÷��̾� 1�� �̱�
	CATexture*			m_pPlayer2Win;				// �����̾� 2�� �̱�

private:
	AVector3**			m_vWallList;				// ���� ���� ����

private:
	AVector3*			m_vCameraPos;				// ī�޶� ����
	AVector3*			m_vCameraLookat;			// ī�޶� �ٶ󺸴� ��
	AVector3*			m_vCameraPosOld;			// ī�޶� ���� ����
	AVector3*			m_vCameraLookatOld;			// ī�޶� ���� �ٶ󺸴� ��
	AInt16				m_nCameraCurFrame;			// �̵��� ������
	AInt16				m_nCameraMaxFrame;			// �̵��� �ִ� ������

private:
	AVector3*			m_vBallPos;					// ���� ��ġ
	AVector3*			m_vBallSpeed;				// ���� �ӵ�
	AReal32*			m_fBallRotateSpeed;			// ���� ȸ�� �ӵ�
	AReal32*			m_fBallDelta;				// ���� �̵� ����

	AVector3*			m_vRotateLine;				// ���� ȸ����
	AReal32*			m_fRotateSum;				// ���� ȸ�� ���� �հ�

	AReal32*			m_fOldRotate;				// ���� ������ ȸ�� ���� (���� ���¿��� �������� ���ư��°� ����)
	AVector3*			m_vOldRotateLine;			// ���� ������ ȸ����

	AReal32				m_fSpinDelta;				// ���� ġ�� ��ġ
	AReal32				m_fGaugePower;				// ������ �Ŀ�
	AReal32				m_fShootingRotate;			// ���� ġ�� ����
	AUInt32				m_nRemainTime;				// ���� �ð�

	AUInt8				m_nState;					// ������ ����Ǵ� ����

private:
	void AnimateCamera(AVector3& vEye, AVector3& vLookAt, AReal32 frame);
	void MoveCamera(void);

private:
	AInt32 WallCollisionCheck(AVector3& ballPos, AVector3& ballLastPos);

	void BallSpin(AReal32 fRotateXZ, AReal32 fRotateY, int index);
	void BallUpdate(int index);

private://������
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