// taskwait.cpp

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
#include "task.h"
#include "taskwait.h"




// Constructeur de l'objet.

CTaskWait::CTaskWait(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
}

// Destructeur de l'objet.

CTaskWait::~CTaskWait()
{
}


// Gestion d'un �v�nement.

BOOL CTaskWait::EventProcess(const Event &event)
{
	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	m_passTime += event.rTime;
	m_bEnded = (m_passTime >= m_waitTime);
	return TRUE;
}


// Assigne le but � atteindre.

Error CTaskWait::Start(float time)
{
	m_waitTime = time;  // dur�e � attendre
	m_passTime = 0.0f;  // dur�e �coul�e
	m_bEnded = FALSE;
	return ERR_OK;
}

// Indique si l'action est termin�e.

Error CTaskWait::IsEnded()
{
	if ( m_engine->RetPause() )  return ERR_CONTINUE;
	if ( m_bEnded )  return ERR_STOP;
	return ERR_CONTINUE;
}


