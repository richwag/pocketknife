#pragma once

#include "stdafx.h"
#include "PathsPropertyPage.h"
#include "AboutPropertyPage.h"
#include "OptionsPropertyPage.h"

class ConfigPropertySheet : public CPropertySheetImpl<ConfigPropertySheet>
{
public:
	ConfigPropertySheet(void)
	{
		AddPage(pathsPage);
		AddPage(optionsPage);
		AddPage(aboutPage);
		SetTitle(L"Pocket Knife Configuration");
	}

	virtual ~ConfigPropertySheet(void)
	{
	}

	BEGIN_MSG_MAP(PropertySheet)
		CHAIN_MSG_MAP(CPropertySheetImpl<ConfigPropertySheet>)
	END_MSG_MAP()

private:
	PathsPropertyPage pathsPage;
	AboutPropertyPage aboutPage;
	OptionsPropertyPage optionsPage;
};
