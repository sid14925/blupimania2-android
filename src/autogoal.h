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

// autogoal.h

#ifndef _AUTOGOAL_H_
#define	_AUTOGOAL_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType : int;



enum AutoGoalPhase
{
	AGO_FIX		= 1,	// ballon fixé au sol
	AGO_FLY		= 2,	// ballon pris par blupi
	AGO_LASTFLY	= 3,	// dernier ballon pris par blupi
};



class CAutoGoal : public CAuto
{
public:
	CAutoGoal(CInstanceManager* iMan, CObject* object);
	~CAutoGoal();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int phase);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();
	Error		SetAction(int action, float time=0.2f);

protected:
	void		ChangeColumn();
	void		CameraStart();
	void		CameraProgress(float progress);

protected:
	AutoGoalPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_jostle;
	D3DVECTOR		m_pos;
	D3DVECTOR		m_eyeStart;
	D3DVECTOR		m_lookatStart;
	D3DVECTOR		m_eyeGoal;
	D3DVECTOR		m_lookatGoal;
	BOOL			m_bLastGoal;
	float			m_startView;
	float			m_goalView;
};


#endif //_AUTOGOAL_H_
