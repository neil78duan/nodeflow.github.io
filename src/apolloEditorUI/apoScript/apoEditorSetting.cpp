/*
* file apoEditorSetting. h
*
* settings of editor
*
* create by duan
*
* 2017.2.20
*/

#include "apoEditorSetting.h"

#include "listdialog.h"
#include "newnodedialog.h"


using namespace LogicEditorHelper;
apoEditorSetting* apoEditorSetting::s_setting = NULL;

apoEditorSetting*apoEditorSetting::getInstant()
{
	if (!s_setting)	{
		s_setting = new apoEditorSetting();
	}
	return s_setting;
}

void apoEditorSetting::destroyInstant()
{
	if (s_setting)	{
		delete s_setting;
		s_setting = NULL;
	}
}

apoEditorSetting::apoEditorSetting()
{
	m_projXmlCfg = NULL;
}

apoEditorSetting::~apoEditorSetting()
{
	m_userDefineList.clear();
	MY_DESTROY_XML(m_projXmlCfg);
}


bool apoEditorSetting::Init(const char *ioFile, const char *editorSettingFile, int encodeType)
{
	char ioSetting[ND_FILE_PATH_SIZE];
	
	if (!nd_absolute_filename(ioFile, ioSetting, sizeof(ioSetting))) {
		nd_logerror("can not get file %s absolute path\n", ioFile);
		return false;
	}
	
	if (!setConfigFile(editorSettingFile, encodeType,false))  {
		return false;
	}

	m_encodeName = nd_get_encode_name(encodeType);
	m_projXmlCfg = new ndxml_root;	
	ndxml_initroot(m_projXmlCfg);

	if (0 != ndxml_load_ex(ioSetting, m_projXmlCfg, m_encodeName.c_str())) {
		nd_logerror("open file %s error", ioSetting);
		return false;
	}

	m_projConfigFile = ioSetting ;
	
	//get nodeflow install path
	const char *p =getProjectConfig("tool_installed_path") ;
	if(p) {
		LogicCompiler::get_Instant()->setShellRunTool(p) ;
	}
	

	return true;
}


const char *apoEditorSetting::getProjectConfig(const char *cfgName)
{
	ndxml *node = ndxml_getnode(m_projXmlCfg, "setting_config");
	if (node){
		ndxml *cfgxml = ndxml_getnode(node, cfgName);
		if (cfgxml)	{
			return ndxml_getval(cfgxml);
		}
	}
	return NULL;
}


bool apoEditorSetting::loadSettingFromScript(ndxml *script)
{
	m_userDefineList.clear();

	const char *userDefEnum = _getScriptSetting(script, "user_define_enum");
	if (userDefEnum){
		return loadUserDefEnum(userDefEnum);
	}
	return false;
}

void apoEditorSetting::destroyScriptSetting()
{
	m_userDefineList.clear();
}

bool apoEditorSetting::onLoad(ndxml &cfgRoot)
{
	ndxml *data = ndxml_getnode(&cfgRoot, "blueprint_node_name");
	if (data){
		if (m_exeNodeName.Create(data) == -1) {
			return false;
		}
	}

	ndxml *alias = ndxml_getnode(&cfgRoot, "alias");
	if (alias) {
		m_alias.Create(alias);
	}

	return true;
}


const char *apoEditorSetting::getExeNodeName(const char *xmlNodeName)
{
	return m_exeNodeName.GetAlia(xmlNodeName);
}


#define MY_LOAD_XML(_xml_name, _filename,_encode) \
	ndxml_root _xml_name;				\
	ndxml_initroot(&_xml_name);			\
	if (ndxml_load_ex((char*)_filename, &_xml_name,_encode)) {	\
		nd_logerror("open file %s error", _filename);	\
		return false;							\
			}


bool apoEditorSetting::loadUserDefEnum(const char *userDefEnumFile)
{

	//XMLDialog *xmlDlg = (XMLDialog *)pDlg;
	MY_LOAD_XML(xml_root, userDefEnumFile, "utf8");
	ndxml *xmlData = ndxml_getnode(&xml_root, "userData");
	if (!xmlData){
		return false;
	}
	for (int i = 0; i < ndxml_getsub_num(xmlData); i++){
		ndxml *dataEnum = ndxml_getnodei(xmlData, i);
		const char *type = ndxml_getattr_val(dataEnum, "src_type");
		const char *name = ndxml_getattr_val(dataEnum, "dataName");

		if (!type || !name) {
			nd_logmsg("unknown type typename =NULL or name=null\n");
			continue;
		}

		if (0 == ndstricmp(type, "file")){
			const char *file = ndxml_getattr_val(dataEnum, "filename");
			if (!file || !*file) {
				nd_logmsg("unknown type file name\n");
				continue;
			}
			MY_LOAD_XML(xml_subfile, file, "utf8");

			if (!loadUserdefDisplayList(xml_subfile, name)) {
				nd_logmsg("load user define enum from %s error\n", file);
			}

			ndxml_destroy(&xml_subfile);
		}
		else if (0 == ndstricmp(type, "list")) {

			if (!loadUserdefDisplayList(*dataEnum, name)) {
				nd_logmsg("load user define enum  %s error\n", name);
			}
		}
		else {
			nd_logmsg("unknown type %s\n", type);
		}
	}
	ndxml_destroy(&xml_root);
	return true;
}

