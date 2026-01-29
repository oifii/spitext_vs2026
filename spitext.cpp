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

// spitext.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "spiutility.h"
#include "spitext.h"
#include <stdio.h>
#include <assert.h>
#include <ShellAPI.h>

//#include <string>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream

using namespace std;



// Global Variables:
HINSTANCE hInst;								// current instance

//TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
//TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szTitle[1024]={L"spitexttitle"};					// The title bar text
TCHAR szWindowClass[1024]={L"spitextclass"};			// the main window class name

int global_x=0;
int global_y=0;
//2021nov23, spi, begin
//string global_spitextstring="some text";
string global_spitextstring=SPITEXT_DEFAULTSTRING;
//2021nov23, spi, end
HFONT global_hFont = NULL;
int global_fontheight=480;
int global_fontwidth=-1; //will be computed within WM_PAINT handler

int global_starttime_sec=-1; //user specified, -1 for not specified
int global_endtime_sec=-1; //user specified, -1 for not specified

int global_timetodisplay_sec; //calculated
DWORD global_startstamp_ms;
DWORD global_nowstamp_ms;

char charbuffer[1024]={""};
char charbuffer_prev[1024]={""};

BYTE global_alpha=220;

//global_monitor is either empty "" (global_x and global_y will be considered absolute) 
//or "1" or "2" or "2.1" or "2.2" or "2.3" or "2.4" specifying monitor (global_x and global_y will be considered relative)
//global_monitor can also be using ":" instead of "." in which case there will be 16 posisble submonitors instead of 4, i.e. "1:1", "1:2", "1:3", ..., "1:15", "1:16", etc. for specifying monitor (global_x and global_y will be considered relative)
string global_monitor = "";
//global_hmonitor is either an empty string "" (global_x and global_y will be considered absolute) or the HMONITOR numeric value embeded into a string i.e. "0x00001E76" specifying a monitor handle (global_x and global_y will be considered relative)
string global_hmonitor = "";
//global_hwnd is either an empty string "" (global_x and global_y will be considered absolute) or the HWND numeric value embeded into a string i.e. "0x00001E76" specifying a window handle (global_x and global_y will be considered relative)
string global_hwnd = "";
//global_windowclass is either an empty string "" (global_x and global_y will be considered absolute) or the window class string i.e. "windowclassofinterest" (global_x and global_y will be considered relative)
string global_windowclass = "";
//global_windowtitle is either an empty string "" (global_x and global_y will be considered absolute) or the window title string i.e. "windowtitleofinterest" (global_x and global_y will be considered relative)
string global_windowtitle = "";
//global_horizontaljustification is either an empty string "" (global_x and global_y will be considered) or i.e. "left", "right", "center" (global_x and global_y will be ignored)
string global_horizontaljustification = "";
//global_verticaljustification is either an empty string "" (global_x and global_y will be considered) or i.e. "top", "bottom", "center" (global_x and global_y will be ignored)
string global_verticaljustification = "";

//global_horizontalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
//int global_horizontalforcefit = 0;
float global_horizontalforcefit = 0.0f;
//global_vecticalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
//int global_verticalforcefit = 0;
float global_verticalforcefit = 0.0f;

int global_fullmonitorssurface = 1; //global_fullmonitorssurface is either 0 for no or 1, the default, for yes do display string over multiple monitors

//new parameters
#include <atlconv.h>
//string global_fontface="Segoe Script"; //see charmap.exe for fontface (Win+R>charmap)
string global_fontface="Arial"; //see charmap.exe for fontface (Win+R>charmap)
int global_idfontcolor=0;
//string global_classname="spicounterclass";
//string global_title="spicountertitle";
//string global_begin="begin.ahk";
string global_begin="";
//string global_starting="starting.ahk";
string global_starting="";
//string global_finishing="finishing.ahk";
string global_finishing="";
//string global_end="end.ahk";
string global_end="";


//keying color (to be made transparent color)
COLORREF global_keyingcolor = RGB(255, 0, 255); //spi original
//COLORREF global_keyingcolor = RGB(0, 0, 0); //carl
//COLORREF global_keyingcolor = RGB(0, 255, 0); //other


//these structures initialize themselves automatically with the list of monitors 
//and the list of all top-level windows on the screens
MonitorRects _monitors; 
EnumWindowsStruct_spitext _windows;
//initially presume global_x and global_y coordinate absolute
POINT myoutputPOINT;

bool globalxyabsolute = true;
RECT myoutputRECT;

FILE* pFILE=NULL;
//SIZE mySIZE;

int global_textmode = -1; //defaults to -1 to display straight text
int global_textformat = -1; //defaults to -1 to display straight text otherwise used by the counter modes

vector<string> global_countermode;
vector<string> global_counterformat;

int global_countermodeCOUNTUP = 0;
int global_countermodeCOUNTDOWN = 1;
int global_countermodeCLOCK = 2;

int global_counterformatHHMMSS = 0;
int global_counterformatHHMM = 1;
int global_counterformatMMSS = 2;
int global_counterformatHH = 3;
int global_counterformatMM = 4;
int global_counterformatSS = 5;

