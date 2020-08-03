#if 0
    Cyber Radio 1 - Internet Radio Client/Server System
    Copyright (C) 1995  John Selbie

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    John Selbie
    1600 NW 4th Avenue
    #24
    Gainesville, FL 32603

    e-mail: jselbie@cis.ufl.edu

#endif


#ifndef TABLES_H
#define TABLES_H

extern short ulawValues[];
typedef struct {
	short lowval;
	short highval;
	unsigned char ulaw;
} linearRange;

extern linearRange linearValue[];

#endif
