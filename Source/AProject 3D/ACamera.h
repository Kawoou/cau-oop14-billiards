#ifndef __APROJECT_CAMERA__
#define __APROJECT_CAMERA__

#pragma once

#define PLANE_EPSILON	5.0f
class CAFrustum
{
private:
	D3DXVECTOR3	m_vtx [8];		//!< 프러스텀을 구성할 정점 8개
	D3DXVECTOR3 m_vPos;
	D3DXPLANE	m_plane [6];	//!< 프러스텀을 구성하는 6개의 평면

public:
	CAFrustum (void);
	~CAFrustum (void);

public:
	void Make (D3DXMATRIXA16* pmatViewproj);
	bool IsIn (D3DXVECTOR3* pv, bool b3DCheck = true);
	bool IsInSphere (D3DXVECTOR3* pv, float radius, bool b3DCheck = true);
	void Draw (void);
	D3DXVECTOR3* GetPos (void) { return &m_vPos; }
};

class CACamera
{
private:
	CAFrustum*		m_pFrustum;

private:
	D3DXVECTOR3		m_vEye;			//!< 현재 위치
	D3DXVECTOR3		m_vLookat;		//!< 시선 위치
	D3DXVECTOR3		m_vUp;			//!< 상방 벡터

	D3DXVECTOR3		m_vView;		//!< 향하는 단위방향벡터
	D3DXVECTOR3		m_vCross;		//!< 측면 벡터 Cross (view, up)

	D3DXMATRIXA16	m_matProj;		//!< 프로젝션 행렬
	D3DXMATRIXA16	m_matView;		//!< 카메라 행렬
	D3DXMATRIXA16	m_matBill;		//!< 빌보드 행렬(카메라의 역행렬)

private:
	D3DXVECTOR3		vRight;
	D3DXVECTOR3		vLook;
	D3DXVECTOR3		vUp;

	D3DXVECTOR3		vTRight;
	D3DXVECTOR3		vTLook;
	D3DXVECTOR3		vTUp;

	D3DXVECTOR3		v3OLook;
	D3DXVECTOR3		v3OEye;

	float			tx_angle;
	float			ty_angle;
	bool			m_bLockFrustum;

private:
	CACamera () { ; }

public:
	CACamera (_D3DXMATRIXA16 mmatProj);
	~CACamera ();
	/**
	@brief : 프로젝션 행렬을 얻어낸다.
	*/
	D3DXMATRIXA16* GetProjectionMatrix () { return &m_matProj; }
	/**
	@brief : 카메라 행렬을 얻어낸다.
	*/
	D3DXMATRIXA16* GetViewMatrix () { return &m_matView; }
	/**
	@brief : 빌보드 행렬을 얻어낸다.
	*/
	D3DXMATRIXA16* GetBillMatrix () { return &m_matBill; }

public:
	/**
	@brief : 카메라 행렬을 생성하기위한 기본 벡터값들을 설정한다.
	*/
	D3DXMATRIXA16* SetView (D3DXVECTOR3* pvEye, D3DXVECTOR3* pvLookat, D3DXVECTOR3* pvUp);

	/**
	@brief : 값을 갱신한다.
	*/
	void Flush () { SetView (&m_vEye, &m_vLookat, &m_vUp); }

public:
	/**
	@brief : 카메라의 위치값을 설정한다.
	*/
	void SetEye (D3DXVECTOR3* pv) { m_vEye = *pv; }
	/**
	@brief : 카메라의 시선값을 설정한다.
	*/
	void SetLookat (D3DXVECTOR3* pv) { m_vLookat = *pv; }
	/**
	@brief : 카메라의 상방벡터값을 설정한다.
	*/
	void SetUp (D3DXVECTOR3* pv) { m_vUp = *pv; }

	bool GetLockFrustum (void) { return m_bLockFrustum; }
	void SetLockFrustum (bool bLockFrustum) { m_bLockFrustum = bLockFrustum; }

	CAFrustum* GetFrustum (void) { return m_pFrustum; }

	void UpdateFrustum (void);

public:
	/**
	@brief : 카메라의 위치값을 얻어낸다.
	*/
	D3DXVECTOR3* GetEye () { return &m_vEye; }
	/**
	@brief : 카메라의 시선값을 얻어낸다.
	*/
	D3DXVECTOR3* GetLookat () { return &m_vLookat; }
	/**
	@brief : 카메라의 상방벡터값을 얻어낸다.
	*/
	D3DXVECTOR3*	GetUp() { return &m_vUp; }

public:
	/**
	@brief : 카메라 좌표계의 X축으로 angle만큼 회전한다.
	*/
	D3DXMATRIXA16* RotateLocalX (float angle);
	/**
	@brief : 카메라 좌표계의 Y축으로 angle만큼 회전한다.
	*/
	D3DXMATRIXA16* RotateLocalY (float angle);

public:
	/**
	@brief : 월드좌표계의 *pv값의 위치로 이동한다.
	*/
	D3DXMATRIXA16* MoveTo (D3DXVECTOR3* pv);
	/**
	@brief : 카메라 좌표계의 X축방향으로 dist만큼 전진한다.(후진은 -dist를 넣으면 된다.)
	*/
	D3DXMATRIXA16* MoveLocalX (float dist);
	/**
	@brief : 카메라 좌표계의 Y축방향으로 dist만큼 전진한다.(후진은 -dist를 넣으면 된다.)
	*/
	D3DXMATRIXA16* MoveLocalY (float dist);
	/**
	@brief : 카메라 좌표계의 Z축방향으로 dist만큼 전진한다.(후진은 -dist를 넣으면 된다.)
	*/
	D3DXMATRIXA16* MoveLocalZ (float dist);
};

#endif