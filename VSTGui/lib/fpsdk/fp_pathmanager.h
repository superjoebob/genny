#pragma once

#include <cstdint>

const int PathBufferLengthConst = 1024;
const char PathScanDirExtSeparatorConst = '|';

typedef char TPathBuffer [PathBufferLengthConst]; // http://ruimourato.com/2015/08/21/c-typedef-arrays.html

typedef uint8_t TPathIndex;
typedef uint32_t TPathIndexList;
typedef uint32_t TPathIdentifier;

const TPathIdentifier NoPathIDConst			= 0;
const TPathIndexList NoPathIndexConst		= 255;
const TPathIndexList AnyPathIndexListConst	= 0;

typedef uint32_t TPathOptions;

const TPathOptions PO_SearchAllPaths	= 1 << 0; // if not specified, it will only scan dirs or look for a file in the first existing folder

typedef uint32_t TAddPathOptions;

const TAddPathOptions APO_CanCreateDir			= 1 << 0;
const TAddPathOptions APO_CanCreateDirForNoPath	= 1 << 1;
const TAddPathOptions APO_DontCheckIfExists		= 1 << 2;

typedef uint32_t TScanDirOptions;

const TScanDirOptions SDO_SortList				= 1 << 0;
const TScanDirOptions SDO_Relative				= 1 << 1;
const TScanDirOptions SDO_NoExt					= 1 << 2;
const TScanDirOptions SDO_Recurse				= 1 << 3;
const TScanDirOptions SDO_CheckIfAny			= 1 << 4;
const TScanDirOptions SDO_CheckHidden			= 1 << 5;
const TScanDirOptions SDO_AddPathIndexToTag		= 1 << 6; // add path index to tags in the list ... use DecomposeSearchDirListTag to separate path index and list index again afterwards
const TScanDirOptions SDO_UsePathIndexAsTag		= 1 << 7; // tags in the list will be the path indexes ... no list index included and no need to use DecomposeSearchDirListTag

typedef uint32_t TLocateFileFlags;

const TLocateFileFlags LFF_32And64Bit	= 0;
const TLocateFileFlags LFF_32BitOnly	= 1;
const TLocateFileFlags LFF_64BitOnly	= 2;

const TLocateFileFlags LFF_BitSizeMaskConst	= 3;

class IScanDirCallback
{
public:
    virtual void __stdcall ItemFound(int32_t Index, TPathIndex PathIndex, char* FileName, intptr_t& ItemTag) = 0;
};

class ISearchDirCallback 
{
public:
    virtual void __stdcall ItemFound(TPathIndex PathIndex, char* Name, char* Path, int32_t Attributes, double TimeStamp, BOOL& Stop) = 0;
};

class ISearchDirStringList
{
public:
    virtual void __stdcall Add(char* SetItem, intptr_t SetTag) = 0;
    virtual void __stdcall Sort() = 0;
};

typedef BOOL (__stdcall *TPMLFProgressEvent)();

class IPathManager
{
public:
    // get the path for a specific ID and index
    virtual BOOL __stdcall GetPath(TPathIdentifier ID, TPathIndex Index, char* Buffer) = 0;
    // get the path index that corresponds to a full filename or path
    virtual TPathIndex __stdcall GetPathIndexFor(TPathIdentifier ID, char *FileName) = 0;

    // look for all files with the provided extension in all specified path types
    virtual BOOL __stdcall ScanDir(TPathIdentifier ID, char* SubPath, char* ValidExt, ISearchDirStringList* List, TScanDirOptions Options, TPathIndexList PathIndexes, IScanDirCallback* Callback) = 0;
    // search through the specified paths for files that match the filter, using FindFirst/FindNext
    virtual void __stdcall SearchDir(TPathIdentifier ID, char* SubPath, char* FileMask, int32_t Attributes, ISearchDirCallback* Callback, TPathIndexList PathIndexes, TScanDirOptions Options) = 0;
    // search a file in all subdirs of the specified path types
    virtual BOOL __stdcall Locate(TPathIdentifier ID, char* FileName, TPMLFProgressEvent ProgressEvent, TLocateFileFlags Flags, TPathIndexList PathIndexes, char *Buffer) = 0;
    // find a random file in the specified location
    virtual void __stdcall RandomDirFile(TPathIdentifier ID, char* SubPath, char* ValidExt, TScanDirOptions Options, TPathIndexList PathIndexes, double Value, char* Buffer) = 0; // Value can be -1 to be ignored
    // look for the exact filename or folder (optionally with subpath) in the specified paths and return the full filename
    virtual BOOL __stdcall AddPath(TPathIdentifier ID, char* FileName, TPathIndexList PathIndexes, TAddPathOptions Options, char* Buffer) = 0;
    // look for the filename or folder, but only return it if FileName.FileExt exists
    virtual BOOL __stdcall AddPathWithExt(TPathIdentifier ID, char* FileName, char* FileExt, TPathIndexList PathIndexes, TAddPathOptions Options, char* Buffer) = 0;
    // remove the specified path from a filename or folder
    virtual void __stdcall RemovePath(TPathIdentifier ID, char* FileName, char* Buffer) = 0;
    // break a filename up into path id, path index and relative filename, if it's in one of the paths managed by this object
    // the Context parameter is used to differentiate paths that are in the root path, like the effect and plugin settings paths
    virtual BOOL __stdcall DecomposeFileName(char* FileName, char* Context, TPathIdentifier& ID, TPathIndex& Index, char* Buffer) = 0;

    // get value of a root path
    virtual BOOL __stdcall GetRootPath(TPathIndex PathIndex, uint32_t Index, char* Buffer) = 0;

    // get the path id and index that corresponds to a full filename or path
    // the Context parameter is used to differentiate paths that are in the root path, like the effect and plugin settings paths
    virtual void __stdcall GetPathIDAndIndexFor(char* FileName, char* Context, TPathIdentifier& OutID, TPathIndex& OutIndex) = 0;
};

TPathIndexList AddPathIndexToList(TPathIndex Index, TPathIndexList List);
TPathIndexList RemovePathIndexFromList(TPathIndex Index, TPathIndexList List);
bool IsPathIndexInList(TPathIndex Index, TPathIndexList List);
TPathIndexList ConvertPathIndexToList(TPathIndex Index);
intptr_t ComposeSearchDirListTag(TPathIndex PathIndex, int ListIndex);
void DecomposeSearchDirListTag(intptr_t Tag, TPathIndex& PathIndex, int& ListIndex);
