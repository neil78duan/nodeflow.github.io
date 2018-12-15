/* file machoLoader.cpp
 * 
 * loader of Mach-O file 
 *
 * create by duan 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <mach-o/swap.h>
#include <sys/stat.h>
#include "machoLoader.h"
#include "nfsection_def.h"
//#include "nd_common/nd_common.h"


int bin_to_exe(void *data, size_t s, const char *outputFile, const char* machoTemplFile)
{
	MachoLoader loader;
	if(s >= HUGE_SECTION) {
		return -1 ;
	}
	else if(s > COMMON_SECTION) {
		std::string fileName = machoTemplFile ;
		fileName += "_huge" ;
		
		if(!loader.LoadFile(fileName.c_str())){
			fprintf(stderr,"load file %s error\n",fileName.c_str()) ;
			return -1 ;
		}
	}
	
	else if(s > COMMON_SECTION) {
		std::string fileName = machoTemplFile ;
		fileName += "_tiny" ;
		
		if(!loader.LoadFile(fileName.c_str())){
			fprintf(stderr,"load file %s error\n",fileName.c_str()) ;
			return -1 ;
		}
	}
	else {
		if(!loader.LoadFile(machoTemplFile)){
			fprintf(stderr,"load file %s error\n",machoTemplFile) ;
			return -1 ;
		}
	}
	
	size_t datasize = 0 ;
	char *p = loader.GetSectionData("__DATA", "__nfcode",datasize);
	if(!p) {
		return -1 ;
	}

	if(data && s >0) {
		if(s > datasize + 16) {
			return -1;
		}
		size_t *psize =(size_t *) p ;
		*psize = s ;
		memcpy(p + 16, data, s) ;
	}
	if( loader.SaveFile(outputFile) ) {
		chmod(outputFile,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH) ;
		return 0 ;
	}
	return -1;
}
////////////////////

nfSegmentOfMacho::nfSegmentOfMacho() :m_seg(0),m_org_addr(0),m_org_length(0)
{
}
nfSegmentOfMacho::~nfSegmentOfMacho()
{
	if(m_seg) {
		//free(m_seg);
	}
}
void nfSegmentOfMacho::dump()
{
	if(LC_SEGMENT_64==m_seg->header.cmd) {
		uint64_t offset ;
		segment_command_64 *pcmd = (segment_command_64*)m_seg ;
		offset =sizeof(segment_command_64) ;
		fprintf(stderr, "segment %s addr=%p size=%lld fileoffset=%lld\n",pcmd->segname,pcmd->vmaddr,pcmd->cmdsize,pcmd->fileoff);
		for (uint32_t i=0; i<pcmd->nsects; i++) {
			section_64 *psec = (section_64*) (((char*)pcmd) + offset) ;
			offset += sizeof(section_64) ;//psec->size ;
			
			fprintf(stderr, "section %s offset=%d, align=%d addr=%p,size=%lld\n", psec->sectname,psec->offset,psec->align, psec->addr, psec->size) ;
		}
	}
	else if(LC_SEGMENT==m_seg->header.cmd) {
		segment_command *pcmd = (segment_command*)m_seg ;
		uint32_t offset ;
		offset =sizeof(segment_command_64) ;
		fprintf(stderr, "segment %s addr=%d size=%x fileoffset=%x\n",pcmd->segname,pcmd->vmaddr,pcmd->cmdsize,pcmd->fileoff);
		for (uint32_t i=0; i<pcmd->nsects; i++) {
			section *psec = (section*) ((char*)pcmd + offset) ;
			offset += psec->size ;
			
			fprintf(stderr, "section %s offset=%d, align=%d addr=%d,size=%x\n", psec->sectname,psec->offset,psec->align, psec->addr, psec->size) ;
		}
	}
	else {
		fprintf(stderr, "segment id= %d size=%d\n", m_seg->header.cmd, m_seg->header.cmdsize) ;
	}
}

const char *nfSegmentOfMacho::getName()
{
	segment_command_64 *pcmd = (segment_command_64*)m_seg ;
	return pcmd->segname ;
}

void *nfSegmentOfMacho::getSectionData(const char *sectionName,size_t &size)
{
	if(LC_SEGMENT_64==m_seg->header.cmd) {
		segment_command_64 *pcmd = (segment_command_64*)m_seg ;
		uint64_t offset =sizeof(segment_command_64) ;
		for (uint32_t i=0; i<pcmd->nsects; i++) {
			section_64 *psec = (section_64*) (((char*)pcmd) + offset) ;
			offset += sizeof(section_64) ;//psec->size ;
			if(0==strcmp(psec->sectname,sectionName)) {
				size = psec->size ;
				return m_org_addr + psec->offset ;
			}
		}
	}
	else if(LC_SEGMENT==m_seg->header.cmd) {
		segment_command *pcmd = (segment_command*)m_seg ;
		uint32_t offset =sizeof(segment_command) ;
		for (uint32_t i=0; i<pcmd->nsects; i++) {
			section *psec = (section*) (((char*)pcmd) + offset) ;
			offset += sizeof(section) ;//psec->size ;
			if(0==strcmp(psec->sectname,sectionName)) {
				size_t dataoffset = psec->offset - pcmd->fileoff - offset ;
				size = psec->size ;
				return (char*)m_seg + dataoffset ;
			}
		}
	}
	return NULL;
}

int nfSegmentOfMacho::Load(char * ps,bool is64)
{
	m_seg = (nf_macho_segment*)ps ;
	return m_seg->header.cmdsize ;
	
}

///////////////////
MachoLoader::MachoLoader()
{
	
	m_org_addr =0;
	m_org_length =0;
	
	m_is64 =false;
	m_isfat = false;
	m_isswap = false ;
	
	m_segment_num = 0;
	//m_section_num = 0;
	
	memset(&m_header, 0,sizeof(m_header)) ;
	//memset(&m_segments, 0,sizeof(m_segments)) ;
	//memset(&m_sections, 0,sizeof(m_sections)) ;
	//memset(&m_sectionData, 0,sizeof(m_sectionData)) ;
}

MachoLoader::~MachoLoader()
{
	if(m_org_addr) {
		free(m_org_addr) ;
	}
}

int MachoLoader::loadSegmentCmd(char * ps)
{
	char *input_addr = ps ;
	for (uint32_t i=0; i<m_header.header64.ncmds; ++i) {
		m_segments[m_segment_num].init(m_org_addr, m_org_length);
		int ret = m_segments[m_segment_num].Load(ps,m_is64) ;
		
		if(ret > 0) {
			m_segments[m_segment_num].dump();
			++m_segment_num ;
			ps += ret ;
		}
		else {
			return -1 ;
		}
	}

	return (int)(input_addr - ps);
}

int MachoLoader::loadMachoHeader(FILE *pfStream)
{
	//read magic
	uint32_t magic = 0 ;
	size_t ret = fread(&magic,1,sizeof(magic),pfStream) ;
	if(ret <= 0) {
		fclose(pfStream) ;
		return 0 ;
	}
	if(magic == MH_MAGIC_64 || magic == MH_CIGAM_64) {
		m_is64 = true ;
	}
	if(magic == FAT_MAGIC || magic == FAT_CIGAM) {
		m_isfat = true ;
	}
	if(magic == MH_CIGAM || magic == MH_CIGAM_64 || magic == FAT_CIGAM) {
		m_isswap = true ;
	}
	
	fseek(pfStream,0, SEEK_SET) ;
	if(m_is64) {
		fread(&m_header,1,sizeof(mach_header_64),pfStream) ;
		if(m_isswap){
			swap_mach_header_64(&m_header.header64,NX_UnknownByteOrder) ;
		}
		return sizeof(mach_header_64) ;
	}
	else {
		fread(&m_header,1,sizeof(mach_header),pfStream) ;
		if(m_isswap){
			swap_mach_header(&m_header.header,NX_UnknownByteOrder) ;
		}
		return sizeof(mach_header) ;
	}
}

bool MachoLoader::LoadFile(const char *filePath)
{
	FILE *pfStream = fopen(filePath,"r") ;
	
	if(!pfStream) {
		return false ;
	}
	int head_size =loadMachoHeader(pfStream) ;
	if(!head_size) {
		fclose(pfStream) ;
		return false;
	}
	
	fseek(pfStream, 0, SEEK_END);
	m_org_length = ftell(pfStream);
	fseek(pfStream, 0, SEEK_SET);
	
	
	m_org_addr =(char*) malloc(m_org_length +1) ;
	
	if(!m_org_addr){
		fclose(pfStream) ;
		return false  ;
	}
	fread(m_org_addr,1, m_org_length, pfStream) ;
	
	m_org_addr[m_org_length] = 0 ;
	fclose(pfStream) ;
	if(-1==loadSegmentCmd(m_org_addr + head_size) ) {
		fclose(pfStream) ;
		return false ;
	}
	
	return true ;
}


bool MachoLoader::SaveFile(const char *filePath)
{
	FILE *pf = fopen(filePath, "wb");
	if(!pf) {
		return -1 ;
	}
	fwrite(m_org_addr, 1,m_org_length , pf) ;
	fclose(pf) ;
	return true ;
}

struct section* MachoLoader::AddNewSection(const char *segmentName, const char *sectionName, size_t size, void *data)
{
	return NULL;
}
char *MachoLoader::GetSectionData(const char *segmentName, const char *sectionName,size_t &size)
{
	for (int i=0; i<m_segment_num; ++i) {
		const char *pName = m_segments[i].getName() ;
		if(pName && 0==strcmp(pName,segmentName)) {
			void *p = m_segments[i].getSectionData(sectionName,size) ;
			return (char*) p ;
		}
	}
	return NULL ;
}





