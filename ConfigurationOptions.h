#pragma once

#include "stdafx.h"
#include "FileCollection.h"
#include <memory>

class ConfigurationOptions
{
public:
	enum MenuFormat {Short, Long};

	static MenuFormat GetFormat() 
	{
		MenuFormat result = Long;

		CRegKey key;
		if (ERROR_SUCCESS == OpenRegKey(key))
		{
			DWORD format = result;

			if (ERROR_SUCCESS == key.QueryDWORDValue(L"MenuFormat", format))
			{
				if (format == Short || format == Long)
				{
					result = (MenuFormat)format;
				}
			}
		}

		return result;
	}

	static void SetFormat(MenuFormat format)
	{
		CRegKey key;
		if (ERROR_SUCCESS == OpenRegKey(key))
		{
			key.SetDWORDValue(L"MenuFormat", format);
		}
	}

	static int GetExploreKey()
	{
		int result = VK_CONTROL;

		CRegKey key;
		if (ERROR_SUCCESS == OpenRegKey(key))
		{
			DWORD d = result;

			if (ERROR_SUCCESS == key.QueryDWORDValue(L"ExploreKey", d))
			{
				result = static_cast<int>(d);
			}
		}

		return result;
	}

	static int GetMoveKey()
	{
		int result = VK_SHIFT;

		CRegKey key;
		if (ERROR_SUCCESS == OpenRegKey(key))
		{
			DWORD d = result;

			if (ERROR_SUCCESS == key.QueryDWORDValue(L"MoveKey", d))
			{
				result = static_cast<int>(d);
			}
		}

		return result;
	}

	static void SetMoveKey(int moveKey)
	{
		CRegKey key;
		if (ERROR_SUCCESS == OpenRegKey(key))
		{
			key.SetDWORDValue(L"MoveKey", moveKey);
		}
	}

	static void SetExploreKey(int exploreKey)
	{
		CRegKey key;
		if (ERROR_SUCCESS == OpenRegKey(key))
		{
			key.SetDWORDValue(L"ExploreKey", exploreKey);
		}
	}

	static long OpenRegKey(CRegKey& key)
	{
		return key.Create(HKEY_CURRENT_USER, L"Software\\Intellisys\\Pocketknife");
	}

	class DestinationPathCollection
	{
	public:
		void AddPath(const std::wstring& newPath)
		{
			using namespace std;

			for (size_t j = 0; j < paths.size(); j++)
			{
				if (paths[0] == newPath)
				{
					return;
				}
			}

			paths.push_back(newPath);
		}

		void Clear()
		{
			paths.clear();
		}

		FileCollection::iterator Iterator()
		{
			return paths.begin();
		}

		size_t Count() const
		{
			return paths.size();
		}

		void SerializeToRegistry()
		{
			//using namespace boost;

			std::shared_ptr<wchar_t> multiString(FileCollection::CreateMultiString(paths));

			CRegKey key;
			if (ERROR_SUCCESS == ConfigurationOptions::OpenRegKey(key))
			{
				key.SetMultiStringValue(L"DestinationPaths", multiString.get());
			}
		}

		void SerializeFromRegistry()
		{
			//using namespace boost;
			using namespace std;

			CRegKey key;
			if (ERROR_SUCCESS == ConfigurationOptions::OpenRegKey(key))
			{
				ULONG length = 0;

				if (ERROR_SUCCESS == key.QueryMultiStringValue(L"DestinationPaths", NULL, &length))
				{
					shared_ptr<wchar_t> buffer(new wchar_t[length]);

					if (ERROR_SUCCESS == key.QueryMultiStringValue(L"DestinationPaths", buffer.get(), &length))
					{
						--length;
						paths.clear();

						ULONG start = 0;

						for (ULONG j = 0; j < length; j++)
						{
							if (buffer.get()[j] == L'\0')
							{
								wstring newPath(buffer.get() + start);
								paths.push_back(newPath);
								start = j + 1;
							}
						}
					}
				}
			}
		}

		friend class ConfigurationOptions;

	private:
		FileCollection paths;

		DestinationPathCollection()
		{
		}
	};

	static DestinationPathCollection& DestinationPaths()
	{
		return destPaths;
	}

private:
	static DestinationPathCollection destPaths;

	ConfigurationOptions()
	{
	}
};