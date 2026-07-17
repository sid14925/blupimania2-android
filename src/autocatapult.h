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

// autocatapult.h

#ifndef _AUTOCATAPULT_H_
#define	_AUTOCATAPULT_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType : int;


enum AutoCatapultPhase
{
	ACAP_WAIT	= 1,	// attend ordre
	ACAP_UP		= 2,	// marteau prend de l'élan
	ACAP_DOWN	= 3,	// marteau descend
	ACAP_FLY	= 4,	// caisse vole
	ACAP_RETURN	= 5,	// marteur au départ
	ACAP_FALL	= 6,	// caisse tombe dans trou
};



class CAutoCatapult : public CAuto
{
public:
	CAutoCatapult(CInstanceManager* iMan, CObject* object);
	~CAutoCatapult();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int part);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	BOOL		IsRunning();
	Error		RetError();

protected:
	void		LockZone(BOOL bLock);
	float		CalcDistFactor();
	D3DVECTOR	CalcPosition(float dist);
	CObject*	SearchObject(D3DVECTOR center, float radius);

protected:
	AutoCatapultPhase m_phase;
	float			m_progress;
	float			m_speed;
	ObjectType		m_type;
	CObject*		m_pBox;
	float			m_distFactor;
	float			m_hammerAngle[5];
	BOOL			m_bFall;
	float			m_lastParticule;
	int				m_channelSound;
};


#endif //_AUTOCATAPULT_H_
