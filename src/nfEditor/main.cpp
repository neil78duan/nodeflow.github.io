
#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>

#include <qtextcodec.h>
#include <QString>
#include <QDir>

//必须先包含qt在包含其他
#include "nd_common/nd_common.h"
#include "logic_parser/logicEngineRoot.h"
#include "logic_parser/logic_compile.h"

#include "apoScript/apoEditorSetting.h"
#include "apoScript/editorFrame.h"
#include "workdirdialog.h"
//#include "apoScript/lineinputdialog.h"

#ifdef WIN32
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Advapi32.lib")
#endif


static void workingConfigInit()
{
    QString workingPath ;
#if defined (__ND_MAC__)
    const char *rootConfog = "../cfg/io_config.xml";
    if (!trytoGetSetting(workingPath)) {
         QMessageBox::critical(NULL, "Error", "can not get working path !");
        exit(1);
    }
#else
    const char *rootConfog = "../cfg/io_config.xml";
    workingPath = "../bin";

#endif
    const char *scriptConfig = "../cfg/editor_config_setting.json";

    if (!QDir::setCurrent(workingPath)) {
        QMessageBox::critical(NULL, "Error", "can not enter working path !");
        exit(1);
    }

    //use utf8
    ndstr_set_code(APO_QT_SRC_TEXT_ENCODE);

    if (!nd_existfile(scriptConfig)) {
        QString errTips;
        errTips.ndsprintf("can not open script setting file ,currentpath=%s!", nd_getcwd());
        QMessageBox::critical(NULL, "Error", errTips);

        exit(1);
    }
    if (!nd_existfile(rootConfog)) {
        QMessageBox::critical(NULL, "Error", "can not found root config file !");
        exit(1);
    }

    if (!apoEditorSetting::getInstant()->Init(rootConfog, scriptConfig, APO_QT_SRC_TEXT_ENCODE)) {
        QMessageBox::critical(NULL, "Error", "can not found root config file !");
        exit(1);
    }
    if (!LogicCompiler::get_Instant()->setConfigFile(scriptConfig)) {
        QMessageBox::critical(NULL, "Error", "load config file error, Please Restart", QMessageBox::Yes);
        exit(1);
    }

}


extern bool bp_init_console_input(QWidget *parent); 
int runEditor(int argc, char *argv[])
{
    QApplication a(argc, argv);
    workingConfigInit();

    //load dbl data
    //const char *package_file = apoEditorSetting::getInstant()->getProjectConfig("game_data_package_file");
    //if (package_file) {
     //   DBLDatabase::get_Instant()->LoadBinStream(package_file);
    //}

    EditorFrame mainFrame;
    if (!mainFrame.myInit()) {
        QMessageBox::critical(NULL, "Error", "init setting error!");
        exit(1);
    }

	bp_init_console_input(&mainFrame);
    mainFrame.showMaximized();
    return a.exec();

}

int main(int argc, char *argv[])
{
   return runEditor(argc, argv) ;
}
