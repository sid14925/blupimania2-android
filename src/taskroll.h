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

// taskroll.h

#ifndef _TASKROLL_H_
#define	_TASKROLL_H_


class CInstanceManager;
class CTerrain;
class CObject;

enum Sound : int;


enum TaskRoll
{
	TRO_NULL,
	TRO_ROLL,		// roule une sphère
};



class CTaskRoll : public CTask
{
public:
	CTaskRoll(CInstanceManager* iMan, CObject* object);
	~CTaskRoll();

	BOOL		EventProcess(const Event &event);

	Error		Start(D3DVECTOR dir);
	Error		IsEnded();

protected:
	void		ExploProxi();
	void		ExploTremble(float progress);
	void		ExploObject();
	CObject*	SearchBox(D3DVECTOR center, float radius);
	CObject*	SearchObject(D3DVECTOR center, float radius);
	CObject*	SearchBlupi(D3DVECTOR center, float radius);
	BOOL		IsPosFree(D3DVECTOR center);
	BOOL		IsHole(D3DVECTOR center);
	BOOL		IsSpace(D3DVECTOR center);

protected:
	ObjectType	m_type;
	TaskRoll	m_phase;
	int			m_total;
	int			m_totalMash;
	CObject*	m_pMash[51];	// 1+25+25
	ObjectType	m_mashType;
	float		m_time;
	float		m_progress;
	float		m_speed;
	D3DVECTOR	m_dir;
	D3DVECTOR	m_startPos;
	D3DVECTOR	m_goalPos;
	D3DVECTOR	m_boxAngle;
	BOOL		m_bImpossible;
	BOOL		m_bMash;
	BOOL		m_bExplo;
	BOOL		m_bHole;
	BOOL		m_bSpace;
	BOOL		m_bPipe;
	BOOL		m_bPlouf;
	BOOL		m_bError;
	BOOL		m_bFallSound;
	Sound		m_middleSound;
	float		m_lastParticuleSmoke;
};


#endif //_TASKROLL_H_
