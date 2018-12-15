#include "texteditorpopdlg.h"
#include "ui_texteditorpopdlg.h"

TextEditorPopDlg::TextEditorPopDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextEditorPopDlg)
{
    ui->setupUi(this);
    ui->myText->setFocus();
}

TextEditorPopDlg::~TextEditorPopDlg()
{
    delete ui;
}


void TextEditorPopDlg::setTips(const QString &tips)
{
	ui->labTips->setText(tips);
}

void TextEditorPopDlg::setMyText(const char *text)
{
    //ui->myText->setText(text) ;
    ui->myText->setPlainText(text);
}

bool TextEditorPopDlg::getMyText(QString &outText)
{
    //outText = ui->myText->text();
    QTextDocument *doc = ui->myText->document();
    outText = doc->toPlainText();
	return true;
}
