#include "alienfx-gui.h"
#include "common.h"

extern groupset* FindMapping(int mid, vector<groupset>* set = conf->active_set);
extern bool SetColor(HWND hDlg, AlienFX_SDK::Afx_colorcode*);
extern void RedrawButton(HWND hDlg, AlienFX_SDK::Afx_colorcode*);
extern void RedrawGridButtonZone(RECT* what = NULL, bool recalc = false);

int clrListID = 0;

void RebuildGEColorsList(HWND hDlg, groupset* mmap) {
	HWND eff_list = GetDlgItem(hDlg, IDC_COLORS_LIST);

	ListView_DeleteAllItems(eff_list);
	ListView_SetExtendedListViewStyle(eff_list, LVS_EX_FULLROWSELECT);

	HIMAGELIST hOld = ListView_GetImageList(eff_list, LVSIL_SMALL);
	if (hOld) {
		ImageList_Destroy(hOld);
	}

	if (!ListView_GetColumnWidth(eff_list, 0)) {
		LVCOLUMNA lCol{ LVCF_FMT, LVCFMT_LEFT };
		ListView_InsertColumn(eff_list, 0, &lCol);
		ListView_SetColumnWidth(eff_list, 0, LVSCW_AUTOSIZE_USEHEADER);// width);
	}
	if (mmap) {
		COLORREF* picData = NULL;
		HBITMAP colorBox = NULL;
		HIMAGELIST hSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON),
			ILC_COLOR32, 1, 1);
		vector<AlienFX_SDK::Afx_colorcode>* colors = &mmap->effect.effectColors;
		for (int i = 0; i < colors->size(); i++) {
			LVITEMA lItem{ LVIF_TEXT | LVIF_IMAGE | LVIF_STATE, i };
			picData = new COLORREF[GetSystemMetrics(SM_CXSMICON) * GetSystemMetrics(SM_CYSMICON)];
			fill_n(picData, GetSystemMetrics(SM_CXSMICON) * GetSystemMetrics(SM_CYSMICON),
				RGB(colors->at(i).b, colors->at(i).g, colors->at(i).r));
			colorBox = CreateBitmap(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 1, 32, picData);
			delete[] picData;
			ImageList_Add(hSmall, colorBox, NULL);
			DeleteObject(colorBox);
			string name = i ? "Phase " + to_string(i) : "Background";
			lItem.iImage = i;
			lItem.pszText = (LPSTR)name.c_str();
			// check selection...
			if (i == clrListID) {
				lItem.state = LVIS_SELECTED;
			}
			else
				lItem.state = 0;
			ListView_InsertItem(eff_list, &lItem);
		}
		ListView_SetImageList(eff_list, hSmall, LVSIL_SMALL);
	}
	ListView_EnsureVisible(eff_list, clrListID, false);
}

void ChangeAddGEColor(HWND hDlg, groupset* mmap, int newColorID) {
	// change color.
	if (mmap) {
		vector<AlienFX_SDK::Afx_colorcode>* clr = &mmap->effect.effectColors;
		if (newColorID < clr->size())
			SetColor(GetDlgItem(hDlg, IDC_BUT_GECOLOR), &clr->at(newColorID));
		else {
			AlienFX_SDK::Afx_colorcode act{ 0 };
			// add new color
			if (clrListID < clr->size())
				act = clr->at(clrListID);
			if (clr->empty())
				clr->push_back(act);
			clr->push_back(act);
			newColorID = (int)clr->size() - 1;
			if (SetColor(GetDlgItem(hDlg, IDC_BUT_GECOLOR), &clr->at(newColorID)))
				clrListID = newColorID;
			else
				clr->erase(clr->begin() + newColorID);
		}
		RebuildGEColorsList(hDlg, mmap);
		RedrawGridButtonZone(NULL, true);
	}
}

void UpdateEffectInfo(HWND hDlg, groupset* mmap) {
	if (mmap) {
		CheckDlgButton(hDlg, IDC_CHECK_CIRCLE, mmap->effect.flags & GE_FLAG_CIRCLE);
		CheckDlgButton(hDlg, IDC_CHECK_RANDOM, mmap->effect.flags & GE_FLAG_RANDOM);
		CheckDlgButton(hDlg, IDC_CHECK_PHASE, mmap->effect.flags & GE_FLAG_PHASE);
		CheckDlgButton(hDlg, IDC_CHECK_BACKGROUND, mmap->effect.flags & GE_FLAG_BACK);
		ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_COMBO_TRIGGER), mmap->effect.trigger);
		ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_COMBO_GEFFTYPE), mmap->effect.type);
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_SPEED), TBM_SETPOS, true, mmap->effect.speed - 80);
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_WIDTH), TBM_SETPOS, true, mmap->effect.width);
		SetSlider(sTip2, mmap->effect.speed - 80);
		SetSlider(sTip3, mmap->effect.width);
	}
	RebuildGEColorsList(hDlg, mmap);
}

