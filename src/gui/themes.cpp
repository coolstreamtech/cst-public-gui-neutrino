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

#include <global.h>
#include <neutrino.h>
#include <gui/themes.h>
#include <system/setting_helpers.h>
#include <gui/widget/messagebox.h>
#include <driver/screen_max.h>
#include <errno.h>
#include <algorithm>
#include <unistd.h>
#include <dirent.h>
#include <gui/widget/colorchooser.h>
#include <sigc++/sigc++.h>

#define THEMEDIR DATADIR "/neutrino/themes/"
#define USERDIR "/var" THEMEDIR
#define COLORCONF "color.conf"
#define INFOFILE "theme.info"
#define DEFAULT_THEMEDIR THEMEDIR "default"
#define DEFAULT_THEME_FILE DEFAULT_THEMEDIR "/" COLORCONF

using namespace std;

CThemes::CThemes()
: themefile('\t')
{
	width 	= w_max (40, 10);
	notifier = new CColorSetupNotifier;
	nameInput = NULL;
	initColorVars();
	v_cbdata.clear();
}

void CThemes::initColorVars()
{
	//read current settings into vars
	//theme name
	cur_theme = old_theme	= g_settings.theme_name;

	//head
	colors[0].p_curVal  	= &g_settings.menu_Head_alpha;
	colors[0].oldVal  	= *colors[0].p_curVal;
	colors[0].key 		= "menu_Head_alpha";
	colors[0].defaultVal 	= 0x00;

	colors[1].p_curVal  	= &g_settings.menu_Head_red;
	colors[1].oldVal  	= *colors[1].p_curVal;
	colors[1].key 		= "menu_Head_red";
	colors[1].defaultVal 	= 0x00;

	colors[2].p_curVal  	= &g_settings.menu_Head_green;
	colors[2].oldVal  	= *colors[2].p_curVal;
	colors[2].key 		= "menu_Head_green";
	colors[2].defaultVal 	= 0x0A;

	colors[3].p_curVal  	= &g_settings.menu_Head_blue;
	colors[3].oldVal  	= *colors[3].p_curVal;
	colors[3].key 		= "menu_Head_blue";
	colors[3].defaultVal 	= 0x19;

	//head text
	colors[4].p_curVal  	= &g_settings.menu_Head_Text_alpha;
	colors[4].oldVal  	= *colors[4].p_curVal;
	colors[4].key 		= "menu_Head_Text_alpha";
	colors[4].defaultVal 	= 0x00;

	colors[5].p_curVal  	= &g_settings.menu_Head_Text_red;
	colors[5].oldVal  	= *colors[5].p_curVal;
	colors[5].key 		= "menu_Head_Text_red";
	colors[5].defaultVal 	= 0x5f;

	colors[6].p_curVal  	= &g_settings.menu_Head_Text_green;
	colors[6].oldVal  	= *colors[6].p_curVal;
	colors[6].key 		= "menu_Head_Text_green";
	colors[6].defaultVal 	= 0x46;

	colors[7].p_curVal  	= &g_settings.menu_Head_Text_blue;
	colors[7].oldVal  	= *colors[7].p_curVal;
	colors[7].key 		= "menu_Head_Text_blue";
	colors[7].defaultVal 	= 0x00;

	//contents
	colors[8].p_curVal  	= &g_settings.menu_Content_alpha;
	colors[8].oldVal  	= *colors[8].p_curVal;
	colors[8].key 		= "menu_Content_alpha";
	colors[8].defaultVal 	= 0x14;

	colors[9].p_curVal  	= &g_settings.menu_Content_red;
	colors[9].oldVal  	= *colors[9].p_curVal;
	colors[9].key 		= "menu_Content_red";
	colors[9].defaultVal 	= 0x00;

	colors[10].p_curVal  	= &g_settings.menu_Content_green;
	colors[10].oldVal  	= *colors[10].p_curVal;
	colors[10].key 		= "menu_Content_green";
	colors[10].defaultVal 	= 0x0f;

	colors[11].p_curVal  	= &g_settings.menu_Content_blue;
	colors[11].oldVal  	= *colors[11].p_curVal;
	colors[11].key 		= "menu_Content_blue";
	colors[11].defaultVal 	= 0x23;

	//contents text
	colors[12].p_curVal  	= &g_settings.menu_Content_Text_alpha;
	colors[12].oldVal  	= *colors[12].p_curVal;
	colors[12].key 		= "menu_Content_Text_alpha";
	colors[12].defaultVal 	= 0x00;

	colors[13].p_curVal  	= &g_settings.menu_Content_Text_red;
	colors[13].oldVal  	= *colors[13].p_curVal;
	colors[13].key 		= "menu_Content_Text_red";
	colors[13].defaultVal 	= 0x64;

	colors[14].p_curVal  	= &g_settings.menu_Content_Text_green;
	colors[14].oldVal  	= *colors[14].p_curVal;
	colors[14].key 		= "menu_Content_Text_green";
	colors[14].defaultVal 	= 0x64;

	colors[15].p_curVal  	= &g_settings.menu_Content_Text_blue;
	colors[15].oldVal  	= *colors[15].p_curVal;
	colors[15].key 		= "menu_Content_Text_blue";
	colors[15].defaultVal 	= 0x64;

	//contents selected
	colors[16].p_curVal  	= &g_settings.menu_Content_Selected_alpha;
	colors[16].oldVal  	= *colors[16].p_curVal;
	colors[16].key 		= "menu_Content_Selected_alpha";
	colors[16].defaultVal 	= 0x14;

	colors[17].p_curVal  	= &g_settings.menu_Content_Selected_red;
	colors[17].oldVal  	= *colors[17].p_curVal;
	colors[17].key 		= "menu_Content_Selected_red";
	colors[17].defaultVal 	= 0x19;

	colors[18].p_curVal  	= &g_settings.menu_Content_Selected_green;
	colors[18].oldVal  	= *colors[18].p_curVal;
	colors[18].key 		= "menu_Content_Selected_green";
	colors[18].defaultVal 	= 0x37;

	colors[19].p_curVal  	= &g_settings.menu_Content_Selected_blue;
	colors[19].oldVal  	= *colors[19].p_curVal;
	colors[19].key 		= "menu_Content_Selected_blue";
	colors[19].defaultVal 	= 0x64;

	//contents selected text
	colors[20].p_curVal  	= &g_settings.menu_Content_Selected_Text_alpha;
	colors[20].oldVal  	= *colors[20].p_curVal;
	colors[20].key 		= "menu_Content_Selected_Text_alpha";
	colors[20].defaultVal 	= 0x00;

	colors[21].p_curVal  	= &g_settings.menu_Content_Selected_Text_red;
	colors[21].oldVal  	= *colors[21].p_curVal;
	colors[21].key 		= "menu_Content_Selected_Text_red";
	colors[21].defaultVal 	= 0x00;

	colors[22].p_curVal  	= &g_settings.menu_Content_Selected_Text_green;
	colors[22].oldVal  	= *colors[22].p_curVal;
	colors[22].key 		= "menu_Content_Selected_Text_green";
	colors[22].defaultVal 	= 0x00;

	colors[23].p_curVal  	= &g_settings.menu_Content_Selected_Text_blue;
	colors[23].oldVal  	= *colors[23].p_curVal;
	colors[23].key 		= "menu_Content_Selected_Text_blue";
	colors[23].defaultVal 	= 0x00;

	//contents inactive
	colors[24].p_curVal  	= &g_settings.menu_Content_inactive_alpha;
	colors[24].oldVal  	= *colors[24].p_curVal;
	colors[24].key 		= "menu_Content_inactive_alpha";
	colors[24].defaultVal 	= 0x14;

	colors[25].p_curVal  	= &g_settings.menu_Content_inactive_red;
	colors[25].oldVal  	= *colors[25].p_curVal;
	colors[25].key 		= "menu_Content_inactive_red";
	colors[25].defaultVal 	= 0x00;

	colors[26].p_curVal  	= &g_settings.menu_Content_inactive_green;
	colors[26].oldVal  	= *colors[26].p_curVal;
	colors[26].key 		= "menu_Content_inactive_green";
	colors[26].defaultVal 	= 0x0f;

	colors[27].p_curVal  	= &g_settings.menu_Content_inactive_blue;
	colors[27].oldVal  	= *colors[27].p_curVal;
	colors[27].key 		= "menu_Content_inactive_blue";
	colors[27].defaultVal 	= 0x23;

	//contents inactive text
	colors[28].p_curVal  	= &g_settings.menu_Content_inactive_Text_alpha;
	colors[28].oldVal  	= *colors[28].p_curVal;
	colors[28].key 		= "menu_Content_inactive_Text_alpha";
	colors[28].defaultVal 	= 0x00;

	colors[29].p_curVal  	= &g_settings.menu_Content_inactive_Text_red;
	colors[29].oldVal  	= *colors[29].p_curVal;
	colors[29].key 		= "menu_Content_inactive_Text_red";
	colors[29].defaultVal 	= 55;

	colors[30].p_curVal  	= &g_settings.menu_Content_inactive_Text_green;
	colors[30].oldVal  	= *colors[30].p_curVal;
	colors[30].key 		= "menu_Content_inactive_Text_green";
	colors[30].defaultVal 	= 70;

	colors[31].p_curVal  	= &g_settings.menu_Content_inactive_Text_blue;
	colors[31].oldVal  	= *colors[31].p_curVal;
	colors[31].key 		= "menu_Content_inactive_Text_blue";
	colors[31].defaultVal 	= 85;

	//infobar
	colors[32].p_curVal  	= &g_settings.infobar_alpha;
	colors[32].oldVal  	= *colors[32].p_curVal;
	colors[32].key 		= "infobar_alpha";
	colors[32].defaultVal 	= 0x14;

	colors[33].p_curVal  	= &g_settings.infobar_red;
	colors[33].oldVal  	= *colors[33].p_curVal;
	colors[33].key 		= "infobar_red";
	colors[33].defaultVal 	= 0x00;

	colors[34].p_curVal  	= &g_settings.infobar_green;
	colors[34].oldVal  	= *colors[34].p_curVal;
	colors[34].key 		= "infobar_green";
	colors[34].defaultVal 	= 0x0e;

	colors[35].p_curVal  	= &g_settings.infobar_blue;
	colors[35].oldVal  	= *colors[35].p_curVal;
	colors[35].key 		= "infobar_blue";
	colors[35].defaultVal 	= 0x23;

	//infobar text
	colors[36].p_curVal  	= &g_settings.infobar_Text_alpha;
	colors[36].oldVal  	= *colors[36].p_curVal;
	colors[36].key 		= "infobar_Text_alpha";
	colors[36].defaultVal 	= 0x00;

	colors[37].p_curVal  	= &g_settings.infobar_Text_red;
	colors[37].oldVal  	= *colors[37].p_curVal;
	colors[37].key 		= "infobar_Text_red";
	colors[37].defaultVal 	= 0x64;

	colors[38].p_curVal  	= &g_settings.infobar_Text_green;
	colors[38].oldVal  	= *colors[38].p_curVal;
	colors[38].key 		= "infobar_Text_green";
	colors[38].defaultVal 	= 0x64;

	colors[39].p_curVal  	= &g_settings.infobar_Text_blue;
	colors[39].oldVal  	= *colors[39].p_curVal;
	colors[39].key 		= "infobar_Text_blue";
	colors[39].defaultVal 	= 0x64;

	//events
	colors[40].p_curVal  	= &g_settings.colored_events_alpha;
	colors[40].oldVal  	= *colors[40].p_curVal;
	colors[40].key 		= "colored_events_alpha";
	colors[40].defaultVal 	= 0x00;

	colors[41].p_curVal  	= &g_settings.colored_events_red;
	colors[41].oldVal  	= *colors[41].p_curVal;
	colors[41].key 		= "colored_events_red";
	colors[41].defaultVal 	= 95;

	colors[42].p_curVal  	= &g_settings.colored_events_green;
	colors[42].oldVal  	= *colors[42].p_curVal;
	colors[42].key 		= "colored_events_green";
	colors[42].defaultVal 	= 070;

	colors[43].p_curVal  	= &g_settings.colored_events_blue;
	colors[43].oldVal  	= *colors[43].p_curVal;
	colors[43].key 		= "colored_events_blue";
	colors[43].defaultVal 	= 0x00;
}

