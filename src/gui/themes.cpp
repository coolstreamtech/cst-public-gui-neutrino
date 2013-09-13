/*
	$port: themes.cpp,v 1.16 2010/09/05 21:27:44 tuxbox-cvs Exp $
	
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "themes.h"
#include <global.h>
#include <neutrino.h>
#include "widget/menue.h"
#include <system/setting_helpers.h>
#include <gui/widget/messagebox.h>
#include <driver/screen_max.h>
#include <errno.h>
#include <algorithm>
#include <unistd.h>
#include <dirent.h>


#define THEMEDIR DATADIR "/neutrino/themes/"
#define USERDIR "/var" THEMEDIR
#define COLORCONF "color.conf"
#define INFOFILE "theme.info"
#define DEFAULT_THEMEDIR THEMEDIR "default"
#define DEFAULT_THEME_FILE DEFAULT_THEMEDIR "/" COLORCONF

CThemes::CThemes()
: themefile('\t')
{
	width 	= w_max (40, 10);
	notifier = NULL;
	hasThemeChanged = false;
	nameInput = NULL;
	theme_name = g_settings.theme_name;
}
CThemes::~CThemes()
{
	delete notifier;
	delete nameInput;
}


int CThemes::exec(CMenuTarget* parent, const std::string & actionKey)
{
	int res = menu_return::RETURN_REPAINT;

	if( !actionKey.empty() ){
		if (actionKey=="reset_colors"){
			if (!applyColors(DEFAULT_THEME_FILE)){
				g_settings.theme_name = "default";
// 				handleNotify();
			}
		}else{
			std::string th_path = getThemePath(actionKey);
			if (applyColors(th_path)){
				g_settings.theme_name = theme_name = actionKey;
				printf("[CThemes] %s: g_settings.theme_name = %s\n", __FUNCTION__, g_settings.theme_name.c_str());
			}
		}

		return res;
	}


	if (parent)
		parent->hide();

	if ( !hasThemeChanged )
		rememberOldTheme( true );

	return initMenu();
}


themes_t CThemes::getThemeMetaData()
{
	DIR *theme_dir;
	struct dirent *entry;
	theme_data_t data;
	themes_t res;
	res.clear();

	std::string theme_dirs[] = {THEMEDIR, USERDIR};

	for(uint i=0; i<2;i++){
		theme_dir = opendir(theme_dirs[i].c_str());
		if(theme_dir){
			do {
				entry = readdir(theme_dir);
				if (entry)
				{ 
					std::string theme_path = theme_dirs[i];
					theme_path += entry->d_name;
					std::string config_file 	= theme_path + "/" + COLORCONF;
					std::string info_file		= theme_path + "/" + INFOFILE;

					if (access(config_file.c_str(), F_OK) == 0 ){
						data.dirname = entry->d_name;
						data.name = getName(info_file);
						if (data.name == "unknown"){ //avoid unnamed entries, use dir name
							data.name = data.dirname;
						}

						res.push_back(data);
					}
				}
			} while (entry);
			closedir(theme_dir);
		}
		else
			printf("[CThemes] %s... error while loading theme from %s: %s\n", __FUNCTION__, theme_dirs[i].c_str(), strerror(errno));
	}

	sort(res.begin(), res.end());
	return res;
}

void CThemes::initMenuThemes(CMenuWidget &themes)
{
	themes_t v_th_data = getThemeMetaData();
	
	size_t th_count = v_th_data.size();
	bool has_themes = th_count == 0 ? false : true;
	CMenuForwarder* oj = NULL;
	
	if (!has_themes)
		return;

	for(size_t i=0; i<th_count;i++) {	
		std::string 	file = v_th_data[i].dirname + "/";
				file += COLORCONF;
		std::string 	path[2] = {THEMEDIR + file, USERDIR + file};
		
		for(int j=0; j<2; j++){
			if (access(path[j].c_str(), F_OK) == 0 ){
				std::string s_name =  v_th_data[i].name;
				if(!s_name.empty()){
					s_name[0] = (char)toupper(s_name[0]); //capitalize first letter
					oj = new CMenuForwarderNonLocalized(s_name.c_str(), true, "", this, v_th_data[i].dirname.c_str());
					themes.addItem( oj );
				}
			}
		}
	}
}

int CThemes::initMenu()
{
	CMenuWidget themes (LOCALE_COLORMENU_MENUCOLORS, NEUTRINO_ICON_SETTINGS, width);
	
	themes.addIntroItems(LOCALE_COLORTHEMEMENU_HEAD2);
	
	//reset to default colors
	themes.addItem(new CMenuForwarder(LOCALE_COLORTHEMEMENU_NEUTRINO_THEME, true, NULL, this, "reset_colors", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));

	//set name for new theme
	if (nameInput == NULL)
		nameInput = new CStringInputSMS(LOCALE_COLORTHEMEMENU_NAME, &theme_name, 30, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "abcdefghijklmnopqrstuvwxyz0123456789- ");
	//open sms box
	themes.addItem(new CMenuForwarder(LOCALE_COLORTHEMEMENU_SAVE, true , NULL, nameInput, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
	themes.addItem(GenericMenuSeparatorLine);
	
	//read available themes from filesystem and add theme names to menue
	initMenuThemes(themes);

	int res = themes.exec(NULL, "");

	//save a new theme
	if (!theme_name.empty())
		saveTheme();

	if (hasThemeChanged) {
		if (ShowLocalizedMessage(LOCALE_MESSAGEBOX_INFO, LOCALE_COLORTHEMEMENU_QUESTION, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo, NEUTRINO_ICON_SETTINGS) != CMessageBox::mbrYes)
			rememberOldTheme( false );
		else
			hasThemeChanged = false;
	}
	return res;
}

void CThemes::saveTheme()
{
	std::string new_dir = USERDIR + theme_name;
	std::string new_theme_file = new_dir + "/" + COLORCONF;

	// Don't show SAVE if UserDir does'nt exist
	// mkdir must be called for each subdir which does not exist 
	if ( access(new_dir.c_str(), F_OK) != 0 ) { // check for existance
	//	mkdir (USERDIR, S_IRUSR | S_IREAD | S_IWUSR | S_IWRITE | S_IXUSR | S_IEXEC) == 0) {
		if (system (((std::string)"mkdir -p " + new_dir).c_str()) != 0)
			printf("[CThemes] %s... error while creating theme dir %s: %s\n", __FUNCTION__, new_dir.c_str(), strerror(errno));
	}	
	if (saveFile(new_theme_file))
		g_settings.theme_name = theme_name;
}

void CThemes::rememberOldTheme(bool remember)
{
	if ( remember ) {
		oldThemeValues[0]  = g_settings.menu_Head_alpha;
		oldThemeValues[1]  = g_settings.menu_Head_red;
		oldThemeValues[2]  = g_settings.menu_Head_green;
		oldThemeValues[3]  = g_settings.menu_Head_blue;
		oldThemeValues[4]  = g_settings.menu_Head_Text_alpha;
		oldThemeValues[5]  = g_settings.menu_Head_Text_red;
		oldThemeValues[6]  = g_settings.menu_Head_Text_green;
		oldThemeValues[7]  = g_settings.menu_Head_Text_blue;
		oldThemeValues[8]  = g_settings.menu_Content_alpha;
		oldThemeValues[9]  = g_settings.menu_Content_red;
		oldThemeValues[10] = g_settings.menu_Content_green;
		oldThemeValues[11] = g_settings.menu_Content_blue;
		oldThemeValues[12] = g_settings.menu_Content_Text_alpha;
		oldThemeValues[13] = g_settings.menu_Content_Text_red;
		oldThemeValues[14] = g_settings.menu_Content_Text_green;
		oldThemeValues[15] = g_settings.menu_Content_Text_blue;
		oldThemeValues[16] = g_settings.menu_Content_Selected_alpha;
		oldThemeValues[17] = g_settings.menu_Content_Selected_red;
		oldThemeValues[18] = g_settings.menu_Content_Selected_green;
		oldThemeValues[19] = g_settings.menu_Content_Selected_blue;
		oldThemeValues[20] = g_settings.menu_Content_Selected_Text_alpha;
		oldThemeValues[21] = g_settings.menu_Content_Selected_Text_red;
		oldThemeValues[22] = g_settings.menu_Content_Selected_Text_green;
		oldThemeValues[23] = g_settings.menu_Content_Selected_Text_blue;
		oldThemeValues[24] = g_settings.menu_Content_inactive_alpha;
		oldThemeValues[25] = g_settings.menu_Content_inactive_red;
		oldThemeValues[26] = g_settings.menu_Content_inactive_green;
		oldThemeValues[27] = g_settings.menu_Content_inactive_blue;
		oldThemeValues[28] = g_settings.menu_Content_inactive_Text_alpha;
		oldThemeValues[29] = g_settings.menu_Content_inactive_Text_red;
		oldThemeValues[30] = g_settings.menu_Content_inactive_Text_green;
		oldThemeValues[31] = g_settings.menu_Content_inactive_Text_blue;
		oldThemeValues[32] = g_settings.infobar_alpha;
		oldThemeValues[33] = g_settings.infobar_red;
		oldThemeValues[34] = g_settings.infobar_green;
		oldThemeValues[35] = g_settings.infobar_blue;
		oldThemeValues[36] = g_settings.infobar_Text_alpha;
		oldThemeValues[37] = g_settings.infobar_Text_red;
		oldThemeValues[38] = g_settings.infobar_Text_green;
		oldThemeValues[39] = g_settings.infobar_Text_blue;
		oldThemeValues[40] = g_settings.colored_events_alpha;
		oldThemeValues[41] = g_settings.colored_events_red;
		oldThemeValues[42] = g_settings.colored_events_green;
		oldThemeValues[43] = g_settings.colored_events_blue;
	} else {
		g_settings.menu_Head_alpha 			= oldThemeValues[0];
		g_settings.menu_Head_red 			= oldThemeValues[1];
		g_settings.menu_Head_green 			= oldThemeValues[2];
		g_settings.menu_Head_blue 			= oldThemeValues[3];
		g_settings.menu_Head_Text_alpha 		= oldThemeValues[4];
		g_settings.menu_Head_Text_red 			= oldThemeValues[5];
		g_settings.menu_Head_Text_green 		= oldThemeValues[6];
		g_settings.menu_Head_Text_blue 			= oldThemeValues[7];
		g_settings.menu_Content_alpha 			= oldThemeValues[8];
		g_settings.menu_Content_red 			= oldThemeValues[9];
		g_settings.menu_Content_green 			= oldThemeValues[10];
		g_settings.menu_Content_blue 			= oldThemeValues[11];
		g_settings.menu_Content_Text_alpha 		= oldThemeValues[12];
		g_settings.menu_Content_Text_red 		= oldThemeValues[13];
		g_settings.menu_Content_Text_green 		= oldThemeValues[14];
		g_settings.menu_Content_Text_blue 		= oldThemeValues[15];
		g_settings.menu_Content_Selected_alpha 		= oldThemeValues[16];
		g_settings.menu_Content_Selected_red 		= oldThemeValues[17];
		g_settings.menu_Content_Selected_green 		= oldThemeValues[18];
		g_settings.menu_Content_Selected_blue 		= oldThemeValues[19];
		g_settings.menu_Content_Selected_Text_alpha 	= oldThemeValues[20];
		g_settings.menu_Content_Selected_Text_red 	= oldThemeValues[21];
		g_settings.menu_Content_Selected_Text_green 	= oldThemeValues[22];
		g_settings.menu_Content_Selected_Text_blue 	= oldThemeValues[23];
		g_settings.menu_Content_inactive_alpha 		= oldThemeValues[24];
		g_settings.menu_Content_inactive_red 		= oldThemeValues[25];
		g_settings.menu_Content_inactive_green 		= oldThemeValues[26];
		g_settings.menu_Content_inactive_blue		= oldThemeValues[27];
		g_settings.menu_Content_inactive_Text_alpha 	= oldThemeValues[28];
		g_settings.menu_Content_inactive_Text_red 	= oldThemeValues[29];
		g_settings.menu_Content_inactive_Text_green 	= oldThemeValues[30];
		g_settings.menu_Content_inactive_Text_blue 	= oldThemeValues[31];
		g_settings.infobar_alpha 			= oldThemeValues[32];
		g_settings.infobar_red 				= oldThemeValues[33];
		g_settings.infobar_green 			= oldThemeValues[34];
		g_settings.infobar_blue 			= oldThemeValues[35];
		g_settings.infobar_Text_alpha 			= oldThemeValues[36];
		g_settings.infobar_Text_red 			= oldThemeValues[37];
		g_settings.infobar_Text_green 			= oldThemeValues[38];
		g_settings.infobar_Text_blue 			= oldThemeValues[39];
		g_settings.colored_events_alpha			= oldThemeValues[40];
		g_settings.colored_events_red 			= oldThemeValues[41];
		g_settings.colored_events_green 		= oldThemeValues[42];
		g_settings.colored_events_blue 			= oldThemeValues[43];

		handleNotify();
	}
}

//get theme name from optional info file
std::string CThemes::getName(const std::string& info_file_path)
{
	std::string ret = "unknown";

	if(themefile.loadConfig(info_file_path))
		ret = themefile.getString( "name", ret );
	else
		printf("[neutrino theme] %s not found\n", info_file_path.c_str());

	return ret;
}

bool CThemes::readFile(const std::string& themepath)
{
	if(themefile.loadConfig(themepath))
	{
		g_settings.menu_Head_alpha = themefile.getInt32( "menu_Head_alpha", 0x00 );
		g_settings.menu_Head_red = themefile.getInt32( "menu_Head_red", 0x00 );
		g_settings.menu_Head_green = themefile.getInt32( "menu_Head_green", 0x0A );
		g_settings.menu_Head_blue = themefile.getInt32( "menu_Head_blue", 0x19 );
		g_settings.menu_Head_Text_alpha = themefile.getInt32( "menu_Head_Text_alpha", 0x00 );
		g_settings.menu_Head_Text_red = themefile.getInt32( "menu_Head_Text_red", 0x5f );
		g_settings.menu_Head_Text_green = themefile.getInt32( "menu_Head_Text_green", 0x46 );
		g_settings.menu_Head_Text_blue = themefile.getInt32( "menu_Head_Text_blue", 0x00 );
		g_settings.menu_Content_alpha = themefile.getInt32( "menu_Content_alpha", 0x14 );
		g_settings.menu_Content_red = themefile.getInt32( "menu_Content_red", 0x00 );
		g_settings.menu_Content_green = themefile.getInt32( "menu_Content_green", 0x0f );
		g_settings.menu_Content_blue = themefile.getInt32( "menu_Content_blue", 0x23 );
		g_settings.menu_Content_Text_alpha = themefile.getInt32( "menu_Content_Text_alpha", 0x00 );
		g_settings.menu_Content_Text_red = themefile.getInt32( "menu_Content_Text_red", 0x64 );
		g_settings.menu_Content_Text_green = themefile.getInt32( "menu_Content_Text_green", 0x64 );
		g_settings.menu_Content_Text_blue = themefile.getInt32( "menu_Content_Text_blue", 0x64 );
		g_settings.menu_Content_Selected_alpha = themefile.getInt32( "menu_Content_Selected_alpha", 0x14 );
		g_settings.menu_Content_Selected_red = themefile.getInt32( "menu_Content_Selected_red", 0x19 );
		g_settings.menu_Content_Selected_green = themefile.getInt32( "menu_Content_Selected_green", 0x37 );
		g_settings.menu_Content_Selected_blue = themefile.getInt32( "menu_Content_Selected_blue", 0x64 );
		g_settings.menu_Content_Selected_Text_alpha = themefile.getInt32( "menu_Content_Selected_Text_alpha", 0x00 );
		g_settings.menu_Content_Selected_Text_red = themefile.getInt32( "menu_Content_Selected_Text_red", 0x00 );
		g_settings.menu_Content_Selected_Text_green = themefile.getInt32( "menu_Content_Selected_Text_green", 0x00 );
		g_settings.menu_Content_Selected_Text_blue = themefile.getInt32( "menu_Content_Selected_Text_blue", 0x00 );
		g_settings.menu_Content_inactive_alpha = themefile.getInt32( "menu_Content_inactive_alpha", 0x14 );
		g_settings.menu_Content_inactive_red = themefile.getInt32( "menu_Content_inactive_red", 0x00 );
		g_settings.menu_Content_inactive_green = themefile.getInt32( "menu_Content_inactive_green", 0x0f );
		g_settings.menu_Content_inactive_blue = themefile.getInt32( "menu_Content_inactive_blue", 0x23 );
		g_settings.menu_Content_inactive_Text_alpha = themefile.getInt32( "menu_Content_inactive_Text_alpha", 0x00 );
		g_settings.menu_Content_inactive_Text_red = themefile.getInt32( "menu_Content_inactive_Text_red", 55 );
		g_settings.menu_Content_inactive_Text_green = themefile.getInt32( "menu_Content_inactive_Text_green", 70 );
		g_settings.menu_Content_inactive_Text_blue = themefile.getInt32( "menu_Content_inactive_Text_blue", 85 );
		g_settings.infobar_alpha = themefile.getInt32( "infobar_alpha", 0x14 );
		g_settings.infobar_red = themefile.getInt32( "infobar_red", 0x00 );
		g_settings.infobar_green = themefile.getInt32( "infobar_green", 0x0e );
		g_settings.infobar_blue = themefile.getInt32( "infobar_blue", 0x23 );
		g_settings.infobar_Text_alpha = themefile.getInt32( "infobar_Text_alpha", 0x00 );
		g_settings.infobar_Text_red = themefile.getInt32( "infobar_Text_red", 0x64 );
		g_settings.infobar_Text_green = themefile.getInt32( "infobar_Text_green", 0x64 );
		g_settings.infobar_Text_blue = themefile.getInt32( "infobar_Text_blue", 0x64 );
		g_settings.colored_events_alpha = themefile.getInt32( "colored_events_alpha", 0x00 );
		g_settings.colored_events_red = themefile.getInt32( "colored_events_red", 95 );
		g_settings.colored_events_green = themefile.getInt32( "colored_events_green", 70 );
		g_settings.colored_events_blue = themefile.getInt32( "colored_events_blue", 0 );

		handleNotify();
		return true;
	}
	else
		printf("[neutrino theme] %s not found\n", themepath.c_str());
	
	return false;
}

bool CThemes::saveFile(const std::string& themepath)
{
	themefile.setInt32( "menu_Head_alpha", g_settings.menu_Head_alpha );
	themefile.setInt32( "menu_Head_red", g_settings.menu_Head_red );
	themefile.setInt32( "menu_Head_green", g_settings.menu_Head_green );
	themefile.setInt32( "menu_Head_blue", g_settings.menu_Head_blue );
	themefile.setInt32( "menu_Head_Text_alpha", g_settings.menu_Head_Text_alpha );
	themefile.setInt32( "menu_Head_Text_red", g_settings.menu_Head_Text_red );
	themefile.setInt32( "menu_Head_Text_green", g_settings.menu_Head_Text_green );
	themefile.setInt32( "menu_Head_Text_blue", g_settings.menu_Head_Text_blue );
	themefile.setInt32( "menu_Content_alpha", g_settings.menu_Content_alpha );
	themefile.setInt32( "menu_Content_red", g_settings.menu_Content_red );
	themefile.setInt32( "menu_Content_green", g_settings.menu_Content_green );
	themefile.setInt32( "menu_Content_blue", g_settings.menu_Content_blue );
	themefile.setInt32( "menu_Content_Text_alpha", g_settings.menu_Content_Text_alpha );
	themefile.setInt32( "menu_Content_Text_red", g_settings.menu_Content_Text_red );
	themefile.setInt32( "menu_Content_Text_green", g_settings.menu_Content_Text_green );
	themefile.setInt32( "menu_Content_Text_blue", g_settings.menu_Content_Text_blue );
	themefile.setInt32( "menu_Content_Selected_alpha", g_settings.menu_Content_Selected_alpha );
	themefile.setInt32( "menu_Content_Selected_red", g_settings.menu_Content_Selected_red );
	themefile.setInt32( "menu_Content_Selected_green", g_settings.menu_Content_Selected_green );
	themefile.setInt32( "menu_Content_Selected_blue", g_settings.menu_Content_Selected_blue );
	themefile.setInt32( "menu_Content_Selected_Text_alpha", g_settings.menu_Content_Selected_Text_alpha );
	themefile.setInt32( "menu_Content_Selected_Text_red", g_settings.menu_Content_Selected_Text_red );
	themefile.setInt32( "menu_Content_Selected_Text_green", g_settings.menu_Content_Selected_Text_green );
	themefile.setInt32( "menu_Content_Selected_Text_blue", g_settings.menu_Content_Selected_Text_blue );
	themefile.setInt32( "menu_Content_inactive_alpha", g_settings.menu_Content_inactive_alpha );
	themefile.setInt32( "menu_Content_inactive_red", g_settings.menu_Content_inactive_red );
	themefile.setInt32( "menu_Content_inactive_green", g_settings.menu_Content_inactive_green );
	themefile.setInt32( "menu_Content_inactive_blue", g_settings.menu_Content_inactive_blue );
	themefile.setInt32( "menu_Content_inactive_Text_alpha", g_settings.menu_Content_inactive_Text_alpha );
	themefile.setInt32( "menu_Content_inactive_Text_red", g_settings.menu_Content_inactive_Text_red );
	themefile.setInt32( "menu_Content_inactive_Text_green", g_settings.menu_Content_inactive_Text_green );
	themefile.setInt32( "menu_Content_inactive_Text_blue", g_settings.menu_Content_inactive_Text_blue );
	themefile.setInt32( "infobar_alpha", g_settings.infobar_alpha );
	themefile.setInt32( "infobar_red", g_settings.infobar_red );
	themefile.setInt32( "infobar_green", g_settings.infobar_green );
	themefile.setInt32( "infobar_blue", g_settings.infobar_blue );
	themefile.setInt32( "infobar_Text_alpha", g_settings.infobar_Text_alpha );
	themefile.setInt32( "infobar_Text_red", g_settings.infobar_Text_red );
	themefile.setInt32( "infobar_Text_green", g_settings.infobar_Text_green );
	themefile.setInt32( "infobar_Text_blue", g_settings.infobar_Text_blue );
	themefile.setInt32( "colored_events_alpha", g_settings.colored_events_alpha );
	themefile.setInt32( "colored_events_red", g_settings.colored_events_red );
	themefile.setInt32( "colored_events_green", g_settings.colored_events_green );
	themefile.setInt32( "colored_events_blue", g_settings.colored_events_blue );

	bool ret = themefile.saveConfig(themepath.c_str());
	
	return ret;
}


void CThemes::handleNotify()
{
	if (notifier == NULL)
		notifier = new CColorSetupNotifier;
	notifier->changeNotify(NONEXISTANT_LOCALE, NULL);
	hasThemeChanged = false;
	delete notifier;
	notifier = NULL;
}

bool CThemes::applyColors(const std::string& themepath)
{
	std::string theme_path = themepath;
	bool ret = false;

	if ( strstr(theme_path.c_str(), "{U}") != 0 ) 
		theme_path.erase(0, 3);

	ret = readFile(theme_path);

	return ret;
}

void CThemes::loadColorConfig(const std::string& themename)
{
	std::string themepath = getThemePath(themename);
	
	if (!readFile(themepath))
		printf("[CThemes] %s: can't load %s\n", __FUNCTION__, themepath.c_str());
}

void CThemes::saveColorConfig(const std::string& themename)
{
	std::string themepath = getThemePath(themename);
		
	if (!saveFile(themepath))
		printf("[CThemes] %s: can't save %s\n", __FUNCTION__, themepath.c_str());
}

std::string CThemes::getThemePath(const std::string& themename)
{
	std::string 	file = themename + "/";
			file += COLORCONF;
	std::string 	path[2] = {THEMEDIR + file, USERDIR + file};
	
	std::string themepath = path[1];
	
	for(int i=0; i<2; i++){
		if (access(path[i].c_str(), F_OK) == 0 ){
			themepath = path[i];
			break;
		}
	}
	
	return themepath;
}
