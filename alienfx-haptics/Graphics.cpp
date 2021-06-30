#define _WIN32_WINNT 0x500
#include "Graphics.h"
#include <windows.h>
#include <Commctrl.h>
#include "resource_config.h"
#include "FXHelper.h"
#include <algorithm>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib,"Version.lib")

BOOL CALLBACK DialogConfigStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//int bars;
int* freq;
int nCmdShow;
double y_scale;

bool axis_draw = true;

ConfigHandler* config = NULL;
FXHelper* afx = NULL;
WSAudioIn* audio = NULL;

HINSTANCE ghInstance = NULL;

NOTIFYICONDATA niData;

Graphics::Graphics(HINSTANCE hInstance, int mainCmdShow, int* freqp, ConfigHandler *conf, FXHelper *fxproc)
{

	nCmdShow=mainCmdShow;
	//bars = conf->numbars;
	y_scale = conf->res;
	freq=freqp;

	config = conf;
	afx = fxproc;

	ghInstance = hInstance;

	dlg = CreateDialogParam(hInstance,
		MAKEINTRESOURCE(IDD_DIALOG_CONFIG),
		NULL,
		(DLGPROC)DialogConfigStatic, 0);
	if (!dlg) return;

	SendMessage(dlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ALIEN)));
	SendMessage(dlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ALIEN), IMAGE_ICON, 16, 16, 0));

	//ShowWindow(dlg, nCmdShow);
}


