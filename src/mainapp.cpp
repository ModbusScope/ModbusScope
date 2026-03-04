#include "mainapp.h"

#include "dialogs/mainwindow.h"

#include "models/dataparsermodel.h"
#include "models/diagnosticmodel.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/notemodel.h"
#include "models/settingsmodel.h"
#include "util/fileselectionhelper.h"
#include "util/formatdatetime.h"
#include "util/scopelogging.h"
#include "util/util.h"

MainApp::MainApp(QStringList cmdArguments, QObject *parent) : QObject(parent)
{
    /* Setup diagnostic model and logging before all the rest */
    _pDiagnosticModel = new DiagnosticModel();
    ScopeLogging::Logger().initLogging(_pDiagnosticModel);

    _pGuiModel = new GuiModel();
    _pSettingsModel = new SettingsModel();
    _pGraphDataModel = new GraphDataModel();
    _pNoteModel = new NoteModel();
    _pDataParserModel = new DataParserModel();

    _pMainWin = new MainWindow(cmdArguments,
                               _pGuiModel,
                               _pSettingsModel,
                               _pGraphDataModel,
                               _pNoteModel,
                               _pDiagnosticModel,
                               _pDataParserModel);

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
    qCInfo(scopeGeneralInfo) << QString("ModbusScope v%1").arg(Util::currentVersion());
#ifdef DEBUG
    qCInfo(scopeGeneralInfo) << QString("DEV git: %1:%2").arg(GIT_BRANCH, GIT_COMMIT_HASH);
#endif

    qCInfo(scopeGeneralInfo) << QString("Qt library v%1").arg(QLibraryInfo::version().toString());

    qCInfo(scopeGeneralInfo) << QString("OS: %1").arg(QSysInfo::prettyProductName());

    qCInfo(scopeGeneralInfo) << QString("Locale language code: %1").arg(QLocale().bcp47Name());
    qCInfo(scopeGeneralInfo) << QString("Locale decimal separator: %1").arg(QLocale().decimalPoint());
    qCInfo(scopeGeneralInfo) << QString("Locale group separator: %1").arg(QLocale().groupSeparator());
}
