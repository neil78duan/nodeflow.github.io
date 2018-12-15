#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "apoScript/apoUiMainEditor.h"
#include "apoScript/apoXmlTreeView.h"
#include "apoScript/apoBaseExeNode.h"
#include "texteditorpopdlg.h"

//#include "logic_parser/dbl_mgr.h"
#include "logic_parser/logic_compile.h"
#include "logic_parser/logicEngineRoot.h"
#include "logic_parser/logic_editor_helper.h"
//#include "script_event_id.h"


#include "nd_common/nd_common.h"
#include "ndlib.h"
#include "ndlog_wrapper.h"
#include "runFuncDialog.h"
ND_LOG_WRAPPER_IMPLEMENTION(MainWindow);


#include <QTextEdit>
#include <QDockWidget>
#include <QTableWidget>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>



MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent), m_editorSetting(*apoEditorSetting::getInstant()), m_editorWindow(0),
m_fileRoot(0), m_curFile(0), m_currFunction(0), /*m_localDebugOwner(NULL),*/ m_filesWatcher(this),
    ui(new Ui::MainWindow)
{
	m_xmlSearchedRes = NULL;
	m_debugInfo = NULL;
	m_debuggerCli = NULL;
	m_isChangedCurFile = false;
    ui->setupUi(this);


    setWindowTitle(tr("Script editor"));
	
	m_editorWindow = new apoUiMainEditor(this, &m_clipboard);
	m_editorWindow->setAttribute(Qt::WA_DeleteOnClose, true);
	m_editorWindow->setObjectName(QString("MainEditor"));

	QObject::connect(m_editorWindow, SIGNAL(showExenodeSignal(apoBaseExeNode*)),
		this, SLOT(onShowExeNodeDetail(apoBaseExeNode *)));

	QObject::connect(m_editorWindow, SIGNAL(breakPointSignal(const char *, const char *, bool)),
		this, SLOT(onBreakPointEdited(const char *, const char *, bool)));

	QObject::connect(&m_filesWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(onFileChangedExternal(const QString &)));

	setCentralWidget(m_editorWindow);

	ND_LOG_WRAPPER_NEW(MainWindow);

	onDebugEnd();

	m_workingPath = nd_getcwd();

// 	char tmppath[ND_FILE_PATH_SIZE];
// 	if (nd_absolute_filename(m_editorSetting.getProjectConfig("net_data_def"), tmppath, sizeof(tmppath))) {
// 		m_netDataTypeFile = tmppath;
// 	}
	 
}

MainWindow::~MainWindow()
{
	if (m_debuggerCli)	{
		m_debuggerCli->cmdTerminate();
		delete m_debuggerCli;
	}
	closeCurFile();
	if (m_fileRoot){
		ndxml_save(m_fileRoot, m_fileRootPath.c_str());
		ndxml_destroy(m_fileRoot);
		m_fileRoot = 0;
	}
	if (m_xmlSearchedRes ){
		ndxml_free(m_xmlSearchedRes);
		m_xmlSearchedRes = NULL;
	}
    delete ui;

	ND_LOG_WRAPPER_DELETE(MainWindow);

	nd_chdir(m_workingPath.c_str());
}


bool MainWindow::myInit()
{
	apoEditorSetting *g_seting = apoEditorSetting::getInstant();
	m_fileTemplate = g_seting->getProjectConfig("new_template");
	m_confgiFile = g_seting->getConfigFileName();

	const char *toExeCmd = g_seting->getProjectConfig("compile_to_exe_cmd");
	if (toExeCmd) {
		SetCompileExeCmd(toExeCmd);
	}

	setFileRoot(g_seting->getProjectConfig("script_root"));
	showAllView();

	return true;
}

void MainWindow::ClearLog()
{
	//ui->LogText->clear();
	QDockWidget *pDock = this->findChild<QDockWidget*>("outputView");
	if (pDock){
		QTextEdit *pEdit = pDock->findChild<QTextEdit*>("logTextEdit");
		if (pEdit)	{
			pEdit->clear();
			pEdit->update();
		}
	}
	m_logText.clear();
	
}

void MainWindow::WriteLog(const char *logText)
{	
	QDockWidget *pDock = this->findChild<QDockWidget*>("outputView");
	if (pDock){
		QTextEdit *pEdit = pDock->findChild<QTextEdit*>("logTextEdit");
		if (pEdit)	{			
			pEdit->moveCursor(QTextCursor::End);
 			pEdit->insertPlainText(QString(logText));
			pEdit->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
			pEdit->update();
			return;		 
		}
	}
	m_logText.append(QString(logText));	
}

void MainWindow::showAllView()
{
	openFileView();
	openFunctionsView();
	openOutputView();
	//openDetailView();
	return;
}