bool apoEditorSetting::loadMessageDefine(const char *messageFile)
{
	
	MY_LOAD_XML(xml_net_protocol, messageFile, m_encodeName.c_str());
	
	ndxml *funcroot = ndxml_getnode(&xml_net_protocol, "MessageDefine");
	if (funcroot) {
		char buf[256];
		text_vct_t messageList;
		for (int i = 0; i < ndxml_num(funcroot); i++){
			ndxml *fnode = ndxml_getnodei(funcroot, i);
			const char *pDispname = ndxml_getattr_val(fnode, "comment");
			const char *pRealVal = ndxml_getattr_val(fnode, "id");
			const char *p = buildDisplaNameValStr(pRealVal, pDispname, buf, sizeof(buf));
			messageList.push_back(QString(p));
		}
		addDisplayNameList("msg_list", messageList);
	}
	return true;
}

#undef MY_LOAD_XML
bool apoEditorSetting::loadUserdefDisplayList(ndxml_root &xmlNameList, const char *name)
{
	text_vct_t textList;

	ndxml *funcroot = ndxml_getnode(&xmlNameList, name);
	char buf[256];
	if (funcroot) {
		for (int i = 0; i < ndxml_num(funcroot); i++){
			ndxml *fnode = ndxml_getnodei(funcroot, i);
			const char *pDispname = ndxml_getattr_val(fnode, "desc");
			const char *pRealVal = ndxml_getval(fnode);

			const char *p = LogicEditorHelper::buildDisplaNameValStr(pRealVal, pDispname, buf, sizeof(buf));
			textList.push_back(QString(p));
		}
		return addDisplayNameList(name, textList);
	}
	return false;
}

bool apoEditorSetting::addDisplayNameList(const char *name, apoEditorSetting::text_vct_t &text_list)
{
	m_userDefineList[name] = text_list;
	return true;
	//std::pair< text_map_t::iterator, bool> ret = m_userDefineList.insert(std::make_pair(std::string(name), text_list));
	//return ret.second;
}

const apoEditorSetting::text_vct_t* apoEditorSetting::getUserdefList(const char *name) const
{
	text_map_t::const_iterator it = m_userDefineList.find(name);
	if (it != m_userDefineList.end()){
		return &(it->second);
	}
	return NULL;
}

const char *apoEditorSetting::getDisplayAction(const char *xmlname) 
{
	ndxml *moduleInfo = ndxml_getnode(&m_configRoot, "blueprint_show_action");
	if (moduleInfo){
		ndxml *node = ndxml_getnode(moduleInfo, xmlname);
		if (node){
			return ndxml_getval(node);
		}
	}
	return NULL;
}

const char *apoEditorSetting::_getScriptSetting(ndxml *scriptXml, const char *settingName)
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


