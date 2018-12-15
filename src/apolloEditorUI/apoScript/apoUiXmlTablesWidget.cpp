/* 
* file apoUiXmlTablesWidget.cpp
*
* creat by duan 
*
* 2017.2.24
*/

#include "apoScript/apoUiXmlTablesWidget.h"
#include "apoScript/apoEditorSetting.h"
#include "logic_parser/logic_editor_helper.h"
//#include "logic_parser/dbl_mgr.h"
#include "apoScript/mainwindow.h"
#include <QFileDialog>
#include "texteditorpopdlg.h"

using namespace LogicEditorHelper;

apoUiXmlTableWidget::apoUiXmlTableWidget(QWidget *parent) :
QTableWidget(parent)
{
	myInit();
}

apoUiXmlTableWidget::apoUiXmlTableWidget(int rows, int columns, QWidget *parent) :
QTableWidget(rows, columns, parent)
{
	myInit();	
}
apoUiXmlTableWidget::~apoUiXmlTableWidget()
{
}

void apoUiXmlTableWidget::myInit()
{
	m_rootSetting = apoEditorSetting::getInstant();
	m_config = m_rootSetting->getConfig();
	m_alias = &m_rootSetting->getAlias();
	m_editedNode = 0;
	m_rootFile = 0;

	m_beginEditor = false;

	QObject::connect(this, SIGNAL(cellChanged(int, int)), this, SLOT(oCellChanged(int, int)));
	QObject::connect(this, SIGNAL(cellClicked(int, int)), this, SLOT(onCellClicked(int, int)));
	//QObject::connect(this, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onCellDoubleClicked(int, int)));

}
void apoUiXmlTableWidget::preShow()
{
	m_beginEditor = false;
	//QObject::disconnect(this);
	//QObject::connect(this, SIGNAL(cellChanged(int, int)), this, SLOT(oCellChanged(int, int)));
	//QObject::connect(this, SIGNAL(cellClicked(int, int)), this, SLOT(onCellClicked(int, int)));

}
void apoUiXmlTableWidget::postShow()
{
	//QObject::connect(this, SIGNAL(cellChanged(int, int)), this, SLOT(oCellChanged(int, int)));
	//QObject::connect(this, SIGNAL(cellClicked(int, int)), this, SLOT(onCellClicked(int, int)));
	m_beginEditor = true;
}

bool apoUiXmlTableWidget::showDetail(void *data, ndxml_root *xmlfile)
{
	m_beginEditor = true;
	return false;
}


