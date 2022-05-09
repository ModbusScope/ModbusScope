#ifndef DATAPARSERMODEL_H
#define DATAPARSERMODEL_H

#include <QObject>

class DataParserModel : public QObject
{
    Q_OBJECT
public:
    explicit DataParserModel(QObject *parent = nullptr);
    ~DataParserModel();

    void triggerUpdate(void);
    void resetSettings();

    QString fieldSeparator() const;
    QString groupSeparator() const;
    QString decimalSeparator() const;
    QString commentSequence() const;
    quint32 dataRow() const;
    quint32 column() const;
    quint32 labelRow() const;
    bool timeInMilliSeconds() const;
    bool stmStudioCorrection() const;
    QString dataFilePath();
    
    void setFieldSeparator(QString fieldSeparator);
    void setGroupSeparator(QString groupSeparator);
    void setDecimalSeparator(QString decimalSeparator);
    void setCommentSequence(QString commentSequence);
    void setDataRow(quint32 dataRow);
    void setColumn(quint32 column);
    void setLabelRow(quint32 labelRow);
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

    QString _fieldSeparator;
    QString _groupSeparator;
    QString _decimalSeparator;
    QString _commentSequence;
    quint32 _dataRow;
    quint32 _column;
    quint32 _labelRow;
    bool _bTimeInMilliSeconds;
    bool _bStmStudioCorrection;
    QString _dataFilePath;

};

#endif // DATAPARSERMODEL_H
