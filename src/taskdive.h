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

// taskdive.h

#ifndef _TASKDIVE_H_
#define	_TASKDIVE_H_


class CInstanceManager;
class CTerrain;
class CObject;



enum TaskDivePhase
{
	TDI_MARCH,	// va presque au bout
	TDI_TRY,	// essaie de plonger
	TDI_NONO,	// non-non
	TDI_TURN,	// demi-tour

	TDI_END,	// va au bout-bout
	TDI_OUPS,	// saute
	TDI_DIVE,	// plonge
	TDI_WAIT,	// attend sous l'eau
	TDI_UP,		// remonte
	TDI_OSCIL,	// balance
	TDI_JUMP,	// saut looping
	TDI_OH,		// oh !

	TDI_BACK,	// revient
};



class CTaskDive : public CTask
{
public:
	CTaskDive(CInstanceManager* iMan, CObject* object);
	~CTaskDive();

	BOOL	EventProcess(const Event &event);

	Error	Start(CObject *dive);
	Error	IsEnded();

protected:
	void	StartAction(int action, float speed=0.2f);
	void	SetLinSpeed(float speed);

protected:
	TaskDivePhase	m_phase;
	CObject*		m_dive;
	D3DVECTOR		m_startPos;
	D3DVECTOR		m_goalPos;
	D3DVECTOR		m_oupsPos;
	D3DVECTOR		m_divePos;
	float			m_startAngle;
	float			m_time;
	float			m_progress;
	float			m_speed;
	float			m_level;
	BOOL			m_bError;
	BOOL			m_bPlouf;
};


#endif //_TASKDIVE_H_
