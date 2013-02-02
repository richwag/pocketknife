// ContextMenuHandler.h : Declaration of the CContextMenuHandler

#pragma once
#include "resource.h"       // main symbols
#include "PocketKnife.h"
#include <shlobj.h>
#include <comdef.h>
#include <string>
#include <vector>
#include "FileCollection.h"
#include "ContextMenuItem.h"
#include "ConfigurationOptions.h"

struct __declspec(uuid("000214e4-0000-0000-c000-000000000046")) 
IContextMenu; 

_COM_SMARTPTR_TYPEDEF(IContextMenu, __uuidof(IContextMenu)); 

// CContextMenuHandler

class ATL_NO_VTABLE CContextMenuHandler : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CContextMenuHandler, &CLSID_ContextMenuHandler>,
	public IShellExtInit,
	public IContextMenu,
	public IDispatchImpl<IContextMenuHandler, &IID_IContextMenuHandler, &LIBID_PocketKnifeLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CContextMenuHandler()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_CONTEXTMENUHANDLER)

	BEGIN_COM_MAP(CContextMenuHandler)
		COM_INTERFACE_ENTRY(IContextMenuHandler)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IShellExtInit)
		COM_INTERFACE_ENTRY(IContextMenu)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		ConfigurationOptions::DestinationPaths().SerializeFromRegistry();
		return S_OK;
	}

	void FinalRelease() 
	{
	}

public:
	STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, IDataObject* pdtobj, HKEY hkeyProgID)
	{
		using namespace std;

		FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stg = { TYMED_HGLOBAL };
		HDROP hdrop;

		if (FAILED(pdtobj->GetData(&fmt, &stg)))
		{
			return E_INVALIDARG;
		}

		hdrop = (HDROP)GlobalLock(stg.hGlobal);

		if (NULL == hdrop)
		{
			return E_INVALIDARG;
		}

		fileList.clear();
		bool result = FileCollection::CreateFromHDrop(fileList, hdrop);

		GlobalUnlock(stg.hGlobal);
		ReleaseStgMedium(&stg);

		return result ? S_OK : E_INVALIDARG;
	} 

	STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT* pwReserverd, LPSTR pszName, UINT cchMax)
	{
		using namespace std;

		if (uType & GCS_HELPTEXT)
		{
			if (uType & GCS_UNICODE)
			{
				if (idCmd >= 0 && idCmd < (int)menuItemChoices.size())
				{
					ContextMenuItemPtr& item = menuItemChoices[idCmd];
					lstrcpynW((LPWSTR)pszName, item->GetHelpString(), cchMax);
					return S_OK;
				}
			}
		}

		return E_INVALIDARG;
	}

	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici)
	{
		using namespace std;

		if (0 != HIWORD(lpici->lpVerb))
		{
			return E_INVALIDARG;
		}

		int id = LOWORD(lpici->lpVerb);

		if (id >= 0 && id < (int)menuItemChoices.size())
		{
			ContextMenuItemPtr& item = menuItemChoices[id];
			item->ExecuteCommand(lpici);
			return S_OK;
		}

		return E_INVALIDARG;
	}

	STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
	{
		if (uFlags & CMF_DEFAULTONLY)
		{
			return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
		}

		ContextMenuItemCollection allMenuItems;
		GetMenuItems(allMenuItems, idCmdFirst);

		menuItemChoices.clear();
		HMENU popupMenu = CreatePopupMenu(allMenuItems, menuItemChoices);

		InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_POPUP, (UINT_PTR)popupMenu, L"Pocket Knife");

		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, menuItemChoices.size());
	}

