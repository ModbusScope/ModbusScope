#ifndef SIGNALEDITORPANEL_H
#define SIGNALEDITORPANEL_H

#include "util/expressionreferences.h"
#include "util/graphindex.h"

#include <QButtonGroup>
#include <QPersistentModelIndex>
#include <QWidget>

class DataPointDescriptionCache;
class DataPointEditorWidget;
class DataPointService;
class GraphDataModel;
class SettingsModel;

namespace Ui {
class SignalEditorPanel;
}

class SignalEditorPanel : public QWidget
{
    Q_OBJECT

    friend class TestSignalEditorPanel;

public:
    explicit SignalEditorPanel(GraphDataModel* pGraphDataModel,
                               SettingsModel* pSettingsModel,
                               DataPointService* pDataPointService,
                               DataPointDescriptionCache* pDescriptionCache,
                               QWidget* parent = nullptr);
    ~SignalEditorPanel();

    void startAdd();
    void showAddMode();
    void editSignal(GraphIdx graphIdx);

signals:
    void editExpressionRequested(GraphIdx graphIdx);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void handleAdd();
    void onExpressionChanged();
    void setBuildError(const QString& error);
    void updateStatus();
    void onNameEdited(const QString& name);
    void onNameEditingFinished();
    void onAxisToggled();
    void onDataPointSelected(int row);
    void onEditExpression();
    void onModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void onRowsRemoved();
    void onDescriptionChanged(const QString& referenceText);

private:
    bool hasEditedRow() const;
    GraphIdx editedGraph() const;
    void applyMode(bool bEditMode);
    void refresh(bool bReloadEditor);
    void loadReference();
    void updateDataPointList();
    void updateExpressionLabel();
    QString referenceProblem() const;

    Ui::SignalEditorPanel* _pUi;
    DataPointEditorWidget* _pEditor;
    GraphDataModel* _pGraphDataModel;
    DataPointDescriptionCache* _pDescriptionCache;

    QButtonGroup _axisGroup;

    QString _buildError;
    QString _addedMessage;

    //! Edit mode or add mode. Edit mode can briefly have an invalid _editedRow: after its row is
    //! removed or the model is reset, until onRowsRemoved() switches to add mode. hasEditedRow()
    //! guards that window.
    bool _bEditMode{ false };

    //! Row in edit mode; invalid in add mode.
    QPersistentModelIndex _editedRow;

    //! Occurrences of data points in the edited expression, and the one loaded in the editor.
    QList<ExpressionReferences::Reference> _references;
    qsizetype _referenceIdx{ 0 };

    //! Set while the editor is disabled until the loaded data point is described.
    bool _bAwaitingDescription{ false };

    //! Expression the references were found in, or that the panel wrote; its own write does not reload the editor.
    QString _knownExpression;
};

#endif // SIGNALEDITORPANEL_H
