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

// motionfish.h

#ifndef _MOTIONFISH_H_
#define	_MOTIONFISH_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum MotionFishPhase
{
	MFP_WAIT	= 0,	// attend
	MFP_JUMP	= 1,	// saute
};




class CMotionFish : public CMotion
{
public:
	CMotionFish(CInstanceManager* iMan, CObject* object);
	~CMotionFish();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

	float		RetLinSpeed();
	float		RetCirSpeed();
	float		RetLinStopLength();

protected:
	BOOL		EventFrame(const Event &event);
	BOOL		SearchPosition();

protected:
	MotionFishPhase	m_phase;
	float			m_progress;
	float			m_speed;
	D3DVECTOR		m_startPos;
	D3DVECTOR		m_goalPos;
	float			m_level;
	float			m_height;
	BOOL			m_bPlouf1;
	BOOL			m_bPlouf2;
};


#endif //_MOTIONFISH_H_
