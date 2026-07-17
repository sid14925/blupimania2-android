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

// filebuffer.h

#ifndef _FILEBUFFER_H_
#define	_FILEBUFFER_H_


class CInstanceManager;


typedef struct
{
	int		chapter;
	int		rank;
	int		index;
}
LineIndex;



class CFileBuffer
{
public:
	CFileBuffer(CInstanceManager* iMan);
	~CFileBuffer();

	void	Open();
	void	Close();
	BOOL	PutLine(int chapter, char *line);
	BOOL	GetLine(int chapter, int rank, char *line);

protected:

protected:
	CInstanceManager* m_iMan;

	int			m_total;		// lg max de m_buffer
	int			m_length;		// lg utlisée de m_buffer
	char*		m_buffer;		// lignes

	int			m_used;
	LineIndex	m_index[100];
};


#endif //_FILEBUFFER_H_
