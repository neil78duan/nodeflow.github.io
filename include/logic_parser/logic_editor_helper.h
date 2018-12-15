/* file logic_editor_helper.h
 *
 * editor common function
 *
 * create by duan 
 *
 * 2016-1-14
 */

#ifndef _LOGIC_EDITOR_HELPER_H_
#define _LOGIC_EDITOR_HELPER_H_


#include <string>
#include <vector>
#include <set>
#include "nd_common/nd_common.h"
#include "logic_parser/logicApi4c.h"

namespace LogicEditorHelper
{
	typedef std::vector<std::string> text_vect_t;

	enum CreateTemplate
	{
		TEMPLATE_CREAT,		//通过模板创建
		TEMPLATE_DIRECT,	//直接创建
		TEMPLATE_LIST		//选择列表
	};

	enum eDataType{			//数据类型
		EDT_NONE = 0,		//无类型
		EDT_ENUM,			//枚举类型
		EDT_BOOL,			//bool类型
		EDT_NUMERAL,		//数字类型
		EDT_STRING,		//字符类型
		EDT_PASSWORD,		//密码类型
		EDT_HIDE,			//隐藏节点
		EDT_IN_FILE,		//输入文件
		EDT_OUT_FILE,		//输出文件
		EDT_DIR,			//目录
		EDT_REF_DEFINED,
		EDT_USER_DEFINE,
		EDT_KEY_VAL,		//可以key-value值,显示key,实际值是value
		EDT_MULTI_TEXT		//multi-line-text
	};
	enum eReservedType {
		ERT_KIND = 0,
		ERT_VALUE,
		ERT_NAME,
		ERT_CREATETIME,
		ERT_DESC,
		ERT_EXPAND,
		ERT_RW,
		ERT_PARAM,
		ERT_DELETE,
		ERT_TEMPLATE,
		ERT_REF_TYPE,
		ERT_DATA_DEFINE,
		ERT_EXPAND_LIST,
		ERT_RESTRICT,
		ERT_COMP_COND,
		ERT_COMP_VALUE,
		ERT_USER_DEFINE_PARAM,
		ERT_AUTO_INDEX,
		ERT_EXPAND_STAT,
		ERT_ENABLE_DRAG,
		ERT_NEED_CONFIRM,
		ERT_KEY,
		ERT_KEY_VALUE,
		ERT_BE_REF_ONLY,
		ERT_REPLACE_VAL,
		ERT_CREATE_LABEL,
		ERT_VAR_NAME ,
		ERT_RUN_SEQ_LINE,
		ERT_BREAK_ANCHOR,
		ERT_ONLY_DEBUG,
		ERT_BREAKPOINT,
		ERT_DISPLAY_CHILDREN,
		ERT_BLUEPRINT_CTRL,
		ERT_BLUEPRINT_TIPS,
		ERT_BLUEPRINT_DEFNEXT,
		ERT_REAL_CMD ,
	};

	LOGIC_PARSER_CLASS const char *_GetDataTypeName(eDataType dataType);
	LOGIC_PARSER_CLASS const char *_GetReverdWord(eReservedType rType);

	class  LOGIC_PARSER_CLASS  CXMLAlias {
	public:
		struct sAlias {
			std::string str_var;
			std::string str_alias;
		};
        CXMLAlias() : m_num(0), m_created(0), m_pBuf(0){}
		virtual ~CXMLAlias() {
			if (m_pBuf) delete[] m_pBuf;
		}

		sAlias &operator[] (int n){ return m_pBuf[n]; }
		int Create(ndxml *xmlAlias);
		const char *GetAlia(const char *valname);
		int m_num;
		int m_created;
		sAlias *m_pBuf;

	};

	//检查XML 的attribute 是否是保留字
	LOGIC_PARSER_CLASS int CheckReserved(const char *param);
	LOGIC_PARSER_CLASS bool CheckCanDelete(ndxml *xml);
	//得到数据类型 eDataType
	LOGIC_PARSER_CLASS int CheckDataType(const char *param);

	LOGIC_PARSER_CLASS ndxml *getDefinedType(ndxml_root *root, const char *defTypeName);
	//得到xml值的类型
	LOGIC_PARSER_CLASS int GetXmlValType(ndxml *xml, ndxml_root *root);

	LOGIC_PARSER_CLASS const char *_getXmlParamVal(ndxml *xml, ndxml_root *root, eReservedType paramName);

	inline const char *GetEnumValText(ndxml *xml, ndxml_root *root)
	{
		return _getXmlParamVal(xml, root, ERT_VALUE);
	}

	inline const char *GetKeyValueText(ndxml *xml, ndxml_root *root)
	{
		return _getXmlParamVal(xml, root, ERT_KEY);
	}
	inline const char *GetKeyValue(ndxml *xml, ndxml_root *root)
	{
		return _getXmlParamVal(xml, root, ERT_KEY_VALUE);
	}
	