bool apoUiXmlTableWidget::_GetUserDefinData(ndxml *curXml, const char *pUserParam, QString &val)
{
	if (0 == ndstricmp((char*)pUserParam, "func_list")) {
		ListDialog dlg(this);
		ndxml *node = ndxml_getnode(m_rootFile, "baseFunction");
		if (node)	{
			for (int i = 0; i < ndxml_num(node); i++){
				ndxml *funcNode = ndxml_getnodei(node, i);
				const char *name = (char*)ndxml_getattr_val(funcNode, _GetReverdWord(ERT_NAME));
				if (!name) {
					name = (char*)ndxml_getname(funcNode);
				}
				dlg.m_selList.push_back(QString(name));
			}
		}
		const apoEditorSetting::text_vct_t  *pList = m_rootSetting->getUserdefList(pUserParam);
		if (pList)	{
			for (int i = 0; i < pList->size(); i++) {
				dlg.m_selList.push_back((*pList)[i]);
			}
		}

		dlg.InitList();
		if (dlg.exec() == QDialog::Accepted) {
			return  dlg.GetSelectText(val);
		}

	}
	else if (0 == ndstricmp((char*)pUserParam, "extern_call")) {
		const char *cmd = NULL;
		const char *idVal = NULL;
		const char *pVal = NULL;

		cmd = ndxml_getattr_val(curXml, "user_cmd");
		idVal =ndxml_getattr_val(curXml, "user_ref_id");
		if (idVal && *idVal) {
			idVal = ndxml_recursive_getval(curXml, idVal);
			if (idVal && 0 == ndstricmp(idVal, "none")) {
				idVal = NULL;
			}
		}
		pVal = ndxml_getattr_val(curXml, "user_ref_val");
		if (pVal && *pVal) {
			pVal = ndxml_recursive_getval(curXml, pVal);
			if (pVal && 0 == ndstricmp(pVal, "none")) {
				pVal = NULL;
			}
		}
		
		LogicParserEngine &parser = LogicEngineRoot::get_Instant()->getGlobalParser();
		LogicObjectBase *pobjOwner = (LogicObjectBase *)parser.getOwner();
		LogicDataObj cmdId(idVal);
		LogicDataObj cmdVal(pVal);

		apoEditorSetting::text_vct_t  texts;
		if (pobjOwner && pobjOwner->opOperate(cmd, cmdId, cmdVal)) {
			for (int i = 0; i < cmdVal.GetArraySize(); i++) {
				texts.push_back(QString(cmdVal.GetarrayText(i)));
			}
		}
		return _GetUserSelEnumVal(texts, val);
	}
	/*
	else if (0 == ndstricmp((char*)pUserParam, "dbl_excel")) {
		LogicParserEngine &parser = LogicEngineRoot::get_Instant()->getGlobalParser();
		LogicObjectBase *pobjOwner = parser.getOwner();
		LogicDataObj cmdId;
		LogicDataObj cmdVal;

		apoEditorSetting::text_vct_t  texts;
		if (pobjOwner->opOperate(pUserParam, cmdId, cmdVal)) {
			for (int i = 0; i < cmdVal.GetArraySize(); i++) {
				texts.push_back(QString(cmdVal.GetarrayText(i)));
			}
		}		
		return _GetUserSelEnumVal(texts, val);
	}
	else if (0 == ndstricmp((char*)pUserParam, "dbl_excel_col")) {
		//get table name 
		ndxml *xml = m_editedNode;
		if (!xml) {
			return false;
		}
		ndxml *tablexml = ndxml_getnode(xml, "tablename");
		if (!tablexml) {
			nd_logerror(" excel table data");
			return false;
		}
		const char *pname = ndxml_getval(tablexml);
		if (0 == ndstricmp((char*)pname, (char*)"none")) {
			nd_logerror("param not be none");
			return false;
		}

		LogicParserEngine &parser = LogicEngineRoot::get_Instant()->getGlobalParser();
		LogicObjectBase *pobjOwner = parser.getOwner();
		LogicDataObj cmdId(pname);
		LogicDataObj cmdVal;

		apoEditorSetting::text_vct_t  texts;
		if (pobjOwner->opOperate(pUserParam, cmdId, cmdVal)) {
			for (int i = 0; i < cmdVal.GetArraySize(); i++) {
				texts.push_back(QString(cmdVal.GetarrayText(i)));
			}
		}
		return _GetUserSelEnumVal(texts, val);
	}
	*/
	else {
		const apoEditorSetting::text_vct_t  *pList = m_rootSetting->getUserdefList(pUserParam);
		if (pList)	{
			return _GetUserSelEnumVal(*pList, val);
		}

	}

	return false;
}
bool apoUiXmlTableWidget::_GetFileName(bool bOpen, QString & strFile, const char *default_file)
{
	QString fullPath;
	if (bOpen) {
		fullPath = QFileDialog::getOpenFileName(this, tr("open file"), ".", tr("Allfile(*.*);;xmlfile(*.xml)"));
	}
	else {
		fullPath = QFileDialog::getSaveFileName(this, tr("save file"), ".", tr("Allfile(*.*);;xmlfile(*.xml)"));
	}

	if (fullPath.isNull()) {
		fullPath = default_file;
		return false;
	}

	char buf[1024];
	strFile = nd_relative_path(fullPath.toStdString().c_str(), nd_getcwd(), buf, sizeof(buf));

	return true;

}

bool apoUiXmlTableWidget::_OpenFilter(QString &strPath, const QString &tip)
{
	QString fullPath = QFileDialog::getExistingDirectory(this, tip, ".");
	if (fullPath.isNull()) {
		return false;
	}
	char buf[1024];
	strPath = nd_relative_path(fullPath.toStdString().c_str(), nd_getcwd(), buf, sizeof(buf));
	
	return true;
}

bool apoUiXmlTableWidget::_GetUserSelEnumVal(const apoEditorSetting::text_vct_t &enum_vals, QString &val)
{
	ListDialog dlg(this);
	for (int i = 0; i < enum_vals.size(); i++) {
		dlg.m_selList.push_back(enum_vals[i]);
	}

	dlg.InitList();
	dlg.setModal(true);
	if (dlg.exec() == QDialog::Accepted) {
		return dlg.GetSelectText(val);
	}

	return false;
}


