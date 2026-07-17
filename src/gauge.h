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

// gauge.h

#ifndef _GAUGE_H_
#define	_GAUGE_H_


#include "control.h"


class CD3DEngine;


enum GaugeMode
{
	GM_NORMAL	= 0,
	GM_SPEED	= 1,
	GM_RPM		= 2,
	GM_COMPASS	= 3,
	GM_LEVEL1	= 4,
	GM_LEVEL2	= 5,
};



class CGauge : public CControl
{
public:
	CGauge(CInstanceManager* iMan);
	~CGauge();

	BOOL	Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	BOOL	EventProcess(const Event &event);

	void	Draw();

	void	SetLevel(float level);
	float	RetLevel();

	void	SetMode(GaugeMode mode);
	GaugeMode RetMode();

protected:

protected:
	float		m_level;
	GaugeMode	m_mode;
};


#endif //_GAUGE_H_