//2023august07, spi, begin
string global_modestring = "";
int global_bpm = 60;
bool global_randomcharacter = false;
bool global_randomposition = false;
bool global_randomheight = false;
bool global_randomfont = false;
bool global_words = false;
bool global_colors = false;
int global_spitextstring_initiallength = 1; //will be set according to the initial length of global_spitextstring
//const char* fontfaces[] = {"Segoe Script","Arial","Palatino Linotype","DengXian","Ink Free","MV Boli"};
//int numfontfaces = 6;
const char* fontfaces[] = {"Segoe Script","Arial","Courier","Times","DejaVuSans","Symbol"};
int numfontfaces = 6;
const char* words[] = {"Love","Forever","Together","Nature","Earth","Ocean","Sea","Shore","Road","Trip","Adventure","Rock"};
int numwords = 12;
//const char* words[] = {"Sun","Energy","Happy","Times","Moments","Light","High","Higher","Star","Shine"};
//int numwords = 10;
//2023august07, spi, end


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	global_startstamp_ms = GetTickCount();

	LPSTR *szArgList;
	int nArgs;
	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvA failed\n");
		return FALSE;
	}
	LPWSTR *szArgListW;
	int nArgsW;
	szArgListW = CommandLineToArgvW(GetCommandLineW(), &nArgsW);
	if( NULL == szArgListW )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}
	if(nArgs>1)
	{
		global_spitextstring = szArgList[1]; //any text to be displayed or "COUNTUP", "COUNTDOWN", "CLOCK", or a combinaison of "COUNTUP", "COUNTDOWN", "CLOCK" along with "HH", "MM", "SS", 
											 //"HH:MM:SS", "HH:MM", "MM:SS", "COUNTUP HH:MM:SS", "COUNTDOWN MM:SS", "CLOCK HH:MM:SS"
	}
	if(nArgs>2)
	{
		global_starttime_sec = atoi(szArgList[2]); //-1, 0 or positive integer specifying seconds, used to setup display string when global_spitextstring contains "COUNTUP", "COUNTDOWN", "CLOCK" 
													//otherwise with any other global_spitextstring, global_starttime_sec is used as a delay before initiating global_spitextstring display
	}
	if(nArgs>3)
	{
		global_endtime_sec = atoi(szArgList[3]); //-1, 0 or positive integer specifying seconds, used to setup display string when global_spitextstring contains "COUNTUP", "COUNTDOWN", "CLOCK"
												//otherwise with any other global_spitextstring, global_starttime_sec is used as a delay before killing global_spitextstring display
	}
	if(nArgs>4)
	{
		global_x = atoi(szArgList[4]);
	}
	if(nArgs>5)
	{
		global_y = atoi(szArgList[5]);
	}
	if(nArgs>6)
	{
		global_fontheight = atoi(szArgList[6]);
	}
	//new parameters
	if(nArgs>7)
	{
		global_fontface = szArgList[7]; 
	}
	if(nArgs>8)
	{
		global_idfontcolor = atoi(szArgList[8]); 
	}
	if(nArgs>9)
	{
		wcscpy(szWindowClass, szArgListW[9]); 
	}
	if(nArgs>10)
	{
		wcscpy(szTitle, szArgListW[10]); 
	}
	if(nArgs>11)
	{
		global_begin = szArgList[11]; 
	}
	if(nArgs>12)
	{
		global_starting = szArgList[12]; 
	}
	if(nArgs>13)
	{
		global_finishing = szArgList[13]; 
	}
	if(nArgs>14)
	{
		global_end = szArgList[14]; 
	}
	if(nArgs>15)
	{
		global_alpha = atoi(szArgList[15]); 
	}
	if(nArgs>16)
	{
		global_monitor = szArgList[16]; //global_monitor is either empty "" (global_x and global_y will be considered absolute) or i.e. "1" or "2" or "2.1" or "2.2" or "2.3" or "2.4" etc. specifying monitor (global_x and global_y will be considered relative)
										//global_monitor can also be using ":" instead of "." in which case there will be 16 posisble submonitors instead of 4, i.e. "1:1", "1:2", "1:3", ..., "1:15", "1:16", etc. for specifying monitor (global_x and global_y will be considered relative)
	}
	if(nArgs>17)
	{
		global_hmonitor = szArgList[17]; //global_hmonitor is either an empty string "" (global_x and global_y will be considered absolute) or the HMONITOR numeric value embeded into a string i.e. "0x00001E76" specifying a monitor handle (global_x and global_y will be considered relative)
	}
	if(nArgs>18)
	{
		global_hwnd = szArgList[18]; //global_hwnd is either an empty string "" (global_x and global_y will be considered absolute) or the HWND numeric value embeded into a string i.e. "0x00001E76" specifying a window handle (global_x and global_y will be considered relative)
	}
	if(nArgs>19)
	{
		global_windowclass = szArgList[19]; //global_windowclass is either an empty string "" (global_x and global_y will be considered absolute) or the window class string i.e. "windowclassofinterest" (global_x and global_y will be considered relative)
	}
	if(nArgs>20)
	{
		global_windowtitle = szArgList[20]; //global_windowtitle is either an empty string "" (global_x and global_y will be considered absolute) or the window title string i.e. "windowtitleofinterest" (global_x and global_y will be considered relative)
	}
	if(nArgs>21)
	{
		global_horizontaljustification = szArgList[21]; //global_horizontaljustification is either an empty string "" (global_x and global_y will be considered) or i.e. "left", "right", "center" (global_x and global_y will be ignored)
	}
	if(nArgs>22)
	{
		global_verticaljustification = szArgList[22]; //global_verticaljustification is either an empty string "" (global_y and global_y will be considered) or i.e. "top", "bottom", "center" (global_x and global_y will be ignored)
	}
	if(nArgs>23)
	{
		//global_horizontalforcefit = atoi(szArgList[23]); //global_horizontalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
		global_horizontalforcefit = atof(szArgList[23]); //global_horizontalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
		if(global_horizontalforcefit<0.0f) global_horizontalforcefit=0.0f;
		if(global_horizontalforcefit>1.0f) global_horizontalforcefit=1.0f;
	}
	if(nArgs>24)
	{
		//global_vecticalforcefit = atoi(szArgList[24]); //global_vecticalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
		global_verticalforcefit = atof(szArgList[24]); //global_vecticalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
		if(global_verticalforcefit<0.0f) global_verticalforcefit=0.0f;
		if(global_verticalforcefit>1.0f) global_verticalforcefit=1.0f;
	}
	if(nArgs>25)
	{
		global_fullmonitorssurface = atoi(szArgList[25]); //global_fullmonitorssurface is either 0 for no or 1, the default, for yes do display string over multiple monitors
	}
	//2023august07, spi, begin
	if(nArgs>26)
	{
		global_modestring = szArgList[26]; //defaults to "" corresponding to no animation
											 
	}
	if(nArgs>27)
	{
		global_bpm = atoi(szArgList[27]); //defaults to 60 corresponding to a redrawing period of 1 sec
													
	}
	if ( (global_modestring.find("RANDOMCHARACTER") != string::npos) )
	{
		global_randomcharacter = true; 
	}
	if ( (global_modestring.find("WORDS") != string::npos) )
	{
		global_words = true; 
	}
	if ( (global_modestring.find("RANDOMPOSITION") != string::npos) )
	{
		global_randomposition = true; 
	}
	if ( (global_modestring.find("RANDOMHEIGHT") != string::npos) )
	{
		global_randomheight = true; 
	}
	if ( (global_modestring.find("RANDOMFONT") != string::npos) )
	{
		global_randomfont = true; 
	}
	if ( (global_modestring.find("COLORS") != string::npos) )
	{
		global_colors = true; 
	}
	//2023august07, spi, end
	LocalFree(szArgList);
	LocalFree(szArgListW);

	//pFILE = fopen("debug.txt", "w");
	if(pFILE)
	{
		fprintf(pFILE, "global_horizontalforcefit is %f\n", global_horizontalforcefit);
		fprintf(pFILE, "global_verticalforcefit is %f\n", global_verticalforcefit);
		fprintf(pFILE, "\n\n");
	}
	//global_startstamp_ms = GetTickCount();

	//init possible clock modes and formats
	global_countermode.push_back("COUNTUP");
	global_countermode.push_back("COUNTDOWN");
	global_countermode.push_back("CLOCK");
	global_counterformat.push_back("HH:MM:SS");
	global_counterformat.push_back("HH:MM");
	global_counterformat.push_back("MM:SS");
	global_counterformat.push_back("HH");
	global_counterformat.push_back("MM");
	global_counterformat.push_back("SS");

	/*
	int global_countermodeCOUNTUP = 0;
	int global_countermodeCOUNTDOWN = 1;
	int global_countermodeCLOCK = 2;
	*/
	//global_textmode = -1; //default to display straight text
	if ( (global_spitextstring.find(global_countermode[global_countermodeCOUNTUP]) != string::npos) )
	{
		global_textmode = global_countermodeCOUNTUP; 
	}
	else if ( (global_spitextstring.find(global_countermode[global_countermodeCOUNTDOWN]) != string::npos) )
	{
		global_textmode = global_countermodeCOUNTDOWN; 
	}
	else if ( (global_spitextstring.find(global_countermode[global_countermodeCLOCK]) != string::npos) )
	{
		global_textmode = global_countermodeCLOCK; 
	}


	/*
	int global_counterformatHHMMSS = 0;
	int global_counterformatHHMM = 1;
	int global_counterformatMMSS = 2;
	int global_counterformatHH = 3;
	int global_counterformatMM = 4;
	int global_counterformatSS = 5;
	*/
	if(global_textmode>-1)
	{
		//some counter mode, set counter initial time to be displayed
		if(global_starttime_sec>-1)
		{
			global_timetodisplay_sec = global_starttime_sec;
		}
		else
		{
			global_timetodisplay_sec = 0;
		}

		global_textformat = global_counterformatHHMMSS; //default when unspecified by user
		//some counter mode, check if counter format specified
		if( (global_spitextstring.find(global_counterformat[global_counterformatHHMMSS]) != string::npos) )
		{
			global_textformat = global_counterformatHHMMSS;
		}
		else if( (global_spitextstring.find(global_counterformat[global_counterformatHHMM]) != string::npos) )
		{
			global_textformat = global_counterformatHHMM;
		}
		else if( (global_spitextstring.find(global_counterformat[global_counterformatMMSS]) != string::npos) )
		{
			global_textformat = global_counterformatMMSS;
		}
		else if( (global_spitextstring.find(global_counterformat[global_counterformatHH]) != string::npos) )
		{
			global_textformat = global_counterformatHH;
		}
		else if( (global_spitextstring.find(global_counterformat[global_counterformatMM]) != string::npos) )
		{
			global_textformat = global_counterformatMM;
		}
		else if( (global_spitextstring.find(global_counterformat[global_counterformatSS]) != string::npos) )
		{
			global_textformat = global_counterformatSS;
		}
		//2021nov29, spi, begin
		if(global_textmode==global_countermodeCOUNTDOWN)
		{
			if(global_starttime_sec<0) global_starttime_sec = 0;
			if(global_endtime_sec>global_starttime_sec) 
			{
				int temp = global_endtime_sec;
				global_endtime_sec = global_starttime_sec;
				global_starttime_sec = temp;
				global_timetodisplay_sec = global_starttime_sec;
			}
		}
		else
		{
			if( (global_endtime_sec-global_starttime_sec)<=0 ) global_endtime_sec = -1; //no ending timer will be set
		}
		//2021nov29, spi, end
	}
	else
	{
		//2021nov23, spi, begin
		//string display mode
		if(global_spitextstring.empty()) global_spitextstring = SPITEXT_DEFAULTSTRING; //force to be at least 1 space so textextent and fontsize can be computed
		global_spitextstring_initiallength = global_spitextstring.length();
		//2021nov23, spi, end
		//string display mode, use start and end time as delay and kill time
		if(global_starttime_sec<=0) global_starttime_sec = 0; //no delay start timer will be set
		if(global_endtime_sec<=0) global_endtime_sec = -1; //no ending timer will be set
		if( (global_endtime_sec-global_starttime_sec)<=0 ) global_endtime_sec = -1; //no ending timer will be set
	}


	//these structures initialize themselves automatically with the list of monitors 
	//and the list of all top-level windows on the screens
	//MonitorRects _monitors; 
	//EnumWindowsStruct_spitext _windows;
	if(pFILE)
	{
		int numberofmonitor=_monitors.rcMonitors.size();
		if(numberofmonitor>0)
		{
			//idmonitor is valid
			for(int indexmonitor=0; indexmonitor<numberofmonitor; indexmonitor++)
			{
				RECT myRECT = _monitors.rcMonitors[indexmonitor];
				HMONITOR hmonitor = _monitors.hMonitors[indexmonitor];
				fprintf(pFILE, "detecting monitors id %d (left %d, right %d, top %d, bottom %d)\n", indexmonitor+1, myRECT.left, myRECT.right, myRECT.top, myRECT.bottom);
				stringstream ss;
				ss << std::hex << hmonitor;
				string hexstring(ss.str());
				fprintf(pFILE, "detecting monitors id %d (hmonitor: dec %d, hex 0x%s)\n", indexmonitor+1, hmonitor, hexstring.c_str());
			}
			fprintf(pFILE, "combined global rectangle from all monitors (left %d, right %d, top %d, bottom %d)\n", _monitors.rcCombined.left, _monitors.rcCombined.right, _monitors.rcCombined.top, _monitors.rcCombined.bottom);
		}
		else
		{
			fprintf(pFILE, "error, not detecting any monitors!\n");
			fclose(pFILE);
			return(FALSE);
		}
	}