CThemes::~CThemes()
{
	delete notifier;
	delete nameInput;
	cleanUpCallBackBeforePaint();
}


int CThemes::exec(CMenuTarget* parent, const string & actionKey)
{
	if (parent)
		parent->hide();

	int res = menu_return::RETURN_REPAINT;

	if( !actionKey.empty() ){
		if(actionKey=="reset_colors"){
			if (loadTheme(DEFAULT_THEME_FILE)){
				cur_theme = "default";
				printf("[CThemes] %s: cur_theme = %s\n", __FUNCTION__, cur_theme.c_str());
			}
		}else{
			string th_path = getThemePath(actionKey);
			if (loadTheme(th_path)){
				cur_theme = actionKey;
				printf("[CThemes] %s: error: cur_theme = %s\n", __FUNCTION__, cur_theme.c_str());
			}
		}

		return res;
	}

	return initMenu();
}

//init main menu
int CThemes::initMenu()
{
	CMenuWidget themes (LOCALE_COLORMENU_MENUCOLORS, NEUTRINO_ICON_COLORS, width);	
	themes.addIntroItems(LOCALE_COLORTHEMEMENU_HEAD2);
	
	//reset to default colors
	themes.addItem(new CMenuForwarder(LOCALE_COLORTHEMEMENU_NEUTRINO_THEME, true, NULL, this, "reset_colors", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	
// 	//init theme name edit
// 	initThemeNameEdit();
// 	themes.addItem(new CMenuForwarder(LOCALE_COLORTHEMEMENU_SAVE, true , NULL, nameInput, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
	themes.addItem(GenericMenuSeparatorLine);
	
	//read available themes from filesystem and add theme names to menue
	initMenuThemes(themes);

	int res = themes.exec(NULL, "");

	//save a new theme
// 	if (!theme_name.empty())
// 		saveTheme();
	
	if (isChangedColors()){
		initThemeNameEdit();
		nameInput->exec(NULL, "");
	}

	if (isChangedThemeName()) {
		if (ShowLocalizedMessage(LOCALE_MESSAGEBOX_INFO, LOCALE_COLORTHEMEMENU_QUESTION, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo, NEUTRINO_ICON_SETTINGS) != CMessageBox::mbrYes)
			applyThemeColors(true); //restore
		else
			applyThemeColors(false); //no restore
			
		if (saveTheme(false, g_settings.theme_name)) //not custom
			handleNotify();

	}
	
	return res;
}

//init input box
void CThemes::initThemeNameEdit()
{
	if (nameInput == NULL)
		nameInput = new CStringInputSMS(LOCALE_COLORTHEMEMENU_NAME, &cur_theme, 30, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "abcdefghijklmnopqrstuvwxyz0123456789- ");
}

//callback function to apply current themes to current CMenuWidget
//this function assigns selected color settings befor the CMenuWidget object will paint its menue items
void CThemes::CallBackBeforePaint(void* arg, void*)
{
	printf("[CThemes]    [%s]  Callback: void arg: %p\n", __FUNCTION__, arg);
	callback_data_t *event = static_cast<callback_data_t*>(arg);
	
	printf("[CThemes]    [%s]  Callback: cbdata: %p, , = %s\n", __FUNCTION__, event, event->datastr.c_str());
	CThemes *themes = event->themes;
	themes->exec(NULL, event->datastr);
}


//clean up v_cbdata args, because of allocated CallBackBeforePaint args
void CThemes::cleanUpCallBackBeforePaint()
{
	if (v_cbdata.empty())
		return;
	
	for(size_t i=0; i<v_cbdata.size();i++){
		if (v_cbdata[i]){
			printf("[CThemes]    [%s]  clean up v_cbdata[%d]  %p, = %s\n", __FUNCTION__, i, v_cbdata[i], v_cbdata[i]->datastr.c_str());
			delete v_cbdata[i];
		}
		v_cbdata[i] = NULL;
	}
	v_cbdata.clear();
}

//init sub menu contains available theme list
void CThemes::initMenuThemes(CMenuWidget &themes)
{
	themes_t v_th_data;
	v_th_data.clear();
	v_th_data = getThemeMetaData();
	
	size_t th_count = v_th_data.size();
	bool has_themes = th_count == 0 ? false : true;

	//clean up possible callback_data_t args
	cleanUpCallBackBeforePaint();

	if (!has_themes)
		return;

	for(size_t i=0; i<th_count;i++) {	
		string 	file = v_th_data[i].dirname + "/";
				file += COLORCONF;
		string 	path[2] = {THEMEDIR + file, USERDIR + file};

		for(int j=0; j<2; j++){
			if (access(path[j].c_str(), F_OK) == 0 ){
				string s_name =  v_th_data[i].name;
				if(!s_name.empty()){
					s_name[0] = (char)toupper(s_name[0]); //capitalize first letter

					//color theme settings
					CMenuWidget *th_colors = new CMenuWidget(LOCALE_COLORTHEMEMENU_EDIT, NEUTRINO_ICON_COLORS, width);

					//assign callback args for setEventBeforePaint provided by CMenuWidget *th_colors
					//arg is a struct with theme dirname as string and an instance of current CThemes as "this"
					callback_data_t *cbdata = new callback_data_t; //create the struct pointer, Note: we need a cleaner, to find in cleanUpCallBackBeforePaint()
					cbdata->datastr = v_th_data[i].dirname; 
					cbdata->themes = this;

					//now push all args into v_cbdata vector,
					//this provides a possibility for destruction of all allocated args at once, see cleanUpCallBackBeforePaint()
					v_cbdata.push_back(cbdata);

					//casted arg for callback to required void* pointer
					void* ptr = static_cast<void*>(v_cbdata[v_cbdata.size()-1]);

					//finally creating a slot to CMenuWidget *th_colors widget
// 					th_colors->setEventBeforePaint(&CallBackBeforePaint, ptr);
					sigc::slot1<void, void*> sl = sigc::bind<0>(sigc::mem_fun2(*this, &CThemes::CallBackBeforePaint), ptr);
					th_colors->BeforePaint.connect(sl);

					CMenuForwarderNonLocalized *osd_colors = new CMenuForwarderNonLocalized(s_name.c_str(), true, NULL, th_colors, v_th_data[i].dirname.c_str());
					themes.addItem(osd_colors);

					initMenuThemeColors(th_colors, v_th_data[i].dirname);
					CMenuSeparator *sep =  static_cast<CMenuSeparator*>(th_colors->getItem(0));
					sep->setString(s_name);
				}
			}
		}
	}
}


//init color edit menu
void CThemes::initMenuThemeColors(CMenuWidget *menu_colors, const string &themedir)
{
	CColorChooser* chHeadcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, colors[1].p_curVal/*&g_settings.menu_Head_red*/, colors[2].p_curVal/*&g_settings.menu_Head_green*/, colors[3].p_curVal/*&g_settings.menu_Head_blue*/,
			colors[0].p_curVal/*&g_settings.menu_Head_alpha*/, notifier);
	CColorChooser* chHeadTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Head_Text_red, &g_settings.menu_Head_Text_green, &g_settings.menu_Head_Text_blue,
			NULL, notifier);
	CColorChooser* chContentcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Content_red, &g_settings.menu_Content_green, &g_settings.menu_Content_blue,
			&g_settings.menu_Content_alpha, notifier);
	CColorChooser* chContentTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Content_Text_red, &g_settings.menu_Content_Text_green, &g_settings.menu_Content_Text_blue,
			NULL, notifier);
	CColorChooser* chContentSelectedcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Content_Selected_red, &g_settings.menu_Content_Selected_green, &g_settings.menu_Content_Selected_blue,
			&g_settings.menu_Content_Selected_alpha, notifier);
	CColorChooser* chContentSelectedTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Content_Selected_Text_red, &g_settings.menu_Content_Selected_Text_green, &g_settings.menu_Content_Selected_Text_blue,
			NULL, notifier);
	CColorChooser* chContentInactivecolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Content_inactive_red, &g_settings.menu_Content_inactive_green, &g_settings.menu_Content_inactive_blue,
			&g_settings.menu_Content_inactive_alpha, notifier);
	CColorChooser* chContentInactiveTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Content_inactive_Text_red, &g_settings.menu_Content_inactive_Text_green, &g_settings.menu_Content_inactive_Text_blue,
			NULL, notifier);
	
	menu_colors->addIntroItems(LOCALE_COLORMENU_MENUCOLORS);
	
	//apply settings
	
	CMenuForwarderNonLocalized* oj = NULL;
	oj = new CMenuForwarderNonLocalized("Apply Settings...", true, "", this, themedir.c_str(), CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED);
	menu_colors->addItem( oj );
	
	//init theme name edit
	initThemeNameEdit();
	menu_colors->addItem(new CMenuForwarder(LOCALE_COLORTHEMEMENU_SAVE, true , NULL, nameInput, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));

	menu_colors->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUHEAD));
	
	CMenuDForwarder *mf = NULL;
	
	mf = new CMenuDForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chHeadcolor );
	mf->setHint("", LOCALE_MENU_HINT_HEAD_BACK);
	menu_colors->addItem(mf);

	mf = new CMenuDForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chHeadTextcolor );
	mf->setHint("", LOCALE_MENU_HINT_HEAD_TEXTCOLOR);
	menu_colors->addItem(mf);

	menu_colors->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT));
	mf = new CMenuDForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentcolor );
	mf->setHint("", LOCALE_MENU_HINT_CONTENT_BACK);
	menu_colors->addItem(mf);

	mf = new CMenuDForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentTextcolor );
	mf->setHint("", LOCALE_MENU_HINT_CONTENT_TEXTCOLOR);
	menu_colors->addItem(mf);

	menu_colors->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT_INACTIVE));
	mf = new CMenuDForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentInactivecolor );
	mf->setHint("", LOCALE_MENU_HINT_INACTIVE_BACK);
	menu_colors->addItem(mf);

	mf = new CMenuDForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentInactiveTextcolor);
	mf->setHint("", LOCALE_MENU_HINT_INACTIVE_TEXTCOLOR);
	menu_colors->addItem(mf);

	menu_colors->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT_SELECTED));
	mf = new CMenuDForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentSelectedcolor );
	mf->setHint("", LOCALE_MENU_HINT_SELECTED_BACK);
	menu_colors->addItem(mf);

	mf = new CMenuDForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentSelectedTextcolor );
	mf->setHint("", LOCALE_MENU_HINT_SELECTED_TEXT);
	menu_colors->addItem(mf);

	CColorChooser* chInfobarcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.infobar_red,
			&g_settings.infobar_green, &g_settings.infobar_blue, &g_settings.infobar_alpha, notifier);
	CColorChooser* chInfobarTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.infobar_Text_red,
			&g_settings.infobar_Text_green, &g_settings.infobar_Text_blue, NULL, notifier);

	menu_colors->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORSTATUSBAR_TEXT));
	mf = new CMenuDForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chInfobarcolor );
	mf->setHint("", LOCALE_MENU_HINT_INFOBAR_BACK);
	menu_colors->addItem(mf);

	mf = new CMenuDForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chInfobarTextcolor );
	mf->setHint("", LOCALE_MENU_HINT_INFOBAR_TEXTCOLOR);
	menu_colors->addItem(mf);

	CColorChooser* chColored_Events = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR,	&g_settings.colored_events_red,
			&g_settings.colored_events_green, &g_settings.colored_events_blue, NULL, notifier);

	menu_colors->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MISCSETTINGS_INFOBAR_COLORED_EVENTS));
	mf = new CMenuDForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chColored_Events );
	mf->setHint("", LOCALE_MENU_HINT_EVENT_TEXTCOLOR);
	menu_colors->addItem(mf);
}


