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

// modfile.h

#ifndef _MODFILE_H_
#define	_MODFILE_H_


class CInstanceManager;
class CD3DEngine;



typedef struct
{
	char			bUsed;		// TRUE -> utilis�
	char			bSelect;	// TRUE -> s�lectionn�
	D3DVERTEX2		p1;
	D3DVERTEX2		p2;
	D3DVERTEX2		p3;
	D3DMATERIAL7	material;
	char			texName[20];
	float			min;
	float			max;
	int				state;		// 'long' is 8 bytes on LP64 (Android) and would break the 208-byte .MOD file layout
	short			texNum2;
	short			reserve2;
	short			reserve3;
	short			reserve4;
}
ModelTriangle;		// longueur = 208 bytes



class CModFile
{
public:
	CModFile(CInstanceManager* iMan);
	~CModFile();

	BOOL			ReadDXF(char *filename, float min, float max);
	BOOL			AddModel(char *filename, int first, BOOL bEdit=FALSE, BOOL bMeta=TRUE);
	BOOL			ReadModel(char *filename, BOOL bEdit=FALSE, BOOL bMeta=TRUE);
	BOOL			WriteModel(char *filename);

	BOOL			CreateEngineObject(int objRank, int addState=0);
	void			Mirror();
	void			Rotate(float angle);
	void			Translate(const D3DVECTOR &dist);
	void			TerrainNormalAdjust();
	void			TerrainNormalShadow(const D3DVECTOR &pos, float factor);
	void			TerrainRandomize(float rv, float rh);
	void			TerrainTexture(char *name, int rank);

	void			SetTriangleUsed(int total);
	int				RetTriangleUsed();
	int				RetTriangleMax();
	ModelTriangle*	RetTriangleList();

	float			RetHeight(D3DVECTOR pos);

protected:
	BOOL			CreateTriangle(D3DVECTOR p1, D3DVECTOR p2, D3DVECTOR p3, float min, float max);

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;

	ModelTriangle*	m_triangleTable;
	int				m_triangleUsed;
};


#endif //_MODFILE_H_
