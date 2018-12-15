#-------------------------------------------------
#
# Project created by QtCreator 2018-11-12T18:41:37
#
#-------------------------------------------------


ndsdk_dir = ../../../ndsdk
apolib_dir = ../../

macx:{

    QMAKE_MAC_SDK = macosx10.13
    DEFINES += __ND_MAC__
    platform_name = darwin_x86_64

    LIBS += -liconv

}
unix:!macx{
    message(BUILD LINUX!)
    DEFINES += __ND_LINUX__
    platform_name = linux_x86_64

    LIBS +=  -liconv
}

win32{
    message(WIN32!)
    platform_name = Win64

    DEFINES += __ND_WIN__ DO_NOT_CONVERT_PRINT_TEXT
}

CONFIG(debug, debug|release) {
    message(BUILD  -debug)
    LIBS += -lndcommon_s_d -lnfParser_s_d
    DEFINES +=  ND_DEBUG
} else {
    message(BUILD  -release)
    LIBS += -lndcommon_s -lnfParser_s
}

LIBS += -L$$ndsdk_dir/lib/$$platform_name -L$$apolib_dir/lib/$$platform_name

DEFINES += X86_64 ND_COMPILE_AS_DLL

INCLUDEPATH += $$ndsdk_dir/include \
        $$apolib_dir/include \
        $$apolib_dir/include/apoScript

#-------------------------------------------------
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = nfEditor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
    ../apolloEditorUI/apoScript/apoBaseExeNode.cpp \
    ../apolloEditorUI/apoScript/apoEditorSetting.cpp \
    ../apolloEditorUI/apoScript/apoUiBezier.cpp \
    ../apolloEditorUI/apoScript/apoUiDebugger.cpp \
    ../apolloEditorUI/apoScript/apoUiDetailView.cpp \
    ../apolloEditorUI/apoScript/apoUiExeNode.cpp \
    ../apolloEditorUI/apoScript/apoUiMainEditor.cpp \
    ../apolloEditorUI/apoScript/apoUiParam.cpp \
    ../apolloEditorUI/apoScript/apoUiXmlTablesWidget.cpp \
    ../apolloEditorUI/apoScript/apoXmlTreeView.cpp \
    ../apolloEditorUI/apoScript/dragtree.cpp \
    ../apolloEditorUI/apoScript/editorframe.cpp \
    ../apolloEditorUI/apoScript/listdialog.cpp \
    ../apolloEditorUI/apoScript/mainwindow.cpp \
    ../apolloEditorUI/apoScript/newnodedialog.cpp \
    ../apolloEditorUI/apoScript/runFuncDialog.cpp \
    ../apolloEditorUI/apoScript/searchdlg.cpp \
    ../apolloEditorUI/apoScript/texteditorpopdlg.cpp \
    ../apolloEditorUI/apoScript/xmldialog.cpp \
    workdirdialog.cpp \
    ../apolloEditorUI/apoScript/lineinputdialog.cpp

macx:{
    SOURCES += ../machoPlug/machoLoader.cpp
}

HEADERS += \
    ../apolloEditorUI/apoScript/newnodedialog.h \
    ../apolloEditorUI/apoScript/texteditorpopdlg.h \
    ../../include/apoScript/apoBaseExeNode.h \
    ../../include/apoScript/apoEditorSetting.h \
    ../../include/apoScript/apoUiBezier.h \
    ../../include/apoScript/apoUiDebugger.h \
    ../../include/apoScript/apoUiDetailView.h \
    ../../include/apoScript/apoUiExeNode.h \
    ../../include/apoScript/apoUiMainEditor.h \
    ../../include/apoScript/apouinodedef.h \
    ../../include/apoScript/apoUiParam.h \
    ../../include/apoScript/apoUiXmlTablesWidget.h \
    ../../include/apoScript/apoXmlTreeView.h \
    ../../include/apoScript/dragtree.h \
    ../../include/apoScript/editorFrame.h \
    ../../include/apoScript/listdialog.h \
    ../../include/apoScript/mainwindow.h \
    ../../include/apoScript/myqtitemctrl.h \
    ../../include/apoScript/runFuncDialog.h \
    ../../include/apoScript/searchdlg.h \
    ../../include/apoScript/xmldialog.h \
    workdirdialog.h \
    ../apolloEditorUI/apoScript/lineinputdialog.h

macx:{
    HEADERS += ../machoPlug/machoLoader.h \
    ../machoPlug/nfsection_def.h
}

FORMS += \
    ../apolloEditorUI/apoScript/listdialog.ui \
    ../apolloEditorUI/apoScript/mainwindow.ui \
    ../apolloEditorUI/apoScript/newnodedialog.ui \
    ../apolloEditorUI/apoScript/runFuncDialog.ui \
    ../apolloEditorUI/apoScript/searchdlg.ui \
    ../apolloEditorUI/apoScript/texteditorpopdlg.ui \
    ../apolloEditorUI/apoScript/xmldialog.ui \
    workdirdialog.ui \
    ../apolloEditorUI/apoScript/lineinputdialog.ui