#ifdef _DEBUG
	if(pFILE)
	{
		fprintf(pFILE, "\ndetecting the following windows (window classname listing):\n\n");
		if(_windows.hwndstringmap_class.size()>1)
		{
			std::map<HWND, wstring>::iterator it;
			for (it = _windows.hwndstringmap_class.begin(); it != _windows.hwndstringmap_class.end(); it++)
			{
				HWND myHWND = it->first;
				wstring mywindowclassw = it->second;
				string mywindowclass = "";
				if(!mywindowclassw.empty()) mywindowclass = utf8_encode(mywindowclassw);
				fprintf(pFILE, "hwnd,windowclassname %d,%40s\n", myHWND, mywindowclass.c_str());
			}
		}
		fprintf(pFILE, "\n");
		fprintf(pFILE, "\ndetecting the following windows (window title listing):\n\n");
		if(_windows.hwndstringmap_title.size()>1)
		{
			std::map<HWND, wstring>::iterator it;
			for (it = _windows.hwndstringmap_title.begin(); it != _windows.hwndstringmap_title.end(); it++)
			{
				HWND myHWND = it->first;
				wstring mywindowtitlew = it->second;
				string mywindowtitle = "";
				if(!mywindowtitlew.empty()) mywindowtitle = utf8_encode(mywindowtitlew);
				fprintf(pFILE, "hwnd,windowtitle %d,%40s\n", myHWND, mywindowtitle.c_str());
			}
		}
		fflush(pFILE);
	}