	inline ndxml *GetCompCond(ndxml *xml)
	{
		const char *pCompCondName = ndxml_getattr_val(xml, "comp_cond");
		if(pCompCondName){
			return ndxml_getnode(xml, pCompCondName);
		}
		return NULL ;
	}

	template<class TTextList>
	int _getEnumMembers(ndxml *xml, ndxml_root *config_root, TTextList& texts, eReservedType typeName)
	{
		const char *pValue = _getXmlParamVal(xml, config_root, typeName);
		if (!pValue)
			return 0;
		char subtext[128];

		do 	{
			subtext[0] = 0;
			pValue = ndstr_nstr_end(pValue, subtext, ',', sizeof(subtext));
			if (subtext[0])	{
				texts.push_back((subtext));
			}
			if (pValue && *pValue == ',') {
				++pValue;
			}
		} while (pValue && *pValue);
		return(int) texts.size();
	}


	template<class TTextList>
	int getEnumList(ndxml *xml, ndxml_root *config_root, TTextList& texts)
	{
		return _getEnumMembers(xml, config_root, texts, ERT_VALUE);
	}


	template<class TTextList>
	int getKeyList(ndxml *xml, ndxml_root *config_root, TTextList& texts)
	{
		return _getEnumMembers(xml, config_root, texts, ERT_KEY);
	}

	template<class TTextList>
	int getKeyValueList(ndxml *xml, ndxml_root *config_root, TTextList& texts)
	{
		return _getEnumMembers(xml, config_root, texts, ERT_KEY_VALUE);
	}

	LOGIC_PARSER_CLASS const char* GetXmlParam(ndxml *xml);
	LOGIC_PARSER_CLASS const char *_GetXmlName(ndxml *xml, CXMLAlias *alias);
	LOGIC_PARSER_CLASS const char *_GetNodeComment(ndxml *xml);

	LOGIC_PARSER_CLASS const char *_GetXmlDesc(ndxml *xml);

	LOGIC_PARSER_CLASS ndxml *_getRefNode(ndxml*node,const char *xmlPath);
	LOGIC_PARSER_CLASS const char *_getRefNodeAttrName(/*ndxml *node,*/ const char *xmlPath);

	LOGIC_PARSER_CLASS bool _CheckIsChild(ndxml *childXml, const char *parentName);

	//检测节点是否隐藏
	LOGIC_PARSER_CLASS bool CheckHide(ndxml *xml);

	//检测是否是扩展显示所有子节点
	LOGIC_PARSER_CLASS bool CheckExpand(ndxml *xml);

	LOGIC_PARSER_CLASS bool CheckDisplayChildren(ndxml *xml);
	
	LOGIC_PARSER_CLASS bool checkAddNewChild(ndxml *xml);

	LOGIC_PARSER_CLASS int get_string_array(const char *pValue, text_vect_t &texts);

	//检测是否是扩展显示所有子节点
	LOGIC_PARSER_CLASS bool CheckReadOnly(ndxml *xml);
	LOGIC_PARSER_CLASS ndxml *GetCreateTemplate(ndxml *xml, ndxml_root *root);

	LOGIC_PARSER_CLASS time_t getScriptChangedTime(ndxml_root *xmlFile);
	LOGIC_PARSER_CLASS void setScriptChangedTime(ndxml_root *xmlFile, time_t changedTime);

	LOGIC_PARSER_CLASS const char *getEditorVersion();

	LOGIC_PARSER_CLASS bool getRealVarName(const char *pInput, std::string &realname);
	LOGIC_PARSER_CLASS bool getDisplayVarName(const char *pInput, std::string &displayname);
	LOGIC_PARSER_CLASS bool buildValName(const char*realName, const char*display,  std::string &fullText);

	LOGIC_PARSER_CLASS const char *getRealValFromStr(const char *pInput, char *outpub, size_t bufsize);
	LOGIC_PARSER_CLASS const char *getDisplayNameFromStr(const char *pInput, char *outpub, size_t bufsize);
	LOGIC_PARSER_CLASS const char *buildDisplaNameValStr(const char *val, const char*dispName, char *outpub, size_t bufsize);

	LOGIC_PARSER_CLASS bool getBoolValue(const char *value);

	LOGIC_PARSER_CLASS int getClosureName( ndxml *closure, ndxml*funcNode, char *outpub, size_t bufsize);

	LOGIC_PARSER_CLASS const char *getModuleName(ndxml_root *scriptXml);
	LOGIC_PARSER_CLASS bool setModuleName(ndxml_root *scriptXml, const char *moduleName);


	LOGIC_PARSER_CLASS int textToEdit(const char *savedText, std::string &editText);
	LOGIC_PARSER_CLASS int textToSave(const char *srcFromEditor, std::string &saveText);

	inline int stringToOriginal(const char *inputText, std::string &dispText)
	{
		return textToEdit(inputText, dispText);
	}
	inline int stringToSaveFile(const char *inputText, std::string &saveText)
	{
		return textToSave(inputText, saveText);
	}
}
#endif
