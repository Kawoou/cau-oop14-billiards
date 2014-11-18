#ifndef __APROJECT_CAMERA__
#define __APROJECT_CAMERA__

#pragma once

#define PLANE_EPSILON	5.0f
class CAFrustum
{
private:
	D3DXVECTOR3	m_vtx [8];		//!< ���������� ������ ���� 8��
	D3DXVECTOR3 m_vPos;
	D3DXPLANE	m_plane [6];	//!< ���������� �����ϴ� 6���� ���

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
	D3DXVECTOR3		m_vEye;			//!< ���� ��ġ
	D3DXVECTOR3		m_vLookat;		//!< �ü� ��ġ
	D3DXVECTOR3		m_vUp;			//!< ��� ����

	D3DXVECTOR3		m_vView;		//!< ���ϴ� �������⺤��
	D3DXVECTOR3		m_vCross;		//!< ���� ���� Cross (view, up)

	D3DXMATRIXA16	m_matProj;		//!< �������� ���
	D3DXMATRIXA16	m_matView;		//!< ī�޶� ���
	D3DXMATRIXA16	m_matBill;		//!< ������ ���(ī�޶��� �����)

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
	@brief : �������� ����� ����.
	*/
	D3DXMATRIXA16* GetProjectionMatrix () { return &m_matProj; }
	/**
	@brief : ī�޶� ����� ����.
	*/
	D3DXMATRIXA16* GetViewMatrix () { return &m_matView; }
	/**
	@brief : ������ ����� ����.
	*/
	D3DXMATRIXA16* GetBillMatrix () { return &m_matBill; }

public:
	/**
	@brief : ī�޶� ����� �����ϱ����� �⺻ ���Ͱ����� �����Ѵ�.
	*/
	D3DXMATRIXA16* SetView (D3DXVECTOR3* pvEye, D3DXVECTOR3* pvLookat, D3DXVECTOR3* pvUp);

	/**
	@brief : ���� �����Ѵ�.
	*/
	void Flush () { SetView (&m_vEye, &m_vLookat, &m_vUp); }

public:
	/**
	@brief : ī�޶��� ��ġ���� �����Ѵ�.
	*/
	void SetEye (D3DXVECTOR3* pv) { m_vEye = *pv; }
	/**
	@brief : ī�޶��� �ü����� �����Ѵ�.
	*/
	void SetLookat (D3DXVECTOR3* pv) { m_vLookat = *pv; }
	/**
	@brief : ī�޶��� ��溤�Ͱ��� �����Ѵ�.
	*/
	void SetUp (D3DXVECTOR3* pv) { m_vUp = *pv; }

	bool GetLockFrustum (void) { return m_bLockFrustum; }
	void SetLockFrustum (bool bLockFrustum) { m_bLockFrustum = bLockFrustum; }

	CAFrustum* GetFrustum (void) { return m_pFrustum; }

	void UpdateFrustum (void);

public:
	/**
	@brief : ī�޶��� ��ġ���� ����.
	*/
	D3DXVECTOR3* GetEye () { return &m_vEye; }
	/**
	@brief : ī�޶��� �ü����� ����.
	*/
	D3DXVECTOR3* GetLookat () { return &m_vLookat; }
	/**
	@brief : ī�޶��� ��溤�Ͱ��� ����.
	*/
	D3DXVECTOR3*	GetUp() { return &m_vUp; }

public:
	/**
	@brief : ī�޶� ��ǥ���� X������ angle��ŭ ȸ���Ѵ�.
	*/
	D3DXMATRIXA16* RotateLocalX (float angle);
	/**
	@brief : ī�޶� ��ǥ���� Y������ angle��ŭ ȸ���Ѵ�.
	*/
	D3DXMATRIXA16* RotateLocalY (float angle);

public:
	/**
	@brief : ������ǥ���� *pv���� ��ġ�� �̵��Ѵ�.
	*/
	D3DXMATRIXA16* MoveTo (D3DXVECTOR3* pv);
	/**
	@brief : ī�޶� ��ǥ���� X��������� dist��ŭ �����Ѵ�.(������ -dist�� ������ �ȴ�.)
	*/
	D3DXMATRIXA16* MoveLocalX (float dist);
	/**
	@brief : ī�޶� ��ǥ���� Y��������� dist��ŭ �����Ѵ�.(������ -dist�� ������ �ȴ�.)
	*/
	D3DXMATRIXA16* MoveLocalY (float dist);
	/**
	@brief : ī�޶� ��ǥ���� Z��������� dist��ŭ �����Ѵ�.(������ -dist�� ������ �ȴ�.)
	*/
	D3DXMATRIXA16* MoveLocalZ (float dist);
};

#endif