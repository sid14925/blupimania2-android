/*
 * This file is part of the BlupiMania 2 source code.
 * Copyright (C) 2001, Daniel Roux & EPSITEC SA
 * http://epsitec.ch; https://blupi.org; https://www.maniabricks.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

// autolift.h

#ifndef _AUTOLIFT_H_
#define	_AUTOLIFT_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType : int;



enum AutoLiftPhase
{
	ALI_NULL	= 1,	// rien à faire
	ALI_UP1		= 2,	// monte
	ALI_UP2		= 3,	// monte
	ALI_DOWN	= 4,	// redescend
};



class CAutoLift : public CAuto
{
public:
	CAutoLift(CInstanceManager* iMan, CObject* object);
	~CAutoLift();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int phase);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	CObject*	SearchObject(D3DVECTOR center, float radius);
	void		StartAction(int action, float speed=0.2f);
	void		CameraStart1();
	void		CameraStart2();
	void		CameraProgress(float progress);
	void		CameraStop();

protected:
	D3DVECTOR		m_posGround;
	D3DVECTOR		m_posBlupi;
	D3DVECTOR		m_angleBlupi;
	D3DVECTOR		m_lastAngleBlupi;
	CObject*		m_blupi;
	AutoLiftPhase	m_phase;
	float			m_speed;
	float			m_progress;
	float			m_totalRot;
	BOOL			m_bSelect;
	BOOL			m_bCamera;
	D3DVECTOR		m_eyeStart;
	D3DVECTOR		m_lookatStart;
	D3DVECTOR		m_eyeGoal;
	D3DVECTOR		m_lookatGoal;
	D3DVECTOR		m_eyeFinal;
	D3DVECTOR		m_lookatFinal;
	float			m_dirFinal;
};


#endif //_AUTOLIFT_H_
