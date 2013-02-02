#pragma once

class ModuleInfoUtility
{
public:
	static std::wstring GetCopyright()
	{
		using namespace std;
		//using namespace boost;

		wstring result;

		wstring fileName = GetModuleFileName();

		DWORD dw, size;

		if (size = GetFileVersionInfoSize((LPWSTR)fileName.c_str(), &dw))
		{
			shared_ptr<BYTE> mem(new BYTE[size]);
			BYTE* info;
			UINT length;

			if (GetFileVersionInfo((LPWSTR)fileName.c_str(), NULL, size, (LPVOID)mem.get()) &&
				VerQueryValue(mem.get(), L"\\StringFileInfo\\040904b0\\LegalCopyright", (LPVOID*)&info, &length))
			{
				result = (wchar_t*)info;
			}
		}

		return result;
	}

	static std::wstring GetAppVersion()
	{
		using namespace std;
		//using namespace boost;

		wstring result;

		wstring fileName = GetModuleFileName();

		DWORD dw, size;

		if (size = ::GetFileVersionInfoSize((LPWSTR)fileName.c_str(), &dw))
		{
			shared_ptr<BYTE> mem(new BYTE[size]);
			BYTE* info;
			UINT length;

			if (::GetFileVersionInfo((LPWSTR)fileName.c_str(), NULL, size, (LPVOID)mem.get()) &&
				::VerQueryValue(mem.get(), L"\\StringFileInfo\\040904b0\\FileVersion", (LPVOID*)&info, &length))
			{
				result = (wchar_t*)info;
			}
		}

		return result;
	}

private:
	static std::wstring GetModuleFileName()
	{
		wchar_t fileName[_MAX_PATH];

		HINSTANCE instance = _AtlBaseModule.GetModuleInstance();
		::GetModuleFileName(instance, fileName, _MAX_PATH);

		return fileName;
	}
};