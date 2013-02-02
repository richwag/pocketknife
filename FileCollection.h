#pragma once

#include <string>
#include <vector>
#include <shlobj.h>

class FileCollection : public std::vector<std::wstring> 
{
public:
	static wchar_t* CreateMultiString(const FileCollection& fileList)
	{
		using namespace std;

		size_t totalLength = 0;
		
		// Calculate total buffer size.
		for (size_t j = 0; j < fileList.size(); j++)
		{
			totalLength += fileList[j].length() + 1;
		}

		++totalLength;

		// Build the buffer of the source files. Each name is appended to the buffer
		//  followed by a double null at the end. Could be its own class.
		wchar_t* buffer = new wchar_t[totalLength];

		memset(buffer, 0, totalLength * sizeof(wchar_t));

		size_t bufferPos = 0;

		for (size_t j = 0; j < fileList.size(); j++)
		{
			const wstring& file = fileList[j];
			
			for (size_t k = 0; k < file.length(); k++)
			{
				buffer[bufferPos++] = file[k]; 
			}

			++bufferPos;
		}

		return buffer;
	}

	static bool CreateFromHDrop(FileCollection& fileList, HDROP hdrop)
	{
		using namespace std;

		fileList.clear();

		UINT numFiles = DragQueryFile(hdrop, 0xffffffff, NULL, 0);

		if (numFiles == 0)
		{
			return false;
		}

		fileList.clear();
		wchar_t fileName[MAX_PATH];

		for (UINT j = 0; j < numFiles; j++)
		{
			DragQueryFile(hdrop, j, fileName, MAX_PATH);
			fileList.push_back(wstring(fileName));
		}

		return true;
	}
};