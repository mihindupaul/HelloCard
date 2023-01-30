/******************************************************************************************/
/*																						  */
/*	Source File		: HelloCard.c														  */
/*																						  */	
/*	Version			: 2.0																  */
/*																						  */
/*	Target OS		: Windows 2000	Service	Pack 3.0									  */
/*																						  */
/*	Description		: WINMAIN/LISTVIEW/WINDOW MESSAGE HANDLER							  */
/*																						  */
/*	Last Revision	: <29/19/2002>														  */
/*																						  */
/*	Release	Stage	: BETA																  */
/*																						  */
/*	Author			: A.T.Lloyd	(c)	2002,  All rights reserved.							  */
/*					  Tritel Technologies (Pvt.) Ltd.									  */
/******************************************************************************************/


/*********************	Header Files  *********************/
#include "stdafx.h"
#include <iostream>

#include <windows.h>
#include <process.h> 
#include <windowsx.h>
#include <commctrl.h>

#include "log4cplus/logger.h"
#include "log4cplus/fileappender.h"

#include "HelloCard.h"
#include "resource.h"
#include "PhoneLine.h"
#include "ss7line.h"
#include "tritelivr.h"
#include "configuration.h"
#include "tritelcallconnector.h"
#include "timer.h"

/*******************  Constants	Definitions	********************/
#define	ID_LISTVIEW	 20000

CLineManager	g_LineManager;
HWND hWnd;
HWND hToolbar;
/*********************	WinMain	 **************************/
HWND CreateMainWindowToolbar(HWND hParent);
int	WINAPI WinMain(	HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpszCmdLine,	 int nCmdShow )
{
	WNDCLASS wc;
	MSG	msg;


	if(	!hPrevInstance )
	{
		wc.lpszClassName = "HelloCardAppClass";
		wc.lpfnWndProc = MainWndProc;
		wc.style = CS_OWNDC	| CS_VREDRAW | CS_HREDRAW;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon	(hInstance,	MAKEINTRESOURCE(IDI_PHONE2));//LoadIcon( NULL, IDI_APPLICATION );
		wc.hCursor = LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground = (HBRUSH)( COLOR_WINDOW+1	);
		wc.lpszMenuName	= MAKEINTRESOURCE(IDR_HELLOMENU);
		wc.cbClsExtra =	0;
		wc.cbWndExtra =	0;

		RegisterClass( &wc );
	}

	ghInstance = hInstance;

	hWnd = CreateWindow( "HelloCardAppClass",
		"State Transaction Manager v4.4.3",
		WS_OVERLAPPEDWINDOW|WS_MAXIMIZE|WS_VSCROLL,
		0,
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
		);
	hToolbar = CreateMainWindowToolbar(hWnd);
	//	Start Logging Lib
	InitDefaultLog();
	LOG4CPLUS_INFO(log4cplus::Logger::getInstance("default"),"# System Start #");

	ShowWindow(	hWnd, nCmdShow );
	UpdateWindow(hWnd);

	if(init_timers() != 0)
	{
		LOG4CPLUS_INFO(log4cplus::Logger::getInstance("default"),"Timer system Inactive");
	}

	g_LineManager.Initialize();

	while( GetMessage( &msg, NULL, 0, 0	) )	
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	g_LineManager.Finalize();
	close_timers();
	LOG4CPLUS_INFO(log4cplus::Logger::getInstance("default"),"# System	Halt #");

	//	Clean all logging functions
	log4cplus::Logger::shutdown();
	CConfiguration::Cleanup();
	return msg.wParam;
}

/*********************	MainWndProc	***********************/

