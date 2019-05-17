#ifndef DATAFILEHANDLER_H
#define DATAFILEHANDLER_H

#include <QObject>

#include "guimodel.h"
#include "graphdatamodel.h"
#include "notemodel.h"

class DataFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit DataFileHandler(GuiModel* pGuiModel, GraphDataModel* pGraphDataModel, NoteModel* pNoteModel);

    void loadDataFile(QString dataFilePath);

signals:

public slots:
    void selectDataImportFile();

private:

    GuiModel* _pGuiModel;
    GraphDataModel* _pGraphDataModel;
    NoteModel* _pNoteModel;

};

#endif // DATAFILEHANDLER_H
