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

// gamerfile.h

#ifndef _GAMERFILE_H_
#define	_GAMERFILE_H_



class CInstanceManager;



#define MAXGAMERFILE	500
#define MAXFILENAME		20
#define MAXSELECT		5

typedef struct
{
	char	puzzle[MAXFILENAME+2];
	char	numTry;
	char	bPassed;
	float	totalTime;
}
GamerFile;



class CGamerFile
{
public:
	CGamerFile(CInstanceManager* iMan);
	~CGamerFile();

	void	Flush();

	BOOL	Read(char *filename);
	BOOL	Write();
	BOOL	Delete(char *puzzle);

	void	RetSelect(int i, char *filename);
	void	SetSelect(int i, char *filename);

	int		RetNumTry(char *puzzle);
	BOOL	SetNumTry(char *puzzle, int numTry);

	BOOL	RetPassed(char *puzzle);
	BOOL	SetPassed(char *puzzle, BOOL bPassed);

	float	RetTotalTime(char *puzzle);
	BOOL	SetTotalTime(char *puzzle, float totalTime);

protected:
	int		SearchIndex(char *puzzle);
	int		CreateIndex(char *puzzle);

protected:
	CInstanceManager* m_iMan;

	char		m_filename[200];
	BOOL		m_bWriteToDo;
	char		m_select[MAXSELECT][MAXFILENAME+2];
	GamerFile	m_gamerFile[MAXGAMERFILE];
};


#endif //_GAMERFILE_H_
