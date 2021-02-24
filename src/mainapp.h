#ifndef MAINAPP_H
#define MAINAPP_H

#include <QObject>

class MainWindow;
class GraphDataModel;
class NoteModel;
class DiagnosticModel;
class SettingsModel;
class DataParserModel;
class GuiModel;

class MainApp : public QObject
{
    Q_OBJECT
public:
    explicit MainApp(QStringList cmdArguments, QObject *parent = nullptr);
    ~MainApp();

signals:

private:

    SettingsModel * _pSettingsModel;
    GraphDataModel * _pGraphDataModel;
    NoteModel * _pNoteModel;
    DiagnosticModel * _pDiagnosticModel;
    GuiModel * _pGuiModel;
    DataParserModel * _pDataParserModel;

    MainWindow* _pMainWin;

};

#endif // MAINAPP_H