bool MainWindow::openFileView()
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("FilelistView");
	if (pDock)	{
		return false;
	}

	pDock = new QDockWidget(tr("Projects"), this);
	pDock->setObjectName("FilelistView");
	pDock->setAttribute(Qt::WA_DeleteOnClose, true);

	apoXmlTreeView *subwindow = new apoXmlTreeView();
	subwindow->setAttribute(Qt::WA_DeleteOnClose, true);
	subwindow->setObjectName("FileListTree");

	QObject::connect(subwindow, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
		this, SLOT(onFilesTreeCurItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

	QObject::connect(subwindow, SIGNAL(xmlNodeDelSignal(ndxml *)),
		this, SLOT(onFilelistDel(ndxml *)));

	QObject::connect(subwindow, SIGNAL(xmlNodeDisplayNameChangeSignal(ndxml *)), this, SLOT(onFileModuleNameChanged(ndxml*)));

	if (m_fileRoot)	{
		subwindow->setAlias(&m_editorSetting.getAlias());
		ndxml *xmlFiles = ndxml_getnode(m_fileRoot, "script_file_manager");
		if (xmlFiles){
			subwindow->setXmlInfo(xmlFiles, 2, "Projects");
		}
		
	}

	pDock->setWidget(subwindow);
	addDockWidget(Qt::LeftDockWidgetArea, pDock);
	return true;

}
bool MainWindow::openFunctionsView()
{

	QDockWidget *pDock = this->findChild<QDockWidget*>("FunctionView");
	if (pDock){
		return false;
	}
	else {
		pDock = new QDockWidget(tr("Functions"), this);
		pDock->setObjectName("FunctionView");
		pDock->setAttribute(Qt::WA_DeleteOnClose, true);

		//function tree list
		apoXmlTreeView *subwindow = new apoXmlTreeView(NULL, &m_clipboard);
		subwindow->setAttribute(Qt::WA_DeleteOnClose, true);
		subwindow->setObjectName("functionsTree");

		//mask functon create
		subwindow->unCreateNewChild("func_node");
		subwindow->unCreateNewChild("msg_handler_node"); 
		subwindow->unCreateNewChild("event_callback_node");


		QObject::connect(subwindow, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
			this, SLOT(onFunctionsTreeCurItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

		QObject::connect(subwindow, SIGNAL(xmlNodeDelSignal(ndxml *)),
			this, SLOT(onXmlNodeDel(ndxml *)));

		QObject::connect(subwindow, SIGNAL(xmlNodeDisplayNameChangeSignal(ndxml *)), this, SLOT(onFunctionListChanged(ndxml*)));

		if (m_curFile)	{
			subwindow->setAlias(&m_editorSetting.getAlias());
			subwindow->setXmlInfo(m_curFile, 5, m_fileMoudleName.c_str());
		}

		pDock->setWidget(subwindow);
		addDockWidget(Qt::LeftDockWidgetArea, pDock);
	}
	return true;
}
bool MainWindow::openOutputView()
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("outputView");
	if (pDock){
		return false;
	}
	pDock = new QDockWidget(tr("Output"), this);
	pDock->setObjectName("outputView");
	pDock->setAttribute(Qt::WA_DeleteOnClose, true);

	QTextEdit *subwindow = new QTextEdit();

	subwindow->setAttribute(Qt::WA_DeleteOnClose, true);
	subwindow->setObjectName("logTextEdit");
	subwindow->setPlainText(m_logText);

	pDock->setWidget(subwindow);
	addDockWidget(Qt::BottomDockWidgetArea, pDock);
	return true;
}

bool MainWindow::openDetailView()
{
	onShowExeNodeDetail(NULL);
	return true;
}


const char *MainWindow::getScriptSetting(ndxml *scriptXml, const char *settingName)
{
	ndxml *moduleInfo = ndxml_getnode(scriptXml, "moduleInfo");
	if (moduleInfo){
		ndxml *node = ndxml_getnode(moduleInfo, settingName);
		if (node){
			return ndxml_getval(node);
		}
	}
	return NULL;
}


bool MainWindow::loadScriptFromModule(const char *moduleName)
{
	if (!moduleName || !*moduleName)	{
		return false;
	}
	ndxml *fileroot = ndxml_getnode(m_fileRoot, "script_file_manager");
	if (!fileroot){
		return false;		
	}

	for (int i = 0; i < ndxml_getsub_num(fileroot); i++) {
		ndxml *node = ndxml_getnodei(fileroot, i);
		const char *fileName = ndxml_getattr_val(node, "name");
		if (0 == ndstricmp(fileName, moduleName))	{
			m_fileMoudleName = ndxml_getattr_val(node, "name");
			//return loadScriptFile(ndxml_getval(node));
			return loadProject(node);
		}
	}
	return false;
}

bool MainWindow::loadProject(ndxml *projNode)
{
	std::string myPath = ndxml_getval(projNode);
	if (myPath.size() > 0) {
		myPath += "/";
	}
	const char *fileName =ndxml_getattr_val(projNode, "main_file");

	return loadProject(myPath.c_str(), fileName);
}

bool MainWindow::loadProject(const char *projectPath, const char*mainFile)
{
	char absPath[ND_FILE_PATH_SIZE];
	nd_chdir(m_workingPath.c_str());

	if (!nd_absolute_path(projectPath, absPath, sizeof(absPath))) {
		nd_logerror("get absulate path error %s\n",  projectPath);
		return false;
	}

	std::string myPath = absPath;

	if (!nd_full_path(myPath.c_str(), mainFile, absPath, sizeof(absPath))) {
		nd_logerror("make file path error \n" );
		return false;
	}

	if (!_loadProjectMainfile(absPath)) {
		nd_logerror("can not load script file %s \n", absPath);
		return false;
	}

	//change to project path 
	nd_chdir(myPath.c_str());

	LogicEngineRoot *root = LogicEngineRoot::get_Instant();
	root->getPlugin().DestroyPlugins(); 

	ndxml *pluginNode = ndxml_recursive_ref(m_curFile, "moduleInfo/plugin_list");
	if (pluginNode && ndxml_getval(pluginNode)) {
		const char *plgList = ndxml_getval(pluginNode);
		while (plgList) {
			plgList = ndstr_first_valid(plgList);
			if (!plgList) {
				break;
			}
			if (*plgList == ';') {
				++plgList;
			}

			char buf[1024];
			plgList = ndstr_parse_word_n(plgList, buf, (int)sizeof(buf));
#ifdef __ND_WIN__
			std::string pluginPath = "./plugins/";
			pluginPath += buf;
			pluginPath += ".dll";
#elif defined(__ND_MAC__)
			
			std::string pluginPath = "./plugins/lib";
			pluginPath += buf;
			pluginPath += ".dylib";
#else
			std::string pluginPath = "./plugins/lib";
			pluginPath += buf;
			pluginPath += ".so";
#endif 
			if (!root->getPlugin().loadPlugin(buf, pluginPath.c_str())) {
				nd_logerror("load %s plugin error\n", buf);
			}
		}
	}

	const char *filename = getScriptSetting(m_curFile, "user_define_enum");
	if (filename) {
		m_editorSetting.loadUserDefEnum(filename);
	}

	apoEditorSetting::getInstant()->removExtTemplates();
	if (nd_existfile("./setting/extTempl.xml")) {
		//load extend templates
		ndxml_root rootTempl;
		if (0 == ndxml_load_ex("./setting/extTempl.xml", &rootTempl, APO_QT_SRC_ENCODE_NAME)) {
			ndxml *tempNode = ndxml_getnode(&rootTempl, "extend_template");
			if (tempNode) {
				apoEditorSetting::getInstant()->addExtTemplates(tempNode);
			}
		}
		
	}
	//nd_chdir(workPath.c_str());
	//end load config file
	return true;
}

bool MainWindow::loadScriptFile(const char *scriptFile)
{
	char inputPath[1024];
	inputPath[0] = 0;
	if (!nd_getpath(scriptFile, inputPath, sizeof(inputPath))) {
		nd_logerror("can not get path %s\n", scriptFile);
		return false;
	}
	const char *fileName = nd_filename(scriptFile);
	if (!fileName) {
		nd_logerror("can not get file name %s\n", scriptFile);
		return false;
	}
	return loadProject(inputPath,fileName);
}

bool MainWindow::_loadProjectMainfile(const char *scriptFile)
{
	if (!scriptFile) {
		return false;
	}

	if (checkNeedSave()) {
		saveCurFile();
	}

	if (!m_filePath.empty()) {
		m_filesWatcher.removePath(m_filePath.c_str());
	}

	if (m_curFile) {
		ndxml_destroy(m_curFile);
		delete m_curFile;
		m_curFile = NULL;
	}

	LogicEngineRoot::get_Instant()->getGlobalDebugger().clearBreakpoint();

	MY_LOAD_XML_AND_NEW(m_curFile, scriptFile, m_editorSetting.m_encodeName.c_str(), return true);

	//load user define enum 
	m_currFunction = NULL;
	if (!openFunctionsView()) {
		showCurFile();
	}
	m_filePath = scriptFile;

	m_filesWatcher.addPath(scriptFile);
	return true;
}

bool MainWindow::showCurFile()
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("FunctionView");
	if (!pDock){
		return false;
	}

	apoXmlTreeView *tree = this->findChild<apoXmlTreeView*>("functionsTree");
	if (!tree)	{
		return false;
	}
	tree->clear();
	if (m_curFile) {
		tree->setXmlInfo(m_curFile, 5, m_fileMoudleName.c_str());
	}
	return true;
}

bool MainWindow::showFileslist()
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("FilelistView");
	if (!pDock){
		return false;
	}

	apoXmlTreeView *tree = this->findChild<apoXmlTreeView*>("FileListTree");
	if (!tree)	{
		return false;
	}
	
	ndxml *xmlFiles = ndxml_getnode(m_fileRoot, "script_file_manager");
	if (xmlFiles){
		tree->clear();
		tree->setXmlInfo(xmlFiles, 2, "Files");
	}

	return true;
}


void MainWindow::onFileChanged()
{
	m_isChangedCurFile = true;
	setCurFileSave(false);

	LogicEditorHelper::setScriptChangedTime(m_curFile, time(NULL));
}


void MainWindow::onFilelistDel(ndxml *xmlnode)
{
	const char *name = ndxml_getname(xmlnode);
	if (0==ndstricmp(name, "file") ){
		const char *path = ndxml_getval(xmlnode);
		if (path){
			nd_rmfile(path);
		}
	}
	if (xmlnode == m_curFile) {
		m_editorWindow->clearFunction();
		m_editorWindow->update();
		m_curFile = NULL;
	}
	ndxml_delxml(xmlnode, NULL);

	ndxml_save(m_fileRoot, m_fileRootPath.c_str());
}

void MainWindow::onXmlNodeDel(ndxml *xmlnode)
{
	m_isChangedCurFile = true;
	if (xmlnode == m_currFunction || (m_currFunction&&ndxml_get_parent(m_currFunction) == xmlnode))	{
		m_editorWindow->clearFunction();
	}
	ndxml_delxml(xmlnode, NULL);
	onFileChanged();
}


void MainWindow::onFunctionListChanged(ndxml *xmlnode)
{
	if (m_currFunction == xmlnode && m_editorWindow){
		m_editorWindow->onFuncNameChanged(xmlnode);
	}
}


void MainWindow::onFileModuleNameChanged(ndxml *xmlnode)
{
	const char *fileName = ndxml_getval(xmlnode);
	const char *moduleName = ndxml_getattr_val(xmlnode, "name");
	if (moduleName && *moduleName && fileName){

		if (0 == ndstricmp(m_filePath.c_str(), fileName)) {
			m_fileMoudleName = moduleName;
			ndxml *xml = ndxml_getnode(m_fileRoot, "script_file_manager");
			if (xml){
				ndxml_setattrval(xml, "last_edited", moduleName);
			}
			if (m_curFile){
				LogicEditorHelper::setModuleName(m_curFile, moduleName);
			}
			onFileChanged();
		}
		else {
			ndxml_root scriptFile;
			if (0==ndxml_load_ex(fileName,&scriptFile,"utf8") )	{
				LogicEditorHelper::setModuleName(&scriptFile, moduleName);
				ndxml_destroy(&scriptFile);
			}
		}

	}
}


void MainWindow::onDebugTerminate()
{
	EndDebug(true);
	this->update();
}
void MainWindow::onDebugStep(const char *func, const char* node)
{
	//nd_logdebug("recv step in %s : %s\n", func, node);
	showDebugNode(node);

	if (m_debugInfo){
		delete m_debugInfo;
		m_debugInfo = NULL;
	}
	if (m_debuggerCli)	{
		m_debugInfo = m_debuggerCli->getParserInfo();
		if (m_debugInfo)		{
			showDebugInfo(m_debugInfo);
		}
	}
	this->update();
}
void MainWindow::onDebugCmdAck()
{
	if (m_debugInfo){
		delete m_debugInfo;
		m_debugInfo = NULL;
	}
	if (m_debuggerCli)	{
		m_debugInfo = m_debuggerCli->getParserInfo();
		if (m_debugInfo)		{
			showDebugInfo(m_debugInfo);
		}
	}
	this->update();
}

