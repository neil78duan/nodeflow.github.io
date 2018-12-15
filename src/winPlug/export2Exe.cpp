/* file export2Exe.cpp
 *
 * exort script bin to exe 
 *
 * create by duan 
 * all rights reserved 
 *
 * 2018.11.21
 */

#include "nd_common/nd_common.h"
#include "logic_parser/logicApi4c.h"
#include <windows.h>
#include <imagehlp.h>//#include <Dbghelp.h>
#include "pelib.h"

int bin_to_exe(void *data, size_t s, const char *outputFile, const char* peTemplFile)
{
	CPELibrary pelib;
	if (!pelib.OpenFile(peTemplFile)) {
		nd_logerror("load file %s error \n", peTemplFile);
		return -1;
	}
	PIMAGE_SECTION_HEADER pOrgDataHdr = pelib.GetSectionHeader(".nfcode");
	if (!pOrgDataHdr) {
		nd_logerror("file %s format error, is not PE format \n", peTemplFile);
		return -1;
	}
	PCHAR pSecData = pelib.GetSectionData(".nfcode");
	nd_assert(pSecData);

	PIMAGE_SECTION_HEADER newHeader = pelib.AddNewSection(".nfcode2", (DWORD)s, data);
	if (!newHeader) {
		nd_logerror("add new section error \n");
		return -1;
	}
	size_t *pSize = (size_t *) pSecData;

	size_t offset = newHeader->VirtualAddress - (pOrgDataHdr->VirtualAddress + 16);
	*pSize = (offset ^OFFSET_MASK);

	++pSize;
	*pSize = s;

	return pelib.SaveFile(outputFile)? 0 : -1;
}