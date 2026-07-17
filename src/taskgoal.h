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

// taskgoal.h

#ifndef _TASKGOAL_H_
#define	_TASKGOAL_H_


class CInstanceManager;
class CTerrain;
class CObject;



enum TaskGoalPhase
{
	TGO_GOAL	= 1,	// lève les bras
	TGO_FLY		= 2,	// s'envole
};



class CTaskGoal : public CTask
{
public:
	CTaskGoal(CInstanceManager* iMan, CObject* object);
	~CTaskGoal();

	BOOL	EventProcess(const Event &event);

	Error	Start(CObject *goal);
	Error	IsEnded();

protected:
	void	StartAction(int action, float speed=0.2f);
	BOOL	IsLastGoal();

protected:
	TaskGoalPhase	m_phase;
	CObject*		m_goal;
	D3DVECTOR		m_startPos;
	float			m_time;
	float			m_progress;
	float			m_speed;
	BOOL			m_bError;
};


#endif //_TASKGOAL_H_
