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

    static DiagnosticModel& Logger();

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Diagnostic::LogSeverity dataSeverity(quint32 index) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;

    qint32 size() const;

    void clear();

    void setMaxSeverityLevel(Diagnostic::LogSeverity maxSeverity);

    void addLog(Diagnostic& log);
    void addCommunicationLog(Diagnostic::LogSeverity severity, QString message);

signals:

public slots:

private:

    QList<Diagnostic> _logList;
    Diagnostic::LogSeverity _maxSeverityLevel;
};

inline DiagnosticModel& DiagnosticModel::Logger()
{
    static DiagnosticModel errorLogModel;
    return errorLogModel;
}


#endif // DIAGNOSTICLOGMODEL_H