void MainWindow::onScriptRunOK()
{
	nd_logdebug("script run completed\n");
	if (m_debuggerCli)	{
		m_debugInfo = m_debuggerCli->getParserInfo();
		if (m_debugInfo)		{
			showDebugInfo(m_debugInfo);
		}
	}
	m_editorWindow->setDebugNode(NULL);
	this->update();
}


void MainWindow::onFileChangedExternal(const QString &path)
{
	QString text = QString::asprintf("The file %s was changed, do want to reload?", m_filePath.c_str());
	QMessageBox::StandardButton ret = QMessageBox::question(this, "File changed", text);
	if (ret == QMessageBox::Yes)	{
		LogicEngineRoot::get_Instant()->getGlobalDebugger().clearBreakpoint();
		MY_LOAD_XML_AND_NEW(m_curFile, m_filePath.c_str(), m_editorSetting.m_encodeName.c_str(), return);
		m_currFunction = NULL;
		if (!openFunctionsView()) {
			showCurFile();
		}
	}
	
}

bool MainWindow::setCurrSelectedFunction(ndxml *xml)
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("FunctionView");
	if (!pDock){
		return false;
	}

	apoXmlTreeView *tree = this->findChild<apoXmlTreeView*>("functionsTree");
	if (!tree)	{
		return false;
	}
	tree->setSelected(xml);
	return true;
}
bool MainWindow::showCurFunctions()
{	
	m_editorWindow->preShowFunction();
	bool ret = m_editorWindow->showFunction(m_currFunction, m_curFile);
	if (ret == false){
		m_editorWindow->clearFunction();
		QMessageBox::warning(NULL, "Error", "Can not show current Function!", QMessageBox::Ok);
		m_currFunction = NULL;
	}
	return ret ;
}

bool MainWindow::saveCurFile()
{ 
	m_isChangedCurFile = false;

	m_filesWatcher.removePath(m_filePath.c_str());

	if (m_curFile){
// 		ndxml *module = ndxml_getnode(m_curFile, "moduleInfo");
// 		if (module)	{
// 			ndxml *authorxml = ndxml_getnode(module, "Author");
// 			if (authorxml)	{
// 				ndxml_setval(authorxml, nd_get_sys_username());
// 			}
// 		}
		ndxml_save(m_curFile, m_filePath.c_str());
		nd_logmsg("save %s script ok\n", m_filePath.c_str());

		m_filesWatcher.addPath(m_filePath.c_str());

		return true;
	}
	return false;
}

void MainWindow::closeCurFile()
{
	if (m_curFile) {
		ndxml_destroy(m_curFile);
		delete m_curFile;
	}
	m_isChangedCurFile = false;
}

void MainWindow::closeCurFunction()
{
	if (m_editorWindow) {
		m_editorWindow->clearFunction();
		m_editorWindow->update();
	}
	m_currFunction = NULL;
}

bool MainWindow::checkNeedSave()
{
	return m_isChangedCurFile && m_curFile;
}

bool MainWindow::showCommonDeatil(ndxml *xmldata)
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("DetailView");
	if (pDock)	{
		QWidget *w = pDock->findChild<QWidget*>("xmlDetailTable");
		if (w)	{
			w->close();
		}
	}
	else {
		pDock = new QDockWidget(tr("Detail"), this);
		pDock->setObjectName("DetailView");
		pDock->setAttribute(Qt::WA_DeleteOnClose, true);
	}

	apoUiCommonXmlView *subwindow = new apoUiCommonXmlView();
	subwindow->setAttribute(Qt::WA_DeleteOnClose, true);
	subwindow->setObjectName("xmlDetailTable");

	subwindow->showDetail(xmldata, m_curFile);

	QObject::connect(subwindow, SIGNAL(xmlDataChanged()),
		this, SLOT(onFileChanged()));

	pDock->setWidget(subwindow);
	addDockWidget(Qt::RightDockWidgetArea, pDock);
	
	return true;
}


bool MainWindow::showDebugInfo(ndxml *xmldata)
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("DebugView");
	if (pDock)	{
		QWidget *w = pDock->findChild<QWidget*>("xmlDebugTable");
		if (w)	{
			w->close();
		}
	}
	else {
		pDock = new QDockWidget(tr("Debug"), this);
		pDock->setObjectName("DebugView");
		pDock->setAttribute(Qt::WA_DeleteOnClose, true);
	}

	apoUiCommonXmlView *subwindow = new apoUiCommonXmlView();
	subwindow->setAttribute(Qt::WA_DeleteOnClose, true);
	subwindow->setObjectName("xmlDebugTable");

	subwindow->showDetail(xmldata, m_curFile);

	QObject::connect(subwindow, SIGNAL(xmlDataChanged()),
		this, SLOT(onFileChanged()));

	pDock->setWidget(subwindow);
	addDockWidget(Qt::RightDockWidgetArea, pDock);

	return true;
}

void MainWindow::closeDebugInfo()
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("DebugView");
	if (pDock)	{
		pDock->close();
	}
}

void MainWindow::closeDetail()
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("DetailView");
	if (pDock)	{
		pDock->close();
	}
}

void MainWindow::onBreakPointEdited(const char *func, const char *node, bool isAdd)
{
	if (m_debuggerCli){
		if (isAdd)	{
			m_debuggerCli->cmdAddBreakPoint(func, node);
		}
		else{
			m_debuggerCli->cmdDelBreakPoint(func, node);
		}
	}
}

void MainWindow::onShowExeNodeDetail(apoBaseExeNode *exenode)
{
	//////////////////////////-----------------------------
	QDockWidget *pDock = this->findChild<QDockWidget*>("DetailView");
	if (pDock)	{
		QWidget *w = pDock->findChild<QWidget*>("xmlDetailTable");
		if (w)	{
			w->close();
		}
	}
	else {
		pDock = new QDockWidget(tr("Detail"), this);
		pDock->setObjectName("DetailView");
		pDock->setAttribute(Qt::WA_DeleteOnClose, true);
	}

	apoUiDetailView *subwindow = new apoUiDetailView();
	subwindow->setAttribute(Qt::WA_DeleteOnClose, true);
	subwindow->setObjectName("xmlDetailTable");
	if (exenode) {
		subwindow->showDetail(exenode, m_curFile);
	}
	if (m_editorWindow) {
		QObject::connect(subwindow, SIGNAL(xmlDataChanged()),
			m_editorWindow, SLOT(onCurNodeChanged()));
	}
	//QObject::connect(subwindow, SIGNAL(showXmlTextValue(ndxml *xmlnode)),
	//	this, SLOT(onNodeParamTextShow(ndxml *xmlnode)));


	pDock->setWidget(subwindow);

	addDockWidget(Qt::RightDockWidgetArea, pDock);

}

//double click function list
void MainWindow::onFunctionsTreeCurItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	xmlTreeItem *pXmlItem = dynamic_cast<xmlTreeItem*>(current);
	if (!pXmlItem)	{
		return;
	}
	ndxml *pxml = (ndxml *)pXmlItem->getUserData();
	if (!pxml)	{
		return;
	}
	if (pxml == m_currFunction){
		return;
	}

	const char *pAction = m_editorSetting.getDisplayAction( ndxml_getname(pxml) );
    if(!pAction) {
        return ;
    }

	if (0 == ndstricmp(pAction, "function"))	{
		m_currFunction = pxml;
		showCurFunctions();

	}
	else if (0 == ndstricmp(pAction, "detail")) {
		showCommonDeatil(pxml);
		if (m_editorWindow) {
			m_editorWindow->showNodeDetail(NULL);
		}
	}
}

// double click file list
void MainWindow::onFilesTreeCurItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{

	xmlTreeItem *pXmlItem = dynamic_cast<xmlTreeItem*>(current);
	if (!pXmlItem)	{
		return;
	}
	ndxml *pxml = (ndxml *)pXmlItem->getUserData();
	if (!pxml)	{
		return;
	}

	closeDetail();
	m_editorWindow->clearFunction();
	m_currFunction = NULL;

	m_fileMoudleName = ndxml_getattr_val(pxml, "name");
	//if (loadScriptFile(ndxml_getval(pxml))) {
	if (loadProject(pxml)) {
		setDefaultFile(m_fileMoudleName.c_str());
	}
}

//////////////////////////////////////////////////////////////////////////

