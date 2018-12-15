#ifndef TEXTEDITORPOPDLG_H
#define TEXTEDITORPOPDLG_H

#include <QDialog>
#include <QString>

namespace Ui {
class TextEditorPopDlg;
}

class TextEditorPopDlg : public QDialog
{
    Q_OBJECT

public:
    explicit TextEditorPopDlg(QWidget *parent = 0);
    ~TextEditorPopDlg();
	
	void setTips(const QString &tips);
	void setMyText(const char *text);
	bool getMyText(QString &outText);

private:
    Ui::TextEditorPopDlg *ui;
};

#endif // TEXTEDITORPOPDLG_H
