
#pragma once

#define STRICT

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0501
#define _RICHEDIT_VER 0x0300

#define _ATL_APARTMENT_THREADED
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_USE_CSTRING_FLOAT
#define _ATL_USE_DDX_FLOAT
#define _ATL_NO_OLD_NAMES
#define _ATL_DISABLE_DEPRECATED
#define _ATL_ALL_WARNINGS
#define _ATL_NO_OLD_HEADERS_WIN64

// WTL related
#define _WTL_NEW_PAGE_NOTIFY_HANDLERS
#define _WTL_NO_WTYPES
#define _WTL_NO_CSTRING

//#include "resource.h"

// ATL
#include <atlbase.h>
#include <atlstr.h>
#include <atlsync.h>
#include <atlcoll.h>
#include <atltypes.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>

// WTL
#include <atlapp.h>
//extern CAppModule _Module;

#define _Module (*_pModule)
#include <atlmisc.h>
#include <atlctl.h>
#include <atlframe.h>
#include <atlddx.h>
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlscrl.h>
#undef _Module

using namespace ATL;
using namespace WTL;
