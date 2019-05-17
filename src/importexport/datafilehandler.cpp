#include "datafileparser.h"


#include "datafilehandler.h"

DataFileHandler::DataFileHandler(GuiModel* pGuiModel, GraphDataModel* pGraphDataModel, NoteModel* pNoteModel) : QObject(nullptr)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;
    _pNoteModel = pNoteModel;
}

void DataFileHandler::loadDataFile(QString dataFilePath)
{
    DataFileParser dataParser;

    DataFileParser::FileData data;
    if (dataParser.processDataFile(dataFilePath, &data))
    {
        // Set to full auto scaling
        _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);

        // Set to full auto scaling
        _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);

        _pGraphDataModel->clear();
        _pGuiModel->setFrontGraph(-1);

        _pGraphDataModel->add(data.dataLabel, data.timeRow, data.dataRows);

        if (!data.colors.isEmpty())
        {
            for (int idx = 0; idx < data.dataLabel.size(); idx++)
            {
                _pGraphDataModel->setColor(static_cast<quint32>(idx), data.colors[idx]);
            }
        }

        _pNoteModel->clear();
        if (!data.notes.isEmpty())
        {
            foreach(Note note, data.notes)
            {
                _pNoteModel->add(note);
            }
        }
        _pNoteModel->setNotesDataUpdated(false);

        _pGuiModel->setFrontGraph(0);

        _pGuiModel->setProjectFilePath("");
        _pGuiModel->setDataFilePath(dataFilePath);

        _pGuiModel->setGuiState(GuiModel::DATA_LOADED);

    }
}
