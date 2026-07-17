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

// autodoor.h

#ifndef _AUTODOOR_H_
#define	_AUTODOOR_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType : int;


enum AutoDoorPhase
{
	ADOP_WAIT	= 1,	// attend porte fermée
	ADOP_OPEN	= 2,	// ouvre la porte
	ADOP_STOP	= 3,	// attend porte ouverte
	ADOP_CLOSE	= 4,	// ferme la porte
};



class CAutoDoor : public CAuto
{
public:
	CAutoDoor(CInstanceManager* iMan, CObject* object);
	~CAutoDoor();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

	void		WriteSituation();
	void		ReadSituation();

protected:
	void		MoveDoor(float progress);
	void		UpdateLockZone(BOOL bOpen);
	void		FireStopUpdate();
	CObject*	SearchKey();
	CObject*	SearchObject(D3DVECTOR center, float radius);
	void		OpenParticule();

protected:
	ObjectType		m_type;		// OBJECT_DOORn
	AutoDoorPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_lastParticuleFire;
	float			m_lastParticuleRay;
	int				m_partiStop;
	D3DVECTOR		m_posKey;
};


#endif //_AUTODOOR_H_
