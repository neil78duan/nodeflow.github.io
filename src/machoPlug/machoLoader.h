/* file machoLoader.h
 * 
 * loader of Mach-O file 
 *
 * create by duan 
 */

#ifndef _NODEFLOW_MACHO_LOADER_H_
#define _NODEFLOW_MACHO_LOADER_H_

#include <mach-o/getsect.h>
#include <mach-o/loader.h>
#include <stdio.h>

#define MAX_SEGMENT_NUM 32

struct nf_macho_segment
{
	load_command header ;
	char data[0] ;
};

class nfSegmentOfMacho
{
	
	char *m_org_addr ;
	size_t m_org_length ;
	
	nf_macho_segment *m_seg ;
public:
	nfSegmentOfMacho() ;
	~nfSegmentOfMacho() ;
	void init(char *begin, size_t size) {m_org_addr=begin; m_org_length=size;}
	const char *getName() ;
	void *getSectionData(const char *sectionName,size_t &size) ;
	int Load(char * ps,bool is64) ;
	void dump() ;
};
class MachoLoader
{
	char *m_org_addr ;
	size_t m_org_length ;
	
	union {
		struct mach_header header ;
		struct mach_header_64 header64 ;
	}m_header ;
	
	bool m_is64 ;
	bool m_isfat ;
	bool m_isswap ;
	int m_segment_num ;
	
	nfSegmentOfMacho m_segments[MAX_SEGMENT_NUM] ;
	
	int loadSegmentCmd(char * ps) ;
	int loadMachoHeader(FILE *ps) ;
public:
	MachoLoader() ;
	virtual ~MachoLoader() ;
	
	bool LoadFile(const char *filePath) ;
	bool SaveFile(const char *filePath) ;
	
	struct section* AddNewSection(const char *segmentName, const char *sectionName, size_t size, void *data) ;
	char *GetSectionData(const char *segmentName, const char *sectionName,size_t &size);
};
#endif