ndxml* apoEditorSetting::AddNewXmlNode(ndxml *xml, QWidget *parentWindow)
{	
	//ndxml *create_template = GetCreateTemplate(xml, getConfig());
	ndxml *create_template = getNewTemplate(xml, parentWindow);
	if (!create_template) {
		return NULL;
	}

	int tempType = 0;
	char *p = (char*)ndxml_getattr_val(create_template, "create_type");
	if (p)	{
		tempType = atoi(p);
	}

	if (tempType == TEMPLATE_LIST){
		//show with tree-ctrl
		NewNodeDialog dlg;
		dlg.InitTemplate(create_template);
		if (dlg.exec() != QDialog::Accepted) {
			return NULL;
		}
		create_template = dlg.getSelNode();
		p = (char*)ndxml_getattr_val(create_template, "create_type");
		if (p) {
			tempType = atoi(p);
		}
#if 0 //this old code
		ndxml *temp_root = ndxml_getnode(getConfig(), _GetReverdWord(ERT_TEMPLATE));
		if (!temp_root)
			return NULL;
		std::vector<const char*> creat_temp_list;
		ListDialog dlg(parentWindow);
		for (int i = 0; i < ndxml_getsub_num(create_template); ++i){
			ndxml *subnode = ndxml_refsubi(create_template, i);
			const char *temp_real_name = ndxml_getval(subnode);
			ndxml *temp_node = ndxml_refsub(temp_root, temp_real_name);
			QString str1;
			if (temp_node){
				str1 = _GetXmlName(temp_node, &m_alias);
			}
			else{
				str1 = temp_real_name;
			}

			dlg.m_selList.push_back(str1);
			creat_temp_list.push_back(temp_real_name);
		}

		dlg.InitList();
		if (dlg.exec() != QDialog::Accepted) {
			return NULL;
		}

		int seled = dlg.GetSelect();
		if (seled == -1) {
			return NULL;
		}
		create_template = ndxml_refsub(temp_root, creat_temp_list[seled]);
		if (!create_template){
			return NULL;
		}
		p = (char*)ndxml_getattr_val(create_template, "create_type");
		if (p)	{
			tempType = atoi(p);
		}
#endif

	}
	//begin create
	ndxml *new_xml = NULL;
	if (tempType == TEMPLATE_CREAT){
		new_xml = CreateByTemplate(xml, create_template);
		if (new_xml) {
			SetXmlName(new_xml, xml);
		}
	}
	else if (tempType == TEMPLATE_DIRECT) {
		for (int i = 0; i < ndxml_num(create_template); i++)	{
			ndxml *sub1 = ndxml_refsubi(create_template, i);
			new_xml = ndxml_copy(sub1);
			if (new_xml) {
				ndxml_insert(xml, new_xml);
				SetXmlName(new_xml, xml);
			}
		}
	}

	//handle  auto index 
	if (new_xml){
		setXmlValueByAutoIndex(new_xml,xml);
	}
	return new_xml;
}

ndxml *apoEditorSetting::AddNewXmlByTempName(ndxml *xml, const char *templateName)
{
	ndxml *root = getConfig();
	ndxml *template_root = ndxml_getnode(root,"create_template");
	if (!template_root)
		return NULL;

	ndxml *create_template = ndxml_refsub(template_root, templateName);
	if (!create_template)	{
		return NULL;
	}
	while (create_template) {
		const char *pRef = ndxml_getattr_val(create_template, "ref_from");
		if (pRef && (0 == ndstricmp(pRef, "yes") || 0 == ndstricmp(pRef, "true")))	{
			create_template = LogicEditorHelper::_getRefNode(create_template, ndxml_getval(create_template));
		}
		else {
			break;
		}
	}
	

	int tempType = 0;
	char *p = (char*)ndxml_getattr_val(create_template, "create_type");
	if (p)	{
		tempType = atoi(p);
	}
	//begin create
	ndxml *new_xml = NULL;
	if (tempType == TEMPLATE_CREAT){
		new_xml = CreateByTemplate(xml, create_template);
		if (new_xml) {
			SetXmlName(new_xml, xml);
		}
	}
	else if (tempType == TEMPLATE_DIRECT) {
		for (int i = 0; i < ndxml_num(create_template); i++)	{
			ndxml *sub1 = ndxml_refsubi(create_template, i);
			new_xml = ndxml_copy(sub1);
			if (new_xml) {
				ndxml_insert(xml, new_xml);
				SetXmlName(new_xml, xml);
			}
		}
	}

	//handle  auto index 
	if (new_xml){
		int num = ndxml_getsub_num(new_xml);
		for (int i = 0; i < num; i++)	{
			ndxml *sub = ndxml_getnodei(new_xml, i);
			nd_assert(sub);
			setXmlValueByAutoIndex(sub, xml);
		}
	}
	return new_xml;
}


ndxml* apoEditorSetting::getNewTemplate(ndxml *xml, QWidget *parentWindow)
{
	ndxml *root = getConfig();
	const char*template_namelist = ndxml_getattr_val(xml, "create_template");
	if (!template_namelist) {
		return NULL;
	}
	ndxml *template_root = ndxml_getnode(root, "create_template");
	if (!template_root)
		return NULL;

	text_vect_t tempNames;
	if (get_string_array(template_namelist, tempNames) == 0)  {
		return NULL;
	}
	else if (tempNames.size()==1) {
		ndxml *create_template = ndxml_refsub(template_root, tempNames[0].c_str());
		while (create_template) {
			const char *pRef = ndxml_getattr_val(create_template, "ref_from");
			if (pRef && (0 == ndstricmp(pRef, "yes") || 0 == ndstricmp(pRef, "true")))	{
				create_template = LogicEditorHelper::_getRefNode(create_template, ndxml_getval(create_template));
			}
			else {
				break;
			}
		}
		return create_template;
	}

	ListDialog dlg(parentWindow);
	std::vector<ndxml *> xmlnodes;
	for (text_vect_t::iterator it = tempNames.begin(); it != tempNames.end(); ++it)	{
		ndxml *create_template = ndxml_refsub(template_root, (*it).c_str() );
		while (create_template) {
			const char *pRef = ndxml_getattr_val(create_template, "ref_from");
			if (pRef && (0 == ndstricmp(pRef, "yes") || 0 == ndstricmp(pRef, "true")))	{
				create_template = LogicEditorHelper::_getRefNode(create_template, ndxml_getval(create_template));
			}
			else {
				break;
			}
		}

		if (create_template) {
			const char *name = _GetXmlName(create_template, NULL);
			nd_assert(name);
			xmlnodes.push_back(create_template);
			dlg.m_selList.push_back(QString(name));
		}
			
	}

	dlg.InitList();
	if (dlg.exec() != QDialog::Accepted) {
		return NULL;
	}

	int seled = dlg.GetSelect();
	if (seled == -1 || seled >=xmlnodes.size()) {
		return NULL;
	}

	return xmlnodes[seled];
}

