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

// taskmanager.h

#ifndef _TASKMANAGER_H_
#define	_TASKMANAGER_H_


class CInstanceManager;
class CTask;

enum TaskGotoGoal : int;
enum TaskGotoCrash : int;
enum ObjectType : int;



class CTaskManager
{
public:
	CTaskManager(CInstanceManager* iMan, CObject* object);
	~CTaskManager();

	Error	StartTaskWait(float time);
	Error	StartTaskMove(float length, BOOL bNoError);
	Error	StartTaskTurn(float angle);
	Error	StartTaskGoto(D3DVECTOR pos, CObject *target, int part);
	Error	StartTaskPush(int part, int nbTiles);
	Error	StartTaskRoll(D3DVECTOR dir);
	Error	StartTaskDock(CObject *dock, int part);
	Error	StartTaskCatapult(CObject *catapult, int part);
	Error	StartTaskTrax(CObject *trax, int part);
	Error	StartTaskPerfo(CObject *perfo, int part);
	Error	StartTaskGun(CObject *gun, int part);
	Error	StartTaskDrink(CObject *fiole);
	Error	StartTaskGoal(CObject *goal);
	Error	StartTaskDive(CObject *dive);
	Error	StartTaskFire(float delay);

	BOOL	EventProcess(const Event &event);
	Error	IsEnded();
	BOOL	IsPilot();
	BOOL	IsUndoable();
	BOOL	IsStopable();
	BOOL	Stop();
	BOOL	Abort();
	CTask*	RetRunningTask();

protected:

protected:
	CInstanceManager* m_iMan;
	CTask*			m_task;
	CObject*		m_object;
	BOOL			m_bPilot;
};


#endif //_TASKMANAGER_H_
