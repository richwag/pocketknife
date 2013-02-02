#pragma once

#include "FileCollection.h"
#include <shellapi.h>

class CopyFileUtility
{
public:
	static void CopyFile(const std::wstring& destPath, const FileCollection& fileList, bool move)
	{
		using namespace std;
		//using namespace boost;

		shared_ptr<wchar_t> sourceBuffer(FileCollection::CreateMultiString(fileList));

		FileCollection destPathCollection;
		destPathCollection.push_back(destPath);

		shared_ptr<wchar_t> destBuffer(FileCollection::CreateMultiString(destPathCollection));

		SHFILEOPSTRUCT sfo;
		memset(&sfo, 0, sizeof(sfo));
		sfo.wFunc = move ? FO_MOVE : FO_COPY;
		sfo.pFrom = sourceBuffer.get();
		sfo.pTo = destBuffer.get();
		sfo.fFlags = 0;

		SHFileOperation(&sfo);
	}
};