themes_t CThemes::getThemeMetaData()
{
	DIR *theme_dir;
	struct dirent *entry;
	theme_data_t data;
	themes_t res;
	res.clear();

	string theme_dirs[] = {THEMEDIR, USERDIR};

	for(uint i=0; i<2;i++){
		theme_dir = opendir(theme_dirs[i].c_str());
		if(theme_dir){
			do {
				entry = readdir(theme_dir);
				if (entry)
				{ 
					string theme_path = theme_dirs[i];
					theme_path += entry->d_name;
					string config_file 	= theme_path + "/" + COLORCONF;
					string info_file		= theme_path + "/" + INFOFILE;

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


//save theme in directory
bool CThemes::saveTheme(bool isCustom, string themeName)
{
	string themedir = (isCustom ? USERDIR : THEMEDIR);
	
	string new_dir = themedir + themeName;
	string new_theme_file = new_dir + "/" + COLORCONF;
	
	printf("[CThemes] %s... saving colors to %s\n", __FUNCTION__, new_theme_file.c_str());

	// Don't show SAVE if UserDir does'nt exist
	// mkdir must be called for each subdir which does not exist 
	if ( access(new_dir.c_str(), F_OK) != 0 ) { // check for existance
	//	mkdir (USERDIR, S_IRUSR | S_IREAD | S_IWUSR | S_IWRITE | S_IXUSR | S_IEXEC) == 0) {
		if (system (((string)"mkdir -p " + new_dir).c_str()) != 0){
			printf("[CThemes] %s... error while creating theme dir %s: %s\n", __FUNCTION__, new_dir.c_str(), strerror(errno));
			return false;
		}
	}
	
	if (isCustom){
		if (!saveColors(new_theme_file)){
			printf("[CThemes] %s... error while saving theme config %s\n", __FUNCTION__, new_theme_file.c_str());
			return false;
		}
	}

	return true;
}

//apply current color settings or restore
void CThemes::applyThemeColors(bool restore)
{
	if ( restore ) {
		g_settings.theme_name = cur_theme = old_theme;
		for(size_t i=0; i<COLOR_COUNT; i++)
			*colors[i].p_curVal = colors[i].oldVal;
	} else {
		g_settings.theme_name = old_theme = cur_theme;
		for(size_t i=0; i<COLOR_COUNT; i++)
			colors[i].oldVal = *colors[i].p_curVal;
	}
	
}

//return true, if any color was changed
bool CThemes::isChangedColors()
{
	for(size_t i=0; i<COLOR_COUNT; i++){
		if (colors[i].oldVal != *colors[i].p_curVal)
			return true;
	}
	return false;
}

//return true, if theme name was changed
bool CThemes::isChangedThemeName()
{
	string stmp = cur_theme;
	bool ret = (old_theme != stmp) ? true : false;
	printf("[CThemes]    [%s]  theme name changed = %d [old_theme = %s], [cur_theme = %s]\n", __FUNCTION__, ret, old_theme.c_str(), stmp.c_str());
	return ret;
}

//get theme name from optional info file
string CThemes::getName(const string& info_file_path)
{
	string ret = "unknown";

	if(themefile.loadConfig(info_file_path))
		ret = themefile.getString( "name", ret );
	else
		printf("[neutrino theme] %s not found\n", info_file_path.c_str());

	return ret;
}

//load color.conf file, send to colors struct and palette
bool CThemes::readColors(const string& themePath)
{
	if(themefile.loadConfig(themePath))
	{
		for(size_t i=0; i<COLOR_COUNT; i++)
			*colors[i].p_curVal = (uint8_t)themefile.getInt32( colors[i].key, colors[i].defaultVal);

		handleNotify();
		return true;
	}
	else
		printf("[neutrino theme] %s not found\n", themePath.c_str());
	
	return false;
}

//save current color values to color.conf file 
bool CThemes::saveColors(const string& themePath)
{
	for(size_t i=0; i<COLOR_COUNT; i++)
		themefile.setInt32( colors[i].key, *colors[i].p_curVal );

	bool ret = themefile.saveConfig(themePath.c_str());
	
	return ret;
}


//send color settings to palette
void CThemes::handleNotify()
{
	if (notifier == NULL)
		notifier = new CColorSetupNotifier;
	notifier->changeNotify(NONEXISTANT_LOCALE, NULL);

// 	delete notifier;
// 	notifier = NULL;
}

bool CThemes::loadTheme(const string& themePath)
{
	string theme_path = themePath;
	bool ret = false;

	if ( strstr(theme_path.c_str(), "{U}") != 0 ) 
		theme_path.erase(0, 3);

	ret = readColors(theme_path);

	return ret;
}

void CThemes::loadColorConfig(const string& themeName)
{
	string themePath = getThemePath(themeName);
	
	if (!readColors(themePath))
		printf("[CThemes] %s: can't load %s\n", __FUNCTION__, themePath.c_str());
}

void CThemes::saveColorConfig(const string& themeName)
{
	string themePath = getThemePath(themeName);
		
	if (!saveColors(themePath))
		printf("[CThemes] %s: can't save %s\n", __FUNCTION__, themePath.c_str());
}

string CThemes::getThemePath(const string& themeName)
{
	string 	file = themeName + "/";
			file += COLORCONF;
	string 	path[2] = {THEMEDIR + file, USERDIR + file};
	
	string themePath = path[1];
	
	for(int i=0; i<2; i++){
		if (access(path[i].c_str(), F_OK) == 0 ){
			themePath = path[i];
			break;
		}
	}

	return themePath;
}
