#ifndef __pch_common_h__
#define __pch_common_h__

#ifdef _MSC_VER
# pragma once
#endif

//----------------------------------------------------------------------

#pragma warning(disable: 4100)				// Unreferenced formal parameter
#pragma warning(disable: 4189)				// Local variable is initialized but not referenced

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#ifndef VC_EXTRALEAN
# define VC_EXTRALEAN
#endif

#include "BaseTypes.h"

#include <windows.h>
#include <DXUT.h>

#include "mathlib.h"
#include "Utility.h"

// C++ library includes
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cstdio>
#include <cstdarg>

#endif