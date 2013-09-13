/*
	$port: themes.h,v 1.6 2010/06/01 19:58:38 tuxbox-cvs Exp $
	
	Neutrino-GUI  -   DBoxII-Project

	License: GPL

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

	Copyright (C) 2007, 2008, 2009 (flasher) Frank Liebelt
*/

#ifndef __cthemes__
#define __cthemes__
#include <string>
#include <configfile.h>
#include <system/setting_helpers.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <vector>

//required typedefs
typedef struct theme_data_t
{
	std::string name;
	std::string dirname;
	
	bool operator< (const theme_data_t& a) const
	{
		return this->name < a.name ;
	}
} theme_data_struct_t;

typedef std::vector<theme_data_t> themes_t;


class CThemes : public CMenuTarget, CChangeObserver
{
	private:
		CConfigFile themefile;
		CColorSetupNotifier *notifier;
		CStringInputSMS *nameInput;

		int width;
		int oldThemeValues[44];

		bool hasThemeChanged;
		std::string theme_name;
		themes_t getThemeMetaData();
		std::string getName(const std::string& info_file_path);

		int initMenu();
		void initMenuThemes(CMenuWidget &);
		void saveTheme();
		bool readFile(const std::string& themepath);
		bool saveFile(const std::string& themepath);
		bool applyColors(const std::string& themepath);
		void handleNotify();
		std::string getThemePath(const std::string& themename);
		void rememberOldTheme(bool remember);

	public:
		CThemes();
		~CThemes();
		void loadColorConfig(const std::string& themename);
		void saveColorConfig(const std::string& themename);
		int exec(CMenuTarget* parent, const std::string & actionKey);
};

#endif
