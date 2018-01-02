/*
  Winタブレット壁紙変更(WinTabWallChanger)
  2014/02/08 Tubo
*/

#include <stdio.h>
#include <windows.h>
#include "resource.h"

#define TIMER_ID (100)
#define MSG_TRAY WM_USER
#define ID_TRAY (101)

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
int MakeRTrayMenu(HWND);
void SetMenuCheck(long);
void SetTaskTray(HWND, PNOTIFYICONDATA);

HWND hWnd;
HMENU hMenu, hSubMenu;
HINSTANCE hInst;

char szClassName[] = "WinTabWallChanger";
int g_mode;
int g_tyflag  = 0;
int g_iniflag = 0;
int g_checkd  = 500;
BOOL g_forced = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wc;
	hInst = hInstance;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szClassName;

	RegisterClass(&wc);

	hWnd = CreateWindow(szClassName,
						szClassName,
						WS_CAPTION | WS_SYSMENU | WS_DLGFRAME,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						400,
						300,
						NULL,
						NULL,
						hInstance,
						NULL);

	DragAcceptFiles(hWnd, TRUE);

	if(g_mode != 30){
		ShowWindow(hWnd, nCmdShow);
	}
	UpdateWindow(hWnd);

	while(GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	KillTimer(hWnd, TIMER_ID);

	return (msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	HDROP hDrop;
	static char lpszFn[256];
	static char lpszTn[256];
	static char lpszYn[256];
	static char lpszTx[256];
	static NOTIFYICONDATA ni;
	int dx,dy;
	long lRetCode;

	switch(msg){
		case WM_CREATE:
			g_mode = 10;
			hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_TRAY));
			lRetCode = GetPrivateProfileString("WinTabWallChanger",
												"WallY",
												"",
												lpszYn,
												sizeof(lpszYn),
												".\\wtwc.ini");
			if(lRetCode == 0){
				g_iniflag++;
			}
			lRetCode = GetPrivateProfileString("WinTabWallChanger",
												"WallT",
												"",
												lpszTn,
												sizeof(lpszTn),
												".\\wtwc.ini");
			if(lRetCode == 0){
				g_iniflag++;
			}

			lRetCode = GetPrivateProfileInt("WinTabWallChanger",
											"interval",
											500,
											".\\wtwc.ini");
			if(lRetCode != 0){
				g_checkd = lRetCode;
			}

			if(g_iniflag == 0){
				SetTimer(hWnd, TIMER_ID, g_checkd, NULL);
				ShowWindow(hWnd, SW_HIDE);
				SetTaskTray(hWnd, &ni);
				g_mode = 30;
			}
			break;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			switch(g_mode){
				case 10:
					strcpy(lpszTx, "横用の壁紙ファイルをドロップしてください");
					break;
				case 20:
					strcpy(lpszTx, "縦用の壁紙ファイルをドロップしてください");
					break;
				default:
					strcpy(lpszTx, "");
					break;
			}
			TextOut(hdc, 10, 10, (LPCSTR)lpszTx, strlen(lpszTx));
			EndPaint(hWnd, &ps);
			break;

		case WM_DROPFILES:
			hDrop = (HDROP)wParam;
			DragQueryFile(hDrop, 0, lpszFn, 256);
			if(g_mode == 10){
				g_mode = 20;
				strcpy(lpszYn, lpszFn);
				WritePrivateProfileString("WinTabWallChanger",
											"WallY",
											lpszYn,
											".\\wtwc.ini");
				InvalidateRect(hWnd, NULL, TRUE);
			}else if(g_mode == 20){
				g_mode = 30;
				strcpy(lpszTn, lpszFn);
				WritePrivateProfileString("WinTabWallChanger",
											"WallT",
											lpszTn,
											".\\wtwc.ini");
				SetMenuCheck(g_checkd);
				g_forced = true;
                InvalidateRect(hWnd, NULL, TRUE);
			}
			DragFinish(hDrop);

			if(g_mode >= 30){
				SetTimer(hWnd, TIMER_ID, g_checkd, NULL);
				ShowWindow(hWnd, SW_HIDE);
				SetTaskTray(hWnd, &ni);
			}
			break;

		case MSG_TRAY:
			if(wParam == ID_TRAY){
				switch(lParam){
					case WM_RBUTTONDOWN:
						MakeRTrayMenu(hWnd);
						break;
/* 左クリック
					case WM_LBUTTONDOWN:
						KillTimer(hWnd, TIMER_ID);
						Shell_NotifyIcon(NIM_DELETE, &ni);
						ShowWindow(hWnd, SW_SHOW);
						InvalidateRect(hWnd, NULL, TRUE);
						g_mode = 10;
						break;
*/
					default:
						break;
				}
			}
			break;

		case WM_TIMER:
			if(wParam == TIMER_ID){
				dx = GetSystemMetrics(SM_CXSCREEN);
				dy = GetSystemMetrics(SM_CYSCREEN);
				if(dx > dy){
					if(g_tyflag != 1 || g_forced){
						strcpy(lpszFn, lpszYn);
						SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, lpszFn, 0);
						g_tyflag = 1;
						g_forced = false;
					}
				}else{
					if(g_tyflag != 2 || g_forced){
						strcpy(lpszFn, lpszTn);
						SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, lpszFn, 0);
						g_tyflag = 2;
						g_forced = false;
					}
				}
			}
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDM_1000MS:
					SetMenuCheck(IDM_1000MS);
					g_checkd = IDM_1000MS;
					KillTimer(hWnd, TIMER_ID);
					SetTimer(hWnd, TIMER_ID, 3000, NULL);
					break;

				case IDM_500MS:
					SetMenuCheck(IDM_500MS);
					g_checkd = IDM_500MS;
					KillTimer(hWnd, TIMER_ID);
					SetTimer(hWnd, TIMER_ID, 1000, NULL);
					break;

				case IDM_300MS:
					SetMenuCheck(IDM_300MS);
					g_checkd = IDM_300MS;
					KillTimer(hWnd, TIMER_ID);
					SetTimer(hWnd, TIMER_ID, 300, NULL);
					break;
				case IDM_VERSION:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG), hWnd, (DLGPROC)DlgProc);
					break;

				case IDM_SETUP:
					KillTimer(hWnd, TIMER_ID);
					Shell_NotifyIcon(NIM_DELETE, &ni);
					ShowWindow(hWnd, SW_SHOW);
					InvalidateRect(hWnd, NULL, TRUE);
					g_mode = 10;
					break;

				case IDM_END:
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					break;
			}
			break;

		case WM_CLOSE:
			Shell_NotifyIcon(NIM_DELETE, &ni);
			DestroyMenu(hMenu);
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK DlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg){
		case WM_INITDIALOG:
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wp)){
				case IDOK:
					EndDialog(hDlgWnd, IDOK);
					break;
				default:
					return FALSE;
			}
		default:
			return FALSE;
	}
	return TRUE;
}

