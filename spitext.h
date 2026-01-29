/*
 * Copyright (c) 2012-2026 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 1901 rue Gilford, #53
 * Montreal, QC, H2H 1G8
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#ifndef _SPITEXT_H
#define _SPITEXT_H

#include "resource.h"



#define MAX_LOADSTRING 256

/*
#define SPICOUNTERMODE_COUNTUP		0
#define SPICOUNTERMODE_COUNTDOWN	1
#define SPICOUNTERMODE_CLOCK		2
*/
#define ID_TIMER_EVERYSECOND	1
#define ID_TIMER_ONCESTART		2
#define ID_TIMER_ONCEKILL		3


//2023august07, spi, begin
#define SPITEXT_DEFAULTSTRING	" "
//#define SPITEXT_DEFAULTSTRING	"          "
//2023august07, spi, end


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


#endif //_SPITEXT_H
