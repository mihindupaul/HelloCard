/******************************************************************************************/
/*                                                                                        */
/*	Source File		: HelloCard.h														  */
/*																						  */	
/*	Version			: 2.0																  */
/*                                                                                        */
/*	Target OS		: Windows 2000	Service Pack 3.0									  */
/*																						  */
/*	Description		: PROTOTYPES/CONSTANTS/MACRO DEFINITIONS							  */
/*																						  */
/*	Last Revision	: <29/10/2002>													      */
/*																						  */
/*	Release Stage	: BETA																  */
/*																						  */
/*	Author			: A.T.Lloyd	(c) 2002,  All rights reserved.							  */
/*					  Tritel Technologies (Pvt.) Ltd.									  */
/******************************************************************************************/
#ifndef HELLOCARD_DEF
#define HELLOCARD_DEF

#include <srllib.h>
#include <dxxxlib.h>

/*********************  Prototypes  ***********************/

// Windows Procedure

LRESULT WINAPI MainWndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT WINAPI AboutDlgProc( HWND, UINT, WPARAM, LPARAM );
LRESULT WINAPI SS7ChannelDlgProc( HWND, UINT, WPARAM, LPARAM );
LRESULT WINAPI R2ChannelDlgProc( HWND, UINT, WPARAM, LPARAM );
LRESULT WINAPI ISDNChannelDlgProc( HWND, UINT, WPARAM, LPARAM );
LRESULT WINAPI DataBaseDlgProc( HWND, UINT, WPARAM, LPARAM );
LRESULT WINAPI ServerDlgProc( HWND, UINT, WPARAM, LPARAM );
LRESULT WINAPI ResetDlgProc( HWND, UINT, WPARAM, LPARAM );
LRESULT WINAPI CallingDlgProc( HWND, UINT, WPARAM, LPARAM );
void WM_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify);

//ListView UserInterFace

HWND CreateListView(HINSTANCE hInstance, HWND hwndParent);
void ResizeListView(HWND hwndListView, HWND hwndParent);
BOOL InitListView(HWND hwndListView);
void ListviewInsertItem(HWND hwndListView);
void ListviewDeleteItem(HWND hwndListView,int index);
void ListviewSetColumn(int index,int subitem,char* msg);
void ListviewSetColumn(int index,int subitem,std::string& msg);
void ListviewDeleteAllItem();
void ListviewResetRow(int index);
LRESULT ListViewNotify(HWND hWnd, LPARAM lParam);
DWORD WINAPI ThreadFunc( LPVOID lpParam ) ;
void DoTest();

//Timer Functions
void TimerTigger(int nIDEvent);
void AlertTimer(int nIDEvent,int nDuration);
void StartTimer(int nIDEvent,int nDuration);
void StopTimer(int nIDEvent);
void InitDefaultLog();

#endif // for ANSWER_DEF
