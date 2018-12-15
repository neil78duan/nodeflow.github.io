#include "newnodedialog.h"
#include "ui_newnodedialog.h"
#include "apoEditorSetting.h"
#include "logic_parser/logic_editor_helper.h"

NewNodeDialog::NewNodeDialog( QWidget *parent) :
    QDialog(parent),
	ui(new Ui::NewNodeDialog), m_searchIterator(0)
{
	m_confTempRoot = 0;
	m_selNode = 0 ;
    ui->setupUi(this);
}

NewNodeDialog::~NewNodeDialog()
{
	if (m_searchIterator) {
		delete m_searchIterator;
		m_searchIterator = 0;
	}
    delete ui;
	
}


void NewNodeDialog::InitTemplate(ndxml *tempLists)
{
	ui->treeWidget->clear();
	//ui->treeWidget->setColumnCount(1);

	xmlTreeItem *root = new xmlTreeItem(ui->treeWidget, QStringList("Root"));
	root->setUserData(NULL);
	root->setExpanded(true);

	apoEditorSetting*pSetting = apoEditorSetting::getInstant();
	ndxml *tempConfigRoot = ndxml_getnode(pSetting->getConfig(), LogicEditorHelper::_GetReverdWord(LogicEditorHelper::ERT_TEMPLATE));
	if (!tempConfigRoot)
		return ;
	m_confTempRoot = tempConfigRoot;
	createTree(root, tempLists, tempConfigRoot);

}

ndxml * NewNodeDialog::getSelNode()
{
	return m_selNode;
}

bool NewNodeDialog::createTree(xmlTreeItem *hParent, ndxml *tempLists, ndxml *tempConfigRoot)
{
	for (int i = 0; i < ndxml_getsub_num(tempLists); ++i) {
		ndxml *subnode = ndxml_refsubi(tempLists, i);
		if (ndstricmp(ndxml_getname(subnode), "filter") == 0) {
			xmlTreeItem *treeNode = new xmlTreeItem(hParent, QStringList(LogicEditorHelper::_GetXmlName(subnode,NULL)));
			treeNode->setUserData(NULL);
			treeNode->setExpanded(true);
			createTree(treeNode,subnode, tempConfigRoot);
		}
		else {
			const char *temp_real_name = ndxml_getval(subnode);
			ndxml *temp_node = ndxml_refsub(tempConfigRoot, temp_real_name);
			QString str1;
			if (!temp_node) {
				continue;
			}
			str1 = LogicEditorHelper::_GetXmlName(temp_node, NULL);
			xmlTreeItem *leaf = new xmlTreeItem(hParent, QStringList(str1));
			leaf->setUserData((void*)temp_node);
		}
	}
	return true;
}

void NewNodeDialog::onSearch(const QString &searchText, bool restart )
{
	if (restart == true) {
		if (m_searchIterator) {
			delete m_searchIterator;
			m_searchIterator = NULL;
		}
	}

	if (!m_searchIterator) {
		m_searchIterator = new QTreeWidgetItemIterator(ui->treeWidget);
	}

	while (*(*m_searchIterator)) {
		QString str1 = (*(*m_searchIterator))->text(0);

		if (-1 != str1.indexOf(searchText)) {
			ui->treeWidget->setCurrentItem(*(*m_searchIterator));
			//ui->searchNext->setFocus();
			++(*m_searchIterator);
			return;
		}
		++(*m_searchIterator);
	}
	delete m_searchIterator;
	m_searchIterator = NULL;
}

void NewNodeDialog::on_ButtonOK_clicked()
{
	xmlTreeItem *curItem = (xmlTreeItem *)ui->treeWidget->currentItem();
	if (!curItem) {
		return;
	}
	m_selNode = (ndxml*) curItem->getUserData();
	if (!m_selNode ) {
		nd_logerror("can not select node\n");
		return;
	}
	QDialog::accept();
}

void NewNodeDialog::on_ButtonCancel_clicked()
{
	m_selNode = NULL;
	QDialog::reject();
}

void NewNodeDialog::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	on_ButtonOK_clicked();
}

void NewNodeDialog::on_editSearch_textChanged(const QString &arg1)
{
	if (arg1.size() > 0) {
		onSearch(arg1,true);
	}
}

void NewNodeDialog::on_editSearch_returnPressed()
{
	QString str1 = ui->editSearch->text();
	if (str1.size() > 0) {
		onSearch(str1,true);
	}
	//ui->editSearch->setFocus();
}

void NewNodeDialog::on_searchNext_clicked()
{
	QString str1 = ui->editSearch->text();
	if (str1.size() > 0) {
		onSearch(str1);
	}
	//ui->editSearch->setFocus();
}

void NewNodeDialog::keyPressEvent(QKeyEvent *e)
{
	switch (e->key()) {

	case Qt::Key_Enter:
	case Qt::Key_Return:
		e->ignore();
		break;
	case Qt::Key_Escape:
		reject();
		break;
	default:
		e->ignore();
		return;
	}

}