#endif

	//initially presume global_x and global_y coordinate absolute
	//POINT myoutputPOINT;
	myoutputPOINT.x = global_x;
	myoutputPOINT.y= global_y;
	//bool globalxyabsolute = true;
	//RECT myoutputRECT;
	
	HMONITOR myHMONITOR = NULL;
	std::map<HMONITOR, RECT>::iterator it;
	if(global_fullmonitorssurface) 
	{
		//default to the rectangle made from all monitors combined
		myoutputRECT = _monitors.rcCombined;
	}
	else
	{
		//fit to one monitor, rectrict from displaying string extending over multiple monitors
		//HMONITOR myHMONITOR;
		myHMONITOR = MonitorFromPoint(myoutputPOINT, MONITOR_DEFAULTTONEAREST);
		//std::map<HMONITOR, RECT>::iterator it;
		it = _monitors.hmapMonitors.find(myHMONITOR);
		if (it != _monitors.hmapMonitors.end())
		{
			myoutputRECT = (*it).second;
		}
		else
		{
			assert(false);
			return(FALSE);
		}
	}
	if(!global_monitor.empty())
	{
		int idmonitor = atoi(global_monitor.substr(0,1).c_str());
		int idsubmonitor = 0;
		int numsubmonitors = 1;
		if(idmonitor==0) idmonitor=1;
		if(idmonitor>0 && idmonitor<10)
		{
			if(idmonitor <= _monitors.rcMonitors.size())
			{
				//idmonitor is valid
				globalxyabsolute = false;
				myoutputRECT = _monitors.rcMonitors[idmonitor-1];
			}
			if( (global_monitor.size()>2) && ((global_monitor.substr(1,1)==".")||(global_monitor.substr(1,1)==":")) )
			{
				if(global_monitor.substr(1,1)==".")
				{
					idsubmonitor = atoi(global_monitor.substr(2,1).c_str());
					if(idsubmonitor<1) idsubmonitor=1;
					if(idsubmonitor>4) idsubmonitor=4;
					numsubmonitors = 4;
				}
				else if(global_monitor.substr(1,1)==":")
				{
					idsubmonitor = atoi(global_monitor.substr(2,1).c_str());
					if(idsubmonitor==1 && (global_monitor.size()>3))
					{
						//use second digit if present and valid
						int seconddigit = atoi(global_monitor.substr(3,1).c_str());
						if(seconddigit>-1 && seconddigit<7) idsubmonitor=10+seconddigit;
					}
					if(idsubmonitor<1) idsubmonitor=1;
					if(idsubmonitor>16) idsubmonitor=16;
					numsubmonitors = 16;
				}
				if(idsubmonitor>0 && idsubmonitor<17)
				{
					//idsubmonitor is valid
					//divbyfour=true;
					//now, let's divide myoutputRECT by 4 same size rectangles
					//based on https://stackoverflow.com/questions/6190019/split-a-rectangle-into-equal-sized-rectangles
					RECT original = myoutputRECT;
					//int numsubmonitors = 1;
					//if (global_monitor.substr(1,1)==".") numsubmonitors = 4;
					//else if (global_monitor.substr(1,1)==":") numsubmonitors = 16;
					int columns = ceil(sqrt((float)numsubmonitors));
					int fullrows = numsubmonitors / columns;
					int orphans = numsubmonitors % columns;   // how many 'odd-sized' ones on our bottom row.
					//calculate output width and height
					int width =  (original.right-original.left)/columns; //original.width/ columns;
					int height = (original.bottom-original.top)/(orphans == 0 ? fullrows : (fullrows+1)); //original.height / (orphans == 0 ? fullrows : (fullrows+1)); // reduce height if there are orphans
					//calculate output rectangles
					RECT output[16];
					assert(numsubmonitors<=16);
					int i=-1;
					//to be similar to displayfusion software installed on remotedroide
					//i.e. second monitor's submonitors are specified like this
					//2.1 2.3
					//2.2 2.4
					for (int x = 0; x < columns; ++x) //for (int y = 0; y < fullrows; ++y)
					{
						for (int y = 0; y < fullrows; ++y) //for (int x = 0; x < columns; ++x)
						{
							i++;
							//submonitor "X.1"
							output[i].left = original.left + x * width; //output.push_back(CRect(x * width, y * height, width, height));
							//submonitor "X.2"
							output[i].top = original.top + y * height; //output.push_back(CRect(x * width, y * height, width, height));
							//submonitor "X.3"
							output[i].right = output[i].left + width; //output.push_back(CRect(x * width, y * height, width, height));
							//submonitor "X.4"
							output[i].bottom = output[i].top + height; //output.push_back(CRect(x * width, y * height, width, height));
						}
					}
					myoutputRECT = output[idsubmonitor-1];
				}
			}
		}
		if(pFILE)
		{
			fprintf(pFILE, "\nmonitor specified by %s\n", global_monitor.c_str());
			fprintf(pFILE, "idmonitor is %d, idsubmonitor is %d, number of submonitors is %d\n", idmonitor, idsubmonitor, numsubmonitors);
			fprintf(pFILE, "output rect is now, left %d right %d top %d bottom %d\n", myoutputRECT.left, myoutputRECT.right, myoutputRECT.top, myoutputRECT.bottom);
		}
	}
	else if(!global_hmonitor.empty())
	{
		//todo: for hex string, use strtol https://en.cppreference.com/w/c/string/byte/strtol
		HMONITOR suppliedhmonitor;
		if(global_hmonitor.size()>3 && global_hmonitor.substr(0,2)=="0x") 
		{
			global_hmonitor = global_hmonitor.substr(2);
			suppliedhmonitor = (HMONITOR) strtol(global_hmonitor.c_str(), NULL, 16);
		}
		else 
		{
			suppliedhmonitor = (HMONITOR) atol(global_hmonitor.c_str());
		}
		it = _monitors.hmapMonitors.find(suppliedhmonitor);
		if (it != _monitors.hmapMonitors.end())
		{
			myHMONITOR = (*it).first;
			myoutputRECT = (*it).second;
		}
		else
		{
			assert(false);
			if(pFILE) 
			{
				fprintf(pFILE, "error global_hmonitor not found");
				fclose(pFILE);
			}
			return(FALSE);
		}
		globalxyabsolute = false;
	}
	else if(!global_hwnd.empty())
	{
		//todo: for hex string, use strtol https://en.cppreference.com/w/c/string/byte/strtol
		HWND suppliedhwnd;
		if(global_hwnd.size()>3 && global_hwnd.substr(0,2)=="0x") 
		{
			global_hwnd = global_hwnd.substr(2);
			suppliedhwnd = (HWND) strtol(global_hwnd.c_str(), NULL, 16);
		}
		else 
		{
			suppliedhwnd = (HWND) atol(global_hwnd.c_str());
		}

		BOOL bresult = GetWindowRect(suppliedhwnd,  &myoutputRECT);
		if(!bresult)
		{
			assert(false);
			if(pFILE) 
			{
				fprintf(pFILE, "error global_hwnd not found");
				fclose(pFILE);
			}
			return(FALSE);
		}
		myHMONITOR = MonitorFromRect(&myoutputRECT, MONITOR_DEFAULTTONEAREST);
		globalxyabsolute = false;
	}
	else if(!global_windowclass.empty())
	{
		BOOL bresult = false;
		HWND suppliedhwnd;
		if(!global_windowtitle.empty())
		{
			suppliedhwnd = (HWND) FindWindowA(global_windowclass.c_str(), global_windowtitle.c_str());
		}
		else
		{
			suppliedhwnd = (HWND) FindWindowA(global_windowclass.c_str(), NULL);
		}
		if(suppliedhwnd) bresult = GetWindowRect(suppliedhwnd,  &myoutputRECT);
		if(!bresult)
		{
			assert(false);
			if(pFILE) 
			{
				fprintf(pFILE, "error global_windowclass and window title not found");
				fclose(pFILE);
			}
			return(FALSE);
		}
		myHMONITOR = MonitorFromRect(&myoutputRECT, MONITOR_DEFAULTTONEAREST);
		globalxyabsolute = false;
	}
	else if(!global_windowtitle.empty())
	{
		BOOL bresult = false;
		HWND suppliedhwnd = FindWindowA(NULL, global_windowtitle.c_str());
		if(suppliedhwnd) bresult = GetWindowRect(suppliedhwnd,  &myoutputRECT);
		if(!bresult)
		{
			assert(false);
			if(pFILE) 
			{
				fprintf(pFILE, "error global_windowtitle not found");
				fclose(pFILE);
			}
			return(FALSE);
		}
		myHMONITOR = MonitorFromRect(&myoutputRECT, MONITOR_DEFAULTTONEAREST);
		globalxyabsolute = false;
	}

	/*
	//moved into InitInstance()
	if( (global_textmode==-1) && (global_starttime_sec>0) )
	{
		//todo, launch a timer to wait for global_starttime_sec before display;
	}
	if( (global_textmode==-1) && (global_endtime_sec>0) && ((global_endtime_sec-global_starttime_sec)>0) )
	{
		//todo, launch a timer to wait for global_timetodisplay_sec before killing app;
	}
	*/

	if(!global_begin.empty()) ShellExecuteA(NULL, "open", global_begin.c_str(), "", NULL, nCmdShow);
	if(!global_starting.empty()) ShellExecuteA(NULL, "open", global_starting.c_str(), "", NULL, nCmdShow);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_CPPMFC_TRANSPARENTTXT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		if(pFILE) fclose(pFILE);
		return FALSE;
	}

	//hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CPPMFC_TRANSPARENTTXT));
	//hAccelTable = NULL;
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CPPMFC_TRANSPARENTTXT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= L""; //MAKEINTRESOURCE(IDC_CPPMFC_TRANSPARENTTXT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

bool justification(SIZE mySIZE)
{
	bool didjustify = false;

	//horizontal justification
	if(pFILE && !global_horizontaljustification.empty()) 
	{
		fprintf(pFILE, "\nbefore horizontal justification\n");
		fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n", global_x, global_y, mySIZE.cx, mySIZE.cy);
		fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
	}
	if(global_horizontaljustification=="left")
	{
	}
	else if(global_horizontaljustification=="right")
	{
		if( (globalxyabsolute!=0) && (global_fullmonitorssurface==0) ) 
		{
			global_x = global_x - mySIZE.cx;
		}
		else 
		{
			global_x = myoutputRECT.right - mySIZE.cx;
		}
		didjustify=true;
	}
	else if(global_horizontaljustification=="center")
	{
		if( (globalxyabsolute!=0) && (global_fullmonitorssurface==0) ) global_x = global_x - mySIZE.cx/2;
		else global_x = myoutputRECT.left + (myoutputRECT.right-myoutputRECT.left)/2 - mySIZE.cx/2;
		didjustify=true;
	}
	if(pFILE && !global_horizontaljustification.empty()) 
	{
		fprintf(pFILE, "\nafter horizontal justification\n");
		fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n", global_x, global_y, mySIZE.cx, mySIZE.cy);
		fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
	}
	//vertical justification
	if(pFILE && !global_verticaljustification.empty()) 
	{
		fprintf(pFILE, "\nbefore vertical justification\n");
		fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n", global_x, global_y, mySIZE.cx, mySIZE.cy);
		fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
	}
	if(global_verticaljustification=="top")
	{
	}
	else if(global_verticaljustification=="bottom")
	{
		if( (globalxyabsolute!=0) && (global_fullmonitorssurface==0) ) global_y = global_y - mySIZE.cy;
		else global_y = myoutputRECT.bottom - mySIZE.cy;
		didjustify=true;
	}
	else if(global_verticaljustification=="center")
	{
		if( (globalxyabsolute!=0) && (global_fullmonitorssurface==0) ) global_y = global_y - mySIZE.cy/2;
		else global_y = myoutputRECT.top + (myoutputRECT.bottom-myoutputRECT.top)/2 - mySIZE.cy/2;
		didjustify=true;
	}
	if(pFILE && !global_verticaljustification.empty()) 
	{
		fprintf(pFILE, "\nafter vertical justification\n");
		fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n", global_x, global_y, mySIZE.cx, mySIZE.cy);
		fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
	}
	return didjustify;
}

