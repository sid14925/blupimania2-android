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

// check.cpp

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
#include "restext.h"
#include "text.h"
#include "check.h"




// Constructeur de l'objet.

CCheck::CCheck(CInstanceManager* iMan) : CControl(iMan)
{
}

// Destructeur de l'objet.

CCheck::~CCheck()
{
	CControl::~CControl();
}


// Cr�e un nouveau bouton.

BOOL CCheck::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
	char	name[200];
	char*	p;

	if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

	CControl::Create(pos, dim, icon, eventMsg);

	GetResource(RES_EVENT, eventMsg, name);
	p = strchr(name, '\\');
	if ( p != 0 )  *p = 0;
	SetName(name);

	return TRUE;
}


// Gestion d'un �v�nement.

BOOL CCheck::EventProcess(const Event &event)
{
	if ( m_state & STATE_DEAD )  return TRUE;

	CControl::EventProcess(event);

	if ( event.event == EVENT_LBUTTONDOWN &&
		 (m_state & STATE_VISIBLE)        &&
		 (m_state & STATE_ENABLE)         )
	{
		if ( CControl::Detect(event.pos) )
		{
			Event newEvent = event;
			newEvent.event = m_eventMsg;
			m_event->AddEvent(newEvent);
			return FALSE;
		}
	}

	if ( m_bFocus &&
		 (m_state & STATE_VISIBLE) &&
		 (m_state & STATE_ENABLE)  &&
		 event.event == EVENT_KEYDOWN &&
		 (event.param == VK_RETURN ||
		  event.param == VK_BUTTON1) )
	{
		Event newEvent = event;
		newEvent.event = m_eventMsg;
		m_event->AddEvent(newEvent);
	}

	return TRUE;
}


// Dessine le bouton.

void CCheck::Draw()
{
	FPOINT		iDim, pos;
	float		zoomExt, zoomInt;
	int			icon;

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	iDim = m_dim;
	m_dim.x = m_dim.y*0.75f;  // carr�

	if ( m_state & STATE_SHADOW )
	{
		DrawShadow(m_pos, m_dim);
	}
	if ( m_bFocus )
	{
		DrawFocus(m_pos, m_dim);
	}

	m_engine->SetTexture("button1.tga");
//?	m_engine->SetState(D3DSTATENORMAL);
	m_engine->SetState(D3DSTATETTb);

	zoomExt = 1.00f;
	zoomInt = 0.95f;

	icon = 2;
	if ( m_state & STATE_HILIGHT )
	{
		icon = 1;
	}
	if ( m_state & STATE_PRESS )
	{
		icon = 3;
		zoomInt *= 0.9f;
	}
	if ( (m_state & STATE_ENABLE) == 0 )
	{
		icon = 7;
	}
	if ( m_state & STATE_DEAD )
	{
		icon = 17;
	}
	DrawPart(icon, zoomExt, 0.0f);  // dessine le bouton

	if ( (m_state & STATE_ENABLE) == 0 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTb);
		DrawPart(icon, zoomExt, 8.0f/256.0f);  // blanc par-dessus
	}

	if ( (m_state & STATE_DEAD) == 0 )
	{
		m_engine->SetState(D3DSTATETTw);

		if ( m_state & STATE_CHECK )
		{
			if ( m_state & STATE_RADIO )  icon = 30;  // rond
			else                          icon = 16;  // vu
			DrawPart(icon, zoomInt, 0.0f);  // dessine l'ic�ne
		}
	}

	m_dim = iDim;

	if ( m_state & STATE_DEAD )  return;

	// Dessine le nom.
	pos.x = m_pos.x+m_dim.y/0.9f;
	pos.y = m_pos.y+m_dim.y*0.5f;
	pos.y -= m_engine->RetText()->RetHeight(m_fontSize, m_fontType)*0.6f;
	m_engine->RetText()->DrawText(m_name, pos, m_dim.x, 1, m_fontSize, m_fontStretch, m_fontType, 0);
}


