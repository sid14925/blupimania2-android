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

// autofiole.h

#ifndef _AUTOFIOLE_H_
#define	_AUTOFIOLE_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType : int;


class CAutoFiole : public CAuto
{
public:
	CAutoFiole(CInstanceManager* iMan, CObject* object);
	~CAutoFiole();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int part);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:

protected:
	D3DVECTOR	m_pos;
	ObjectType	m_type;
	float		m_lastParticule;
};


#endif //_AUTOFIOLE_H_
