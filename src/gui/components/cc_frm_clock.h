/*
	Based up Neutrino-GUI - Tuxbox-Project
	Copyright (C) 2001 by Steffen Hehn 'McClean'

	Classes for generic GUI-related components.
	Copyright (C) 2012, 2013, Thilo Graf 'dbt'

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

#ifndef __CC_CLOCK__
#define __CC_CLOCK__


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <driver/neutrinofonts.h>
#include "cc_base.h"
#include "cc_frm.h"
#include "cc_timer.h"

//! Sub class of CComponents. Show clock with digits on screen. 
/*!
Usable as simple fixed display or as ticking clock.
*/

class CComponentsFrmClock : public CComponentsForm
{
	private:
		CComponentsTimer *cl_timer;
		void ShowTime();

	protected:
		///refresh interval in seconds
		int cl_interval;

		///raw time chars
		char cl_timestr[20];

		///handle paint clock within thread and is not similar to cc_allow_paint
		bool paintClock;

		///object: font render object
		Font **cl_font;

		int cl_font_type;
		int dyn_font_size;

		///text color
		int cl_col_text;
		
		///current time format
		std::string cl_format;
		///primary time format
		std::string cl_format_str;
		///secondary time format for blink
		std::string cl_blink_str;

		///time string align, default allign is ver and hor centered
		int cl_align;

		///initialize clock contents  
		void initCCLockItems();
		///initialize timestring, called in initCCLockItems()
		virtual void initTimeString();
		///initialize of general alignment of timestring segments within form area
		void initSegmentAlign(int* segment_width, int* segment_height);
		
		///start ticking clock, returns true on success, if false causes log output
		bool startClock();
		///stop ticking clock, returns true on success, if false causes log output
		bool stopClock();
		///switch between primary and secondary format
		void toggleFormat();
		///return pointer of font object
		inline Font** getClockFont();

	public:
		CComponentsFrmClock( 	const int& x_pos = 1, const int& y_pos = 1, const int& w = 200, const int& h = 48,
					const char* format_str = "%H:%M",
					const char* secformat_str = NULL,
					bool activ=false,
					const int& interval_seconds = 1,
					CComponentsForm *parent = NULL,
					bool has_shadow = CC_SHADOW_OFF,
					fb_pixel_t color_frame = COL_LIGHT_GRAY, fb_pixel_t color_body = COL_MENUCONTENT_PLUS_0, fb_pixel_t color_shadow = COL_MENUCONTENTDARK_PLUS_0);
		virtual ~CComponentsFrmClock();

		///set font type or font size for segments
		virtual void setClockFont(int font);
		virtual void setClockFontSize(int font_size, int share=CNeutrinoFonts::FONT_ID_SHARE);

		///set text color
		virtual void setTextColor(fb_pixel_t color_text){ cl_col_text = color_text;};

		///set alignment of timestring, possible modes see align types in cc_types.h 
		virtual void setClockAlignment(int align_type){cl_align = align_type;};

		///use string expession: "%H:%M" = 12:22, "%H:%M:%S" = 12:22:12
		///set current time format string, 1st parameter set the default format, 2nd parameter sets an alternatively format for use as blink effect
		virtual void setClockFormat(const char* prformat_str, const char* secformat_str = NULL);

		///start and paint ticking clock
		virtual bool Start();
		///stop ticking clock, but don't hide, use kill() or hide() to remove from screen
		virtual bool Stop();

		///returns true, if clock is running
		virtual bool isRun() const {return cl_timer ? true : false;};
		///set refresh interval in seconds, default value=1 (=1 sec)
		virtual void setClockIntervall(const int& seconds){cl_interval = seconds;};

		///show clock on screen
		virtual void paint(bool do_save_bg = CC_SAVE_SCREEN_YES);

		///reinitialize clock contents
		virtual void refresh() { initCCLockItems(); }
};

#endif
