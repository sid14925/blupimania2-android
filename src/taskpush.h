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

// taskpush.h

#ifndef _TASKPUSH_H_
#define	_TASKPUSH_H_


class CInstanceManager;
class CTerrain;
class CWater;
class CObject;

enum Sound : int;


enum TaskPush
{
	TPU_NULL,
	TPU_ROLL,		// roule une sphère
	TPU_PUSH,		// pousse une caisse
};



class CTaskPush : public CTask
{
public:
	CTaskPush(CInstanceManager* iMan, CObject* object);
	~CTaskPush();

	BOOL		EventProcess(const Event &event);

	Error		Start(int part, int nbTiles);
	Error		IsEnded();
	BOOL		IsStopable();
	BOOL		Stop();

protected:
	BOOL		IsPusherAdhoc();
	void		SetLockZone(BOOL bLock);
	D3DVECTOR	RetBoxStartPos(int rank);
	D3DVECTOR	RetBoxGoalPos(int rank);
	CObject*	SearchBox(D3DVECTOR center, float radius);
	CObject*	SearchObject(D3DVECTOR center, float radius);
	void		JostlePlant(D3DVECTOR center);
	BOOL		IsPosFree(D3DVECTOR center);
	BOOL		IsHole(D3DVECTOR center);
	BOOL		IsSpace(D3DVECTOR center);
	D3DVECTOR	RetAngleFreeFall(float angle);
	void		AdaptBox(int i, const D3DVECTOR &pos);
	void		StartAction(int action, float speed=0.2f);
	void		ProgressAction(float progress);
	void		ProgressLinSpeed(float speed);
	void		ProgressCirSpeed(float speed);

protected:
	ObjectType	m_type;
	BOOL		m_bFirst;
	BOOL		m_bLast;
	BOOL		m_bMiddle;
	BOOL		m_bStop;
	BOOL		m_bPull;
	int			m_part;
	int			m_nbTiles;
	BOOL		m_bRepeat;
	BOOL		m_bMultiTiles;
	TaskPush	m_phase;
	int			m_maxBox;
	int			m_totalBox;
	CObject*	m_pBox[20];
	CObject*	m_pMine;
	CObject*	m_pGoal;
	ObjectType	m_mineType;
	float		m_time;
	float		m_progress;
	float		m_speed;
	float		m_randFreeFall;
	float		m_level;
	D3DVECTOR	m_startPos;
	D3DVECTOR	m_goalPos;
	D3DVECTOR	m_boxStartPos;
	D3DVECTOR	m_boxGoalPos;
	D3DVECTOR	m_mineStartPos;
	D3DVECTOR	m_mineGoalPos;
	BOOL		m_bImpossible;
	BOOL		m_bTask;
	BOOL		m_bMine;
	BOOL		m_bMineDown;
	BOOL		m_bGoal;
	BOOL		m_bHole;
	BOOL		m_bSpace;
	BOOL		m_bPipe;
	BOOL		m_bColomnFall;
	BOOL		m_bFreeFall;
	BOOL		m_bBoum;
	BOOL		m_bWater;
	BOOL		m_bError;
	BOOL		m_bEndingJostle;
	BOOL		m_bMiddleSound;
	BOOL		m_bFallSound;
	Sound		m_middleSound;
	float		m_lastParticuleSmoke;
};


#endif //_TASKPUSH_H_
