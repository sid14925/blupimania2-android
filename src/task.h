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

// task.h

#ifndef _TASK_H_
#define	_TASK_H_


class CInstanceManager;
class CD3DEngine;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CWater;
class CCamera;
class CMotion;
class CObject;
class CRobotMain;
class CMainUndo;
class CDisplayText;
class CSound;



#define PUSH_DIST		8.0f		// distance d'une caisse pour la pousser



class CTask
{
public:
	CTask(CInstanceManager* iMan, CObject* object);
	virtual ~CTask();

	virtual BOOL	EventProcess(const Event &event);
	virtual	Error	IsEnded();
	virtual BOOL	Abort();
	virtual BOOL	IsUndoable();
	virtual BOOL	IsStopable();
	virtual BOOL	Stop();

	virtual void	WriteSituation();
	virtual void	ReadSituation();

protected:

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CLight*			m_light;
	CParticule*		m_particule;
	CTerrain*		m_terrain;
	CWater*			m_water;
	CCamera*		m_camera;
	CMotion*		m_motion;
	CObject*		m_object;
	CRobotMain*		m_main;
	CMainUndo*		m_undo;
	CDisplayText*	m_displayText;
	CSound*			m_sound;
};


#endif //_TASK_H_
