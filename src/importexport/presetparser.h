#ifndef PRESETPARSER_H
#define PRESETPARSER_H

#include <QObject>
#include <QMessageBox>
#include <QDomDocument>
#include <QDateTime>

class PresetParser : public QObject
{
    Q_OBJECT
public:
		
    typedef struct _Preset
    {
        _Preset() : commentSequence(QString("")), column(1), labelRow(1),
                    dataRow(1),bDynamicSession(false), bTimeInMilliSeconds(true), keyword(QString("")), bStmStudioCorrection(false) {}

        QString name;
        QChar fieldSeparator;
        QChar decimalSeparator;
        QChar thousandSeparator;
        QString commentSequence;
        quint32 column;
        qint32 labelRow;
        quint32 dataRow;
        
        bool bDynamicSession;
        bool bTimeInMilliSeconds;

        QString keyword;

        bool bStmStudioCorrection;
        
    } Preset;

    explicit PresetParser();

    virtual void parsePresets(QString fileContent);

    virtual PresetParser::Preset preset(quint32 index);
    virtual quint32 presetCount();

signals:

public slots:

private:

    bool parsePresetTag(const QDomElement &element, Preset *pPreset);

    QList<PresetParser::Preset> _presetList;

};

#endif // PRESETPARSER_H
