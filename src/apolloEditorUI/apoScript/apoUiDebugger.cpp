/* file apoUiDebugger.cpp
*
* debug helper of ui
*
* create by duan
*
* 2017.5.11
*/

#include "apoUiDebugger.h"

// 
// DeguggerScriptOwner::DeguggerScriptOwner()
// {
// }
// 
// DeguggerScriptOwner::~DeguggerScriptOwner()
// {
// }
// 
// bool DeguggerScriptOwner::getOtherObject(const char*objName, LogicDataObj &val)
// {
// 	if (0 == ndstricmp(objName, "LogFile")) {
// 		val.InitSet("ndlog.log");
// 		return true;
// 	}
// 	else if (0 == ndstricmp(objName, "LogPath")) {
// 		val.InitSet("../../log");
// 		return true;
// 	}
// 	else if (0 == ndstricmp(objName, "WritablePath")) {
// 		val.InitSet("../../log");
// 		return true;
// 	}
// 
// 	else if (0 == ndstricmp(objName, "SelfName")) {
// 		val.InitSet("apoDebugger");
// 		return true;
// 	}
// 	else if (0 == ndstricmp(objName, "self")) {
// 		val.InitSet((void*)this, OT_OBJ_BASE_OBJ);
// 		return true;
// 	}
// 
//     bool ret = _myBase::getOtherObject(objName, val);
// 	if (ret) {
// 		return ret;
// 	}
// 	return false;
// }

//////////////////////////////////////////////////////////////////////////

DebuggerQObj::DebuggerQObj()
{
}

DebuggerQObj::~DebuggerQObj()
{
}


//////////////////////////////////////////////////////////////////////////


ApoDebugger::ApoDebugger()
{
}

ApoDebugger::~ApoDebugger()
{
}



bool ApoDebugger::onEnterStep(const char *func, const char *node)
{
	m_stepFunc = func;
	m_stepNode = node;
	emit m_obj.stepSignal(m_stepNode.c_str(), m_stepNode.c_str());
	return true;
}

void ApoDebugger::onTerminate()
{
	emit m_obj.terminateSignal();
}
void ApoDebugger::onCommandOk()
{
	emit m_obj.commondOkSignal();
}


void ApoDebugger::onScriptRunOk()
{
	emit m_obj.scriptRunOKSignal();
}
extern QString  bp_console_input();

bool ApoDebugger::onConsoleInput(std::string &text)
{
	QString qs1 = bp_console_input();
	if (qs1.size()) {
		text = qs1.toStdString();
		return true;
	}
	return false;
}