void SetMenuCheck(long id)
{
	char buf[80];
	CheckMenuItem(hMenu, IDM_1000MS, MF_BYCOMMAND | MFS_UNCHECKED);
	CheckMenuItem(hMenu, IDM_500MS,  MF_BYCOMMAND | MFS_UNCHECKED);
	CheckMenuItem(hMenu, IDM_300MS,  MF_BYCOMMAND | MFS_UNCHECKED);
	CheckMenuItem(hMenu, id, MF_BYCOMMAND | MFS_CHECKED);

	sprintf(buf, "%d", id);
	WritePrivateProfileString("WinTabWallChanger",
								"interval",
								buf,
								".\\wtwc.ini");
}

void SetTaskTray(HWND hWnd, PNOTIFYICONDATA ni)
{
	HICON hIcon;

	hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));
	ni->cbSize = sizeof(NOTIFYICONDATA);
	ni->hIcon = hIcon;
	ni->hWnd = hWnd;
	ni->uCallbackMessage = MSG_TRAY;
	ni->uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	ni->uID = ID_TRAY;
	strcpy(ni->szTip, szClassName);

	Shell_NotifyIcon(NIM_ADD, ni);

	return;
}

int MakeRTrayMenu(HWND hWnd)
{
	POINT pt;

	hSubMenu = GetSubMenu(hMenu, 0);
	SetMenuCheck(g_checkd);
	GetCursorPos(&pt);
	SetForegroundWindow(hWnd);
	TrackPopupMenu(hSubMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);

	return 0;
}