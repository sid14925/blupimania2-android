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

// taskfire.h

#ifndef _TASKFIRE_H_
#define	_TASKTIRE_H_


class CInstanceManager;
class CTerrain;
class CObject;


class CTaskFire : public CTask
{
public:
	CTaskFire(CInstanceManager* iMan, CObject* object);
	~CTaskFire();

	BOOL		EventProcess(const Event &event);

	Error		Start(float delay);
	Error		IsEnded();
	BOOL		Abort();

protected:

protected:
	float		m_delay;
	float		m_progress;
	BOOL		m_bError;
	BOOL		m_bRay;
	BOOL		m_bOrganic;
	float		m_time;
	float		m_speed;
	float		m_lastParticule;
	float		m_lastSound;
	int			m_soundChannel;
};


#endif //_TASKFIRE_H_
