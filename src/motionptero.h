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

// motionptero.h

#ifndef _MOTIONPTERO_H_
#define	_MOTIONPTERO_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



typedef struct
{
	float		speed;		// vitesse (~1.0)
	float		time;		// temps absolu
	D3DVECTOR	pos;		// position relative
}
PteroDesc;




class CMotionPtero : public CMotion
{
public:
	CMotionPtero(CInstanceManager* iMan, CObject* object);
	~CMotionPtero();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

	float		RetLinSpeed();
	float		RetCirSpeed();
	float		RetLinStopLength();

protected:
	BOOL		EventFrame(const Event &event);
	BOOL		CreateShadow(int i, float radius, float intensity, D3DShadowType type);
	void		MoveShadow(int i, float progress);

protected:
	float		m_progress;
	float		m_speed;
	D3DVECTOR	m_startPos;
	D3DVECTOR	m_goalPos;
	int			m_total;			// nb total d'oiseaux
	PteroDesc	m_birdTable[10];	// descripteurs des oiseaux
};


#endif //_MOTIONPTERO_H_