//wnd size is determined by fontheight and text string extent
SIZE CreateFontAndComputeWndPosAndSize(HWND hWnd)
{
	//2023august07, spi, begin
	if(global_hFont) DeleteObject(global_hFont);
	//2023august07, spi, end

	//global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
	//global_hFont=CreateFontW(global_fontheight,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,L"Segoe Script");
	//global_hFont=CreateFontA(global_fontheight,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,global_fontface.c_str());
	global_hFont=CreateFontA(global_fontheight,0,0,0,FW_BOLD,0,0,0,0,0,0,PROOF_QUALITY,0,global_fontface.c_str());
	

	SIZE mySIZE;
	HDC myHDC = GetDC(hWnd);
	HGDIOBJ prevHGDIOBJ = SelectObject(myHDC, global_hFont);

	if(global_textmode < 0)
	{
		//GetTextExtentPoint32A(myHDC, "88:88:88", strlen("88:88:88"), &mySIZE);
		GetTextExtentPoint32A(myHDC, global_spitextstring.c_str(), global_spitextstring.length(), &mySIZE);
	}
	else
	{
		//if global_spitextstring specifies a counter mode and a counter format, fetch it properly
		global_spitextstring = "88:88:88";
		if(global_textformat<0) global_spitextstring = "88:88:88";
		else if(global_textformat==global_counterformatHHMMSS) global_spitextstring = "88:88:88";
		else if(global_textformat==global_counterformatHHMM) global_spitextstring = "88:88";
		else if(global_textformat==global_counterformatMMSS) global_spitextstring = "88:88";
		else if(global_textformat==global_counterformatHH) global_spitextstring = "88";
		else if(global_textformat==global_counterformatMM) global_spitextstring = "88";
		else if(global_textformat==global_counterformatSS) global_spitextstring = "88";
		GetTextExtentPoint32A(myHDC, global_spitextstring.c_str(), global_spitextstring.length(), &mySIZE);
	}

	if(globalxyabsolute==false)
	{
		//relative coordinates
		global_x = myoutputRECT.left + global_x;
		global_y = myoutputRECT.top + global_y;
		if(pFILE) 
		{
			if(global_x > myoutputRECT.right) fprintf(pFILE, "\nwarning, global_x is considered relative and exceeds targeted monitor or window rect\n");
			if(global_y > myoutputRECT.bottom) fprintf(pFILE, "\nwarning, global_y is considered relative and exceeds targeted monitor or window rect\n");			
		}
	}
	else
	{
		//absolute coordinates
		myoutputPOINT.x = global_x;
		myoutputPOINT.y = global_y;
		if(global_fullmonitorssurface)
		{
			bool found=false;
			found = PtInRect(&(_monitors.rcCombined),myoutputPOINT);
			if(!found)
			{
				assert(false);
				if(pFILE) 
				{
					fprintf(pFILE, "\nwarning, global_x and global_y are considered absolute but do not point inside the combined rect of all monitors\n");
				}
			}
		}
		else
		{
			bool found=false;
			for (int i = 0; i < _monitors.rcMonitors.size(); ++i)
			{
				if (PtInRect(&(_monitors.rcMonitors[i]),myoutputPOINT))
				{
					found=true;
					break;
				}
			}
			if(!found)
			{
				assert(false);
				if(pFILE) 
				{
					fprintf(pFILE, "\nwarning, global_x and global_y are considered absolute but do not point inside any monitor rects\n");
				}
			}
		}		
	}

	if(!global_horizontaljustification.empty() || !global_verticaljustification.empty())
	{
		justification(mySIZE);
	}
	//if(!globalxyabsolute && (global_horizontalforcefit || global_verticalforcefit) && (global_spitextstring.length()>0) && (mySIZE.cx>0))
	if( (!globalxyabsolute) && ((global_horizontalforcefit>0.0f) || (global_verticalforcefit>0.0f)) && (global_spitextstring.length()>0) && (mySIZE.cx>0))
	{
		if(pFILE) 
		{
			fprintf(pFILE, "\n");
			if((global_horizontalforcefit>0.0f)) fprintf(pFILE, "before horizontal force fit\n");
			if((global_verticalforcefit>0.0f)) fprintf(pFILE, "before vertical force fit\n");
			fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n", global_x, global_y, mySIZE.cx, mySIZE.cy);
			fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
		}
		POINT topleftPOINT;
		topleftPOINT.x = global_x;
		topleftPOINT.y = global_y;
		POINT bottomrightPOINT;
		bottomrightPOINT.x = global_x + mySIZE.cx;
		bottomrightPOINT.y = global_y + mySIZE.cy;
		if (!PtInRect(&myoutputRECT,topleftPOINT) || !PtInRect(&myoutputRECT,bottomrightPOINT))
		{
			//if(global_horizontalforcefit)
			if(global_horizontalforcefit>0.0f)
			{
				if(global_x<myoutputRECT.left) global_x = myoutputRECT.left;
				if(global_x>myoutputRECT.right) global_x = myoutputRECT.right;
			}
			//if(global_verticalforcefit)
			if(global_verticalforcefit>0.0f)
			{
				if(global_y<myoutputRECT.top) global_y = myoutputRECT.top;
				if(global_y>myoutputRECT.bottom) global_y = myoutputRECT.bottom;
			}

			int maxwidth = myoutputRECT.right - myoutputRECT.left;
			assert(maxwidth>0);
			int maxheight = myoutputRECT.bottom - myoutputRECT.top;
			assert(maxheight>0);

			//2020sept23, spi, begin
			//global_horizontalforcefit and global_verticalforcefit can now be between 0.0 and 1.0, specifying a percentage (a scale) when greater than 0
			int newmaxwidth = maxwidth;
			int newmaxheight = maxheight;
			if(global_horizontalforcefit>0.0f)
			{
				newmaxwidth = maxwidth * global_horizontalforcefit;
				if(global_x<(myoutputRECT.left+(maxwidth-newmaxwidth)/2)) global_x = myoutputRECT.left+(maxwidth-newmaxwidth)/2;
				if(global_x>(myoutputRECT.right-(maxwidth-newmaxwidth)/2)) global_x = myoutputRECT.right-(maxwidth-newmaxwidth)/2;
			}
			//if(global_verticalforcefit)
			if(global_verticalforcefit>0.0f)
			{
				newmaxheight = maxheight * global_verticalforcefit;
				if(global_y<(myoutputRECT.top+(maxheight-newmaxheight)/2)) global_y = myoutputRECT.top+(maxheight-newmaxheight)/2;
				if(global_y>(myoutputRECT.bottom-(maxheight-newmaxheight)/2)) global_y = myoutputRECT.bottom-(maxheight-newmaxheight)/2;
			}
			//2020sept23, spi, end

			//regle de 3
			//global_fontheight -> mySIZE.cx/global_spitextstring.length()
			//newfontheight -> maxwidth/global_spitextstring.length()
			//int newfontheight = maxwidth * global_fontheight / mySIZE.cx;
			int newfontheight = newmaxwidth * global_fontheight / mySIZE.cx;
			//if( (global_verticalforcefit>0.0f) && newfontheight>maxheight) newfontheight = maxheight;
			//if( (global_verticalforcefit>0.0f) && newfontheight>maxheight) newfontheight = maxheight*global_verticalforcefit;
			if( (global_verticalforcefit>0.0f) && newfontheight>newmaxheight) newfontheight = maxheight;
			//create new font using newfontheight
			HFONT newhfont = CreateFontA(newfontheight,0,0,0,FW_BOLD,0,0,0,0,0,0,PROOF_QUALITY,0,global_fontface.c_str());
			if(newhfont)
			{
				//deselect current font
				SelectObject(myHDC, prevHGDIOBJ);
				//delete current font
				DeleteObject(global_hFont);
				//select new font
				global_hFont = newhfont;
				global_fontheight = newfontheight;
				prevHGDIOBJ = SelectObject(myHDC, global_hFont);
				//recompute new string extent size
				GetTextExtentPoint32A(myHDC, global_spitextstring.c_str(), global_spitextstring.length(), &mySIZE);
				//if enabled, justify once again
				if(!global_horizontaljustification.empty() || !global_verticaljustification.empty())
				{
					justification(mySIZE);
				}
			}
			else
			{
				if(pFILE) 
				{
					fprintf(pFILE, "\nwarning, create font of new size %d did not succeed\n", newfontheight);
					fprintf(pFILE, "warning, force fit may not be successful\n");
				}
			}
		}

		if(pFILE) 
		{
			fprintf(pFILE, "\n");
			if((global_horizontalforcefit>0.0f)) fprintf(pFILE, "after horizontal force fit\n");
			if((global_verticalforcefit>0.0f)) fprintf(pFILE, "after vertical force fit\n");
			fprintf(pFILE, "global_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n",global_x, global_y, mySIZE.cx, mySIZE.cy);
			fprintf(pFILE, "global_fontheight is %d\n", global_fontheight);
		}
	}

	if(pFILE) 
	{
		fprintf(pFILE, "\ndisplaying string %s at:\n", global_spitextstring.c_str());
		fprintf(pFILE, "\nglobal_x, global_y, mySIZE.cx, mySIZE.cy are %d %d %d %d\n",global_x, global_y, mySIZE.cx, mySIZE.cy);
		fflush(pFILE);
	}
	//2023august07, spi, begin
	//SetWindowPos(hWnd, NULL, global_x, global_y, mySIZE.cx, mySIZE.cy, SWP_NOZORDER);
	SelectObject(myHDC, prevHGDIOBJ);
	ReleaseDC(hWnd, myHDC);
	return mySIZE;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	/*
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	*/
	DWORD Flags1 = WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_TRANSPARENT;
	DWORD Flags2 = WS_POPUP;

	//hWnd = CreateWindowEx(Flags1, szWindowClass, szTitle, Flags2, global_x, global_y, 1920, 1200, 0, 0, hInstance, 0);
	hWnd = CreateWindowEx(Flags1, szWindowClass, szTitle, Flags2, 0, 0, 100, 100, 0, 0, hInstance, 0);
	//hWnd = CreateWindowEx(Flags1, utf8_decode(global_classname).c_str(), szTitle, Flags2, 0, 0, 100, 100, 0, 0, hInstance, 0);
	if (!hWnd)
	{
		return FALSE;
	}

	SIZE mySIZE = CreateFontAndComputeWndPosAndSize(hWnd);
	SetWindowPos(hWnd, NULL, global_x, global_y, mySIZE.cx, mySIZE.cy, SWP_NOZORDER);
	//2023august07, spi, end
	
	//HRGN GGG = CreateRectRgn(0, 0, 1920, 1200);
	//InvertRgn(GetDC(hWnd), GGG);
	//SetWindowRgn(hWnd, GGG, false);
	
	//COLORREF RRR = RGB(255, 0, 255);
	//COLORREF global_keyingcolor = RGB(255, 0, 255);

	//2020sept22, spi, begin
	//SetLayeredWindowAttributes(hWnd, global_keyingcolor, (BYTE)0, LWA_COLORKEY);
	//SetLayeredWindowAttributes(hWnd, global_keyingcolor, (BYTE)global_alpha, LWA_COLORKEY);
	SetLayeredWindowAttributes(hWnd, global_keyingcolor, (BYTE)global_alpha, LWA_COLORKEY|LWA_ALPHA);
	if(pFILE)
	{
		fprintf(pFILE, "\n\nSetLayeredWindowAttributes() with global_alpha %d\n", global_alpha);
	}
	//2020sept22, spi, end
	/*
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);
	*/
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	/*
	DeleteObject(GGG);
	*/

	//SetTimer(hWnd, 1, 1000, NULL);

	//moved into InitInstance()
	if( (global_textmode>-1) 
		|| global_randomcharacter==true 
		|| global_randomposition==true
		|| global_randomheight==true
		|| global_randomfont==true) //|| ( (global_starttime_sec>=0)||(global_endtime_sec>=0) ) )
	{
		//when in counter modes
		//set timer to update display or to keep track of start time and kill time
		//2023august07, spi, begin
		int period_miliseconds = 1000; 
		if(global_bpm!=60 && global_bpm>0 && (global_textmode<0)) period_miliseconds = 1000 * 60 / global_bpm;
		//SetTimer(hWnd, ID_TIMER_EVERYSECOND, 1000, NULL); //every seconds timer
		SetTimer(hWnd, ID_TIMER_EVERYSECOND, period_miliseconds, NULL); //every seconds timer
		//2023august07, spi, end
	}
	if( (global_textmode<0) && (global_starttime_sec>0) )
	{
		//todo, launch a timer to wait for global_starttime_sec before display;
		//set timer to start to display
		SetTimer(hWnd, ID_TIMER_ONCESTART, global_starttime_sec*1000, NULL); 
		if(pFILE)
		{
			fprintf(pFILE, "SetTimer() ID_TIMER_ONCESTART with global_starttime_sec %d\n", global_starttime_sec);
		}
	}
	if( (global_textmode<0) && (global_endtime_sec>0) && ((global_endtime_sec-global_starttime_sec)>0) )
	{
		//todo, launch a timer to wait for global_timetodisplay_sec before killing app;
		//set timer to kill display
		//SetTimer(hWnd, ID_TIMER_ONCEKILL, (global_endtime_sec-global_starttime_sec)*1000, NULL); 
		SetTimer(hWnd, ID_TIMER_ONCEKILL, global_endtime_sec*1000, NULL); 
		if(pFILE)
		{
			fprintf(pFILE, "SetTimer() ID_TIMER_ONCEKILL with global_endtime_sec %d\n", global_endtime_sec);
		}
	}

	return TRUE;
}

