// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <Windows.h>
#include <tchar.h>
#include <WinSock2.h>

#include <string>
#include <vector>
#include <assert.h>
#include <sstream>
#include <stdint.h>

#include "TypeDefines.h"

#include <zlib.h>

#include "ObjectType.h"
#include "FileEnumerator.h"
#include "FileRepo.h"
#include "ObjectId.h"
#include "Utilities.h"
#include "File.h"
#include "MappedFile.h"
#include "QuickIndex.h"
#include "QuickIndexImpl.h"
#include "FastHash.h"
#include "ObjectDb.h"
#include "ObjectStreamImpl.h"
#include "FileBuffer.h"
#include "Sha1Hash.h"
#include "FileUtil.h"
#include "Version.h"
#include "ErrorCode.h"
#include "FileReadBuffer.h"
#include "FileRepoImpl.h"
