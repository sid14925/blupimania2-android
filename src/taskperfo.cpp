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

// taskperfo.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "terrain.h"
#include "object.h"
#include "motion.h"
#include "motionblupi.h"
#include "motionperfo.h"
#include "auto.h"
#include "sound.h"
#include "robotmain.h"
#include "task.h"
#include "tasklist.h"
#include "taskperfo.h"




// Constructeur de l'objet.

CTaskPerfo::CTaskPerfo(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{

	m_time = 0.0f;
	m_bError= TRUE;
}

// Destructeur de l'objet.

CTaskPerfo::~CTaskPerfo()
{
}


// Gestion d'un �v�nement.

BOOL CTaskPerfo::EventProcess(const Event &event)
{
	float		progress;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_bError )  return FALSE;

	m_time += event.rTime;
	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( progress >= 0.67f && !m_bAuto )
	{
		m_bAuto = TRUE;

		if ( m_part == 3 )  // avance ?
		{
			m_perfo->StartTaskList(TO_MOVE, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, 0, 8.0f);
		}

		if ( m_part == 4 )  // bouton gauche (tourne � droite) ?
		{
			m_perfo->StartTaskList(TO_TURN, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, 0, PI/2.0f);
		}

		if ( m_part == 5 )  // bouton droite (tourne � gauche) ?
		{
			m_perfo->StartTaskList(TO_TURN, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, 0, -PI/2.0f);
		}
	}

	return TRUE;
}


// Assigne le but � atteindre.

Error CTaskPerfo::Start(CObject *perfo, int part)
{
	m_main->IncTotalManip();

	if ( perfo == 0 )
	{
		m_bError = TRUE;
		return ERR_GENERIC;
	}

	m_perfo = perfo;
	m_part = part;
	m_time = 0.0f;
	m_progress = 0.0f;
	m_speed = 1.0f/1.5f;
	m_bAuto = FALSE;
	m_bError = FALSE;

//?	m_sound->Play(SOUND_BLUPIohhh, m_object->RetPosition(0), 1.0f);
	StartAction(MBLUPI_TRAX);
	return ERR_OK;
}

// Indique si l'action est termin�e.

Error CTaskPerfo::IsEnded()
{
	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_bError )
	{
		return ERR_STOP;
	}

	if ( m_progress >= 1.0f )
	{
		return ERR_STOP;
	}

	return ERR_CONTINUE;
}


// D�marre une action pour un objet.

void CTaskPerfo::StartAction(int action, float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action, speed);
}

