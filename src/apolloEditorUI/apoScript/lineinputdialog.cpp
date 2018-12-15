
#include "lineinputdialog.h"
#include "ui_lineinputdialog.h"

#include "logic_parser/logicParser.h"
#include "logic_parser/logicEngineRoot.h"
#include <string>

LineInputDialog::LineInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LineInputDialog)
{
    ui->setupUi(this);
}

LineInputDialog::~LineInputDialog()
{
    delete ui;
}

void LineInputDialog::on_pushButton_clicked()
{

    QDialog::accept();
}

QString LineInputDialog::getInput()
{
	return ui->lineEdit->text();
}

static QWidget *_mainWindow=NULL;
// QMainWindow* getMainWindow() 
// { 
// 	foreach(QWidget *w, qApp->topLevelWidgets()) 
// 		if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w))           
// 			return mainWin;    
// 	return nullptr; 
// }

QString bp_console_input()
{
	LineInputDialog dlg;
	if (dlg.exec() != QDialog::Accepted) {
		return QString();
	}
	return dlg.getInput();
}


int bp_get_user_input(ILogicParser *, LogicDataObj &data)
{
	LocalDebugger &debugger = LogicEngineRoot::get_Instant()->getGlobalDebugger();
	std::string inText;
	if (debugger.requestConsoleInput(inText)) {
		if (inText.size()) {
			data.InitSet(inText.c_str());
			return inText.size();
		}
	}
	return 0;
}

bool bp_init_console_input(QWidget *parent)
{
	_mainWindow = parent;

	LogicEngineRoot::setConsoleInput(bp_get_user_input);
	return true;
}