LRESULT	CALLBACK MainWndProc( HWND hWnd, UINT uMsg,	WPARAM wParam,LPARAM lParam	)
{

	switch(	uMsg )
	{

		HANDLE_MSG(hWnd,WM_COMMAND,WM_OnCommand);

	case WM_CREATE:

		//Create Viewlist and Set The Style	As Extended	Style
		hwndListView = CreateListView(ghInstance, hWnd);
		InitListView(hwndListView);
		ListView_SetExtendedListViewStyle(hwndListView,LVS_EX_GRIDLINES);
		EnableMenuItem(GetMenu(hWnd), ID_ACTION_STOP, MF_DISABLED |	MF_GRAYED);
		break;

	case WM_SIZE:

		//Resize The View List
		
		SendMessage(hToolbar,uMsg,wParam,lParam);
		ResizeListView(hwndListView, hWnd);
		
		break;


	case WM_NOTIFY:

		//View List	Notify Event Handler
		return ListViewNotify(hWnd,	lParam);

		break;

	case WM_CLOSE:

		if(g_LineManager.IsRunning())
		{
			MessageBox(hWnd,"Stop the service befor exit","Exit",MB_OK|MB_ICONEXCLAMATION);
		}
		else
		{
			DestroyWindow(hWnd);
		}

		break;

	case WM_DESTROY:
		
		PostQuitMessage( 0 );

		break;

	default:

		return(	DefWindowProc( hWnd, uMsg, wParam, lParam ));
	}

	return 0;
}

/*********************	WM_COMMAND ***********************/

void WM_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT	codeNotify)
{

	DWORD	nIEnd;
	HANDLE hThread=NULL; DWORD dwExitCode =	0;
	ATL::CRegKey rk;
	switch (id)
	{

	case ID_ACTION_START:

		/*****Initialize the Listview Items	by the number of the incomming Lines***/
		//Open Registry	Key
		if(rk.Open(HKEY_LOCAL_MACHINE,"Software\\Tritel\\Hello\\Board\\SS7",KEY_READ) == 0)
		{
			if(!rk.QueryDWORDValue("IEND",nIEnd))
			{
				//	Add list view item for all channels
				for(DWORD i=0;i<nIEnd;i++)
					ListviewInsertItem(hwndListView);				
			}
		}

		//Create Thread	For	Hanling	All	Dialogic Events
		if(g_LineManager.Start() == 0)
		{
			//Enable-Diable	Menu Options
			EnableMenuItem(GetMenu(hWnd), ID_ACTION_START, MF_DISABLED | MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), ID_ACTION_STOP, MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd), ID_CONFIGURATION_CHANNEL_SETTINS,	MF_DISABLED	| MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), ID_CONFIGURATION_DATABASE_SETTINS, MF_DISABLED | MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), ID_CONFIGURATION_SERVERSETTINGS, MF_DISABLED | MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), ID_ACTION_EXIT, MF_DISABLED |	MF_GRAYED);
		}
		break;

	case ID_ACTION_STOP:

		//Enable-Diable	Menu Options
		if(g_LineManager.Stop() == 0)
		{
			EnableMenuItem(GetMenu(hWnd), ID_ACTION_STOP, MF_DISABLED |	MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd), ID_ACTION_START, MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd), ID_ACTION_EXIT, MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd), ID_CONFIGURATION_CHANNEL_SETTINS,	MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd), ID_CONFIGURATION_DATABASE_SETTINS, MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd), ID_CONFIGURATION_SERVERSETTINGS, MF_ENABLED);

			//Delete All Items From	The	ListView
			ListviewDeleteAllItem();
		}
		break;

	case ID_ACTION_EXIT:

		PostQuitMessage(0);

		break;

	case ID_CONFIGURATION_SS7_CHANNEL_SETTINS:

		DialogBox(ghInstance, (LPCTSTR)IDD_SS7CHANNEL, hWnd, (DLGPROC)SS7ChannelDlgProc);
		break;

	case ID_CONFIGURATION_R2_CHANNEL_SETTINS:
		
		// DialogBox(ghInstance, (LPCTSTR)IDD_R2CHANNEL, hWnd,	(DLGPROC)R2ChannelDlgProc);
		break;

	case ID_CONFIGURATION_DATABASE_SETTINS:

		DialogBox(ghInstance, (LPCTSTR)IDD_DATASOURCE, hWnd, (DLGPROC)DataBaseDlgProc);
		break;

	case ID_CONFIGURATION_SERVERSETTINGS:

		DialogBox(ghInstance, (LPCTSTR)IDD_SERVER, hWnd, (DLGPROC)ServerDlgProc);
		break;


	case ID_CONFIGURATION_REMOVEBLOCK:

		g_LineManager.RemoveBlock();

		break;

	case ID_CONFIGURATION_CHANNEL_RESET:
		g_LineManager.AddUserEvent(31);
		DialogBox(ghInstance, (LPCTSTR)IDD_RESETCHANNEL, hWnd, (DLGPROC)ResetDlgProc);
		break;

	case ID_CONFIGURATION_CHANNEL_RESETALL:

		g_LineManager.ResetAllChannels();

		break;

	case ID_CONFIGURATION_CALLINGNUMBER:

		//DialogBox(ghInstance, (LPCTSTR)IDD_CALLINGNUMBER, hWnd,	(DLGPROC)CallingDlgProc);
		break;

	case ID_HELP_ABOUT:

		DialogBox(ghInstance, (LPCTSTR)IDD_ABOUT, hWnd,	(DLGPROC)AboutDlgProc);
		break;
	case ID_HELP_TEST:
//#ifdef DEBUG
		DoTest();
//#endif
		break;
	}

}

/*********************	AboutDlgProc ***********************/
LRESULT	CALLBACK AboutDlgProc( HWND	hDlg, UINT uMsg, WPARAM	wParam,	LPARAM lParam )
{
	HWND hwndStatic;

	switch(uMsg)
	{

	case WM_INITDIALOG:

		hwndStatic = GetDlgItem(hDlg, IDC_PRODUCT);
		SetWindowText(hwndStatic, "Hello Card");
		hwndStatic = GetDlgItem(hDlg, IDC_COMPANY);
		SetWindowText(hwndStatic, "Tritel Technologies (Pvt) Ltd");
		hwndStatic = GetDlgItem(hDlg, IDC_VERSION);
		SetWindowText(hwndStatic, "4.0");

		break;

	case WM_COMMAND:

		switch(LOWORD (wParam))
		{
		case IDOK:

			EndDialog(hDlg,	0);

			return TRUE;
		}

		break;

	case WM_CLOSE:

		EndDialog(hDlg,	0);

		break;



	}
	return FALSE;
}

/*********************	SS7ChannelDlgProc ***********************/

LRESULT	CALLBACK SS7ChannelDlgProc(	HWND hDlg, UINT	uMsg, WPARAM wParam, LPARAM	lParam )
{

	ATL::CRegKey rk;
	DWORD dwIStart=0,dwIEnd=0,dwOStart=0,dwOEnd=0;

	switch(uMsg)
	{

	case WM_INITDIALOG:

		if(rk.Create(HKEY_LOCAL_MACHINE,TEXT("Software\\Tritel\\Hello\\Board\\SS7")) == ERROR_SUCCESS)
		{
			rk.QueryDWORDValue("ISTART",dwIStart);
			rk.QueryDWORDValue("IEND",dwIEnd);
			rk.QueryDWORDValue("OSTART",dwOStart);
			rk.QueryDWORDValue("OEND",dwOEnd);

			SetDlgItemInt(hDlg,IDC_SS7_INCOMMING_START,dwIStart,FALSE);
			SetDlgItemInt(hDlg,IDC_SS7_INCOMMING_END,dwIEnd,FALSE);
			SetDlgItemInt(hDlg,IDC_SS7_OUTGOING_START,dwOStart,FALSE);
			SetDlgItemInt(hDlg,IDC_SS7_OUTGOING_END,dwOEnd,FALSE);
		}

		break;

	case WM_COMMAND:

		switch(LOWORD (wParam))
		{
		case ID_SAVE:

			//Get The Values From the User
			dwIStart=GetDlgItemInt(hDlg, IDC_SS7_INCOMMING_START, NULL,	FALSE);
			dwIEnd=GetDlgItemInt(hDlg, IDC_SS7_INCOMMING_END, NULL,	FALSE);
			dwOStart=GetDlgItemInt(hDlg, IDC_SS7_OUTGOING_START, NULL, FALSE);
			dwOEnd=GetDlgItemInt(hDlg, IDC_SS7_OUTGOING_END, NULL, FALSE);
			
			if(rk.Create(HKEY_LOCAL_MACHINE,TEXT("Software\\Tritel\\Hello\\Board\\SS7")) == ERROR_SUCCESS)
			{
				rk.SetDWORDValue("ISTART",dwIStart);
				rk.SetDWORDValue("IEND",dwIEnd);
				rk.SetDWORDValue("OSTART",dwOStart);
				rk.SetDWORDValue("OEND",dwOEnd);
			}

			EndDialog(hDlg,0);
			break;


		case ID_CANCEL:
			EndDialog(hDlg,	0);
			break;
		}

		break;

	case WM_CLOSE:

		EndDialog(hDlg,	0);

		break;

	}

	return FALSE;
}
/*********************	DataBaseDlgProc	***********************/