BOOL CALLBACK TabGridDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND speed_slider = GetDlgItem(hDlg, IDC_SLIDER_SPEED),
		width_slider = GetDlgItem(hDlg, IDC_SLIDER_WIDTH);

	groupset* mmap = FindMapping(eItem);

	switch (message)
	{
	case WM_INITDIALOG:
	{
		UpdateCombo(GetDlgItem(hDlg, IDC_COMBO_TRIGGER), { "Off", "Continues", "Random", "Keyboard", "Event"});
		UpdateCombo(GetDlgItem(hDlg, IDC_COMBO_GEFFTYPE), { "Running light", "Wave", "Gradient", "Fill" });
		SendMessage(speed_slider, TBM_SETRANGE, true, MAKELPARAM(-80, 80));
		SendMessage(width_slider, TBM_SETRANGE, true, MAKELPARAM(1, 80));
		sTip2 = CreateToolTip(GetDlgItem(hDlg, IDC_SLIDER_SPEED), sTip2);
		sTip3 = CreateToolTip(GetDlgItem(hDlg, IDC_SLIDER_WIDTH), sTip3);
		UpdateEffectInfo(hDlg, mmap);

	} break;
	case WM_APP + 2: {
		mmap = FindMapping(eItem);
		UpdateEffectInfo(hDlg, mmap);
	} break;
	case WM_COMMAND: {
		if (!mmap)
			return false;
		bool state = IsDlgButtonChecked(hDlg, LOWORD(wParam)) == BST_CHECKED;
		switch (LOWORD(wParam))
		{
		case IDC_COMBO_TRIGGER:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				mmap->effect.trigger = ComboBox_GetCurSel(GetDlgItem(hDlg, LOWORD(wParam)));
				RedrawGridButtonZone(NULL, true);
			}
			break;
		case IDC_COMBO_GEFFTYPE:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				mmap->effect.type = ComboBox_GetCurSel(GetDlgItem(hDlg, LOWORD(wParam)));
			}
			break;
		case IDC_CHECK_CIRCLE:
			SetBitMask(mmap->effect.flags, GE_FLAG_CIRCLE, state);
			break;
		case IDC_CHECK_RANDOM:
			SetBitMask(mmap->effect.flags, GE_FLAG_RANDOM, state);
			break;
		case IDC_CHECK_PHASE:
			SetBitMask(mmap->effect.flags, GE_FLAG_PHASE, state);
			break;
		case IDC_CHECK_BACKGROUND:
			SetBitMask(mmap->effect.flags, GE_FLAG_BACK, state);
			break;
		case IDC_BUT_GECOLOR:
				ChangeAddGEColor(hDlg, mmap, clrListID);
			break;
		case IDC_BUT_ADD_COLOR:
			if (HIWORD(wParam) == BN_CLICKED) {
				ChangeAddGEColor(hDlg, mmap, (int)mmap->effect.effectColors.size());
			}
			break;
		case IDC_BUT_REMOVE_COLOR:
			if (HIWORD(wParam) == BN_CLICKED && clrListID < mmap->effect.effectColors.size()) {
				if (mmap->effect.effectColors.size() <= 2) {
					mmap->effect.effectColors.clear();
					clrListID = 0;
				}
				else {
					mmap->effect.effectColors.erase(mmap->effect.effectColors.begin() + clrListID);
					if (clrListID)
						clrListID--;
				}
				RebuildGEColorsList(hDlg, mmap);
				RedrawGridButtonZone(NULL, true);
			}
			break;
		}
		mmap->gridop.passive = true;
	} break;
	case WM_HSCROLL:
		switch (LOWORD(wParam)) {
		case TB_THUMBTRACK: case TB_ENDTRACK:
			if (mmap) {
				if ((HWND)lParam == speed_slider) {
					mmap->effect.speed = (BYTE)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0) + 80;
					SetSlider(sTip2, mmap->effect.speed - 80);
				}
				if ((HWND)lParam == width_slider) {
					mmap->effect.width = (BYTE)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
					SetSlider(sTip3, mmap->effect.width);
				}
			}
		} break;
	case WM_DRAWITEM:
		if (mmap && clrListID < mmap->effect.effectColors.size()) {
			RedrawButton(GetDlgItem(hDlg, IDC_BUT_GECOLOR), &mmap->effect.effectColors[clrListID]);
		}
	    break;
	case WM_NOTIFY:
		if (((NMHDR*)lParam)->idFrom == IDC_COLORS_LIST) {
			switch (((NMHDR*)lParam)->code) {
			case LVN_ITEMCHANGED:
			{
				NMLISTVIEW* lPoint = (LPNMLISTVIEW)lParam;
				if (lPoint->uNewState & LVIS_SELECTED && lPoint->iItem != -1) {
					// Select other item...
					clrListID = lPoint->iItem;
					if (clrListID < mmap->effect.effectColors.size())
						RedrawButton(GetDlgItem(hDlg, IDC_BUT_GECOLOR), &mmap->effect.effectColors[clrListID]);
				}
			} break;
			case NM_DBLCLK:
				ChangeAddGEColor(hDlg, mmap, clrListID);
				break;
			}
			break;
		}
		break;
	default: return false;
	}
	return true;
}