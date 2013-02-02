#include "StdAfx.h"
#include "pathspropertypage.h"

LRESULT SubclassedListView::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return page->OnDropFiles(uMsg, wParam, lParam, bHandled);
}
