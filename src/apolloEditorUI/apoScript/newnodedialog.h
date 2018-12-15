#ifndef NEWNODEDIALOG_H
#define NEWNODEDIALOG_H

#include "dragtree.h"
#include "myqtitemctrl.h"
#include "nd_common/nd_common.h"
#include "logic_parser/logic_editor_helper.h"

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class NewNodeDialog;
}

class NewNodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewNodeDialog(QWidget *parent = 0);
    ~NewNodeDialog();
	void InitTemplate(ndxml *tempLists);
	ndxml * getSelNode();
private slots:
    void on_ButtonOK_clicked();

    void on_ButtonCancel_clicked();
	

    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_editSearch_textChanged(const QString &arg1);

    void on_editSearch_returnPressed();

    void on_searchNext_clicked();

private:

	void keyPressEvent(QKeyEvent *) ;
	bool createTree(xmlTreeItem *hParent,ndxml *tempList, ndxml *tempConfigRoot);
	void onSearch(const QString &searchText,bool restart= false);
    Ui::NewNodeDialog *ui;
	ndxml *m_confTempRoot;
	ndxml *m_selNode;

	QTreeWidgetItemIterator* m_searchIterator;
};

#endif // NEWNODEDIALOG_H
