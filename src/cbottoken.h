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

// cbottoken.h

#ifndef _CBOTTOKEN_H_
#define	_CBOTTOKEN_H_



enum ObjectType : int;



// Procédures.

extern char* RetObjectName(ObjectType type);
extern char* RetHelpFilename(ObjectType type);
extern char* RetHelpFilename(const char *token);
extern BOOL IsType(const char *token);
extern BOOL IsFunction(const char *token);
extern char* RetHelpText(const char *token);



#endif //_CBOTTOKEN_H_
