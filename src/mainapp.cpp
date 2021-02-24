#include "mainapp.h"

#include "mainwindow.h"

#include "graphdatamodel.h"
#include "notemodel.h"
#include "diagnosticmodel.h"
#include "settingsmodel.h"
#include "dataparsermodel.h"
#include "guimodel.h"

MainApp::MainApp(QStringList cmdArguments, QObject *parent) : QObject(parent)
{

    _pGuiModel = new GuiModel();
    _pSettingsModel = new SettingsModel();
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);
    _pNoteModel = new NoteModel();
    _pDiagnosticModel = new DiagnosticModel();
    _pDataParserModel = new DataParserModel();

    _pMainWin = new MainWindow(cmdArguments,
                               _pGuiModel,
                               _pSettingsModel,
                               _pGraphDataModel,
                               _pNoteModel,
                               _pDiagnosticModel,
                               _pDataParserModel);

    _pMainWin->show();

    /* Update interface via model */
    _pGuiModel->triggerUpdate();
    _pSettingsModel->triggerUpdate();
    _pDataParserModel->triggerUpdate();
}

MainApp::~MainApp()
{
    delete _pMainWin;

    delete _pDataParserModel;
    delete _pDiagnosticModel;
    delete _pNoteModel;
    delete _pGraphDataModel;
    delete _pSettingsModel;
    delete _pGuiModel;
}