private:
	ContextMenuItemCollection menuItemChoices;
	FileCollection fileList;

	// Gets the collection of all menu items used for building the context menu.
	void GetMenuItems(ContextMenuItemCollection& menuItems, UINT& idCmdFirst)
	{
		using namespace std;

		menuItems.push_back(ContextMenuItemPtr(new CopyPathToClipboardMenuItem(idCmdFirst++, fileList)));
		menuItems.push_back(ContextMenuItemPtr(new CopyFileToClipboardMenuItem(idCmdFirst++, fileList)));
		menuItems.push_back(ContextMenuItemPtr(new AddPathMenuItem(idCmdFirst++, fileList)));
		menuItems.push_back(ContextMenuItemPtr(new ConfigurationMenuItem(idCmdFirst++)));
		menuItems.push_back(ContextMenuItemPtr(new AddToEnvironmentMenuItem(idCmdFirst++, fileList)));
		menuItems.push_back(ContextMenuItemPtr(new SeparatorMenuItem()));
		idCmdFirst++;

		switch (ConfigurationOptions::GetFormat())
		{
		case ConfigurationOptions::Short:
			{
				bool moveKeyDown = GetKeyState(ConfigurationOptions::GetMoveKey()) < 0;
				bool exploreKeyDown = GetKeyState(ConfigurationOptions::GetExploreKey()) < 0;

				if (moveKeyDown)
				{
					AddCopyToMoveToDestinationPaths(menuItems, true, true, idCmdFirst);
				}
				else if (exploreKeyDown)
				{
					AddExploreDestinationPaths(menuItems, true, idCmdFirst);
				}
				else
				{
					AddCopyToMoveToDestinationPaths(menuItems, true, false, idCmdFirst);
				}
			}
			break;
		case ConfigurationOptions::Long:
			ContextMenuItemPtr copyMenu(new SubMenuMenuItem(0, L"Copy To", L"Copy selected files here"));
			AddCopyToMoveToDestinationPaths(dynamic_cast<SubMenuMenuItem*>(copyMenu.get())->GetSubItems(), false, false, idCmdFirst);
			menuItems.push_back(copyMenu);

			ContextMenuItemPtr moveMenu(new SubMenuMenuItem(0, L"Move To", L"Move selected files here"));
			AddCopyToMoveToDestinationPaths(dynamic_cast<SubMenuMenuItem*>(moveMenu.get())->GetSubItems(), false, true, idCmdFirst);
			menuItems.push_back(moveMenu);

			ContextMenuItemPtr exploreMenu(new SubMenuMenuItem(0, L"Explore", L"Move selected files here"));
			AddExploreDestinationPaths(dynamic_cast<SubMenuMenuItem*>(exploreMenu.get())->GetSubItems(), false, idCmdFirst);
			menuItems.push_back(exploreMenu);
			break;
		}

	}

	void AddCopyToMoveToDestinationPaths(ContextMenuItemCollection& menuItems, bool prefixPathName, 
		bool move, UINT& idCmdFirst)
	{
		using namespace std;

		vector<wstring>::iterator destPaths = ConfigurationOptions::DestinationPaths().Iterator();

		wstring verb = move ? L"Move " : L"Copy ";
		wstring prompt = verb + L"selected file(s) here.";

		wstring pathNamePrefix;

		if (prefixPathName)
		{
			pathNamePrefix += verb + L"To ";
		}

		for (size_t j = 0; j < ConfigurationOptions::DestinationPaths().Count(); j++)
		{
			const wstring& destPath = *destPaths++;
			ContextMenuItemPtr menuItem(new CopyToDestinationMenuItem(idCmdFirst++, pathNamePrefix + destPath, prompt, 
				fileList, destPath, move));
			menuItems.push_back(menuItem);
		}

		if (menuItems.size() != 0)
		{
			++idCmdFirst;
			menuItems.push_back(ContextMenuItemPtr(new SeparatorMenuItem()));
		}

		ContextMenuItemPtr menuItem(new CopyToDestinationMenuItem(idCmdFirst++, pathNamePrefix + L"Browse...", prompt, 
			fileList, L"", move));
		menuItems.push_back(menuItem);
	}

	void AddExploreDestinationPaths(ContextMenuItemCollection& menuItems, bool prefixPathName, UINT& idCmdFirst)
	{
		using namespace std;

		vector<wstring>::iterator destPaths = ConfigurationOptions::DestinationPaths().Iterator();

		wstring pathNamePrefix;

		if (prefixPathName)
		{
			pathNamePrefix += L"Explore ";
		}

		for (size_t j = 0; j < ConfigurationOptions::DestinationPaths().Count(); j++)
		{
			wstring& destPath = *destPaths++;
			ContextMenuItemPtr menuItem(new ExploreMenuItem(idCmdFirst++, pathNamePrefix + destPath, destPath));
			menuItems.push_back(menuItem);
		}
	}

	HMENU CreatePopupMenu(ContextMenuItemCollection& menuItems, ContextMenuItemCollection& calledItems)
	{
		using namespace std;

		HMENU popupMenu = ::CreatePopupMenu();

		vector<ContextMenuItemPtr>::iterator i = menuItems.begin();

		while (i != menuItems.end())
		{
			ContextMenuItemPtr& item = *i;

			if (typeid(*item) == typeid(SeparatorMenuItem))
			{
				AppendMenu(popupMenu, MF_SEPARATOR | MF_BYPOSITION, 0, 0);

				// Notice that we copy the item to the calledItems collections.
				//  Items in this collection are the ones that will be used in an
				//  InvokeCommand or GetCommandString call.
				calledItems.push_back(item);
			}
			else if (typeid(*item) == typeid(SubMenuMenuItem))
			{
				SubMenuMenuItem* subMenu = dynamic_cast<SubMenuMenuItem*>(item.get());
				HMENU subPopupMenu = CreatePopupMenu(subMenu->GetSubItems(), calledItems);
				InsertMenu(popupMenu, GetMenuItemCount(popupMenu), MF_BYPOSITION | MF_POPUP, (UINT_PTR)subPopupMenu, subMenu->GetMenuText());
			}
			else
			{
				UINT flags = MF_BYPOSITION;
				if (item->IsEnabled() == false)
				{
					flags |= MF_DISABLED | MF_GRAYED;
				}

				AppendMenu(popupMenu, flags, item->GetMenuId(), item->GetMenuText());

				// Again, save the item for use later in an InvokeCommand or GetCommandString
				//  call.
				calledItems.push_back(item);
			}
			++i;
		}

		return popupMenu;
	}
};

OBJECT_ENTRY_AUTO(__uuidof(ContextMenuHandler), CContextMenuHandler)
