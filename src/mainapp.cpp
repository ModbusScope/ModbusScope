#include "mainapp.h"

#include "mainwindow.h"

#include "graphdatamodel.h"
#include "notemodel.h"
#include "diagnosticmodel.h"
#include "settingsmodel.h"
#include "dataparsermodel.h"
#include "guimodel.h"

#include "fileselectionhelper.h"
#include "scopelogging.h"
#include "util.h"
#include "formatdatetime.h"

MainApp::MainApp(QStringList cmdArguments, QObject *parent) : QObject(parent)
{

    _pGuiModel = new GuiModel();
    _pSettingsModel = new SettingsModel();
    _pGraphDataModel = new GraphDataModel();
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

    ScopeLogging::Logger().initLogging(_pDiagnosticModel);
    FileSelectionHelper::setGuiModel(_pGuiModel);

    logInitialInfo();

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

void MainApp::logInitialInfo()
{
    qCInfo(scopeGeneralInfo) << QString("App start %1").arg(FormatDateTime::currentDateTime());

    qCInfo(scopeGeneralInfo) << QString("ModbusScope v%1").arg(Util::currentVersion());
#ifdef DEBUG
    qCInfo(scopeGeneralInfo) << QString("DEV git: %1:%2").arg(GIT_BRANCH, GIT_COMMIT_HASH);
#endif

    qCInfo(scopeGeneralInfo) << QString("Qt library v%1").arg(QLibraryInfo::version().toString());

    qCInfo(scopeGeneralInfo) << QString("OS: %1").arg(QSysInfo::prettyProductName());
}