int apoEditorSetting::GetCreateName(ndxml *xml_template, char *buf, size_t size)
{
	char *p;
	p = (char*)ndxml_getattr_val(xml_template, "name");
	if (!p) {
		return -1;
	}
	else {
		ndstrncpy(buf, p, size);
	}
	return 0;

}

ndxml * apoEditorSetting::CreateByTemplate(ndxml *root, ndxml *xml_template)
{
	char *val, *p;
	char name[128];
	if (0 != GetCreateName(xml_template, name, sizeof(name))){
		return NULL;
	}
	val = (char*)ndxml_getattr_val(xml_template, "value");

	ndxml *new_xml = ndxml_addsubnode(root, name, val);
	if (!new_xml)
		return NULL;

	ndxml *attr_xml = ndxml_refsub(xml_template, "attribute");
	if (attr_xml){
		for (int i = 0; i < ndxml_getsub_num(attr_xml); i++) {
			ndxml *sub_node = ndxml_refsubi(attr_xml, i);
			if (sub_node) {
				p = (char*)ndxml_getattr_val(sub_node, "name");
				val = (char*)ndxml_getattr_val(sub_node, "value");
				if (p && val) {
					ndxml_addattrib(new_xml, p, val);
				}
			}
		}
	}

	ndxml *child_node = ndxml_refsub(xml_template, "sub_node");
	if (child_node) {
		for (int i = 0; i < ndxml_getsub_num(child_node); i++) {
			ndxml *sub_template = ndxml_refsubi(child_node, i);
			if (sub_template)
				CreateByTemplate(new_xml, sub_template);
		}
	}
	return new_xml;
}

void apoEditorSetting::SetXmlName(ndxml *xml_node, ndxml *xmlParent)
{
	const char *autoIndexName = ndxml_getattr_val(xml_node, "name_auto_index");
	if (!autoIndexName || !*autoIndexName)	{
		return;
	}

	const char *pAutoIndex = ndxml_getattr_val(xmlParent, autoIndexName);
	const char *pName = ndxml_getattr_val(xml_node, "name");
	if (pAutoIndex && pName){
		int a = atoi(pAutoIndex);
		char buf[1024];
		ndsnprintf(buf, sizeof(buf), "%s%s", pName, pAutoIndex);
		ndxml_setattrval(xml_node, "name", buf);

		++a;
		ndsnprintf(buf, sizeof(buf), "%d", a);
		ndxml_setattrval(xmlParent, autoIndexName, buf);
	}

}


void apoEditorSetting::setXmlValueByAutoIndex(ndxml *node, ndxml *xmlParent)
{
	const char *autoName = ndxml_getattr_val(node, "auto_index_ref");
	if (autoName && *autoName){
		const char *pAutoIndex = ndxml_getattr_val(xmlParent, autoName);
		char buf[1024];

		if (pAutoIndex && *pAutoIndex){
			int a = atoi(pAutoIndex);

			ndsnprintf(buf, sizeof(buf), "%s%s", ndxml_getval(node), pAutoIndex);
			ndxml_setval(node, buf);

			++a;
			ndsnprintf(buf, sizeof(buf), "%d", a);
			ndxml_setattrval(xmlParent, autoName, buf);
		}
		else {
			ndsnprintf(buf, sizeof(buf), "%s1", ndxml_getval(node));
			ndxml_setval(node, buf);
			ndxml_setattrval(xmlParent, autoName, "2");
		}
		return;
	}

	else {
		int num = ndxml_getsub_num(node);
		for (int i = 0; i < num; i++)	{
			ndxml *sub = ndxml_getnodei(node, i);
			nd_assert(sub);
			setXmlValueByAutoIndex(sub, xmlParent);
		}
	}
	

}