// 
// bool MainWindow::setConfig(const char *cfgFile, const char *messageFile)
// {
// 	m_messageFile = messageFile;
// 	m_confgiFile = cfgFile;
// 	if (m_editorSetting.setConfigFile(cfgFile, E_SRC_CODE_UTF_8)) {
// 		if (m_editorWindow) {
// 			m_editorWindow->setSettingConfig(&m_editorSetting);
// 		}
// 		return true;
// 	}
// 	return false;
// }
bool MainWindow::setFileRoot(const char *project_file)
{
	//get root file's absulate path 
	char tmpbuf[ND_FILE_PATH_SIZE];
	m_fileRootPath = nd_absolute_filename(project_file,tmpbuf, sizeof(tmpbuf)) ;
	if (m_fileRootPath.empty()) {
		nd_logerror("can not found file %s\n", m_fileRootPath.c_str());
		return false;
	}

	MY_LOAD_XML_AND_NEW(m_fileRoot, m_fileRootPath.c_str(),m_editorSetting.m_encodeName.c_str(), return false);

	ndxml *fileroot = ndxml_getnode(m_fileRoot, "script_file_manager");
	if (!fileroot){
		return true;
	}
	//m_fileRootPath = rootFile;
	const char *lastOpen = ndxml_getattr_val(fileroot, "last_edited");
	if (!lastOpen)	{
		return true;
	}

	for (int i = 0; i < ndxml_getsub_num(fileroot); i++) {
		ndxml *node = ndxml_getnodei(fileroot, i);
		const char *fileName = ndxml_getattr_val(node, "name");
		if (0 == ndstricmp(fileName, lastOpen))	{

			m_fileMoudleName = ndxml_getattr_val(node, "name");
			loadProject(node);
			//loadScriptFile(ndxml_getval(node));
			break;
		}
	}
	return true;
}


bool MainWindow::setDefaultFile(const char *lastEditfile)
{
	ndxml *fileroot = ndxml_getnode(m_fileRoot, "script_file_manager");
	if (!fileroot){
		return false;
	}
	ndxml_setattrval(fileroot, "last_edited", lastEditfile);

	ndxml_save(m_fileRoot, m_fileRootPath.c_str());
// 
// 	for (int i = 0; i < ndxml_getsub_num(fileroot); i++) {
// 		ndxml *node = ndxml_getnodei(fileroot, i);
// 
// 		const char *fileName = ndxml_getval(node);
// 		if (0 == ndstricmp(fileName, lastEditfile))	{
// 			ndxml_setattrval(fileroot, "last_edited", ndxml_getattr_val(node, "name"));
// 			break;
// 		}
// 	}
	return true;
}

//menu
void MainWindow::on_actionViewList_triggered()
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("FilelistView");
	if (pDock){
		pDock->close();
		return;
	}
	else {
		openFileView();	
	}
	
}



void MainWindow::on_actionViewOutput_triggered()
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("outputView");
	if (pDock){

		QTextEdit *pEdit = this->findChild<QTextEdit*>("logTextEdit");
		if (pEdit)	{
			m_logText = pEdit->toPlainText();
		}
		pDock->close();
		return;
	}
	else {        
		openOutputView();
	}

}


void MainWindow::on_actionFunctionView_triggered()
{

    QDockWidget *pDock = this->findChild<QDockWidget*>("FunctionView");
    if (pDock){
        pDock->close();
        return;
    }
    else {
		openFunctionsView();
    }
}

void MainWindow::on_actionDetail_triggered()
{
    QDockWidget *pDock = this->findChild<QDockWidget*>("DetailView");
    if (pDock){
        pDock->close();
        return;
    }
    else {
		openDetailView();
    }
}

void MainWindow::on_actionFileNew_triggered()
{
	QString filter = QFileDialog::getSaveFileName(this, tr("New project"), ".", tr("Allfile(*)"));
	if (filter.isNull()) {
		return;
	}
	std::string curPath = nd_getcwd();
	nd_chdir(m_workingPath.c_str());

	char buf[1024];
	nd_relative_path(filter.toStdString().c_str(), m_workingPath.c_str(), buf, sizeof(buf));
	const char *fileName = nd_filename(buf);

	if (-1 == nd_mkdir(buf)) {
		nd_logerror("can not create direct %s\n", buf);
		return;
	}

	std::string strFile = buf;
	strFile += "/";
	strFile += fileName;
	strFile += ".xml";

	/////////////////////////////////////////////////
	// select project type
	ListDialog listDlg;
	ndxml flists;
	if (-1 == ndxml_load(m_fileTemplate.c_str(), &flists)) {
		return;
	}
	ndxml *myroot = ndxml_getnode(&flists, "file_list");
	if (!myroot) {
		return;
	}
	for (size_t i = 0; i < ndxml_getsub_num(myroot); i++){
		ndxml *node = ndxml_getnodei(myroot, (int)i);
		nd_assert(node);
		listDlg.m_selList.push_back(QString(ndxml_getattr_val(node, "desc")));
	}

	listDlg.InitList();
	if (listDlg.exec() != QDialog::Accepted) {
		return;
	}
	int seled = listDlg.GetSelect();
	if (seled == -1) {
		return;
	}
	ndxml *nodeFile =  ndxml_getnodei(myroot, seled);
	if (!nodeFile) {
		return;
	}
	std::string myTemplFile = ndxml_getval(nodeFile);
	if (myTemplFile.empty()) {
		return;
	}
	char file_path_buff[1024];
	nd_getpath(m_fileTemplate.c_str(), file_path_buff, sizeof(file_path_buff));

	nd_full_path(file_path_buff, myTemplFile.c_str(), file_path_buff, sizeof(file_path_buff));

	//////////////////////////////////////////////
	
	
	

	ndxml_root *xml = ndxml_getnode(m_fileRoot, "script_file_manager");
	if (!xml){
		QMessageBox::warning(NULL, "Error", "File Error!!!", QMessageBox::Ok);
		nd_chdir(curPath.c_str());
		return;
	}
	if (-1 == nd_cpfile(file_path_buff, strFile.c_str())) {
		QMessageBox::warning(NULL, "Error", "SAVE File Error!!!", QMessageBox::Ok);
		nd_chdir(curPath.c_str());
		return;
	}
	ndxml_root newFileXml;
	if (0 == ndxml_load(strFile.c_str(), &newFileXml)) {
		LogicEditorHelper::setScriptChangedTime(&newFileXml,time(NULL)) ;
		ndxml_destroy(&newFileXml);
	}

	ndxml *newNode = ndxml_addnode(xml, "file", buf);
	if (newNode) {
		ndxml_addattrib(newNode, "name", fileName);
		strFile = fileName;
		strFile += ".xml";
		ndxml_addattrib(newNode, "main_file", strFile.c_str());
	}
	ndxml_save(m_fileRoot, m_fileRootPath.c_str());

	//copy file to setting 
	std::string aimPath = buf; 
	aimPath += "/";
	aimPath += "setting";

	if (0== nd_mkdir(aimPath.c_str())) {
		buf[0] = 0;
		std::string srcPath = nd_getpath(m_fileTemplate.c_str(), buf, sizeof(buf));
#define CPY_SETTING_FILE(_name) do {	\
			std::string srcFile = srcPath + _name;			\
			std::string destFile = aimPath + _name;			\
			if(-1==nd_cpfile(srcFile.c_str(), destFile.c_str()) ) {	\
				nd_logerror("copy file error : %s\n", nd_last_error());	\
			}		\
		}while(0)

		CPY_SETTING_FILE("/func_list.xml");
		CPY_SETTING_FILE("/events_id.xml");
		CPY_SETTING_FILE("/userDefList.xml");

		aimPath += "/error_list.xml";
		common_export_error_list(aimPath.c_str());
	}
	
	showFileslist();
}

