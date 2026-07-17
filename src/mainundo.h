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

// mainundo.h

#ifndef _MAINUNDO_H_
#define	_MAINUNDO_H_



class CInstanceManager;
class CRobotMain;
class CEvent;
class CD3DEngine;
class CInterface;
class CWindow;
class CControl;
class CParticule;
class CTerrain;
class CWater;
class CCamera;
class CSound;
class CButton;
class CMenu;

enum ObjectType : int;



#define MAXUNDO		100



class CMainUndo
{
public:
	CMainUndo(CInstanceManager* iMan);
	~CMainUndo();

	void		Flush();
	BOOL		Record();
	BOOL		IsUndoable();
	BOOL		Undo();

	void		WriteTokenInt(char *token, int value);
	void		WriteTokenFloat(char *token, float value);
	void		WriteTokenPos(char *token, D3DVECTOR value);

	BOOL		ReadTokenInt(char *token, int &value);
	BOOL		ReadTokenFloat(char *token, float &value);
	BOOL		ReadTokenPos(char *token, D3DVECTOR &value);

	CObject*	SearchObjectID(int id);

protected:
	void		DeletePyro(CObject *pObj);
	BOOL		IsUndoObject(CObject *pObj);
	BOOL		IsOperable();

	void		WriteOpen();
	BOOL		WriteLine(char *text);
	void		WriteClose();
	void		Add(char *situation, int len);

protected:
	CInstanceManager* m_iMan;
	CRobotMain*		m_main;
	CEvent*			m_event;
	CInterface*		m_interface;
	CTerrain*		m_terrain;
	CWater*			m_water;

	char			m_currentLine[500];
	int				m_currentIndex;
	int				m_currentMax;
	char*			m_currentState;
	char*			m_currentRead;

	int				m_undoIndex;
	char*			m_undoBuffer[MAXUNDO];
	char*			m_undoResource[MAXUNDO];
	char*			m_undoLockZone[MAXUNDO];
	int				m_undoTotalManip[MAXUNDO];
};


#endif //_MAINUNDO_H_
