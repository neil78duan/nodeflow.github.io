#include "listdialog.h"
#include "ui_listdialog.h"
#include <string>
#include "logic_parser/logic_editor_helper.h"

ListDialog::ListDialog(QWidget *parent, const char *title) :
    QDialog(parent),
    ui(new Ui::ListDialog)
{
    ui->setupUi(this);
    m_selIndex = -1 ;

}

ListDialog::~ListDialog()
{
    delete ui;
}

void ListDialog::on_mylist_doubleClicked(const QModelIndex &index)
{
    m_selIndex = index.row() ;
	QDialog::accept();
}

void ListDialog::InitList()
{
    for(std_vctstrings_t::iterator it= m_selList.begin(); it!= m_selList.end();++it) {
        std::string str1 = it->toStdString() ;
        char buf[1024];
        const char *p = LogicEditorHelper::getDisplayNameFromStr(str1.c_str(),buf, sizeof(buf)) ;
        ui->mylist->addItem(p);
    }

}

void ListDialog::on_mylist_clicked(const QModelIndex &index)
{
    m_selIndex = index.row() ;
}

void ListDialog::on_Search_clicked()
{
	QString searchQstr = ui->searchText->text();
	if (searchQstr.size() == 0)	{
		m_selIndex = -1;
		ui->mylist->clearSelection();
		return;
	}

	int i = 0;
	std_vctstrings_t::iterator searchBegin = m_selList.begin();
	if (m_selIndex != -1){
		if (m_selIndex >= m_selList.size())	{
			m_selIndex = 0;
		}
		searchBegin += m_selIndex+1;
		i = m_selIndex+1;
	}

	m_selIndex = -1;
	for (; searchBegin != m_selList.end(); ++searchBegin) {
		if (-1 != searchBegin->indexOf(searchQstr)) {
			m_selIndex = i;
			break;
		}
		++i;
	}

	ui->mylist->clearSelection();
	if (m_selIndex >=0 ){
		ui->mylist->setCurrentRow(m_selIndex);
	}
}


void ListDialog::accept()
{
	QWidget *pCtrl = focusWidget();
	if (pCtrl) {
		//on search 
		if (pCtrl == (QWidget *)ui->searchText) {
			on_Search_clicked();
			return;
		}
		else if (pCtrl == (QWidget *) ui->mylist) {

			QModelIndex selIndex= ui->mylist->currentIndex(); 
			if (selIndex.row() != -1) {
				on_mylist_doubleClicked(selIndex);
				return;
			}
		}
		else if(pCtrl ==(QWidget *)ui->buttonBox->button(QDialogButtonBox::Ok)) {
			
			QModelIndex selIndex = ui->mylist->currentIndex();
			if (selIndex.row() == -1) {
				m_selIndex = 0;
				ui->mylist->setCurrentRow(m_selIndex);
				return;
			}
			on_mylist_doubleClicked(selIndex);
		}
	}
	else {
		nd_logmsg("please select the node you want create \n");
	}
	//QDialog::accept();
}
void ListDialog::reject()
{
	QDialog::reject();
}