void MainWindow::on_actionFileOpen_triggered()
{
	char mypath[1024];
	QString fullPath = QFileDialog::getOpenFileName(this, tr("open file"), ".", tr("Allfile(*.*);;xmlfile(*.xml)"));
	if (fullPath.isNull()) {
		return;
	}
	if (!nd_relative_path(fullPath.toStdString().c_str(), nd_getcwd(), mypath, sizeof(mypath))) {
		return;
	}

	ndxml_root xmlScript;
	ndxml_initroot(&xmlScript);
	//nd_get_encode_name(ND_ENCODE_TYPE)
	if (-1 == ndxml_load_ex(mypath, &xmlScript, "utf8")) {
		nd_logerror("can not open file %s", fullPath.toStdString().c_str());
		return;
	}
#define CHECK_FILE(_xml, _nodeName) \
	do 	{	\
		ndxml *xml = ndxml_getnode((_xml), _nodeName) ;	\
		if(!xml) {	\
			nd_logerror("can not load file %s Maybe file is destroyed\n", fullPath.toStdString().c_str());	\
			return ;	\
		}				\
	} while (0)

	CHECK_FILE(&xmlScript, "blueprint_setting");
	CHECK_FILE(&xmlScript, "moduleInfo");
	CHECK_FILE(&xmlScript, "baseFunction");

	if (!loadScriptFile(mypath)) {
		nd_logerror("can not load file %s\n", fullPath.toStdString().c_str());	
		return;
	}
	//////////////////////////////////////////////////////////////////////////

	char inputPath[1024];
	inputPath[0] = 0;
	if (!nd_getpath(mypath, inputPath, sizeof(inputPath))) {
		nd_logerror("can not get path %s\n", fullPath.toStdString().c_str());
		return;
	}

	ndxml *fileroot = ndxml_getnode(m_fileRoot, "script_file_manager");
	if (fileroot){
		bool isFound = false;
		for (int i = 0; i < ndxml_getsub_num(fileroot); i++) {
			ndxml *node = ndxml_getnodei(fileroot, i);
			const char *filePath = ndxml_getval(node);
#ifdef WIN32
			if (0 == ndstricmp(filePath, inputPath)) {
#else 
			if (0 == ndstrcmp(filePath, inputPath)) {
#endif
				m_fileMoudleName = ndxml_getattr_val(node, "name");
				//loadScriptFile(ndxml_getval(node));
				loadProject(node);
				isFound = true;
				break;
			}
		}
		if (!isFound) {

			const char *fileName = nd_filename(mypath);
			char moduleName[128];
			ndstr_nstr_end(fileName, moduleName, '.', 128);
			ndxml *newxml = ndxml_addnode(fileroot, "file", inputPath);
			if (newxml)		{
				ndxml_addattrib(newxml, "name", moduleName);
				ndxml_addattrib(newxml, "main_file", fileName);
			}

			ndxml_save(m_fileRoot, m_fileRootPath.c_str());
			showFileslist();
		}
	}
	

}

void MainWindow::on_actionFileClose_triggered()
{
	if (checkNeedSave()){
		saveCurFile();
		ndxml_destroy(m_curFile);
		delete m_curFile;
	}
	m_curFile = NULL;
	m_editorWindow->clearFunction();
	showCurFile();

}

void MainWindow::on_actionCompile_triggered()
{
	//ClearLog();
	on_actionSave_triggered();
	if (m_filePath.size()) {
		std::string outFile;
		compileScript(m_filePath.c_str(), outFile);
	}
}

void MainWindow::on_actionSave_triggered()
{
	saveCurFile();
	setCurFileSave(true);
}

void MainWindow::on_actionExit_triggered()
{
	close();
}

void MainWindow::on_actionExit_without_save_triggered()
{
	setCurFileSave(true);
	close();

}


void MainWindow::on_actionCompileAll_triggered()
{
	saveCurFile();
	ndxml_root *xml = ndxml_getnode(m_fileRoot, "script_file_manager");
	if (!xml){
		return ;
	}
	bool ret = true;
	int num = ndxml_num(xml);
	std::string curPath = nd_getcwd();
	for (int i = 0; i < num; i++) {
		std::string outFile;
		ndxml *node = ndxml_getnodei(xml, i);
		if (!node)
			continue;

		if (!compileScriptNode(node)) {
			ret = false;
			break;
		}
	}

	nd_chdir(curPath.c_str());
}

void MainWindow::on_actionClearLog_triggered()
{
	m_logText.clear();

	QDockWidget *pDock = this->findChild<QDockWidget*>("outputView");
	if (pDock){
		QTextEdit *pEdit = pDock->findChild<QTextEdit*>("logTextEdit");
		if (pEdit)	{
			pEdit->clear();
		}
	}
}

void MainWindow::on_actionRun_triggered()
{
	on_actionSave_triggered();
	ClearLog();

	Run(false);

}

void MainWindow::on_actionTest_triggered()
{
	on_actionSave_triggered();
	ndxml_root *xml = ndxml_getnode(m_fileRoot, "script_file_manager");
	if (!xml){
		return;
	}

	bool ret = true;
	int num = ndxml_num(xml);
	std::string curPath = nd_getcwd();
	for (int i = 0; i < num; i++) {
		std::string outFile;
		ndxml *node = ndxml_getnodei(xml, i);
		if (!node)
			continue;

		if (!compileScriptNode(node,true)) {
			ret = false;
			break;
		}
	}

	nd_chdir(curPath.c_str());

}


void MainWindow::on_actionCancel_scale_triggered()
{
	if (m_editorWindow && m_currFunction)	{
		showCurFunctions();
	}
}

void MainWindow::on_actionRunDebug_triggered()
{
	on_actionSave_triggered();
	ClearLog();
	closeDetail();

	Run(true);
}

void MainWindow::on_actionStepOver_triggered()
{
	if (m_debuggerCli)	{
		bool bExeed = false;
		int ret = -1;
		//run to next node 
		apoBaseExeNode *pDbgNode = m_editorWindow->getCurDebugNode();
		if (pDbgNode)	{
			apoBaseExeNode *aimNode = pDbgNode->getMyNextNode();
			if (aimNode)	{
				ndxml *xmlnode = aimNode->getBreakPointAnchor();
				if (xmlnode){
					char buf[1024];
					if (LogicCompiler::getFuncStackInfo(xmlnode, buf, sizeof(buf))) {

						bExeed = true;
						ret = m_debuggerCli->cmdRunTo(LogicEditorHelper::_GetXmlName(m_currFunction,NULL), buf);
					}
				}
			}
		}


		if (!bExeed ) {
			ret = m_debuggerCli->cmdStep();
		}

		if (ret == -1)	{
			EndDebug(false);
		}

	}
}


void MainWindow::on_actionStepIn_triggered()
{
	if (m_debuggerCli)	{
		if (m_debuggerCli->cmdStep() == -1) {
			EndDebug(false);
		}
	}
}

void MainWindow::on_actionContinue_triggered()
{
	if (m_debuggerCli)	{
		if (m_debuggerCli->cmdContinue() == -1) {
			EndDebug(false);
		}
	}
}

void MainWindow::on_actionStepOut_triggered()
{

	if (m_debuggerCli)	{
		if (m_debuggerCli->cmdRunOut() == -1) {
			EndDebug(false);
		}
	}
}

void MainWindow::on_actionAttach_triggered()
{
	Process_vct_t processes;
	if (!ApoDebugger::getRunningProcess(processes)) {
		nd_logerror("no process could be attached\n");
		return;
	}

	ListDialog dlg(this);

	for (Process_vct_t::iterator it = processes.begin(); it != processes.end() ; it++)	{		
		dlg.m_selList.push_back(QString(it->name.c_str()));
	}

	dlg.InitList();
	if (dlg.exec() != QDialog::Accepted) {
		return ;
	}

	int seled = dlg.GetSelect();
	if (seled == -1) {
		return ;
	}

	if (seled >= processes.size()) {
		return;
	}
	

	NDUINT32 pId = processes[seled].pId;
	m_debuggerCli = new ApoDebugger;

	int ret = m_debuggerCli->Attach(pId, E_SRC_CODE_UTF_8);
	if (-1 == ret) {
		QMessageBox::warning(NULL, "Error", "Attach process error", QMessageBox::Ok);

		nd_logerror("attache process %s error\n", processes[seled].name.c_str());
		delete m_debuggerCli;
		m_debuggerCli = NULL;
		return;
	}
	else if (1 == ret) {

		QMessageBox::warning(NULL, "Error", "You selected process is not running in debug", QMessageBox::Ok);

		nd_logerror("The process %s do not surport debug\n", processes[seled].name.c_str());

		delete m_debuggerCli;
		m_debuggerCli = NULL;
		return;
	}

	QObject::connect(&m_debuggerCli->m_obj, SIGNAL(stepSignal(const char *, const char *)),
		this, SLOT(onDebugStep(const char *, const char*)));

	QObject::connect(&m_debuggerCli->m_obj, SIGNAL(terminateSignal()),
		this, SLOT(onDebugTerminate()));

	QObject::connect(&m_debuggerCli->m_obj, SIGNAL(commondOkSignal()),
		this, SLOT(onDebugCmdAck()));


	QObject::connect(&m_debuggerCli->m_obj, SIGNAL(scriptRunOKSignal()),
		this, SLOT(onScriptRunOK()));

	closeCurFunction();

	onAttached(processes[seled].scriptModule.c_str());

}

void MainWindow::on_actionDeattch_triggered()
{
	onDeattched();
	if (m_debuggerCli)	{
		m_debuggerCli->Deattach();
		delete m_debuggerCli;
		m_debuggerCli = NULL;
	}

	nd_logmsg("----------Deattached----------\n");
}

static int getScriptExpEncodeType(ndxml *scriptXml)
{
	ndxml *moduleInfo = ndxml_getnode(scriptXml, "moduleInfo");
	if (moduleInfo){
		ndxml *node = ndxml_getnode(moduleInfo, "script_out_encode");
		if (node){
			return ndxml_getval_int(node);
		}
	}
	return 0;
}

static bool getScriptExpDebugInfo(ndxml *scriptXml)
{
	ndxml *moduleInfo = ndxml_getnode(scriptXml, "moduleInfo");
	if (moduleInfo){
		ndxml *node = ndxml_getnode(moduleInfo, "script_with_debug");
		if (node){
			return ndxml_getval_int(node) ? true : false;
		}
	}
	return false;
}


bool MainWindow::runFunction(const char *binFile, const char *srcFile, int argc, const char* argv[])
{
	bool ret = true;
	//DeguggerScriptOwner apoOwner;
	//if (!apoOwner.loadDataType(m_editorSetting.getProjectConfig("net_data_def"))) {
// 	if (!apoOwner.loadDataType(m_netDataTypeFile.c_str())) {	
// 		WriteLog("load data type error\n");
// 		return false;
// 	}

	LogicEngineRoot *scriptRoot = LogicEngineRoot::get_Instant();
	nd_assert(scriptRoot);
	scriptRoot->setPrint(ND_LOG_WRAPPER_PRINT(MainWindow), NULL);
	scriptRoot->setOutPutEncode(E_SRC_CODE_UTF_8);

	LogicParserEngine &parser = scriptRoot->getGlobalParser();
	parser.setSimulate(true);

	if (argc ==0 )	{
		if (0 != scriptRoot->LoadScript(binFile, &scriptRoot->getGlobalParser() )){
			WriteLog("load script error n");
			LogicEngineRoot::destroy_Instant();
			return false;
		}

		parser.eventNtf(1/*APOLLO_EVENT_SERVER_START*/, 0) ;
		WriteLog("start run script...\n");
		if (0 != scriptRoot->test()){
			WriteLog("run script error\n");
			ret = false;
		}
		else {
			nd_logmsg("!!!!!!!!!!!!!!!!!!!RUN SCRIPT %s  SUCCESS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", binFile);
		}
		parser.eventNtf(2/*APOLLO_EVENT_SERVER_STOP*/, 0);
	}
	else {		
		if (0 != scriptRoot->LoadScript(binFile,NULL)){
			WriteLog("load script error n");
			LogicEngineRoot::destroy_Instant();
			return false;
		}

		parser.eventNtf(1/*APOLLO_EVENT_SERVER_START*/, 0);
		int ret = parser.runCmdline(argc, argv, E_SRC_CODE_UTF_8);
		if (ret && parser.getErrno() != NDERR_WOULD_BLOCK)	{
			nd_logmsg("run function %s error : %d \n", argv[0], ret);
			ret = false;
		}
		else {
			//nd_logmsg("run function %s SUCCESS \n", argv[0]);
		}
		//parser.eventNtf(APOLLO_EVENT_UPDATE, 0);
		parser.eventNtf(2/*APOLLO_EVENT_SERVER_STOP*/, 0);
	}
	LogicEngineRoot::destroy_Instant();

	if (ret == false)	{
		LogicParserEngine &parser = scriptRoot->getGlobalParser();
		const char *lastError = parser.getLastErrorNode();
		if (lastError)	{
			nd_logerror("run in %s\n", lastError);
			showRuntimeError(srcFile, lastError);

		}
	}

	return ret;
}

bool MainWindow::StartDebug(const char *binFile, const char *srcFile, int argc, const char* argv[])
{
	//bool ret = true;	
// 	m_localDebugOwner = new DeguggerScriptOwner();
	
	//if (!apoOwner.loadDataType(m_editorSetting.getProjectConfig("net_data_def"))) {
// 	if (!apoOwner.loadDataType(m_netDataTypeFile.c_str())) {
// 		WriteLog("load data type error\n");
// 		return false;
// 	}

	m_debuggerCli = new ApoDebugger();

	LogicEngineRoot *scriptRoot = LogicEngineRoot::get_Instant();
	nd_assert(scriptRoot);

	TestLogicObject  &apoOwner = scriptRoot->getDefParserOwner();

	scriptRoot->setPrint(ND_LOG_WRAPPER_PRINT(MainWindow), NULL);
	scriptRoot->setOutPutEncode(E_SRC_CODE_UTF_8);

	LogicParserEngine &parser = scriptRoot->getGlobalParser();
	parser.setSimulate(true);	

	if (0 != scriptRoot->LoadScript(binFile, NULL)){
		WriteLog("load script error n");
		EndDebug(false);
		return false;
	}


	QObject::connect(&m_debuggerCli->m_obj, SIGNAL(stepSignal(const char *, const char *)),
		this, SLOT(onDebugStep(const char *, const char* )));

	QObject::connect(&m_debuggerCli->m_obj, SIGNAL(terminateSignal()),
		this, SLOT(onDebugTerminate()));
	QObject::connect(&m_debuggerCli->m_obj, SIGNAL(commondOkSignal()),
		this, SLOT(onDebugCmdAck()));

	QObject::connect(&m_debuggerCli->m_obj, SIGNAL(scriptRunOKSignal()),
		this, SLOT(onScriptRunOK()));


	onDebugStart();
	parser.eventNtf(1/*APOLLO_EVENT_SERVER_START*/, 0);


	nd_logmsg("======== Begin deubg %s ...==========\n", argv[0]);
	int ret = m_debuggerCli->localStrat(&parser,argc, argv, E_SRC_CODE_UTF_8);
	if (-1==ret) {
		EndDebug(false);
	}


	return true;
}

void MainWindow::EndDebug(bool bSuccess)
{
	onDebugEnd();
	if (m_debuggerCli)	{
		LogicEngineRoot *scriptRoot = LogicEngineRoot::get_Instant();
		nd_assert(scriptRoot);
		LogicParserEngine &parser = scriptRoot->getGlobalParser();

		if (bSuccess){
			parser.eventNtf(2/*APOLLO_EVENT_SERVER_STOP*/, 0);
		}
		else {
			const char *lastError = parser.getLastErrorNode();
			if (lastError)	{
				nd_logerror("run in %s\n", lastError);
				showRuntimeError(m_filePath.c_str(), lastError);
			}
		}

		m_debuggerCli->postEndDebug();

		//delete m_localDebugOwner;
		//m_localDebugOwner = NULL;

		delete m_debuggerCli;
		m_debuggerCli = NULL;

		LogicEngineRoot::destroy_Instant();
	}

	nd_logmsg("----------Debug end----------\n");
}

void MainWindow::onDebugStart()
{

	ui->actionRunDebug->setDisabled(true);
	ui->actionAttach->setDisabled(true);

	ui->actionStepIn->setDisabled(false);
	ui->actionStepOver->setDisabled(false);
	ui->actionContinue->setDisabled(false);
	ui->actionStepOut->setDisabled(false);
	ui->actionDeattch->setDisabled(false);

	closeDetail();
}

void MainWindow::onDebugEnd()
{
	closeDebugInfo();
	if (m_editorWindow)	{
		m_editorWindow->setDebugNode(NULL);
	}

	
	ui->actionRunDebug->setDisabled(false);
	ui->actionAttach->setDisabled(false);

	ui->actionStepIn->setDisabled(true);
	ui->actionStepOver->setDisabled(true);
	ui->actionContinue->setDisabled(true);
	ui->actionStepOut->setDisabled(true);
	ui->actionDeattch->setDisabled(true);

}
void MainWindow::onAttached(const char *moduleName)
{
	bool ret = loadScriptFromModule(moduleName);
	if (ret){
		std::string outFile;
		LocalDebugger & debuggerHost = LogicEngineRoot::get_Instant()->getGlobalDebugger();
		debuggerHost.clearBreakpoint();

		if (compileScript(m_filePath.c_str(), outFile,false,true)) {
			breakPoint_vct  breakpoints = debuggerHost.getBreakPoints();
			if (breakpoints.size()> 0)	{
				ndxml *nodes = ndxml_from_text("<nodes/>");
				nd_assert(nodes);
				for (breakPoint_vct::iterator it = breakpoints.begin(); it != breakpoints.end(); it++){
					if (it->tempBreak)	{
						continue;
					}
					ndxml *node = ndxml_addnode(nodes, "node", NULL);
					if (node )	{
						ndxml_addattrib(node, "func", it->functionName.c_str());
						ndxml_addattrib(node, "exenode", it->nodeName.c_str());
					}
				}
				m_debuggerCli->cmdAddBreakPointBatch(nodes);
				ndxml_free(nodes);
			}
		}
	}
	onDebugStart();
}

void MainWindow::onDeattched()
{
	onDebugEnd();
}


bool MainWindow::Run(bool bIsDebug)
{

	const char *curFunc = m_editorWindow ? m_editorWindow->getEditedFunc() : NULL;
	RunFuncDialog dlg(this);
	dlg.initFunctionList(m_curFile, curFunc);

	if (dlg.exec() != QDialog::Accepted) {
		return false;
	}
	QString qfuncName = dlg.getFunc();
	QString qargs = dlg.getArgs();
	if (qfuncName.isEmpty()) {
		nd_logerror("run function name is NULL\n");
		return false;
	}

	int argc = 0;
	char argbuf[10][64];
	char *argv[10];

	for (int i = 0; i < 10; i++){
		argv[i] = argbuf[i];
	}

	//get func 
	std::string funcName = qfuncName.toStdString();
	std::string input_arg = qargs.toStdString();

	ndstrncpy(argbuf[0], funcName.c_str(), sizeof(argbuf[0]));

	if (input_arg.size() > 0)	{
		nd_logmsg("begin run %s %s\n", funcName.c_str(), input_arg.c_str());
		argc = ndstr_parse_command(input_arg.c_str(), &argv[1], 64, 9);
	}
	else {
		nd_logmsg("begin run %s\n", funcName.c_str());
	}
	++argc;

	std::string outFile;
	if (!compileScript(m_filePath.c_str(), outFile,false, bIsDebug)) {
		return false;
	}

	if (bIsDebug) {
		return StartDebug(outFile.c_str(), m_filePath.c_str(), argc, (const char**)argv);		
	}
	else {
		return runFunction(outFile.c_str(), m_filePath.c_str(), argc, (const char**)argv);
	}

}

bool MainWindow::compileScriptNode(ndxml *node,bool bWithRun)
{
	char absPath[ND_FILE_PATH_SIZE];
	const char *pFileName = ndxml_getattr_val(node, "main_file");
	if (!pFileName) {
		nd_logerror("can not get file name \n");
		return false;
	}
	const  char *projPath = ndxml_getval(node);
	if (!projPath) {
		nd_logerror("can not get file project path \n");
		return false;
	}
	nd_chdir(m_workingPath.c_str());

	if (!nd_absolute_path(projPath, absPath, sizeof(absPath))) {
		nd_logerror("get absulate path error %s\n", projPath);
		return false;
	}
	std::string path1 = absPath;

	nd_full_path(path1.c_str(), pFileName, absPath, sizeof(absPath));

	nd_chdir(path1.c_str());
	std::string outputFile;
	return compileScript(absPath, outputFile, bWithRun, false, -1);
}

bool MainWindow::compileScript(const char *scriptFile, std::string &outputFile, bool bWithRun, bool bDebug, int outEncodeType)
{
	int isDebug = bDebug? 1 : -1 ;
	return compileWithInterSetting(scriptFile, outputFile, &isDebug, &outEncodeType,bWithRun);
}

bool MainWindow::compileWithInterSetting(const char *scriptFile, std::string &outputFile, int *outDebug, int *outEncodeType, bool bWithRun )
{
	ndxml_root xmlScript;
	ndxml_initroot(&xmlScript);
	//nd_get_encode_name(ND_ENCODE_TYPE)
	if (-1 == ndxml_load_ex(scriptFile, &xmlScript,"utf8" )) {
		return false;
	}
	const char*inFile = scriptFile;

	const char *outFile = getScriptSetting(&xmlScript, "out_file");
	if (!outFile){
		nd_logerror("compile %s error !!!\nMaybe file is destroyed\n", scriptFile);
		return false;
	}
	outputFile = outFile;

	int outEncode = getScriptExpEncodeType(&xmlScript);
	if (outEncodeType ){
		if (*outEncodeType == -1) {
			*outEncodeType = outEncode;
		}
		else {
			outEncode = *outEncodeType ;
		}
	}

	bool withDebug = getScriptExpDebugInfo(&xmlScript);;
	if (outDebug)	{
		if (*outDebug == -1) {
			*outDebug =withDebug ;
		}
		else {
			withDebug = *outDebug ? true: false ;
		}
	}

	std::string outPath = outFile;
	ndxml_destroy(&xmlScript);

	outFile = outPath.c_str();

	int orderType = ND_L_ENDIAN;
	
	const char *orderName = m_editorSetting.getProjectConfig("bin_data_byte_order");
	if (orderName) {
		orderType = atoi(orderName);
 	}

	LogicCompiler &lgcompile = *LogicCompiler::get_Instant();

	if (!lgcompile.compileXml(inFile, outFile, outEncode, withDebug, orderType)) {

		const char *pFunc = lgcompile.m_cur_function.c_str();
		const char *pStep = lgcompile.m_cur_step.c_str();
		char func_name[256];
		char step_name[256];
		if (outEncode == E_SRC_CODE_GBK) {
			pFunc = nd_gbk_to_utf8(pFunc, func_name, sizeof(func_name));
			pStep = nd_gbk_to_utf8(pStep, step_name, sizeof(step_name));
		}
		nd_logerror("compile error file %s, function %s, step %s , stepindex %d\n",
			lgcompile.m_cur_file.c_str(), pFunc, pStep, lgcompile.m_cur_node_index);


		showCompileError(inFile, lgcompile.getErrorStack()) ;
		return false;
	}

	WriteLog("!!!!!!!!!!COMPILE script success !!!!!!!!!!!\n ");
	if (!bWithRun)	{
		return true;
	}
	return runFunction(outFile,scriptFile, 0, NULL);

}


bool MainWindow::showCompileError(const char *xmlFile, stackIndex_vct &errorStack)
{
	if (!loadScriptFile(xmlFile)) {
		nd_logerror("can not load %s file\n", xmlFile);
		return false;
	}

	ndxml *xmlroot = m_curFile;
	for (size_t i = 0; i < errorStack.size(); i++){
		int index = errorStack[i];
		ndxml *node = ndxml_getnodei(xmlroot, index);
		xmlroot = node;

		const char *pAction = m_editorSetting.getDisplayAction(ndxml_getname(node));
		if (pAction && 0 == ndstricmp(pAction, "function")) {
			m_currFunction = node;
			setCurrSelectedFunction(m_currFunction);
			showCurFunctions();
		}


		const compile_setting* compileSetting = m_editorSetting.getStepConfig(ndxml_getname(node));
		if (!compileSetting) {
			continue;
		}

		if (compileSetting->ins_type == E_INSTRUCT_TYPE_CMD){
			if (!m_editorWindow->setCurDetail(node, true)) {
				nd_logerror("can not show current detail node \n");
				return false;
			}
		}
		else if (compileSetting->ins_type == E_INSTRUCT_TYPE_PARAM) {
			if (!m_editorWindow->setCurNodeSlotSelected(node, true))	{
				nd_logerror("can not show current param\n");
				return false;
			}
		}
	}
	return true;
}


bool MainWindow::showDebugNode(const char *nodeInfo)
{
	stackIndex_vct debugStack;
	getStackFromName(nodeInfo, debugStack);

	ndxml *xmlroot = m_curFile;
	for (size_t i = 0; i < debugStack.size(); i++){
		int index = debugStack[i];
		ndxml *node = ndxml_getnodei(xmlroot, index);
		xmlroot = node;

		const char *pAction = m_editorSetting.getDisplayAction(ndxml_getname(node));
		if (pAction && 0 == ndstricmp(pAction, "function")) {
			if (m_currFunction != node)	{
				closeCurFunction();
				m_currFunction = node;
				showCurFunctions();
			}
		}


		const compile_setting* compileSetting = m_editorSetting.getStepConfig(ndxml_getname(node));
		if (!compileSetting) {
			continue;
		}

		if (compileSetting->ins_type == E_INSTRUCT_TYPE_CMD){
			if (!m_editorWindow->setDebugNode(node)) {
				nd_logerror("can not show current debug node \n");
				return false;
			}
		}
	}
	return true;
}

bool  MainWindow::showRuntimeError(const char *scriptFile, const char *errNodeInfo)
{
	stackIndex_vct stackIndex;
	getStackFromName(errNodeInfo, stackIndex);

	return showCompileError(scriptFile, stackIndex);
}

bool MainWindow::getStackFromName(const char *nodeInfo, stackIndex_vct &stackIndex)
{
	const char *p = ndstrchr(nodeInfo, '.');

	while (p && *p) {
		if (*p == '.') {
			++p;
		}
		char buf[10];
		buf[0] = 0;
		p = ndstr_nstr_ansi(p, buf, '.', 10);
		if (buf[0])	{
			stackIndex.push_back(atoi(buf));
		}
	}

	return true;
}

void MainWindow::setCurFileSave(bool isSaved)
{
	QDockWidget *pDock = this->findChild<QDockWidget*>("FunctionView");
	if (!pDock){
		return;
	}

	apoXmlTreeView *pEdit = this->findChild<apoXmlTreeView*>("functionsTree");
	if (!pEdit)	{
		return;
	}

	QString rootName = pEdit->rootName();

	if (isSaved){
		if (rootName[0] == '*'){
			rootName.remove(0, 1);
			pEdit->setRootName(rootName);
		}
	}
	else{
		if (rootName[0] != '*'){
			rootName.insert(0,"*");
			pEdit->setRootName(rootName);
		}
	}

}

void MainWindow::on_actionUndo_triggered()
{
	if (m_editorWindow) {
		m_editorWindow->ShowFuncUndo();
	}
}

void MainWindow::on_actionRedo_triggered()
{
	if (m_editorWindow) {
		m_editorWindow->ShowFuncRedo();
	}

}



// double click file list
void MainWindow::onSearchResultListChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{

	xmlTreeItem *pXmlItem = dynamic_cast<xmlTreeItem*>(current);
	if (!pXmlItem)	{
		return;
	}
	ndxml *pxml = (ndxml *)pXmlItem->getUserData();
	if (!pxml)	{
		return;
	}

	closeDetail();
	m_editorWindow->clearFunction();
	m_currFunction = NULL;

	showRuntimeError(m_filePath.c_str(), ndxml_getval(pxml));

}

#include "apoScript/searchdlg.h"
void MainWindow::on_actionSearch_triggered()
{
	SearchDlg dlg(m_curFile,this);
	if (dlg.exec() != QDialog::Accepted) {
		return ;
	}
	ndxml *res = dlg.getResult();
	if (!res) {
		QMessageBox::warning(this, "searched", "nothing to be found!", QMessageBox::Ok);
		return;
	}

	apoXmlTreeView *subwindow = NULL;
	QDockWidget *pDock = this->findChild<QDockWidget*>("SearchResultView");
	if (!pDock)	{
		pDock = new QDockWidget(tr("Search"), this);
		pDock->setObjectName("SearchResultView");
		pDock->setAttribute(Qt::WA_DeleteOnClose, true);

		subwindow = new apoXmlTreeView();
		subwindow->setAttribute(Qt::WA_DeleteOnClose, true);
		subwindow->setObjectName("SearchResultTree");

		pDock->setWidget(subwindow);
		addDockWidget(Qt::RightDockWidgetArea, pDock);

		QObject::connect(subwindow, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
			this, SLOT(onSearchResultListChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
	}
	else {
		subwindow = pDock->findChild<apoXmlTreeView*>("SearchResultTree");
	}

	if (m_xmlSearchedRes){
		ndxml_free(m_xmlSearchedRes);
	}
	m_xmlSearchedRes= ndxml_copy(res);

	subwindow->clear();

	subwindow->setXmlInfo(m_xmlSearchedRes, 2, "Result");

}

#define _OUT_PUT_TIME(_pf) do { \
	ndfprintf(_pf,"/* please do not change this file ,\n * auto create by program \n * create time %s \n */\n\n", nd_get_datetimestr() ) ; \
}while(0)

extern int bin_to_exe(void *data, size_t s, const char *outputFile, const char* peTemplFile);
void MainWindow::on_actionBuild_to_EXE_triggered()
{
	on_actionSave_triggered();
	if (!m_filePath.size()) {
		nd_logerror("not selected file\n");
		return ;
	}
	int isDebug = -1;
	int encodeType = ND_ENCODE_TYPE;
	std::string outFile;

	if (!compileWithInterSetting(m_filePath.c_str(), outFile, &isDebug, &encodeType)) {
		nd_logerror("compile file error\n");
		return;
	}
	nd_logmsg("compile script to %s success \n", outFile.c_str());
#ifdef __ND_LINUX__
	#include <sys/stat.h>
	
	chmod(outFile.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH) ;
#else
	//load file 
	size_t binSize = 0;
	void *binData =nd_load_file(outFile.c_str(), &binSize);
	if (!binData || binSize == 0) {
		nd_logerror("load bin data error\n");
		return;
	}

#ifdef __ND_WIN__
	std::string outExeFile = outFile + ".exe";
	std::string templFile = m_workingPath + "\\peTempl.exe";
	
	if (-1 == bin_to_exe(binData, binSize, outExeFile.c_str(), templFile.c_str())) {
		nd_logerror("compile to PE file error\n");
	}
	
#elif defined(__ND_MAC__)
	char path_buf[ND_FILE_PATH_SIZE];
	
	std::string outExeFile = nd_getpath(outFile.c_str(), path_buf, sizeof(path_buf)) ;
	outExeFile += "/" ;
	outExeFile += nd_file_name_without_ext(outFile.c_str(), path_buf, sizeof(path_buf)) ;
	
	std::string templFile = m_workingPath + "/cppTmplMachO";
	nd_logmsg("template file %s\n", templFile.c_str()) ;
	if (-1 == bin_to_exe(binData, binSize, outExeFile.c_str(), templFile.c_str())) {
		nd_logerror("compile to PE file error\n");
	}
	
#endif
#endif
	
}

bool MainWindow::exp2cpp(const char *binData, size_t binSize)
{
	std::string curentPath = nd_getcwd();
	nd_chdir(m_workingPath.c_str());	

	FILE *pf = fopen("../cfg/cpptmpl/apo_script_data.cpp", "w");
	if (!pf) {
		nd_chdir(curentPath.c_str());
		nd_logerror("open out put file error\n");
		return false;
	}
	_OUT_PUT_TIME(pf);


	ndfprintf(pf, "#include \"logic_parser/logicApi4c.h\" \n") ;
	ndfprintf(pf, "\n#pragma comment(lib, \"Ws2_32.lib\")\n\n");
	ndfprintf(pf, "#include <stdlib.h>\n#include <stdio.h>\n");

	ndfprintf(pf, "#pragma data_seg(push, stack1, \".nftxt\") \n");
	ndfprintf(pf, "size_t binScriptLength= 0x%x;\n\n", (int)binSize);

	ndfprintf(pf, "size_t data_addr_offset = 0;\n");

	ndfprintf(pf, "unsigned char binScriptData[] = { \n");

	for (size_t i = 0; i < binSize; ++i) {
		if ((i & 15) == 0) {
			ndfprintf(pf, "\n");
		}

		if (i == binSize - 1) {
			ndfprintf(pf, "0x%x\n", binData[i]);
		}
		else {
			ndfprintf(pf, "0x%x, ", binData[i]);
		}
	}
	ndfprintf(pf,"}; \n\n");

	ndfprintf(pf, "#pragma data_seg(pop, stack1)\n");

	ndfprintf(pf, "#pragma optimize( \"\", off )\n");
	ndfprintf(pf, "static void *__get_addr_bridge()\n{\tchar *p = (char*)binScriptData;\n\treturn (void*)(p + data_addr_offset);\n\t}\n");
		
	ndfprintf(pf,"int main(int argc, const char *argv[])\n{\n"
			"\treturn logic_script_main_entry( argc, argv, binScriptData,binScriptLength);\n}\n") ;
	
	ndfprintf(pf, "\n#pragma optimize( \"\", on )\n");

	fflush(pf);
	fclose(pf);

	nd_logmsg("gen CPP file success \n");
// 
// 	if (m_compileToExeCmd.length()) {
// 		//compile to exe 
// 		char moudleName[128];
// 		char toExeCmd[1024];
// 		char outPath[1024];
// 
// 		const char *pFileName = nd_filename(m_filePath.c_str());
// 		ndstr_str_end(pFileName, moudleName, '.');
// 
// 		nd_getpath(m_filePath.c_str(), outPath, sizeof(outPath));
// 
// #ifdef WIN32
// 		ndsnprintf(toExeCmd, sizeof(toExeCmd), " %s %s %s %s", m_compileToExeCmd.c_str(), outPath, moudleName, isDebug?"debug":"release");
// #else
// 		ndsnprintf(toExeCmd, sizeof(toExeCmd), "sh %s %s %s %s", m_compileToExeCmd.c_str(), outPath, moudleName, isDebug ? "debug":"release");
// #endif
// 		int ret = system(toExeCmd);
// 
// 		if (0 != ret) {
// 			nd_logerror("run compile-cpp-exe shell %s\nerror: %s \n current working path:%s\n",
// 						toExeCmd, nd_last_error(),nd_getcwd());
// 		}
// 		else {
// 			WriteLog("===============compile script to exe-file success =========");
// 		}
// 	}
// 	else {
// 		WriteLog("Can not found cmd to compile cpp to exe\n");
// 	}
	nd_chdir(curentPath.c_str());
		
	return true;
}

void MainWindow::on_actionCopy_triggered()
{
	QWidget *pCtrl = this->focusWidget();
	if (!pCtrl) {
		return;
	}
	if (dynamic_cast<apoBaseExeNode *>(pCtrl)) {
		apoBaseExeNode *exeNode = dynamic_cast<apoBaseExeNode *>(pCtrl);

		ndxml *xml = (ndxml *)exeNode->getMyUserData();
		if (xml) {
			m_clipboard.pushClipboard(xml, exeNode->getType());
		}
	}
	else if (dynamic_cast<apoXmlTreeView *>(pCtrl)) {
		apoXmlTreeView *tree = this->findChild<apoXmlTreeView*>("functionsTree");
		if (!tree || pCtrl!=tree) {
			return ;
		}
		QAction act("Copy");
		tree->onPopMenu(&act);
	}
}

void MainWindow::on_actionPaste_triggered()
{
	QWidget *pCtrl = this->focusWidget();
	if (!pCtrl) {
		return;
	}
	if (pCtrl == m_editorWindow) {
		m_editorWindow->pasteClipBoard();
	}
	else if (dynamic_cast<apoXmlTreeView *>(pCtrl)) {
		apoXmlTreeView *tree = this->findChild<apoXmlTreeView*>("functionsTree");
		if (!tree || pCtrl != tree) {
			return;
		}
		QAction act("Paste");
		tree->onPopMenu(&act);
	}
}