void DrawTextXOR(HDC hdc, const char* charbuffer, int charbufferlength)
{

	//2021nov23, spi, begin
	if(charbuffer==NULL || charbufferlength<1) return;
	//2021nov23, spi, end

	HDC myMemHDC = CreateCompatibleDC(hdc);
	HFONT hOldFont_memhdc=(HFONT)SelectObject(myMemHDC,global_hFont);
	SIZE mySIZE2;
	GetTextExtentPoint32A(myMemHDC, charbuffer, charbufferlength, &mySIZE2);

	HBITMAP myHBITMAP = CreateCompatibleBitmap(hdc, mySIZE2.cx, mySIZE2.cy);
	HGDIOBJ prevHBITMAP = SelectObject(myMemHDC, myHBITMAP);
	//COLORREF crOldBkColor = SetBkColor(myMemHDC, RGB(0xFF, 0xFF, 0xFF));
	//COLORREF crOldBkColor = SetBkColor(myMemHDC, RGB(0x00, 0x00, 0xFF));
	//COLORREF crOldBkColor = SetBkColor(myMemHDC, RGB(0x00, 0x00, 0x00));
	//COLORREF crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0xFF, 0xFF, 0xFF)); //not visible
	COLORREF crOldTextColor_memhdc;
	if(global_idfontcolor==0)
	{
		crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0xFF, 0x00, 0xFF)); //white
	}
	else if(global_idfontcolor==1)
	{
		crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0x00, 0xFF, 0xFF)); //blue
	}
	else if(global_idfontcolor==2)
	{
		crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0xFF, 0xFF, 0x00)); //red
	}
	else if(global_idfontcolor==3)
	{
		crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0x00, 0xFF, 0x00)); //black
	}
	else if(global_idfontcolor==4)
	{
		crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0xFF, 0x00, 0x00)); //yellow
	}
	else if(global_idfontcolor==5)
	{
		crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0xA0, 0x00, 0x20)); //green lime
	}
	else if(global_idfontcolor==6)
	{
		crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0x00, 0x00, 0x00)); //green
	}

	//int nOldDrawingMode_memhdc = SetROP2(myMemHDC, R2_NOTXORPEN); //XOR mode, always have to erase what's drawn.
	//int iOldBkMode_memhdc = SetBkMode(myMemHDC, TRANSPARENT);
	//HFONT hOldFont_memhdc=(HFONT)SelectObject(myMemHDC,global_hFont);
	//TextOutA(myMemHDC, 1, 1, "test string", 11);
	TextOutA(myMemHDC, 0, 0, charbuffer, charbufferlength);
	strcpy(charbuffer_prev, charbuffer);
	//Rectangle(myMemHDC, 0, 0, 1000, 800);
	//BitBlt(hdc, 0, 0, 1000, 800, myMemHDC, 0, 0, SRCCOPY); 
	BitBlt(hdc, 0, 0, mySIZE2.cx, mySIZE2.cy, myMemHDC, 0, 0, 0x00990066); //XOR mode, always have to erase what's drawn.
	//BitBlt(hdc, global_x, global_y, mySIZE2.cx, mySIZE2.cy, myMemHDC, 0, 0, 0x00990066); //XOR mode, always have to erase what's drawn.
	SelectObject(myMemHDC, prevHBITMAP);
	DeleteDC(myMemHDC);
	DeleteObject(myHBITMAP);
	//DeleteDC(myMemHDC2);
	return;		
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		
    case WM_ERASEBKGND:
		{
			
			RECT rect;
			GetClientRect(hWnd, &rect);
			//FillRect((HDC)wParam, &rect, CreateSolidBrush(RGB(0, 0, 0)));
			//FillRect((HDC)wParam, &rect, CreateSolidBrush(RGB(255, 0, 255))); //keying color
			FillRect((HDC)wParam, &rect, CreateSolidBrush(global_keyingcolor)); //keying color
			
		}
		break;
		
	case WM_TIMER:
		switch (wParam)
		{
		case ID_TIMER_EVERYSECOND:
		case ID_TIMER_ONCESTART:
		case ID_TIMER_ONCEKILL:

			///////////////////
			//update time stamp
			///////////////////
			global_nowstamp_ms = GetTickCount();

			////////////////
			//for all modes
			///////////////
			//2023august07, spi, begin
			if(wParam==ID_TIMER_EVERYSECOND) //every beat with bpm different than 60
			{
				if(global_randomcharacter==true 
					|| global_randomposition==true
					|| global_randomheight==true
					|| global_randomfont==true)
				{
					//InvalidateRect(hWnd, NULL, FALSE);
					//UpdateWindow(hWnd);
					//0) change text, position, fontsize					
					if(global_randomcharacter==true)
					{
						//if (global_spitextstring =="12345678") global_spitextstring = "87654321";
						//  else global_spitextstring = "12345678";
						//if (global_spitextstring!=SPITEXT_DEFAULTSTRING) global_spitextstring = SPITEXT_DEFAULTSTRING;
						if (global_spitextstring.substr(0,1)!=SPITEXT_DEFAULTSTRING) 
						{
							int maxi=global_spitextstring.length();
							global_spitextstring = "";
							for(int i=0; i<maxi;i++)
							{
								global_spitextstring += SPITEXT_DEFAULTSTRING;
							}
						}
						else 
						{
							if(global_words==true)
							{
								if(numwords>1) global_spitextstring = words[RandomInt(0,numwords-1)];
							}
							else
							{
								global_spitextstring = "";
								for(int i=0; i<global_spitextstring_initiallength; i++)
								{
									//global_spitextstring += RandomInt(48,90); //0-9-A-Z
									global_spitextstring += RandomInt(48,255); //do not allow one ascii 32
								}
							}
						}	
					}
					if(global_randomheight==true)
					{
						//if (global_spitextstring =="12345678") global_spitextstring = "87654321";
						//  else global_spitextstring = "12345678";
						//int myint = RandomInt(12,2160); //12 point height to full 4k screen height
						int myint = RandomInt(24,1080); //24 point height to full 2k screen height
						//int myint = RandomInt(80,180); //small point height to larger height
						global_fontheight = myint;// + '\0';
					}
					if(global_randomfont==true)
					{
						if(global_colors==true) 
						{
							//global_idfontcolor = RandomInt(0,6); //all avail colors
							//global_idfontcolor = RandomInt(1,2); //blue and red only
							global_idfontcolor = RandomInt(4,6); //yellow, lime and green
							//global_idfontcolor = RandomInt(2,4); //red and yellow only
						}
						
						if(1)
						{
							if(numfontfaces>1) global_fontface = fontfaces[RandomInt(0,numfontfaces-1)];
						}
					}
					//1) find new position and size
					SIZE mySIZE = CreateFontAndComputeWndPosAndSize(hWnd);
					//1.5)
					
					if(global_randomposition==true)
					{
						
						global_x = RandomInt(myoutputRECT.left, myoutputRECT.right);
						global_y = RandomInt(myoutputRECT.top, myoutputRECT.bottom);
						/*
						global_x = RandomInt(myoutputRECT.left, myoutputRECT.right-mySIZE.cx);
						global_y = RandomInt(myoutputRECT.top, myoutputRECT.bottom-mySIZE.cy);
						*/
					}
					
					//2) move hwnd
					SetWindowPos(hWnd, NULL, global_x, global_y, mySIZE.cx, mySIZE.cy, SWP_NOZORDER);//|SWP_NOREDRAW);
					//InvalidateRect(hWnd, NULL, FALSE);
					//UpdateWindow(hWnd);
				}
			}
			//2023august07, spi, end
			//1) check for start condition
			if(wParam==ID_TIMER_ONCESTART)
			{
				//InvalidateRect(hWnd, NULL, FALSE);
				KillTimer(hWnd, ID_TIMER_ONCESTART);
			}
			//2) check for end condition
			else if(wParam==ID_TIMER_ONCEKILL)
			{
				int nShowCmd = false;
				//ShellExecuteA(NULL, "open", "c:\\temp\\batch.bat", "", NULL, nShowCmd);
				//ShellExecuteA(NULL, "open", "finishing.ahk", "", NULL, nShowCmd);
				if(!global_finishing.empty()) ShellExecuteA(NULL, "open", global_finishing.c_str(), "", NULL, nShowCmd);
				KillTimer(hWnd, ID_TIMER_ONCEKILL);
				PostMessage(hWnd, WM_DESTROY, 0, 0);
			}

			////////////////////////
			//text string mode only
			///////////////////////
			if(global_textmode<0 && ((wParam==ID_TIMER_ONCESTART)||(wParam==ID_TIMER_ONCEKILL)) )
			{
				//nothing to do in here every second
				//only redraw when start and kill
				InvalidateRect(hWnd, NULL, FALSE);
			}

			////////////////////
			//counter modes only
			////////////////////
			if(global_textmode==global_countermodeCLOCK)
			{
				InvalidateRect(hWnd, NULL, FALSE);
			}
			else if(global_textmode==global_countermodeCOUNTUP)
			{
				//1) calculate time to display
				if(global_starttime_sec<0) global_starttime_sec=0;
				int elapsed_sec = (global_nowstamp_ms-global_startstamp_ms)/1000;
				global_timetodisplay_sec = global_starttime_sec + elapsed_sec;
				InvalidateRect(hWnd, NULL, FALSE);

				//2) check for end condition
				if(global_endtime_sec>-1 && ((global_endtime_sec-global_starttime_sec)-elapsed_sec)<1)
				{
					int nShowCmd = false;
					//ShellExecuteA(NULL, "open", "c:\\temp\\batch.bat", "", NULL, nShowCmd);
					//ShellExecuteA(NULL, "open", "finishing.ahk", "", NULL, nShowCmd);
					if(!global_finishing.empty()) ShellExecuteA(NULL, "open", global_finishing.c_str(), "", NULL, nShowCmd);
					KillTimer(hWnd, ID_TIMER_ONCEKILL);
					PostMessage(hWnd, WM_DESTROY, 0, 0);
				}
			}
			else if(global_textmode==global_countermodeCOUNTDOWN)
			{
				//calculate time to display
				if(global_starttime_sec<0) global_starttime_sec=0;
				int elapsed_sec = (global_nowstamp_ms-global_startstamp_ms)/1000;
				global_timetodisplay_sec = global_starttime_sec - elapsed_sec;
				InvalidateRect(hWnd, NULL, FALSE);

				//2) check for end condition
				if(global_endtime_sec>-1 && ((global_starttime_sec-global_endtime_sec)-elapsed_sec)<1) //original
				{
					int nShowCmd = false;
					//ShellExecuteA(NULL, "open", "c:\\temp\\batch.bat", "", NULL, nShowCmd);
					//ShellExecuteA(NULL, "open", "finishing.ahk", "", NULL, nShowCmd);
					if(!global_finishing.empty()) ShellExecuteA(NULL, "open", global_finishing.c_str(), "", NULL, nShowCmd);
					KillTimer(hWnd, ID_TIMER_ONCEKILL);
					PostMessage(hWnd, WM_DESTROY, 0, 0);
				}
			}
			else
			{
				assert(false);
			}
			return 0;
		}
		break;
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);

			//SaveDC(hdc);
			//int nOldDrawingMode = SetROP2(hdc, R2_NOTXORPEN); //XOR mode, always have to erase what's drawn.

			//int iOldBkMode = SetBkMode(hdc, TRANSPARENT);
			COLORREF crOldTextColor = SetTextColor(hdc, RGB(0xFF, 0x00, 0x00));
			HGDIOBJ hOldFont=(HFONT)SelectObject(hdc,global_hFont);
			
			
			if(global_textmode<0)
			{
				global_nowstamp_ms = GetTickCount();
				int elapsed_sec = (global_nowstamp_ms-global_startstamp_ms)/1000;
				if(global_starttime_sec>0 && (global_starttime_sec-elapsed_sec)>0)
				{
					//string text mode
					//sprintf(charbuffer, "%s", "delayed"); //empty display
					sprintf(charbuffer, "%s", ""); //empty display
				}
				else
				{
					//string text mode
					sprintf(charbuffer, "%s", global_spitextstring.c_str());
				}
			}
			else
			{
				SYSTEMTIME st;
				//GetSystemTime(&st);
				GetLocalTime(&st);
				int hh = st.wHour;
				int mm = st.wMinute;
				int ss = st.wSecond;
				if(global_textmode==global_countermodeCLOCK)
				{
					//sprintf(charbuffer, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
					//sprintf(charbuffer, "%02d.%02d:%02d", st.wHour, st.wMinute, st.wSecond);
					//sprintf(charbuffer, mytimeformat.c_str(), st.wHour, st.wMinute, st.wSecond);
				}
				else if(global_textmode==global_countermodeCOUNTUP)
				{
					hh = global_timetodisplay_sec / 3600;
					mm = (global_timetodisplay_sec % 3600) / 60;
					ss = global_timetodisplay_sec % 60;
					//sprintf(charbuffer, "%02d:%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, "%02d.%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, mytimeformat.c_str(), hh, mm, ss);
				}
				else if(global_textmode==global_countermodeCOUNTDOWN)
				{
					hh = global_timetodisplay_sec / 3600;
					mm = (global_timetodisplay_sec % 3600) / 60;
					ss = global_timetodisplay_sec % 60;
					//sprintf(charbuffer, "%02d:%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, "%02d.%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, mytimeformat.c_str(), hh, mm, ss);
					
				}
				else
				{
					//ERROR
					assert(false);
					hh=99;
					mm=99;
					ss=99;
					//sprintf(charbuffer, "%02d:%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, "%02d.%02d:%02d", hh, mm, ss);
					//sprintf(charbuffer, mytimeformat.c_str(), hh, mm, ss);
				}
				string mytimeformat = "%02d.%02d:%02d";
				if(global_textformat==global_counterformatHHMMSS)
				{
					mytimeformat = "%02d.%02d:%02d";
					sprintf(charbuffer, mytimeformat.c_str(), hh, mm, ss);
				}
				else if(global_textformat==global_counterformatHHMM)
				{
					mytimeformat = "%02d.%02d";
					sprintf(charbuffer, mytimeformat.c_str(), hh, mm);
				}
				else if(global_textformat==global_counterformatMMSS)
				{
					mytimeformat = "%02d:%02d";
					sprintf(charbuffer, mytimeformat.c_str(), mm, ss);
				}
				else if(global_textformat==global_counterformatHH ||
						global_textformat==global_counterformatMM ||
						global_textformat==global_counterformatSS)
				{
					mytimeformat = "%02d";
					if(global_textformat==global_counterformatHH) sprintf(charbuffer, mytimeformat.c_str(), hh);
					else if(global_textformat==global_counterformatMM) sprintf(charbuffer, mytimeformat.c_str(), mm);
					else if(global_textformat==global_counterformatSS) sprintf(charbuffer, mytimeformat.c_str(), ss);
				}

			}

			//TextOutA(hdc, 50, 50, charbuffer, charbufferlength);
			int charbufferlength = strlen(charbuffer);
			//2023august07, spi, begin
			//if(strcmp(charbuffer_prev, "")) DrawTextXOR(hdc, charbuffer_prev, strlen(charbuffer));
			//if(strcmp(charbuffer_prev, "")) DrawTextXOR(hdc, charbuffer_prev, strlen(charbuffer_prev)); //when charbuffer_prev not empty, erase before drawing
			if(strcmp(charbuffer_prev, "")!=0) DrawTextXOR(hdc, charbuffer_prev, strlen(charbuffer_prev)); //when charbuffer_prev not empty, erase before drawing
			//2023august07, spi, end
			DrawTextXOR(hdc, charbuffer, charbufferlength);
			
			SetTextColor(hdc, crOldTextColor);
			//SetBkMode(hdc, iOldBkMode);
			SelectObject(hdc,hOldFont);

			//SetROP2(hdc, nOldDrawingMode); 
			//RestoreDC(hdc, -1);

			EndPaint(hWnd, &ps);
		}
		break;
	//2021nov23, spi, begin
	case WM_SETTEXT:
			////////////////////////
			//text string mode only
			///////////////////////
			if(global_textmode<0)
			{
				
				//erase previous text
				//global_spitextstring = "";
				//InvalidateRect(hWnd, NULL, FALSE);
				//Sleep(100);

				//get new text string
				WCHAR myWCHAR[1024];
				WCHAR myCHAR[1024];
				if(lParam!=NULL)
				{
					//wsprintf(myWCHAR, L"%s", (WCHAR*)lParam);
					//wcscpy(myWCHAR, (WCHAR*)lParam);
					wstring mywstring((WCHAR*)lParam); //careful lParam is a non-null terminated string
					string mystring((CHAR*)lParam); //careful lParam is a non-null terminated string
					//global_spitextstring = utf8_encode(mywstring);
					global_spitextstring = utf8_encode(mywstring);
					if (global_spitextstring.empty()) global_spitextstring = SPITEXT_DEFAULTSTRING;
					//redraw
					//InvalidateRect(hWnd, NULL, FALSE);
					InvalidateRect(hWnd, NULL, TRUE);
				}
			}
		break;
	//2021nov23, spi, end
	case WM_DESTROY:
		DeleteObject(global_hFont);
		KillTimer(hWnd, ID_TIMER_EVERYSECOND);
		KillTimer(hWnd, ID_TIMER_ONCESTART);
		KillTimer(hWnd, ID_TIMER_ONCEKILL);
		if(!global_end.empty()) ShellExecuteA(NULL, "open", global_end.c_str(), "", NULL, 0);
		if(pFILE) fclose(pFILE);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