void Graphics::start(){
	ShowWindow(dlg, nCmdShow);
	UpdateWindow(dlg);

	while(GetMessage(&Msg, NULL, 0, 0) != 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}

double Graphics::getYScale() {
	return y_scale;
}

void DrawFreq(HDC hdc, LPRECT rcClientP);

void Graphics::refresh(){

	if (!IsIconic(dlg)) {
		HWND hysto = GetDlgItem(dlg, IDC_VIEW_LEVELS);
		RECT levels_rect;
		GetClientRect(hysto, &levels_rect);

		HBRUSH hb = CreateSolidBrush(RGB(0, 0, 0));

		FillRect(GetDC(hysto), &levels_rect, hb);
		DeleteObject(hb);

		DrawFreq(GetDC(hysto), &levels_rect);
	}
}

void Graphics::ShowError(char* T)
{
	MessageBox(dlg, T, "Error!", MB_OK);
}

void Graphics::SetAudioObject(WSAudioIn* wsa)
{
	audio = wsa;
}

HWND Graphics::GetDlg()
{
	return dlg;
}

void DrawFreq(HDC hdc, LPRECT rcClientP)
{
	unsigned i, rectop;
	char szSize[100]; //freq axis

	//setting collors:
	SetDCBrushColor(hdc, RGB(255, 255, 255));
	SetDCPenColor(hdc, RGB(255, 255, 39));
	SetTextColor(hdc, RGB(255, 255, 255));
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SelectObject(hdc, GetStockObject(DC_PEN));
	SetBkMode(hdc, TRANSPARENT);

	if (axis_draw) {
		//draw x axis:
		MoveToEx(hdc, 10, rcClientP->bottom - 21, (LPPOINT)NULL);
		LineTo(hdc, rcClientP->right - 10, rcClientP->bottom - 21);
		LineTo(hdc, rcClientP->right - 15, rcClientP->bottom - 26);
		MoveToEx(hdc, rcClientP->right - 10, rcClientP->bottom - 21, (LPPOINT)NULL);
		LineTo(hdc, rcClientP->right - 15, rcClientP->bottom - 16);
		//TextOut(hdc, rcClientP->right - 45, rcClientP->bottom - 27, "f(kHz)", 6);

		//draw y axis:
		MoveToEx(hdc, 10, rcClientP->bottom - 21, (LPPOINT)NULL);
		LineTo(hdc, 10, 10);
		LineTo(hdc, 15, 15);
		MoveToEx(hdc, 10, 10, (LPPOINT)NULL);
		LineTo(hdc, 5, 15);
		//TextOut(hdc, 15, 10, "[Power]", 7);
		//wsprintf(szSize, "%6d", (int)y_scale);
		//TextOut(hdc, 150, 10, szSize, 6);
		//TextOut(hdc, 10, 40, "255", 3);
		//TextOut(hdc, 10, (rcClientP->bottom) / 2, "128", 3);
		//TextOut(hdc, 10, rcClientP->bottom - 35, "  0", 3);
		//axis_draw = false;
		int oldvalue = (-1);
		double coeff = 22 / (log(22.0));
		for (i = 0; i <= 22; i++) {
			int frq = int(22 - round((log(22.0 - i) * coeff)));
			if (frq > oldvalue) {
				wsprintf(szSize, "%2d", frq);
				TextOut(hdc, ((rcClientP->right - 20) * i) / 22 + 10, rcClientP->bottom - 20, szSize, 2);
				oldvalue = frq;
			}
		}
	}

	for (i = 0; i < config->numbars; i++) {
		rectop = ((255 - freq[i]) * (rcClientP->bottom - 21)) / 255;
		if (rectop < 10) rectop = 10;
		Rectangle(hdc, ((rcClientP->right - 20) * i) / config->numbars + 10, rectop, ((rcClientP->right - 20) * (i + 1)) / config->numbars - 2 + 10, rcClientP->bottom - 21);
		//wsprintf(szSize, "%3d", freq[i]);
		//TextOut(hdc, ((rcClientP->right - 20) * i) / bars + 10, rectop - 15, szSize, 3);
	}
} 

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG: {
		HRSRC hResInfo;
		DWORD dwSize;
		HGLOBAL hResData;
		LPVOID pRes, pResCopy;
		UINT uLen;
		VS_FIXEDFILEINFO* lpFfi;

		HWND version_text = GetDlgItem(hDlg, IDC_STATIC_VERSION);

		hResInfo = FindResource(ghInstance, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
		dwSize = SizeofResource(ghInstance, hResInfo);
		hResData = LoadResource(ghInstance, hResInfo);
		pRes = LockResource(hResData);
		pResCopy = LocalAlloc(LMEM_FIXED, dwSize);
		CopyMemory(pResCopy, pRes, dwSize);
		FreeResource(hResData);

		VerQueryValue(pResCopy, TEXT("\\"), (LPVOID*)&lpFfi, &uLen);
		char buf[255];

		DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
		DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;

		sprintf_s(buf, 255, "Version: %d.%d.%d.%d", HIWORD(dwFileVersionMS), LOWORD(dwFileVersionMS), HIWORD(dwFileVersionLS), LOWORD(dwFileVersionLS));

		SetWindowText(version_text, buf);

		LocalFree(pResCopy);
		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	case WM_NOTIFY:
		switch (LOWORD(wParam)) {
		case IDC_SYSLINK_HOMEPAGE:
			switch (((LPNMHDR)lParam)->code)
			{

			case NM_CLICK:
			case NM_RETURN:
				ShellExecute(NULL, "open", "https://github.com/T-Troll/alienfx-tools", NULL, NULL, SW_SHOWNORMAL);
				break;
			} break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void RedrawButton(HWND hDlg, unsigned id, BYTE r, BYTE g, BYTE b) {
	RECT rect;
	HBRUSH Brush = NULL;
	HWND tl = GetDlgItem(hDlg, id);
	GetWindowRect(tl, &rect);
	HDC cnt = GetWindowDC(tl);
	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.top = rect.left = 0;
	// BGR!
	Brush = CreateSolidBrush(RGB(r, g, b));
	FillRect(cnt, &rect, Brush);
	DrawEdge(cnt, &rect, EDGE_RAISED, BF_RECT);
	DeleteObject(Brush);
}

bool SetColor(HWND hDlg, int id, BYTE* r, BYTE* g, BYTE* b) {
	CHOOSECOLOR cc;                 
	static COLORREF acrCustClr[16];
	bool ret;
	// Initialize CHOOSECOLOR 
	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hDlg;
	cc.lpCustColors = (LPDWORD)acrCustClr;
	cc.rgbResult = RGB(*r, *g, *b);
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ret = ChooseColor(&cc))
	{
		*r = cc.rgbResult & 0xff;
		*g = cc.rgbResult >> 8 & 0xff;
		*b = cc.rgbResult >> 16 & 0xff;
	}
	RedrawButton(hDlg, id, *r, *g, *b);
	return ret;
}

mapping* FindMapping(int lid) {
	if (lid != -1) {
		AlienFX_SDK::mapping lgh = afx->afx_dev.GetMappings()->at(lid);
		for (int i = 0; i < config->mappings.size(); i++)
			if (config->mappings[i].devid == lgh.devid && config->mappings[i].lightid == lgh.lightid)
				return &config->mappings[i];
	}
	return NULL;
}

void ReloadLightList(HWND light_list) {
	size_t lights = afx->afx_dev.GetMappings()->size();
	SendMessage(light_list, LB_RESETCONTENT, 0, 0);
	for (int i = 0; i < lights; i++) {
		AlienFX_SDK::mapping lgh = afx->afx_dev.GetMappings()->at(i);
		if (afx->LocateDev(lgh.devid) && !lgh.flags) {
			int pos = (int)SendMessage(light_list, LB_ADDSTRING, 0, (LPARAM)(lgh.name.c_str()));
			SendMessage(light_list, LB_SETITEMDATA, pos, i);
		}
	}
	RedrawWindow(light_list, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
}

HWND CreateToolTip(HWND hwndParent)
{
	// Create a tooltip.
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hwndParent, NULL, ghInstance, NULL);

	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hwndParent;
	ti.hinst = ghInstance;
	ti.lpszText = (LPSTR)"0";

	GetClientRect(hwndParent, &ti.rect);

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
	return hwndTT;
}

char sBuff[4], lBuff[4];
HWND sTip = 0, lTip = 0;

void SetSlider(HWND tt, char* buff, int value) {
	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(ti);
	ti.lpszText = buff;
	if (tt) {
		SendMessage(tt, TTM_ENUMTOOLS, 0, (LPARAM)&ti);
		_itoa_s(value, buff, 4, 10);
		ti.lpszText = buff;
		SendMessage(tt, TTM_SETTOOLINFO, 0, (LPARAM)&ti);
	}
}

BOOL CALLBACK DialogConfigStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	HWND freq_list = GetDlgItem(hDlg, IDC_FREQ);
	HWND light_list = GetDlgItem(hDlg, IDC_LIGHTS);
	HWND hLowSlider = GetDlgItem(hDlg, IDC_SLIDER_LOWCUT);
	HWND hHiSlider = GetDlgItem(hDlg, IDC_SLIDER_HICUT);
	mapping* map = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
	{
		double coeff = 22030 / log(21);
		char frqname[55]; int prevfreq = 20;
		for (int i = 1; i < 21; i++) {
			int frq = 22050 - (int) round((log(21-i) * coeff));
			sprintf_s(frqname, 55, "%d-%dHz", prevfreq, frq);
			prevfreq = frq;
			SendMessage(freq_list, LB_ADDSTRING, 0, (LPARAM)frqname);
		}

		ReloadLightList(light_list);

		SendMessage(hLowSlider, TBM_SETRANGE, true, MAKELPARAM(0, 255));
		SendMessage(hHiSlider, TBM_SETRANGE, true, MAKELPARAM(0, 255));

		SendMessage(hLowSlider, TBM_SETTICFREQ, 16, 0);
		SendMessage(hHiSlider, TBM_SETTICFREQ, 16, 0);

		sTip = CreateToolTip(hLowSlider);
		lTip = CreateToolTip(hHiSlider);

		CheckMenuItem(GetMenu(hDlg), config->inpType ? ID_INPUT_DEFAULTINPUTDEVICE : ID_INPUT_DEFAULTOUTPUTDEVICE, MF_CHECKED);

	}
	break;

	case WM_COMMAND:
	{
		int lbItem = (int)SendMessage(light_list, LB_GETCURSEL, 0, 0);
		int lid = (int)SendMessage(light_list, LB_GETITEMDATA, lbItem, 0);
		int fid = (int)SendMessage(freq_list, LB_GETCURSEL, 0, 0);
		map = FindMapping(lid);
		switch (LOWORD(wParam))
		{
		case ID_INPUT_DEFAULTOUTPUTDEVICE:
			config->inpType = 0;
			CheckMenuItem(GetMenu(hDlg), ID_INPUT_DEFAULTINPUTDEVICE, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hDlg), ID_INPUT_DEFAULTOUTPUTDEVICE, MF_CHECKED);
			audio->RestartDevice(0);
			config->Save();
			break;
		case ID_INPUT_DEFAULTINPUTDEVICE:
			config->inpType = 1;
			CheckMenuItem(GetMenu(hDlg), ID_INPUT_DEFAULTINPUTDEVICE, MF_CHECKED);
			CheckMenuItem(GetMenu(hDlg), ID_INPUT_DEFAULTOUTPUTDEVICE, MF_UNCHECKED);
			audio->RestartDevice(1);
			config->Save();
			break;
		case ID_FILE_EXIT: case IDOK: PostMessage(hDlg, WM_CLOSE, 0, 0); break;
		case ID_HELP_ABOUT: // about dialogue here
			DialogBox(ghInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
			break;
		case IDC_RESET: {
			switch (HIWORD(wParam))
			{
			case BN_CLICKED: {
				if (MessageBox(hDlg, "Do you really want to reset settings and colors for ALL lights?", "Warning!",
					MB_YESNO | MB_ICONWARNING) == IDYES) {
					config->mappings.clear();
					// clear colors...
					RedrawButton(hDlg, IDC_BUTTON_LPC, 0, 0, 0);
					RedrawButton(hDlg, IDC_BUTTON_HPC, 0, 0, 0);
					//  clear cuts....
					SendMessage(hLowSlider, TBM_SETPOS, true, 0);
					SendMessage(hHiSlider, TBM_SETPOS, true, 255);
					SetSlider(sTip, sBuff, 0);
					SetSlider(lTip, lBuff, 255);
					// clear selections
					SendMessage(freq_list, LB_SETSEL, FALSE, -1);
					SendMessage(light_list, LB_SETCURSEL, -1, 0);

				}
			} break;
			}
		} break;
		case IDC_LIGHTS: // should reload mappings
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE: {
				// check in config - do we have mappings?
				if (map == NULL) {
					mapping newmap;
					AlienFX_SDK::mapping lgh = afx->afx_dev.GetMappings()->at(lid);
					newmap.devid = lgh.devid;
					newmap.lightid = lgh.lightid;
					newmap.colorfrom.ci = 0;
					newmap.colorto.ci = 0;
					newmap.lowcut = 0;
					newmap.hicut = 255;
					config->mappings.push_back(newmap);
					std::sort(config->mappings.begin(), config->mappings.end(), ConfigHandler::sortMappings);
					map = FindMapping(lid);
				}
				// load freq....
				EnableWindow(freq_list, TRUE);
				SendMessage(freq_list, LB_SETSEL, FALSE, -1);
				for (int j = 0; j < map->map.size(); j++) {
					SendMessage(freq_list, LB_SETSEL, TRUE, map->map[j]);
				}
				RedrawWindow(freq_list, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				// load colors...
				RedrawButton(hDlg, IDC_BUTTON_LPC, map->colorfrom.cs.red, map->colorfrom.cs.green, map->colorfrom.cs.blue);
				RedrawButton(hDlg, IDC_BUTTON_HPC, map->colorto.cs.red, map->colorto.cs.green, map->colorto.cs.blue);
				// load cuts...
				SendMessage(hLowSlider, TBM_SETPOS, true, map->lowcut);
				SendMessage(hHiSlider, TBM_SETPOS, true, map->hicut);
				SetSlider(sTip, sBuff, map->lowcut);
				SetSlider(lTip, lBuff, map->hicut);
			}
			break;
		}
		break;
		case IDC_FREQ: { // should update mappings list
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE: {
				if (map != NULL) {
					// add mapping
					std::vector <unsigned char>::iterator Iter = map->map.begin();
					int i = 0;
					for (i = 0; i < map->map.size(); i++)
						if (map->map[i] == fid)
							break;
						else
							Iter++;
					if (i == map->map.size())
						// new mapping, add and select
						map->map.push_back(fid);
					else
						map->map.erase(Iter);
					// remove selection
				}
				else {
					SendMessage(freq_list, LB_SETSEL, FALSE, -1);
				}
			} break;
			}
		} break;		
		case IDC_BUTTON_LPC:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED: {
				if (map != NULL) {
					SetColor(hDlg, IDC_BUTTON_LPC, &map->colorfrom.cs.red,
						&map->colorfrom.cs.green, &map->colorfrom.cs.blue);
				}
			} break;
		} break;
		case IDC_BUTTON_HPC:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED: {
				if (map != NULL) {
					SetColor(hDlg, IDC_BUTTON_HPC, &map->colorto.cs.red,
						&map->colorto.cs.green, &map->colorto.cs.blue);
				}
			} break;
			} break;
		case IDC_BUTTON_REFRESH:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED: {
				afx->FillDevs();
				ReloadLightList(light_list);
			} break;
			} break;
		case IDC_MINIMIZE:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED: {
				ZeroMemory(&niData, sizeof(NOTIFYICONDATA));
				niData.cbSize = sizeof(NOTIFYICONDATA);
				niData.uID = IDI_ALIEN;
				niData.uFlags = NIF_ICON | NIF_MESSAGE;
				niData.hIcon =
					(HICON)LoadImage(GetModuleHandle(NULL),
						MAKEINTRESOURCE(IDI_ALIEN),
						IMAGE_ICON,
						GetSystemMetrics(SM_CXSMICON),
						GetSystemMetrics(SM_CYSMICON),
						LR_DEFAULTCOLOR);
				niData.hWnd = hDlg;
				niData.uCallbackMessage = WM_APP + 1;
				Shell_NotifyIcon(NIM_ADD, &niData);
				ShowWindow(hDlg, SW_HIDE);
			} break;
			} break;
		case IDC_BUTTON_REMOVE: {
			switch (HIWORD(wParam))
			{
			case BN_CLICKED: {
				if (map != NULL) {
					std::vector <mapping>::iterator Iter;
					AlienFX_SDK::mapping lgh = afx->afx_dev.GetMappings()->at(lid);
					for (Iter = config->mappings.begin(); Iter != config->mappings.end(); Iter++)
						if (Iter->devid == lgh.devid && Iter->lightid == lgh.lightid) {
							config->mappings.erase(Iter);
							// clear colors...
							RedrawButton(hDlg, IDC_BUTTON_LPC, 0, 0, 0);
							RedrawButton(hDlg, IDC_BUTTON_HPC, 0, 0, 0);
							//  clear cuts....
							SendMessage(hLowSlider, TBM_SETPOS, true, 0);
							SendMessage(hHiSlider, TBM_SETPOS, true, 255);
							SetSlider(sTip, sBuff, 0);
							SetSlider(lTip, lBuff, 255);
							// clear selections
							SendMessage(freq_list, LB_SETSEL, FALSE, -1);
							SendMessage(light_list, LB_SETCURSEL, -1, 0);
							break;
						}
				}
			} break;
			}
		} break;
		}
	} break;
	case WM_HSCROLL: {
		int lbItem = (int)SendMessage(light_list, LB_GETCURSEL, 0, 0);
		int lid = (int)SendMessage(light_list, LB_GETITEMDATA, lbItem, 0);
		switch (LOWORD(wParam)) {
		case TB_THUMBTRACK: case TB_ENDTRACK: {
			map = FindMapping(lid);
			if (map != NULL) {
				if ((HWND)lParam == hLowSlider) {
					map->lowcut = (UCHAR) SendMessage(hLowSlider, TBM_GETPOS, 0, 0);
					SetSlider(sTip, sBuff, map->lowcut);
				}
				if ((HWND)lParam == hHiSlider) {
					map->hicut = (UCHAR) SendMessage(hHiSlider, TBM_GETPOS, 0, 0);
					SetSlider(lTip, lBuff, map->hicut);
				}
			}
		} break;
		}
	} break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) {
			// go to tray...

			ZeroMemory(&niData, sizeof(NOTIFYICONDATA));
			niData.cbSize = sizeof(NOTIFYICONDATA);
			niData.uID = IDI_ALIEN;
			niData.uFlags = NIF_ICON | NIF_MESSAGE;
			niData.hIcon =
				(HICON)LoadImage(GetModuleHandle(NULL),
					MAKEINTRESOURCE(IDI_ALIEN),
					IMAGE_ICON,
					GetSystemMetrics(SM_CXSMICON),
					GetSystemMetrics(SM_CYSMICON),
					LR_DEFAULTCOLOR);
			niData.hWnd = hDlg;
			niData.uCallbackMessage = WM_APP + 1;
			Shell_NotifyIcon(NIM_ADD, &niData);
			ShowWindow(hDlg, SW_HIDE);
		} break;
	case WM_APP + 1: {
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONUP:
			ShowWindow(hDlg, SW_RESTORE);
			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
			SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
			Shell_NotifyIcon(NIM_DELETE, &niData);
			break;
		case WM_RBUTTONUP:
		case WM_CONTEXTMENU:
			PostMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}
		break;
	} break;
	case WM_DRAWITEM:
		switch (((DRAWITEMSTRUCT*)lParam)->CtlID) {
		case IDC_BUTTON_LPC: case IDC_BUTTON_HPC:
			RedrawButton(hDlg, ((DRAWITEMSTRUCT*)lParam)->CtlID, 0, 0, 0);
			break;
		case IDC_VIEW_LEVELS:
			HWND hysto = GetDlgItem(hDlg, IDC_VIEW_LEVELS);
			if (hysto) {
				RECT levels_rect;
				GetClientRect(hysto, &levels_rect);

				HBRUSH hb = CreateSolidBrush(RGB(0, 0, 0));

				FillRect(GetDC(hysto), &levels_rect, hb);
				DeleteObject(hb);

				DrawFreq(GetDC(hysto), &levels_rect);
			}
		}
		break;
	case WM_PAINT: {
		HWND hysto = GetDlgItem(hDlg, IDC_VIEW_LEVELS);
		if (hysto) {
			RECT levels_rect;
			GetClientRect(hysto, &levels_rect);

			HBRUSH hb = CreateSolidBrush(RGB(0, 0, 0));

			FillRect(GetDC(hysto), &levels_rect, hb);
			DeleteObject(hb);

			DrawFreq(GetDC(hysto), &levels_rect);
		}
		return false;
	} break;
	case WM_CLOSE: DestroyWindow(hDlg); break;
	case WM_DESTROY: config->Save(); Shell_NotifyIcon(NIM_DELETE, &niData); DestroyWindow(hDlg); PostQuitMessage(0); break;
	default: return false;
	}

	return TRUE;
}