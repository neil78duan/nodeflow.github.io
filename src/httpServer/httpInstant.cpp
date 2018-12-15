/*
* file HttpInstant.h
*
* instant of gm server
*
* create by duan
*
* 2017. 7.8
*/

#include "httpServer/httpScriptApi.h"
#include "httpServer/db_mysql_api.h"
#include "httpInstant.h"


HttpInstant::HttpInstant()
{
	
}

HttpInstant::~HttpInstant()
{
}

void HttpInstant::OnCreate()
{	
	ND_TRACE_FUNC() ;
}


void HttpInstant::OnDestroy()
{
	ND_TRACE_FUNC() ;
}

void HttpInstant::OnInitilize()
{
	ND_TRACE_FUNC() ;
	GetDeftListener()->setScriptEngine((void*)m_scriptObj.getScriptHandler(), NULL);
	EnableAlarm(true);	
	
	if (-1 == ReadPathInfo(m_config_name)) {
		error_exit("read config error %s\n" AND config_file);
	}

	if (-1 == LoadScriptData()) {
		error_exit("load game data error\n");
	}
	m_scriptObj.SendEvent0(1);	

}

void HttpInstant::OnClose()
{
	ND_TRACE_FUNC() ;

	m_scriptObj.SendEvent0(2);
	EnableAlarm(false);
	
}

int HttpInstant::setHttpHandler(const char*name, http_reqeust_func func)
{
	NDHttpListener *h = (NDHttpListener*)GetDeftListener();
	nd_assert(h);
	h->installRequest_c(name, func);
	return 0;
}

int HttpInstant::UpdateMinute()
{
	return 0;
}


int	HttpInstant::UpdateSecond()
{
	ND_TRACE_FUNC();

	return 0;
}




int HttpInstant::UpdateFrame(ndtime_t tminterval)
{
	ND_TRACE_FUNC() ;
	ndtime_t now = nd_time() ;
	time_t date_time = app_inst_time( NULL ) ;
	
	return 0 ;
}

int HttpInstant::ReadPathInfo(const char *configname)
{
	ND_TRACE_FUNC();
	int ret = -1;
	ndxml *xmlroot, *xmlnode,*xmlSub;
	ndxml_root xmlfile;
	const char *pVal = NULL;
	char tmpbuf[ND_FILE_PATH_SIZE];
	//const char *priv_keyfile ,*pub_keyfile;
	ret = ndxml_load(config_file, &xmlfile);
	if (0 != ret) {
		nd_logfatal("ReadConfig(%s) %s\n", config_file, nd_last_error());
		return -1;
	}

	xmlroot = ndxml_getnode(&xmlfile, "root");
	if (!xmlroot) {
		ret = -1;
		nd_logfatal(" xml %s file root-node not exist\n", config_file);
		goto READ_EXIT;
	}
	xmlnode = ndxml_getnode(xmlroot, "path_setting");
	if (!xmlnode) {
		ret = -1;
		nd_logfatal(" xml %s file root-node not exist\n", config_file);
		goto READ_EXIT;
	}


#define GET_VAL_XML(_name)	\
	pVal =NULL ;					\
	xmlSub = ndxml_refsub(xmlnode, _name);	\
	if (xmlSub) {					\
		pVal = ndxml_getval(xmlSub) ;\
	}

	GET_VAL_XML("home_path");
	if (pVal) {
		nd_chdir(pVal);
		m_webHome = pVal;
	}
	
	GET_VAL_XML("download_path");
	if (pVal) {
		GetMyListener()->setReadablePath(pVal);
	}
	GET_VAL_XML("upload_path");
	if (pVal) {
		GetMyListener()->setWritablePath(pVal);
	}
	GET_VAL_XML("main_script");
	if (pVal) {
		m_scriptFile= nd_full_path(NULL,pVal, tmpbuf,sizeof(tmpbuf));
	}
	

	ret = 0;
READ_EXIT:
	ndxml_destroy(&xmlfile);
	if (-1 == ret) {
		nd_logfatal("load xml from file");
	}
	return ret;
}

int HttpInstant::LoadScriptData()
{
	LogicEngineRoot *script = LogicEngineRoot::get_Instant();

	if (-1 == script->LoadScript(m_scriptFile.c_str(), m_scriptObj.getScriptHandler())) {
		nd_logerror("LOAD game script error\n");
		return -1;
	}
	if (CheckIsDeveVer()) {
		script->getGlobalDebugger().runHost();
	}

	nfHttpListener *httpListener =(nfHttpListener *) GetDeftListener();
	if (httpListener) {
		httpListener->setPath(m_webHome.c_str(), m_webHome.c_str());
	}
	return 0;
}
