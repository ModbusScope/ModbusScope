#ifndef DATAPARSERMODEL_H
#define DATAPARSERMODEL_H

#include <QLocale>

#include <QObject>

class DataParserModel : public QObject
{
    Q_OBJECT
public:
    explicit DataParserModel(QObject *parent = nullptr);
    ~DataParserModel();

    void triggerUpdate(void);
    void resetSettings();

    QChar fieldSeparator() const;
    QChar groupSeparator() const;
    QChar decimalSeparator() const;
    QString commentSequence() const;
    quint32 dataRow() const;
    quint32 column() const;
    qint32 labelRow() const;
    bool timeInMilliSeconds() const;
    bool stmStudioCorrection() const;
    QString dataFilePath();
    
    void setFieldSeparator(QChar fieldSeparator);
    void setGroupSeparator(QChar groupSeparator);
    void setDecimalSeparator(QChar decimalSeparator);
    void setCommentSequence(QString commentSequence);
    void setDataRow(quint32 dataRow);
    void setColumn(quint32 column);
    void setLabelRow(qint32 labelRow);
    void setTimeInMilliSeconds(bool timeInMilliSeconds);
    void setStmStudioCorrection(bool stmStudioCorrection);
    void setDataFilePath(QString path);

signals:
    void fieldSeparatorChanged();
    void groupSeparatorChanged();
    void decimalSeparatorChanged();
    void commentSequenceChanged();
    void dataRowChanged();
    void columnChanged();
    void labelRowChanged();
    void timeInMilliSecondsChanged();
    void stmStudioCorrectionChanged();
    void dataFilePathChanged();

private:

    QChar _fieldSeparator;
    QChar _groupSeparator;
    QChar _decimalSeparator;
    QString _commentSequence;
    quint32 _dataRow;
    quint32 _column;
    qint32 _labelRow;
    bool _bTimeInMilliSeconds;
    bool _bStmStudioCorrection;
    QString _dataFilePath;

};

#endif // DATAPARSERMODEL_H