void apoUiXmlTableWidget::onCellClicked(int row, int column)
{
	if (!m_beginEditor) {
		return;
	}
	_beginEdit(row, column);
}
void apoUiXmlTableWidget::oCellChanged(int row, int column)
{
	if (!m_beginEditor)	{
		return;
	}
	xmlTableItem *pItem = dynamic_cast<xmlTableItem*>(this->item(row, column));
	if (!pItem) {
		return;
	}

	bool ret = false;
	
	ndxml *xml = (ndxml *)pItem->getUserData();
	if (xml){
		QString str1 = pItem->text();

		const char*attrName = pItem->getAttrName();
		if (attrName && *attrName)	{
			ndxml_setattrval(xml, attrName, str1.toStdString().c_str());
			onChanged(row, column, ndxml_getattr_val(xml, attrName));
			dataChangedSignal();
			return ;
		}

		if (GetXmlValType(xml, m_config) == EDT_ENUM) {
			int sel = -1;
			apoEditorSetting::text_vct_t  enum_texts;
			if (getEnumList(xml, m_config, enum_texts) > 0) {
				for (int i = 0; i < enum_texts.size(); i++)	{
					if (str1 == QString(enum_texts[i])) {
						sel = i;
						break;
					}
				}
			}
			if (-1 != sel) {
				str1.ndsprintf("%d", sel);
				ndxml_setval(xml, str1.toStdString().c_str());
				//dataChangedSignal();
				ret = true;
			}

		}
		else if (GetXmlValType(xml, m_config) == EDT_KEY_VAL) {
			int sel = -1;
			apoEditorSetting::text_vct_t  enum_texts;
			if (getEnumList(xml, m_config, enum_texts) > 0) {
				for (int i = 0; i < enum_texts.size(); i++)	{
					if (str1 == enum_texts[i]) {
						sel = i;
						break;
					}
				}
			}
			if (-1 != sel) {
				apoEditorSetting::text_vct_t  value_texts;
				getKeyValueList(xml, m_config, value_texts);
				if (sel < value_texts.size()){
					ndxml_setval(xml, value_texts[sel].toStdString().c_str());
					ret = true;
					//dataChangedSignal();
				}
			}

		}
		else if (!str1.isNull()) {
			ndxml_setval(xml, str1.toStdString().c_str());
			//dataChangedSignal();
			ret = true;
		}
	}

	if (ret){
		onChanged(row, column, ndxml_getval(xml));
		dataChangedSignal();
	}
}

bool apoUiXmlTableWidget::_beginEdit(int nRow, int nCol)
{
	xmlTableItem *cell = (xmlTableItem*)item(nRow, nCol);
	if (!cell){
		return false;
	}
	ndxml *xml = (ndxml *)cell->getUserData();
	if (!xml)	{
		return false;
	}
	if (cell->getAttrName()){
		return true;
	}

	bool ret = false;
	QString  str1;
	int type = GetXmlValType(xml, m_config);
	if (type == EDT_IN_FILE || type == EDT_OUT_FILE) {

		if (_GetFileName(type == EDT_IN_FILE, str1, GetXmlParam(xml))) {
			cell->setText(str1);
			if (0 == ndxml_setval(xml, str1.toStdString().c_str())) {
				//onChanged(nRow, nCol, ndxml_getval(xml));
				//dataChangedSignal();
				ret = true;
			}
		}
	}
	else if (EDT_DIR == type) {
		if (_OpenFilter(str1, tr("select direct"))){
			str1 += '/';
			cell->setText(str1);
			if (0 == ndxml_setval(xml, str1.toStdString().c_str())){
				//onChanged(nRow, nCol, ndxml_getval(xml));
				//dataChangedSignal();
				ret = true;
			}
		}
	}
	else if (EDT_USER_DEFINE == type) {

		QString str2;
		const char *pUserparam = ndxml_getattr_val(xml, _GetReverdWord(ERT_USER_DEFINE_PARAM));
		if (_GetUserDefinData(xml, pUserparam, str2)) {
			std::string stdText = str2.toStdString();

			char realval[256];
			str1 = getDisplayNameFromStr(stdText.c_str(), realval, sizeof(realval));
			ret = true;
			cell->setText(str1);
			ndxml_setval(xml, stdText.c_str());
			//dataChangedSignal();
		}
	}
	else if (EDT_BOOL == type) {
		ListDialog dlg(this);
		dlg.m_selList.push_back("FALSE");
		dlg.m_selList.push_back("TRUE");
		dlg.InitList();
		if (dlg.exec() == QDialog::Accepted) {
			dlg.GetSelectText(str1);
			int selval = dlg.GetSelect();

			cell->setText(str1);
			ndxml_setval(xml, selval ? "1" : "0");
			ret = true;
			//onChanged(nRow, nCol, ndxml_getval(xml));
			//dataChangedSignal();
		}

	}
	else if (EDT_ENUM == type)	{
		apoEditorSetting::text_vct_t enum_texts;
		if (getEnumList(xml, m_config, enum_texts) > 0) {
			if (_GetUserSelEnumVal(enum_texts, str1)){
				int sel = -1;
				for (int i = 0; i < enum_texts.size(); i++)	{
					if (str1 == enum_texts[i]) {
						sel = i;
						break;
					}
				}
				if (-1 != sel) {
					QString str3;
					str3.ndsprintf("%d", sel);

					cell->setText(str1);

					ndxml_setval(xml, str3.toStdString().c_str());
					//dataChangedSignal();
					ret = true;
				}

			}
		}
	}
	else if (EDT_KEY_VAL == type)	{
		ret = false;
		apoEditorSetting::text_vct_t key_texts;
		if (getKeyList(xml, m_config, key_texts) > 0) {
			if (_GetUserSelEnumVal(key_texts, str1)){
				int sel = -1;
				for (int i = 0; i < key_texts.size(); i++)	{
					if (str1 == key_texts[i]) {
						sel = i;
						break;
					}
				}
				if (-1 != sel) {
					apoEditorSetting::text_vct_t value_texts;
					getKeyValueList(xml, m_config, value_texts);
					if (sel < value_texts.size()){

						cell->setText(str1);
						ndxml_setval(xml, value_texts[sel].toStdString().c_str());
						ret = true;
					}
				}
			}
		}
	}
	else if (EDT_MULTI_TEXT == type || EDT_STRING == type) {

		const char *val = ndxml_getval(xml);
		if (!val || !*val) {
			return false;
		}
		if (EDT_STRING == type && ndstrlen(val) <=32 ) {
			return false;
		}
		TextEditorPopDlg dlg(this);
		std::string DisplayText;
		LogicEditorHelper::stringToOriginal(val, DisplayText);

		dlg.setMyText(DisplayText.c_str());

		xmlTableItem *cellName = (xmlTableItem*)item(nRow, 0);
		if (cellName) {
			dlg.setTips(cellName->text());
		}

		if (dlg.exec() != QDialog::Accepted) {
			return false;
		}
		QString newText;
		dlg.getMyText(newText);
		std::string SaveText ;
		LogicEditorHelper::stringToSaveFile(newText.toStdString().c_str(), SaveText);


		ndxml_setval(xml, SaveText.c_str());
		cell->setText(SaveText.c_str());
		ret = true;
	}
	if (ret ){
		//onChanged(nRow, nCol, ndxml_getval(xml));
		//dataChangedSignal();
	}
	return ret;
}


