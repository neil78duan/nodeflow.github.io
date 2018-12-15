#ifndef SEARCHDLG_H
#define SEARCHDLG_H

#include "nd_common/nd_common.h"
#include "logic_parser/logic_editor_helper.h"
#include "apoScript/apoEditorSetting.h"

#include <QDialog>

namespace Ui {
class SearchDlg;
}

class SearchDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDlg(ndxml *xmlFile, QWidget *parent = 0);
    ~SearchDlg();

	ndxml *getResult();
private slots:
    void on_SearchButton_clicked();
    void on_CancelButton_clicked();

private:
	int searchText(const char *aimText);
	int searchOnlyCurrent(ndxml *xml, const char *aimText);
	int searchFromXml(ndxml *xml, const char *aimText);
	void storeSearchValue(ndxml *aimXml);
    Ui::SearchDlg *ui;
	ndxml *m_xmlFile;
	ndxml_root m_xmlResult;
};

// paste and copy for editor
class EditCopyPaste
{
public:
	EditCopyPaste();
	~EditCopyPaste();

	void clearClipboard();
	bool pushClipboard(ndxml *data,int userDataType = 0);
	ndxml *getClipboard();
	int getClipDataType(ndxml *data); //reference instructType
	int getUserDataType() {		return m_userDataType;	}

private:
	int m_userDataType;
	ndxml * m_xmlData;

	apoEditorSetting *m_setting;
};


#endif // SEARCHDLG_H
