#include "source/c/fp_plugclass.h"
#include "fp_pathmanager.h"

TPathIndexList AddPathIndexToList(TPathIndex Index, TPathIndexList List) { return (List | (1 << Index)); };
TPathIndexList RemovePathIndexFromList(TPathIndex Index, TPathIndexList List) { return (List & ~(1 << Index)); };
bool IsPathIndexInList(TPathIndex Index, TPathIndexList List) { return (List & (1 << Index) == (1 << Index)); };
TPathIndexList ConvertPathIndexToList(TPathIndex Index)
{
if (Index == NoPathIndexConst)
return 0;
else
return (1 << Index);
};

intptr_t ComposeSearchDirListTag(TPathIndex PathIndex, int ListIndex) { return (((PathIndex & 0xFF) << 24) | (ListIndex & 0xFFFFFF)); };
void DecomposeSearchDirListTag(intptr_t Tag, TPathIndex& PathIndex, int& ListIndex)
{
PathIndex = ((Tag >> 24) & 0xFF);
ListIndex = (Tag & 0xFFFFFF);
};