/*
 * file HttpUpdateInstant.h
 *
 * instant of gm server 
 *
 * create by duan
 *
 * 2018. 10.11
 */

#ifndef _HTTP_INSTANT_H_
#define _HTTP_INSTANT_H_


#include "ndapplib/applib.h"
#include "ndapplib/nd_httpListener.h"

#include "httpServer/httpScriptApi.h"
#include "httpServer/mydatabase.h"
#include "httpServer/db_mysql_api.h"
//#include "ServerPublicData.h"

#include <map>




class HttpInstant : public nd_instance<NDHttpSession, nfHttpListener>
{
public:
	HttpInstant();
	virtual ~HttpInstant();
	
protected:
	int ReadPathInfo(const char *configname);
	int LoadScriptData();

	virtual int UpdateFrame(ndtime_t tminterval);	//update per frame
	int	UpdateSecond();
	void OnInitilize() ;
	void OnClose() ;
	void OnCreate() ;			//call on create
	void OnDestroy() ;
	
	int UpdateMinute() ;		//update minute
		
public:
	int setHttpHandler(const char*name, http_reqeust_func func);
	
	std::string m_scriptFile;
	std::string m_webHome;
public:
	
	nfHttpScriptMgr m_scriptObj;

};

extern HttpInstant &get_instance();

#endif 

