#pragma once

#include "stdafx.h"
#include "resource.h"
#include "ConfigurationOptions.h"

class PathsPropertyPage;

// Class used for subclassing list view to hook its WM_DROPFILES
//  messages so we can forward it the property page.
class SubclassedListView : public CWindowImpl<SubclassedListView>
{
public:
	SubclassedListView()
	{

	}

	~SubclassedListView()
	{
	}

	void SetPropertyPage(PathsPropertyPage* page)
	{
		this->page = page;
	}

	LRESULT OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	BEGIN_MSG_MAP(SubclassedListView)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
	END_MSG_MAP()

private:
	PathsPropertyPage* page;
};

class PathsPropertyPage 
	: public CPropertyPageImpl<PathsPropertyPage>
{
public:
	enum { IDD = IDD_CONFIGURATIONDLG };

	PathsPropertyPage(void)
	{
	}

	virtual ~PathsPropertyPage(void)
	{
	}

	BEGIN_MSG_MAP(PathsPropertyPage)
		NOTIFY_HANDLER(IDC_PATHS_LIST, LVN_ITEMCHANGED, OnLvnItemchangedPathsList)
		COMMAND_HANDLER(IDC_REMOVE_BUTTON, BN_CLICKED, OnBnClickedRemoveButton)
		COMMAND_HANDLER(IDC_ADD_BUTTON, BN_CLICKED, OnBnClickedAddButton)
		COMMAND_HANDLER(IDC_MOVE_UP_BUTTON, BN_CLICKED, OnBnClickedMoveUpButton)
		COMMAND_HANDLER(IDC_MOVE_DOWN_BUTTON, BN_CLICKED, OnBnClickedMoveDownButton)
		CHAIN_MSG_MAP(CPropertyPageImpl<PathsPropertyPage>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
	END_MSG_MAP()

	LRESULT OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		using namespace std;

		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));

		FileCollection droppedFiles;

		if (FileCollection::CreateFromHDrop(droppedFiles, reinterpret_cast<HDROP>(wParam)))
		{
			for (size_t j = 0; j < droppedFiles.size(); j++)
			{
				wstring& file = droppedFiles[j];

				if (PathIsDirectory(file.c_str()))
				{
					listView.AddItem(listView.GetItemCount(), 0, file.c_str());
				}
			}
		}

		return 0;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		using namespace std;

		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));
		listView.AddColumn(L"Path", 0);

		FileCollection::iterator i = ConfigurationOptions::DestinationPaths().Iterator();

		for (int j = 0; j < (int)ConfigurationOptions::DestinationPaths().Count(); j++)
		{
			wstring& file = *i;
			listView.AddItem(j, 0, file.c_str());
			i++;
		}

		listView.SetColumnWidth(0, LVSCW_AUTOSIZE);

		subclassedListView.SetPropertyPage(this);
		subclassedListView.SubclassWindow(GetDlgItem(IDC_PATHS_LIST));

		EnableButtons();

		return TRUE;
	}

	int OnApply()
	{
		using namespace std;

		CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));

		int itemCount = listView.GetItemCount();

		wchar_t buffer[_MAX_PATH];

		ConfigurationOptions::DestinationPaths().Clear();

		for (int j = 0; j < itemCount; j++)
		{
			listView.GetItemText(j, 0, buffer, _MAX_PATH);
			ConfigurationOptions::DestinationPaths().AddPath(wstring(buffer));
		}

		ConfigurationOptions::DestinationPaths().SerializeToRegistry();

	
		// PSNRET_NOERROR = apply OK
		// PSNRET_INVALID = apply not OK, return to this page
		// PSNRET_INVALID_NOCHANGEPAGE = apply not OK, don't change focus
		return PSNRET_NOERROR;
	}

	LRESULT OnLvnItemchangedPathsList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
	{
		EnableButtons();

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
		::EnableWindow(GetDlgItem(IDC_REMOVE_BUTTON), selectedItemCount > 0);
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
			SetModified(TRUE);
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
			SetModified(TRUE);
		}

		return 0;
	}

	LRESULT OnBnClickedAddButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		using namespace std;

		CFolderDialog folderDialog(this->m_hWnd, L"Add Destination", BIF_USENEWUI | BIF_SHAREABLE | BIF_RETURNONLYFSDIRS);

		if (IDOK == folderDialog.DoModal(this->m_hWnd))
		{
			wstring path = folderDialog.GetFolderPath();

			CListViewCtrl listView(GetDlgItem(IDC_PATHS_LIST));

			// TODO: Check for duplicates first
			int itemIndex = listView.AddItem(listView.GetItemCount(), 0, path.c_str());
			listView.SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
			listView.EnsureVisible(itemIndex, false);
			SetModified(TRUE);
			EnableButtons();
		}

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

		SetModified(TRUE);

		EnableButtons();

		return 0;
	}

private:
	SubclassedListView subclassedListView;
};
