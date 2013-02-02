#pragma once

#include "stdafx.h"
#include "FileCollection.h"
#include "resource.h"
#include <cstring>

class AddEnvironmentPathDialog : public CDialogImpl<AddEnvironmentPathDialog>
{
public:
	enum { IDD = IDD_ADD_ENV_VAR_DIALOG };
	enum Environment {USER, SYSTEM};

	AddEnvironmentPathDialog(FileCollection& files) : files(files)
	{
	}

	virtual ~AddEnvironmentPathDialog(void)
	{
	}

	BEGIN_MSG_MAP(AddEnvironmentPathDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		COMMAND_HANDLER(IDC_USER_RADIO, BN_CLICKED, OnBnClickedUserRadio)
		COMMAND_HANDLER(IDC_SYSTEM_RADIO, BN_CLICKED, OnBnClickedSystemRadio)
		COMMAND_HANDLER(IDC_VARIABLES_COMBO, CBN_SELCHANGE, OnCbnSelchangeVariablesCombo)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_MOVE_UP_BUTTON, BN_CLICKED, OnBnClickedMoveUpButton)
		COMMAND_HANDLER(IDC_MOVE_DOWN_BUTTON, BN_CLICKED, OnBnClickedMoveDownButton)
		NOTIFY_HANDLER(IDC_PATHS_LIST, LVN_ITEMCHANGED, OnLvnItemchangedPathsList)
		COMMAND_HANDLER(IDC_REMOVE_BUTTON, BN_CLICKED, OnBnClickedRemoveButton)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		using namespace std;
		CheckRadioButton(IDC_SYSTEM_RADIO, IDC_SYSTEM_RADIO, IDC_SYSTEM_RADIO);
		LoadEnvironmentEntries();

		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));
		listView.AddColumn(L"Path", 0);
		listView.SetColumnWidth(0, -2);

		return 0;
	}

	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		using namespace std;

		CComboBox cb(GetDlgItem(IDC_VARIABLES_COMBO));

		int sel = cb.GetCurSel();
		wchar_t envVariableName[255];

		cb.GetLBText(sel, envVariableName);

		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));

		wstring paths;

		for (int j = 0; j < listView.GetItemCount(); j++)
		{
			wchar_t buffer[_MAX_PATH];
			listView.GetItemText(j, 0, buffer, sizeof(buffer) / sizeof(wchar_t));

			if (paths.length())
			{
				paths.append(L";");
			}

			paths.append(buffer);
		}

		CRegKey regKey;
		GetRegKey(regKey);

		regKey.SetStringValue(envVariableName, paths.c_str());

		ULONG dwReturnValue = 0;

		SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", 
			SMTO_ABORTIFHUNG, 5000, (PDWORD_PTR)&dwReturnValue);

		EndDialog(IDOK);
		return 0;
	}

	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(IDCANCEL);
		return 0;
	}

	LRESULT OnBnClickedUserRadio(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));
		listView.DeleteAllItems();
		LoadEnvironmentEntries();
		return 0;
	}

	LRESULT OnBnClickedSystemRadio(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));
		listView.DeleteAllItems();
		LoadEnvironmentEntries();
		return 0;
	}

	LRESULT OnCbnSelchangeVariablesCombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		using namespace std;

		CComboBox cb(GetDlgItem(IDC_VARIABLES_COMBO));

		int sel = cb.GetCurSel();
		wchar_t envVariableName[255];

		cb.GetLBText(sel, envVariableName);

		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));

		listView.DeleteAllItems();

		CRegKey regKey;
		GetRegKey(regKey);

		wchar_t value[1024];
		ULONG size = sizeof(value) / sizeof(wchar_t);

		if (0 == regKey.QueryStringValue(envVariableName, value, &size))
		{
			wchar_t* path = 0;
			wchar_t* p = value;
			int item = 0;

			while (0 != (path = wcstok(p, L";")))
			{
				listView.AddItem(item++, 0, path);
				p = 0;
			}

			for (size_t j = 0; j < files.size(); j++)
			{
				listView.AddItem(item++, 0, files[j].c_str());
			}
		}

		return 0;
	}