void apoUiXmlTableWidget::dataChangedSignal()
{
	emit xmlDataChanged();
	nd_logmsg("detail changed \n");
}

bool apoUiXmlTableWidget::onChanged(int row, int column, const char *xmlRealValue)
{
	xmlTableItem *cell = (xmlTableItem*)item(row, column);
	if (!cell){
		return false;
	}

	ndxml *xml = (ndxml*)cell->getUserData();
	if (!xml){
		return false;
	}

	if (cell->getAttrName()){
		return true;
	}

	const char *pReplacePath = ndxml_getattr_val(xml, "replace_val");
	if (!pReplacePath)	{
		return true;
	}
	const char *newValue = ndxml_getval(xml);
	if (!newValue){
		return false;
	}
	char tmpbuf[128];
	newValue = getDisplayNameFromStr(newValue, tmpbuf, sizeof(tmpbuf));

	ndxml *repXml = LogicEditorHelper::_getRefNode(xml, pReplacePath);
	if (!repXml){
		return true;
	}
	const char *attrName = LogicEditorHelper::_getRefNodeAttrName( pReplacePath);

	char buf[1024];
	const char *val = NULL;

	if (attrName){
		val = ndxml_getattr_val(repXml, attrName);
	}
	else {
		val = ndxml_getval(repXml);
	}

	//replace data
	const char *p = ndstr_nstr_ansi(val, buf, '$', sizeof(buf));
	ndstrncat(buf, "$", sizeof(buf));
	ndstrncat(buf, newValue, sizeof(buf));

	if (p && *p){
		++p;
		p = ndstrchr(p, '$');
		if (p){
			++p;
			if (*p){
				ndstrncat(buf, p, sizeof(buf));
			}
		}
	}	
	ndstrncat(buf, "$", sizeof(buf));
	
	if (attrName){
		ndxml_setattrval(repXml, attrName, buf);
	}
	else {
		ndxml_setval(repXml,buf);
	}
	return true;
}

ndxml *apoUiXmlTableWidget::_getXml(int row, int column)
{
	xmlTableItem *cell = (xmlTableItem*)item(row, column );
	if (!cell){
		return NULL;
	}
	return  (ndxml *)cell->getUserData();
}


QWidget *apoUiXmlTableWidget::getMainWindows()
{
	MainWindow *pMain = NULL;
	QObject *self = this;
	do{
		QObject *parent = self->parent();
		if (!parent) {
			break;
		}
		pMain = dynamic_cast<MainWindow *>(parent);
		self = parent;

	} while (!pMain);
	return pMain;
}

