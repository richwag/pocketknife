#pragma once

#include "stdafx.h"
#include "ConfigurationOptions.h"
#include "resource.h"

class OptionsPropertyPage 
	: public CPropertyPageImpl<OptionsPropertyPage>
{
public:
	enum { IDD = IDD_OPTIONSDLG };

	class ModifierKey
	{
	public:
		ModifierKey(const wchar_t* keyName, int keyCode) 
			: keyName(keyName), keyCode(keyCode)
		{
		}

		bool operator == (const ModifierKey& other) const
		{
			return other.keyCode == keyCode;
		}

		std::wstring keyName;
		int keyCode;
	};

	typedef std::vector<ModifierKey> KeyCollection;

	OptionsPropertyPage(void)
	{
	}

	virtual ~OptionsPropertyPage(void)
	{
	}

	BEGIN_MSG_MAP(OptionsPropertyPage)
		COMMAND_HANDLER(IDC_LONG_MENU_RADIO, BN_CLICKED, OnBnClickedLongMenuRadio)
		COMMAND_HANDLER(IDC_SHORT_MENU_RADIO, BN_CLICKED, OnBnClickedShortMenuRadio)
		COMMAND_HANDLER(IDC_MOVE_KEY_COMBO, CBN_SELCHANGE, OnCbnSelchangeMoveKeyCombo)
		COMMAND_HANDLER(IDC_EXPLORE_KEY_COMBO, CBN_SELCHANGE, OnCbnSelchangeExploreKeyCombo)
		CHAIN_MSG_MAP(CPropertyPageImpl<OptionsPropertyPage>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		using namespace std;

		if (ConfigurationOptions::GetFormat() == ConfigurationOptions::Long)
		{
			CheckRadioButton(IDC_LONG_MENU_RADIO, IDC_SHORT_MENU_RADIO, IDC_LONG_MENU_RADIO);
		}
		else
		{
			CheckRadioButton(IDC_LONG_MENU_RADIO, IDC_SHORT_MENU_RADIO, IDC_SHORT_MENU_RADIO);
		}

		CComboBox moveComboBox(GetDlgItem(IDC_MOVE_KEY_COMBO));
		CComboBox exploreComboBox(GetDlgItem(IDC_EXPLORE_KEY_COMBO));

		KeyCollection allKeys = GetPossibleModifierKeys();

		int exploreKey = ConfigurationOptions::GetExploreKey();
		int moveKey = ConfigurationOptions::GetMoveKey();

		for (size_t j = 0; j < allKeys.size(); j++)
		{
			ModifierKey key = allKeys[j];

			int itemIndex = moveComboBox.AddString(key.keyName.c_str());
			moveComboBox.SetItemData(itemIndex, key.keyCode);

			if (key.keyCode == moveKey)
			{
				moveComboBox.SetCurSel(itemIndex);
			}

			itemIndex = exploreComboBox.AddString(key.keyName.c_str());
			exploreComboBox.SetItemData(itemIndex, key.keyCode);

			if (key.keyCode == exploreKey)
			{
				exploreComboBox.SetCurSel(itemIndex);
			}
		}

		return 0;
	}

	void EnableControls()
	{
	}

	int OnApply()
	{
		CButton longMenuButton(GetDlgItem(IDC_LONG_MENU_RADIO));
		CButton shortMenuButton(GetDlgItem(IDC_SHORT_MENU_RADIO));

		if (longMenuButton.GetCheck() == BST_CHECKED)
		{
			ConfigurationOptions::SetFormat(ConfigurationOptions::Long);
		}
		else
		{
			ConfigurationOptions::SetFormat(ConfigurationOptions::Short);
		}

		CComboBox moveComboBox(GetDlgItem(IDC_MOVE_KEY_COMBO));
		CComboBox exploreComboBox(GetDlgItem(IDC_EXPLORE_KEY_COMBO));

		int selectedMoveKeyIndex = moveComboBox.GetCurSel();
		int selectedExploreKeyIndex = exploreComboBox.GetCurSel();

		if (selectedMoveKeyIndex == selectedExploreKeyIndex)
		{
			MessageBox(L"Please select different modifier keys.", L"Error", MB_OK | MB_ICONERROR);
			return -1;
		}

		int moveKey = static_cast<int>(moveComboBox.GetItemData(selectedMoveKeyIndex));
		int exploreKey = static_cast<int>(exploreComboBox.GetItemData(selectedExploreKeyIndex));

		ConfigurationOptions::SetMoveKey(moveKey);
		ConfigurationOptions::SetExploreKey(exploreKey);

		return 0;
	}

	LRESULT OnBnClickedLongMenuRadio(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add your control notification handler code here
		SetModified(true);
		EnableControls();

		return 0;
	}

	LRESULT OnBnClickedShortMenuRadio(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add your control notification handler code here
		SetModified(true);
		EnableControls();

		return 0;
	}

private:
	KeyCollection GetAvailableModifierKeys(int keyToExclude)
	{
		using namespace std;

		KeyCollection result;

		KeyCollection possibleKeys = GetPossibleModifierKeys();

		for (size_t j = 0; j < possibleKeys.size(); j++)
		{
			if (possibleKeys[j].keyCode != keyToExclude)
			{
				result.push_back(possibleKeys[j]);
			}
		}

		return result;
	}

	KeyCollection GetPossibleModifierKeys()
	{
		using namespace std;

		static KeyCollection possibleKeys;

		if (possibleKeys.size() == 0)
		{
			possibleKeys.push_back(ModifierKey(L"Left Control", VK_LCONTROL));
			possibleKeys.push_back(ModifierKey(L"Right Control", VK_RCONTROL));
			possibleKeys.push_back(ModifierKey(L"Left Shift", VK_LSHIFT));
			possibleKeys.push_back(ModifierKey(L"Right Shift", VK_RSHIFT));
		}

		return possibleKeys;
	}

	LRESULT OnCbnSelchangeMoveKeyCombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SetModified(TRUE);
		return 0;
	}

	LRESULT OnCbnSelchangeExploreKeyCombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SetModified(TRUE);
		return 0;
	}
};