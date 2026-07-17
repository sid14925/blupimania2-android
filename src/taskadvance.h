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

// taskadvance.h

#ifndef _TASKADVANCE_H_
#define	_TASKADVANCE_H_


class CInstanceManager;
class CTerrain;
class CObject;


class CTaskAdvance : public CTask
{
public:
	CTaskAdvance(CInstanceManager* iMan, CObject* object);
	~CTaskAdvance();

	BOOL	EventProcess(const Event &event);

	Error	Start(float length, BOOL bNoError);
	Error	IsEnded();
	BOOL	IsUndoable();

	void	WriteSituation();
	void	ReadSituation();

protected:
	CObject*	SearchObject(D3DVECTOR center, float radius);
	BOOL		IsPosFree(D3DVECTOR center);
	void		ProgressLinSpeed(float speed);
	void		ProgressCirSpeed(float speed);
	void		StartAction(CObject* pObj, int action);

protected:
	ObjectType	m_type;
	D3DVECTOR	m_startPos;
	D3DVECTOR	m_goalPos;
	float		m_moveAbs;
	float		m_moveDist;
	BOOL		m_bMine;
	CObject*	m_pMine;
	BOOL		m_bGoal;
	BOOL		m_bLostGoal;
	CObject*	m_pGoal;
	BOOL		m_bPerfo;
	CObject*	m_pPerfo;
	BOOL		m_bFall;
	BOOL		m_bError;
};


#endif //_TASKADVANCE_H_