LRESULT	CALLBACK DataBaseDlgProc( HWND hDlg, UINT uMsg,	WPARAM wParam, LPARAM lParam )
{
	ATL::CRegKey rk;
	ULONG len;
	TCHAR tcDataSource[25]="",tcCataLog[25]="",tcUserId[25]="",tcPassWord[25]="";
	switch(uMsg)
	{

	case WM_INITDIALOG:
		if(rk.Create(HKEY_LOCAL_MACHINE,TEXT("Software\\Tritel\\Hello\\DataSource"))== ERROR_SUCCESS)
		{
			len = 25;
			rk.QueryStringValue("DataSource",tcDataSource,&len);
			SetDlgItemText(hDlg,IDC_DATASOURCE,tcDataSource);

			len = 25;
			rk.QueryStringValue("CataLog",tcCataLog,&len);
			SetDlgItemText(hDlg,IDC_CATALOG,tcCataLog);

			len = 25;
			rk.QueryStringValue("UserId",tcUserId,&len);
			SetDlgItemText(hDlg,IDC_USERID,tcUserId);

			len = 25;
			rk.QueryStringValue("PassWord",tcPassWord,&len);
			SetDlgItemText(hDlg,IDC_PASSWORD,tcPassWord);
		}

		break;

	case WM_COMMAND:

		switch(LOWORD (wParam))
		{
		case ID_SAVE:
			//Get The Values From the User
			GetDlgItemText(hDlg, IDC_DATASOURCE,tcDataSource, 25);
			GetDlgItemText(hDlg, IDC_CATALOG,tcCataLog,	25);
			GetDlgItemText(hDlg, IDC_USERID, tcUserId,25);
			GetDlgItemText(hDlg, IDC_PASSWORD,tcPassWord, 25);

			if(rk.Create(HKEY_LOCAL_MACHINE,TEXT("Software\\Tritel\\Hello\\DataSource"))== ERROR_SUCCESS)
			{
				rk.SetStringValue("DataSource",tcDataSource);
				rk.SetStringValue("CataLog",tcCataLog);
				rk.SetStringValue("UserId",tcUserId);
				rk.SetStringValue("PassWord",tcPassWord);
			}

			EndDialog(hDlg,0);
			break;


		case ID_CANCEL:
			EndDialog(hDlg,	0);
			break;
		}

		break;

	case WM_CLOSE:

		EndDialog(hDlg,	0);

		break;

	}
	return FALSE;
}

/*********************	ServerDlgProc ***********************/

