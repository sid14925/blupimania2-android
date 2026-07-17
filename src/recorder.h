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

// recorder.h

#ifndef _RECORDER_H_
#define	_RECORDER_H_



typedef struct
{
	D3DVECTOR	position;
	D3DVECTOR	angle;
}
RecorderEvent;


#define RECORDERBLOC	200

typedef struct RecorderBloc
{
	int				total;		// 0..RECORDERBLOC
	float			time[RECORDERBLOC];
	RecorderEvent	event[RECORDERBLOC];
	RecorderBloc*	prev;		// *bloc précédent
	RecorderBloc*	next;		// *bloc suivant
}
RecorderBloc;


class CRecorder
{
public:
	CRecorder(CInstanceManager* iMan);
	~CRecorder();

	void	SetModel(int model);
	int		RetModel();

	void	SetSubModel(int subModel);
	int		RetSubModel();

	void	SetColor(D3DCOLORVALUE color);
	D3DCOLORVALUE RetColor();

	void	SetType(int rank);
	int		RetType();

	void	SetMission(int rank);
	int		RetMission();

	void	SetCheck(int rank, int value);
	int		RetCheck(int rank);

	void	SetLevel(int level);
	int		RetLevel();

	void	SetGamer(char *name);
	char*	RetGamer();

	void	SetChrono(float time);
	float	RetChrono();

	BOOL	Put(float time, const RecorderEvent &event);
	BOOL	Get(float time, RecorderEvent &event);

	BOOL	Write(char *filename);
	BOOL	Read(char *filename);
	BOOL	ReadHeader(char *filename);

protected:
	void	Init();
	void	Flush();

protected:
	CInstanceManager* m_iMan;

	float			m_lastTime;
	RecorderBloc*	m_bloc;		// *premier bloc
	RecorderBloc*	m_record;	// *dernier bloc
	int				m_model;	// modèle de la voiture (1..n)
	int				m_subModel;	// modèle de la peinture (1..n)
	D3DCOLORVALUE	m_color;	// couleur de la voiture
	int				m_type;		// type de la mission
	int				m_mission;	// numéro de la mission
	int				m_check[10];// checksums divers
	int				m_level;	// niveau de difficulté
	char			m_gamer[20];// nom du joueur
	float			m_chrono;	// temps record
};


#endif //_RECORDER_H_
