#pragma once

#include "stdafx.h"
#include "ModuleInfoUtility.h"

class AboutPropertyPage 
	: public CPropertyPageImpl<AboutPropertyPage>
{
public:
	enum { IDD = IDD_ABOUTDLG };

	AboutPropertyPage(void)
	{
	}

	virtual ~AboutPropertyPage(void)
	{
	}

	BEGIN_MSG_MAP(AboutPropertyPage)
		CHAIN_MSG_MAP(CPropertyPageImpl<AboutPropertyPage>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		using namespace std;
		wstring text;

		text += L"Pocket Knife " + ModuleInfoUtility::GetAppVersion() + L"\r\n\r\n";
		text += ModuleInfoUtility::GetCopyright() + L"\r\n\r\n";
		text += L"Written by: Richard Wagenknecht";

		SetDlgItemText(IDC_PROGRAM_INFO_STATIC, text.c_str());
		return 0;
	}
};