LRESULT	WINAPI ServerDlgProc( HWND hDlg, UINT uMsg,	WPARAM wParam, LPARAM lParam )
{
	ATL::CRegKey rk;
	ULONG len;
	TCHAR tcIpAddress[100]="0.0.0.0",tcPhrases[150]="";
	DWORD dwMaxChannel=0;

	switch(uMsg)
	{

	case WM_INITDIALOG:

		if(rk.Create(HKEY_LOCAL_MACHINE,TEXT("Software\\Tritel\\Hello\\"))== ERROR_SUCCESS)
		{
			len = 100;
			rk.QueryStringValue("IpAddress",tcIpAddress,&len);
			SetDlgItemText(hDlg,IDC_COMIPADDRESS,tcIpAddress);

			len = 150;
			rk.QueryStringValue("Phrases",tcPhrases,&len);
			SetDlgItemText(hDlg,IDC_PHRASES,tcPhrases);

			rk.QueryDWORDValue("MaxChannel",dwMaxChannel);
			SetDlgItemInt(hDlg,IDC_MAXCHANNEL,dwMaxChannel,FALSE);

		}
		break;

	case WM_COMMAND:

		switch(LOWORD (wParam))
		{
		case ID_SAVE:

			//Get The Values From the User
			GetDlgItemText(hDlg, IDC_COMIPADDRESS,tcIpAddress, 100);
			GetDlgItemText(hDlg, IDC_PHRASES,tcPhrases,	150);
			dwMaxChannel=GetDlgItemInt(hDlg, IDC_MAXCHANNEL, NULL, FALSE);

			if(rk.Create(HKEY_LOCAL_MACHINE,TEXT("Software\\Tritel\\Hello\\"))== ERROR_SUCCESS)
			{
				rk.SetStringValue("IpAddress",tcIpAddress);
				rk.SetStringValue("Phrases",tcPhrases);
				rk.SetDWORDValue("MaxChannel",dwMaxChannel);
			}
			EndDialog(hDlg,0);
			break;


		case ID_CANCEL:
			EndDialog(hDlg,	0);
			break;
		}

		break;

	case WM_CLOSE:

		EndDialog(hDlg,	0);

		break;
	}

	return FALSE;



};


/*********************	ResetDlgProc ***********************/

LRESULT	CALLBACK ResetDlgProc( HWND	hDlg, UINT uMsg, WPARAM	wParam,	LPARAM lParam )
{

//	CRegistry Registry;
//	DWORD dwIStart,dwIEnd;
	int	ChannelNo,nIBeginLine,nIEndLine;

	switch(uMsg)
	{

	case WM_INITDIALOG:

		break;

	case WM_COMMAND:

		switch(LOWORD (wParam))
		{
		case IDC_RESET:

			ChannelNo=GetDlgItemInt(hDlg, IDC_LINENUMBER, NULL,	FALSE);

			//Vaildate !!!!! Read from Regisrty	and	Do the Rest

			//Open Registry	Key
//			if(Registry.Open(HKEY_LOCAL_MACHINE,TEXT("Software\\Tritel\\Hello\\Board\\ISDN"))==FALSE)
//			{
//
//			}
//
//			//Incomming	Start Time Slot	Number
//			if(Registry.Read("ISTART",dwIStart)==TRUE)
//				nIBeginLine=dwIStart; 
//
//			//Outgoing End Time	Slot Number
//			if(Registry.Read("IEND",dwIEnd)==TRUE)
//				nIEndLine=dwIEnd;
//
//			//Close	The	Registry
//			Registry.Close();
//
//			if((ChannelNo>=nIBeginLine)&&(ChannelNo<=nIEndLine))
//			{
//			}
////				PhoneLine.ResetLine(ChannelNo);
//			else
//				MessageBox(hDlg,"Invaild Channel Number",NULL,MB_OK);;


			EndDialog(hDlg,0);

			break;

		case ID_CANCEL:

			EndDialog(hDlg,	0);

			return TRUE;
		}
		break;

	case WM_CLOSE:

		EndDialog(hDlg,	0);

		break;

	}

	return FALSE;
}

