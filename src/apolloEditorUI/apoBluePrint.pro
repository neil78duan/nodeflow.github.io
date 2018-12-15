#-------------------------------------------------
#
# Project created by QtCreator 2017-11-18T15:15:42
#
#-------------------------------------------------


ndsdk_dir = ../../../ndsdk
apollo_library = ../../apollolib
apolib_dir = ../..

macx:{

QMAKE_MAC_SDK = macosx10.13
DEFINES += __ND_MAC__
platform_name = darwin_x86_64
# LIBS += -lnd_vm_dbg -lndclient_darwin_x86_64_d -llogic_parser_d

}
unix:!macx{
message(BUILD LINUX!)
DEFINES += __ND_LINUX__
platform_name = linux_x86_64

# LIBS += -lnd_vm_dbg -lndclient_linux_x86_64_d -liconv -llogic_parser_d
}

win32{
message(WIN32!)
platform_name = Win64
DEFINES += __ND_WIN__ DO_NOT_CONVERT_PRINT_TEXT
}

# LIBS += -L$$ndsdk_dir/lib  -L$$ndsdk_dir/lib/$$platform_name -L$$apolib_dir/lib/$$platform_name
DEFINES += X86_64

INCLUDEPATH += $$ndsdk_dir/include \
        $$apolib_dir/include \
        $$apolib_dir/include/apoScript


#----------------------------------------
QT       += widgets

CONFIG(debug, debug|release) {
    message(BUILD win32 -debug)
#    LIBS += -lndclient_s_d -llogic_parser_d
    DEFINES +=  ND_DEBUG
    TARGET = ../$$apollo_library/lib/$$platform_name/apoBluePrint_d
} else {
#    LIBS += -lndclient_s -llogic_parser
    TARGET = ../$$apollo_library/lib/$$platform_name/apoBluePrint
}

TEMPLATE = lib
CONFIG += staticlib

unix {
    target.path = /usr/lib
    INSTALLS += target
}

SOURCES += \
    apoScript/xmldialog.cpp \
    apoScript/listdialog.cpp \
    apoScript/dragtree.cpp \
    apoScript/apoBaseExeNode.cpp \
    apoScript/apoUiBezier.cpp \
    apoScript/apoUiExeNode.cpp \
    apoScript/apoUiMainEditor.cpp \
    apoScript/mainwindow.cpp \
    apoScript/editorframe.cpp \
    apoScript/apoXmlTreeView.cpp \
    apoScript/apoUiDetailView.cpp \
    apoScript/apoUiParam.cpp \
    apoScript/apoUiXmlTablesWidget.cpp \
    apoScript/apoEditorSetting.cpp \
    apoScript/runFuncDialog.cpp \
    apoScript/apoUiDebugger.cpp \
    apoScript/searchdlg.cpp \
    apoScript/texteditorpopdlg.cpp \
    apoScript/newnodedialog.cpp \
    apoScript/lineinputdialog.cpp

HEADERS +=\
    $$apolib_dir/include/apoScript/xmldialog.h \
    $$apolib_dir/include/apoScript/listdialog.h \
    $$apolib_dir/include/apoScript/dragtree.h \
    $$apolib_dir/include/apoScript/myqtitemctrl.h \
    $$apolib_dir/include/apoScript/apoBaseExeNode.h \
    $$apolib_dir/include/apoScript/apoUiBezier.h \
    $$apolib_dir/include/apoScript/apoUiExeNode.h \
    $$apolib_dir/include/apoScript/apoUiMainEditor.h \
    $$apolib_dir/include/apoScript/apouinodedef.h \
    $$apolib_dir/include/apoScript/mainwindow.h \
    $$apolib_dir/include/apoScript/editorFrame.h \
    $$apolib_dir/include/apoScript/apoXmlTreeView.h \
    $$apolib_dir/include/apoScript/apoUiDetailView.h \
    $$apolib_dir/include/apoScript/apoUiParam.h \
    $$apolib_dir/include/apoScript/apoUiXmlTablesWidget.h \
    $$apolib_dir/include/apoScript/runFuncDialog.h \
    $$apolib_dir/include/apoScript/apoUiDebugger.h \
    $$apolib_dir/include/apoScript/searchdlg.h \
    apoScript/texteditorpopdlg.h \
    apoScript/newnodedialog.h \
    apoScript/lineinputdialog.h

FORMS    += \
    apoScript/listdialog.ui \
    apoScript/xmldialog.ui \
    apoScript/mainwindow.ui \
    apoScript/mainwindow.ui \
    apoScript/runFuncDialog.ui \
    apoScript/searchdlg.ui \
    apoScript/texteditorpopdlg.ui \
    apoScript/newnodedialog.ui \
    apoScript/lineinputdialog.ui
