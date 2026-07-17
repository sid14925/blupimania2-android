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

// displaytext.h

#ifndef _DISPLAYTEXT_H_
#define	_DISPLAYTEXT_H_


class CInstanceManager;
class CD3DEngine;
class CInterface;
class CObject;
class CSound;



#define FONTSIZE	12.0f



enum TextType
{
	TT_ERROR	= 1,
	TT_WARNING	= 2,
	TT_INFO		= 3,
	TT_MESSAGE	= 4,
	TT_START	= 5,
};

enum Sound : int;

#define MAXDTLINE	1


class CDisplayText
{
public:
	CDisplayText(CInstanceManager* iMan);
	~CDisplayText();

	void		DeleteObject();

	BOOL		EventProcess(const Event &event);

	void		DisplayError(Error err, float time=10.0f, float size=FONTSIZE);
	void		DisplayText(char *text, float time=10.0f, float size=FONTSIZE, TextType type=TT_INFO, Sound sound=SOUND_CLICK);
	void		HideText(BOOL bHide);
	void		ClearText();
	BOOL		ClearLastText();
	void		SetDelay(float factor);
	void		SetEnable(BOOL bEnable);

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CInterface*		m_interface;
	CSound*			m_sound;

	BOOL			m_bExist[MAXDTLINE];
	float			m_time[MAXDTLINE];

	BOOL			m_bHide;
	BOOL			m_bEnable;
	float			m_delayFactor;
	int				m_channelSound;
};


#endif //_DISPLAYTEXT_H_