private:
	FileCollection& files;

	Environment GetEnvironment()
	{
		if (IsDlgButtonChecked(IDC_SYSTEM_RADIO))
		{
			return SYSTEM;
		}
		else
		{
			return USER;
		}
	}

	void GetRegKey(CRegKey& key)
	{
		switch (GetEnvironment())
		{
		case USER:
			key.Open(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
			break;
		case SYSTEM:
			key.Open(HKEY_CURRENT_USER, L"Environment");
			break;
		}
	}

	void LoadEnvironmentEntries()
	{
		CRegKey key;
		GetRegKey(key);

		DWORD index = 0;
		DWORD valueNameLength = 255;

		wchar_t valueName[255];

		CComboBox cb(GetDlgItem(IDC_VARIABLES_COMBO));
		cb.ResetContent();

		LONG result;

		while (ERROR_NO_MORE_ITEMS != (result = RegEnumValue(key.m_hKey, index++, valueName, &valueNameLength, 0, 0, 0, 0)))
		{
			cb.AddString(valueName);
			valueNameLength = 255;
		}
	}

	LRESULT OnBnClickedMoveUpButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));
		int itemIndex = listView.GetSelectedIndex();

		if (itemIndex > 0)
		{
			TCHAR buffer[_MAX_PATH];
			listView.GetItemText(itemIndex, 0, buffer, _MAX_PATH);
			listView.InsertItem(itemIndex - 1, buffer);
			listView.DeleteItem(itemIndex + 1);
			listView.SelectItem(itemIndex - 1);
			EnableButtons();
		}

		return 0;
	}

	LRESULT OnBnClickedMoveDownButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));
		int itemIndex = listView.GetSelectedIndex();

		if (itemIndex + 1 < listView.GetItemCount())
		{
			TCHAR buffer[_MAX_PATH];
			listView.GetItemText(itemIndex, 0, buffer, _MAX_PATH);
			listView.InsertItem(itemIndex + 2, buffer);
			listView.DeleteItem(itemIndex);
			listView.SelectItem(itemIndex + 1);
			EnableButtons();
		}

		return 0;
	}

	void EnableButtons()
	{
		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));
		UINT selectedItemCount = listView.GetSelectedCount();
		int itemIndex = listView.GetSelectedIndex();
		int itemCount = listView.GetItemCount();

		::EnableWindow(GetDlgItem(IDC_MOVE_UP_BUTTON), selectedItemCount == 1 && itemIndex != 0);
		::EnableWindow(GetDlgItem(IDC_MOVE_DOWN_BUTTON), selectedItemCount == 1 && itemIndex + 1 < itemCount);
		::EnableWindow(GetDlgItem(IDC_REMOVE_BUTTON), selectedItemCount == 1);
	}

	LRESULT OnLvnItemchangedPathsList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
	{
		EnableButtons();

		return 0;
	}

	LRESULT OnBnClickedRemoveButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		using namespace std;

		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));

		UINT selectedItemCount = listView.GetSelectedCount();

		int nextItem = -1;

		// This code is really for multiple selection list views. Since this list view
		//  is single selection, it's overkill...
		std::vector<int> deletedItems;

		for (UINT j = 0; j < selectedItemCount; j++)
		{
			nextItem = listView.GetNextItem(nextItem, LVNI_ALL | LVNI_SELECTED);

			if (nextItem != -1)
			{
				deletedItems.push_back(nextItem);
			}
		}

		for (int j = (int)deletedItems.size() - 1; j >= 0; j--)
		{
			listView.DeleteItem(deletedItems[j]);
		}

		// Select either the item at the same index or the item before it.
		int nextSelectedItem = max(0, min(listView.GetItemCount() - 1, deletedItems[0]));

		listView.SelectItem(nextSelectedItem);

		EnableButtons();

		return 0;
	}
};