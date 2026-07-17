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

// motiongun.h

#ifndef _MOTIONGUN_H_
#define	_MOTIONGUN_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;


#define MGUN_WAIT		0		// attend au repos
#define MGUN_ERROR		1		// opération impossible
#define MGUN_FIRE		2		// feu avec le canon



class CMotionGun : public CMotion
{
public:
	CMotionGun(CInstanceManager* iMan, CObject* object);
	~CMotionGun();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

	float		RetLinSpeed();
	float		RetCirSpeed();
	float		RetLinStopLength();

	void		SetActionLinSpeed(float speed);
	void		SetActionCirSpeed(float speed);

protected:
	BOOL		EventFrame(const Event &event);
	void		ParticuleFrame(float rTime, float smoke, float error);
	void		UpdateTrackMapping(float left, float right);
	void		StartMotor(float freq);
	void		StopMotor();
	void		Fire();
	CObject*	SearchTarget(D3DVECTOR center, float radius);

protected:
	float		m_time;
	float		m_lastParticule;
	float		m_leftTrack;
	float		m_rightTrack;
	int			m_channelSound;
	float		m_motorFreq;
};


#endif //_MOTIONGUN_H_