#define NUM_BUTTONS 4
HWND CreateMainWindowToolbar(HWND hParent)
{   
    HWND hTmp; // Temporary HWND
    INITCOMMONCONTROLSEX icx;
    TBADDBITMAP tbab;
	TBBUTTON tbb[NUM_BUTTONS];
	int i;

    // Ensure common control DLL is loaded
    icx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icx.dwICC = ICC_BAR_CLASSES; // Specify BAR classes
    InitCommonControlsEx(&icx); // Load the common control DLL
    
    // Create the toolbar window
    hTmp = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL, 
            WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hParent, 
            (HMENU) NULL, NULL, NULL);
		// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
	// backward compatibility. 
	SendMessage(hTmp, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);

	// Add the bitmap containing button images to the toolbar.
	tbab.hInst = HINST_COMMCTRL;
	tbab.nID = IDB_STD_SMALL_COLOR;
	SendMessage(hTmp, TB_ADDBITMAP, (WPARAM) NUM_BUTTONS,(LPARAM) &tbab);

	// Add buttons
	ZeroMemory(&tbb, sizeof(TBBUTTON)*NUM_BUTTONS);

	for(i=0;i<4;i++)
	{
		tbb[i].fsState = TBSTATE_ENABLED;
		tbb[i].fsStyle = TBSTYLE_BUTTON;
	}

	tbb[0].iBitmap = IDB_BITMAP1;
	//tbb[0].idCommand = IDS_NEW;

	tbb[1].iBitmap = IDB_BITMAP1;
	//tbb[1].idCommand = IDS_OPEN;

	tbb[2].iBitmap = IDB_BITMAP1;
	//tbb[2].idCommand = IDS_SAVE;

	tbb[3].iBitmap = 0;
	tbb[3].idCommand = 0;
	tbb[3].fsStyle = TBSTYLE_SEP;

	// Add buttons
	SendMessage(hTmp,TB_ADDBUTTONS,NUM_BUTTONS,(LPARAM)&tbb);

	return hTmp;
}
/*********************	CreateListView ***********************/

HWND CreateListView(HINSTANCE hInstance, HWND hwndParent)
{
	DWORD		dwStyle;
	HWND		hwndListView;
	HIMAGELIST	himlSmall;
	HIMAGELIST	himlLarge;
	BOOL		bSuccess = TRUE;

	dwStyle	=	WS_TABSTOP | WS_CHILD |	WS_BORDER |	WS_VISIBLE |LVS_AUTOARRANGE	
		|LVS_REPORT;

	hwndListView = CreateWindowEx(	 WS_EX_CLIENTEDGE,		// ex style
		WC_LISTVIEW,			   // class	name - defined in commctrl.h
		"",						   // dummy	text
		dwStyle,				   // style
		0,						   // x	position
		0,						   // y	position
		0,						   // width
		0,						   // height
		hwndParent,				   // parent
		(HMENU)ID_LISTVIEW,		   // ID
		ghInstance,					  // instance
		NULL);					   // no extra data

	if(!hwndListView)
		return NULL;

	ResizeListView(hwndListView, hwndParent);

	//set the image	lists
	himlSmall =	ImageList_Create(16, 16, ILC_COLORDDB |	ILC_MASK, 1, 0);
	himlLarge =	ImageList_Create(32, 32, ILC_COLORDDB |	ILC_MASK, 1, 0);

	if (himlSmall && himlLarge)
	{
		HANDLE hIcon;


		//set up the small image list
		hIcon =	LoadImage(ghInstance, (LPCTSTR)IDI_PHONE2, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		ImageList_AddIcon(himlSmall, (HICON)hIcon);

		//set up the large image list
		hIcon =	LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_PHONE2));
		ImageList_AddIcon(himlLarge, (HICON)hIcon);

		ListView_SetImageList(hwndListView,	himlSmall, LVSIL_SMALL);
		ListView_SetImageList(hwndListView,	himlLarge, LVSIL_NORMAL);
	}

	return hwndListView;
}

/*********************	InitListView ***********************/

BOOL InitListView(HWND hwndListView)
{
	LV_COLUMN	lvColumn;
	int			i;

	TCHAR		szString[8][25]	= 
	{"LINE", "ANI", "OUT","DESTINATION", "CUSTOMER",
		"START TIME ","STATUS","CONNECTOR"
	};

	//empty	the	list

	ListView_DeleteAllItems(hwndListView);

	//initialize the columns
	lvColumn.mask =	LVCF_FMT | LVCF_WIDTH |	LVCF_TEXT |	LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx	= 150;

	for(i =	0; i < 8; i++)
	{
		lvColumn.pszText = szString[i];
		ListView_InsertColumn(hwndListView,	i, &lvColumn);
	}


	return TRUE;
}

