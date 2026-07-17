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

// automax1x.h

#ifndef _AUTOMAX1X_H_
#define	_AUTOMAX1X_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType : int;



enum AutoMax1xPhase
{
	AMP_WAITOBJ,	// attend l'arrivée d'un objet
	AMP_WAITNULL,	// attend le départ d'un objet
	AMP_OPEN,		// ouvre les portes
	AMP_UP,			// monte le clown
	AMP_TERM,		// fini, bloqué
};



class CAutoMax1x : public CAuto
{
public:
	CAutoMax1x(CInstanceManager* iMan, CObject* object);
	~CAutoMax1x();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int part);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

	void		WriteSituation();
	void		ReadSituation();

protected:
	void		SwingClown(float rTime);

protected:
	D3DVECTOR		m_pos;
	AutoMax1xPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_swingSpeed;
	float			m_swingTime;
	float			m_lastParticule;
};


#endif //_AUTOMAX1X_H_
