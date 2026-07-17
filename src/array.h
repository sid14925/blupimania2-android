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

// array.h

#ifndef _ARRAY_H_
#define	_ARRAY_H_


#include "control.h"


class CD3DEngine;
class CButton;
class CScroll;


#define ARRAYMAXDISPLAY	20		// nb max de lignes visibles
#define ARRAYMAXTOTAL	100		// nb max de lignes total



class CArray : public CControl
{
public:
	CArray(CInstanceManager* iMan);
	~CArray();

	BOOL		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg, float expand);

	void		SetPos(FPOINT pos);
	void		SetDim(FPOINT dim);

	BOOL		SetState(int state, BOOL bState);
	BOOL		SetState(int state);
	BOOL		ClearState(int state);

	BOOL		EventProcess(const Event &event);
	void		Draw();

	void		Flush();

	void		SetTotal(int i);
	int			RetTotal();

	void		SetMultiple(int i);
	int			RetMultiple();

	void		SetSelect(int i);
	int			RetSelect();

	void		SetSelectCap(BOOL bEnable);
	BOOL		RetSelectCap();

	void		SetCheck(int i, BOOL bMode);
	BOOL		RetCheck(int i);

	void		SetEnable(int i, BOOL bEnable);
	BOOL		RetEnable(int i);

	void		SetBlink(BOOL bEnable);
	BOOL		RetBlink();

	void		SetName(int i, char* name);
	char*		RetName(int i);

	void		SetIndex(int i, int index);
	int			RetIndex(int i);

	void		SetSortValue(int i, int value);
	int			RetSortValue(int i);

	void		Sort();

	void		SetTabs(int i, float pos, int justif=1, FontType font=FONT_COLOBOT);
	float		RetTabs(int i);

	void		ShowSelect();

	EventMsg	RetEventMsgButton(int i);
	EventMsg	RetEventMsgScroll();

protected:
	BOOL		MoveAdjust();
	void		UpdateButton();
	void		UpdateScroll();
	void		MoveScroll();
	void		DrawLine(FPOINT pos, FPOINT dim, char *line, BOOL bMultiFont);
	void		DrawCase(char *text, FPOINT pos, float width, int justif, FontType font);

protected:
	CButton*	m_button[ARRAYMAXDISPLAY];
	CScroll*	m_scroll;

	EventMsg	m_eventButton[ARRAYMAXDISPLAY];
	EventMsg	m_eventScroll;

	float		m_expand;
	int			m_totalLine;	// nb total de lignes
	int			m_displayLine;	// nb de lignes visibles
	int			m_selectLine;	// ligne sélectionnée
	int			m_firstLine;	// première ligne visible
	int			m_multiple;		// multiple pour 1ère ligne visible
	BOOL		m_bBlink;
	BOOL		m_bSelectCap;
	float		m_blinkTime;
	float		m_tabs[10];
	int			m_justifs[10];
	FontType	m_font[10];

	FPOINT		h_headPos;
	FPOINT		h_headDim;
	char		m_head[100];
	char		m_text[ARRAYMAXTOTAL][200];
	char		m_check[ARRAYMAXTOTAL];
	char		m_enable[ARRAYMAXTOTAL];
	int			m_index[ARRAYMAXTOTAL];
	int			m_sort[ARRAYMAXTOTAL];
};


#endif //_ARRAY_H_