/*********************	ResizeListView ***********************/
void ResizeListView(HWND hwndListView, HWND	hwndParent)
{
	RECT  rc,x;

	GetClientRect(hwndParent, &rc);

	GetWindowRect(hToolbar,&x);

	MoveWindow(	hwndListView, 
		rc.left,
		rc.top+(x.bottom-x.top),
		rc.right - rc.left,
		rc.bottom -	rc.top - (x.bottom-x.top),
		TRUE);
			

	//	Resize List view to fit into single screen
	ListView_SetColumnWidth(hwndListView,0,80);
	ListView_SetColumnWidth(hwndListView,1,100);
	ListView_SetColumnWidth(hwndListView,2,40);
	ListView_SetColumnWidth(hwndListView,3,100);
	ListView_SetColumnWidth(hwndListView,4,100);
	ListView_SetColumnWidth(hwndListView,5,160);
	ListView_SetColumnWidth(hwndListView,6,100);
	ListView_SetColumnWidth(hwndListView,7,100);
}

/*********************	WM_NOTIFY ***********************/
LRESULT	ListViewNotify(HWND	hWnd, LPARAM lParam)
{
	LPNMHDR	 lpnmh = (LPNMHDR) lParam;
	HWND	 hwndListView =	GetDlgItem(hWnd, ID_LISTVIEW);



	switch(lpnmh->code)
	{


	case LVN_GETDISPINFO:
		{

			LV_DISPINFO	*lpdi =	(LV_DISPINFO *)lParam;
			TCHAR szString[MAX_PATH];

			if(lpdi->item.iSubItem)
			{

				switch (lpdi->item.iSubItem)
				{

				case 1:
					lstrcpy(lpdi->item.pszText,	"");

					break;

				case 2:

					lstrcpy(lpdi->item.pszText,	"");
					break;

				case 3:
					lstrcpy(lpdi->item.pszText,	"");
					break;

				case 4:
					lstrcpy(lpdi->item.pszText,	"");
					break;

				case 5:
					lstrcpy(lpdi->item.pszText,	"");
					break;

				default:
					break;
				}

			}
			else
			{
				if(lpdi->item.mask & LVIF_TEXT)
				{

					wsprintf(szString,	"Line %d", lpdi->item.iItem	+ 1);
					lstrcpy(lpdi->item.pszText, szString);
				}

				if(lpdi->item.mask	& LVIF_IMAGE)
				{
					lpdi->item.iImage = 0;
				}

			}


		}

	}

	return 0;

}

/*********************	Insert ListView	Item ********************/

void ListviewInsertItem(HWND hwndListView)
{
	LVITEM item;

	item.mask =	LVIF_TEXT;
	item.iItem = 0;
	item.iSubItem =	0;
	item.pszText = LPSTR_TEXTCALLBACK;
	ListView_InsertItem(hwndListView, &item);
}

void ListviewResetRow(int index)
{
	ListviewSetColumn(index,1,"");
	ListviewSetColumn(index,2,"");
	ListviewSetColumn(index,3,"");
	ListviewSetColumn(index,4,"");
	ListviewSetColumn(index,5,"");
	ListviewSetColumn(index,6,"Ready");
	ListviewSetColumn(index,7,"Ready");
}

/*********************	SetColumn ListView Item	********************/

void ListviewSetColumn(int index,int subitem, char* msg)
{
	ListView_SetItemText(hwndListView,index,subitem, msg);
}

void ListviewSetColumn(int index,int subitem,std::string& msg)
{
	ListView_SetItemText(hwndListView,index,subitem, const_cast<char*>(msg.c_str()));
}

/*********************	Delete ListView	Item ********************/
void ListviewDeleteAllItem()
{
	ListView_DeleteAllItems(hwndListView);
}

void InitDefaultLog()
{
	using namespace	log4cplus;

	//	Default	Appender
	SharedAppenderPtr myAppender(new RollingFileAppender("hello_card.log"));

	myAppender->setName(_T("myAppenderName"));
	std::auto_ptr<Layout> myLayout = std::auto_ptr<Layout>(new log4cplus::TTCCLayout());
	myAppender->setLayout( myLayout	);

	//	Default	Logger
	Logger myLogger= Logger::getInstance(_T("default"));
	myLogger.addAppender(myAppender);
	myLogger.setLogLevel ( TRACE_LOG_LEVEL );

	//	TODO: configure IVR logger too
}

void DoTest()
{
}