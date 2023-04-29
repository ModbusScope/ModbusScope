#ifndef DIAGNOSTICLOGMODEL_H
#define DIAGNOSTICLOGMODEL_H

#include <QObject>
#include "QAbstractListModel"
#include "diagnostic.h"


class DiagnosticModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit DiagnosticModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Diagnostic::LogSeverity dataSeverity(quint32 index) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;

    qint32 size() const;

    void clear();

    QString toString(quint32 idx) const;
    QString toExportString(quint32 idx) const;

    void setMinimumSeverityLevel(Diagnostic::LogSeverity maxSeverity);
    void addLog(QString category, Diagnostic::LogSeverity severity, qint32 timeOffset, QString message);

private:

    Diagnostic::LogSeverity _minSeverityLevel;
    QList<Diagnostic> _logList;
};

#endif // DIAGNOSTICLOGMODEL_H
