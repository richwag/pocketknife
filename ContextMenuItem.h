#pragma once
#include <shlobj.h>
#include <string>
#include <vector>
#include <shlwapi.h>
#include <fstream>
#include "ConfigurationOptions.h"
#include "CopyFileUtility.h"
#include "PropertySheet.h"
#include <shellapi.h>
#include "AddEnvironmentPathDialog.h"

class ContextMenuItem
{
public:
	ContextMenuItem(int menuId, const std::wstring& menuText, const std::wstring& helpString, FileCollection& fileList) 
		: menuId(menuId), menuText(menuText), helpString(helpString), fileList(fileList)
	{
	}

	ContextMenuItem(int menuId, const std::wstring& menuText, const std::wstring& helpString) 
		: menuId(menuId), menuText(menuText), helpString(helpString), fileList(emptyFileList)
	{
	}

	ContextMenuItem() : fileList(emptyFileList)
	{
	}

	int GetMenuId() const
	{
		return menuId;
	}

	const wchar_t* GetMenuText() const
	{
		return menuText.c_str();
	}

	virtual void ExecuteCommand(LPCMINVOKECOMMANDINFO lpici)
	{
		MessageBox(lpici->hwnd, L"Your command here", NULL, MB_OK);
	}

	virtual bool IsEnabled()
	{
		return true;
	}

	virtual const wchar_t* GetHelpString() const
	{
		return helpString.c_str();
	}

protected:
	FileCollection& fileList;
	static FileCollection emptyFileList;

private:
	int menuId;
	std::wstring menuText;
	std::wstring helpString;
};

typedef std::shared_ptr<ContextMenuItem> ContextMenuItemPtr;
typedef std::vector<ContextMenuItemPtr> ContextMenuItemCollection;

class SubMenuMenuItem : public ContextMenuItem
{
public:
	SubMenuMenuItem(int menuId, const std::wstring& menuText, const std::wstring& helpString) 
		: ContextMenuItem(menuId, menuText, L"")
	{
	}

	ContextMenuItemCollection& GetSubItems()
	{
		return subItems;
	}

private:
	ContextMenuItemCollection subItems;
};

class SeparatorMenuItem : public ContextMenuItem
{
public:
	SeparatorMenuItem()
	{
	}
};

class ConfigurationMenuItem : public ContextMenuItem
{
public:
	ConfigurationMenuItem(int menuId) 
		: ContextMenuItem(menuId, L"Configure...", L"Set PocketKnife Options")
	{
	}

	virtual void ExecuteCommand(LPCMINVOKECOMMANDINFO lpici)
	{
		ConfigPropertySheet sheet;
		sheet.DoModal();
	}
};

/*

*/
class CopyFileToClipboardMenuItem : public ContextMenuItem
{
public:
	CopyFileToClipboardMenuItem(int menuId, FileCollection& fileList)
		: ContextMenuItem(menuId, L"Copy Contents to Clipboard", L"Copy the contents of the selected file to the clipboard", fileList)
	{
	}

	virtual void ExecuteCommand(LPCMINVOKECOMMANDINFO lpici)
	{
		using namespace std;

		if (OpenClipboard(lpici->hwnd))
		{
			EmptyClipboard();

			string fileContents;

			for (size_t j = 0; j < fileList.size(); j++)
			{
				if (!PathIsDirectory(fileList[j].c_str()))
				{
					ifstream file;
					file.open(fileList[j].c_str(), ios::in|ios::binary|ios::ate);

					if (file.is_open())
					{
						ifstream::pos_type size = file.tellg();
						std::shared_ptr<char> memblock(new char [(unsigned int)size]);
						file.seekg (0, ios::beg);
						file.read (memblock.get(), (streamsize)size);
						file.close();

						fileContents.append(memblock.get(), (unsigned int)size);
					}
				}
			}

			if (fileContents.length())
			{
				size_t length = fileContents.length() + 1;
				HANDLE h = GlobalAlloc(GHND, length);
				void* memPtr = GlobalLock(h);
				memset(memPtr, '\0', length);
				memcpy(memPtr, fileContents.c_str(), length);
				GlobalUnlock(h);

				SetClipboardData(CF_TEXT, h);
			}

			CloseClipboard();
		}
	}
};

