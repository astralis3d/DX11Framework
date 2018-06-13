#ifndef __pch_h__
#define __pch_h__

#ifdef _MSC_VER
# pragma once
#endif

#pragma warning(disable: 4100)				// Unreferenced formal parameter
#pragma warning(disable: 4189)				// Local variable is initialized but not referenced

// For detecting memory leaks
#if defined (_DEBUG) | defined (DEBUG)
# define _CRTDBG_MAP_ALLOC
# include <crtdbg.h>
#endif

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#ifndef VC_EXTRALEAN
# define VC_EXTRALEAN
#endif



// Define this to enable renderdoc support
#define USE_RENDERDOC

#define VSYNC 1



// Enable auto linking with necessary DXUT libraries.
#define DXUT_AUTOLIB

#include "BaseTypes.h"

#include <windows.h>
#include <commdlg.h>

#include <DXUT.h>

#include "mathlib.h"
#include "Utility.h"
#include "Timer.h"

#include "D3DStates.h"
#include "D3DConstantBuffer.h"
#include "D3DShaders.h"
#include "D3DTypes.h"

// C++ library includes
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <ctime>

#endif