/*
	Based up Neutrino-GUI - Tuxbox-Project
	Copyright (C) 2001 by Steffen Hehn 'McClean'

	Info Clock Window
	based up CComponentsFrmClock
	Copyright (C) 2013, 2014, Thilo Graf 'dbt'
	Copyright (C) 2013, Michael Liebmann 'micha-bbg'

	License: GPL

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the
	Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
	Boston, MA  02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>
#include <neutrino.h>
#include <gui/volumebar.h>
#include <gui/infoclock.h>



CInfoClock::CInfoClock():CComponentsFrmClock( 0, 0, 0, 50, "%H:%M:%S", NULL, 1, true, NULL, CC_SHADOW_ON, COL_LIGHT_GRAY, COL_MENUCONTENT_PLUS_0,COL_MENUCONTENTDARK_PLUS_0)
{
	Init();
}

CInfoClock* CInfoClock::getInstance()
{
	static CInfoClock* InfoClock = NULL;
	if(!InfoClock)
		InfoClock = new CInfoClock();
	return InfoClock;
}

void CInfoClock::Init()
{
	//assign current font size
	setClockFontSize(g_settings.infoClockFontSize);

	//use current theme colors
	syncSysColors();

	paint_bg = g_settings.infoClockBackground;
	
	//set text color
	if (paint_bg)
		cl_col_text = COL_MENUCONTENT_TEXT;
	else
		cl_col_text = COL_INFOCLOCK_TEXT;

	if (g_settings.infoClockSeconds)
		setClockFormat("%H:%M:%S");
	else
		setClockFormat("%H:%M", "%H %M");

	int x_old = x, y_old = y, width_old = width, height_old = height;
	CVolumeHelper::getInstance()->refresh(cl_font);
	CVolumeHelper::getInstance()->getInfoClockDimensions(&x, &y, &width, &height);
	if ((x_old != x) || (y_old != y) || (width_old != width) || (height_old != height))
		clear();

	// set corner radius depending on clock height
	corner_rad = (g_settings.rounded_corners) ? std::max(height/10, CORNER_RADIUS_SMALL) : 0;

	initCCLockItems();
}

void CInfoClock::ClearDisplay()
{
	kill();
	Init();
}

bool CInfoClock::StartInfoClock()
{
	Init();
	return Start();
}

bool CInfoClock::StopInfoClock()
{
	bool ret = Stop();
	kill();

	return ret;
}

bool CInfoClock::enableInfoClock(bool enable)
{
	bool ret = false;
	if (g_settings.mode_clock) {
		if (enable) {
			if (!paintClock)
				ret = StartInfoClock();
		}
		else {
			if (paintClock)
				ret = StopInfoClock();
		}
	}
	return ret;
}