class AddToEnvironmentMenuItem : public ContextMenuItem
{
public:
	AddToEnvironmentMenuItem(int menuId, FileCollection& fileList) 
		: ContextMenuItem(menuId, L"Add To Environment...", L"Add path(s) to environment variable", fileList)
	{
	}

	virtual void ExecuteCommand(LPCMINVOKECOMMANDINFO lpici)
	{
		AddEnvironmentPathDialog dlg(fileList);
		dlg.DoModal();
	}
};

class CopyPathToClipboardMenuItem : public ContextMenuItem
{
public:
	CopyPathToClipboardMenuItem(int menuId, FileCollection& fileList)
		: ContextMenuItem(menuId, L"Copy Path(s) to Clipboard", L"Copy the path(s) of the selected file(s) to the clipboard", fileList)
	{
	}

	virtual void ExecuteCommand(LPCMINVOKECOMMANDINFO lpici)
	{
		using namespace std;

		if (OpenClipboard(lpici->hwnd))
		{
			wstring crlf(L"\r\n");

			EmptyClipboard();

			size_t totalLength = 0;

			for (size_t j = 0; j < fileList.size(); j++)
			{
				totalLength += fileList[j].length();
			}

			totalLength += (fileList.size() - 1) * crlf.length();
			++totalLength;
			totalLength *= sizeof(wchar_t);

			wstring allFileNames;

			for (size_t j = 0; j < fileList.size(); j++)
			{
				allFileNames.append(fileList[j]);

				if (j > 0)
				{
					allFileNames.append(crlf);
				}
			}

			HANDLE h = GlobalAlloc(GHND, totalLength);
			memcpy(GlobalLock(h), allFileNames.c_str(), totalLength);
			GlobalUnlock(h);

			SetClipboardData(CF_UNICODETEXT, h);

			CloseClipboard();
		}
	}
};

class AddPathMenuItem : public ContextMenuItem
{
public:
	AddPathMenuItem(int menuId, FileCollection& fileList) 
		: ContextMenuItem(menuId, L"Add Destination Path(s)", L"Adds path(s) as destinations for file operations", fileList)
	{
	}

	virtual void ExecuteCommand(LPCMINVOKECOMMANDINFO lpici)
	{
		for (size_t j = 0; j < fileList.size(); j++)
		{
			if (PathIsDirectory(fileList[j].c_str()))
			{
				ConfigurationOptions::DestinationPaths().AddPath(fileList[j]);
			}
		}

		ConfigurationOptions::DestinationPaths().SerializeToRegistry();
	}

	virtual bool IsEnabled()
	{
		bool result = false;

		for (size_t j = 0; j < fileList.size(); j++)
		{
			if (PathIsDirectory(fileList[j].c_str()))
			{
				result = true;
				break;
			}
		}

		return result;
	}
};

class CopyToDestinationMenuItem : public ContextMenuItem
{
public:
	CopyToDestinationMenuItem(int menuId, const std::wstring& menuText, const std::wstring& helpString, FileCollection& fileList, 
		const std::wstring& destPath, bool moveFiles) 
		: ContextMenuItem(menuId, menuText, helpString, fileList), moveFiles(moveFiles), destPath(destPath)
	{
	}

	virtual void ExecuteCommand(LPCMINVOKECOMMANDINFO lpici)
	{
		if (destPath.length() == 0)
		{
			CFolderDialog folderDialog(lpici->hwnd, L"Browse Destination", BIF_USENEWUI | BIF_SHAREABLE | BIF_RETURNONLYFSDIRS);

			if (IDOK == folderDialog.DoModal(lpici->hwnd))
			{
				destPath = folderDialog.GetFolderPath();
			}
		}

		if (destPath.length() != 0)
		{
			CopyFileUtility::CopyFile(destPath, fileList, moveFiles);
		}
	}

private:
	std::wstring destPath;
	bool moveFiles;
};

class ExploreMenuItem : public ContextMenuItem
{
public:
	ExploreMenuItem(int menuId, const std::wstring& menuText, const std::wstring& explorePath) 
		: ContextMenuItem(menuId, menuText, L"Explore folder"), explorePath(explorePath)
	{
	}

	virtual void ExecuteCommand(LPCMINVOKECOMMANDINFO lpici)
	{
		ShellExecute(lpici->hwnd, L"explore", explorePath.c_str(), NULL, explorePath.c_str(), SW_SHOWNORMAL);
	}
private:
	std::wstring explorePath;
};

