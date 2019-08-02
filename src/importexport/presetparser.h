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

    void loadPresetsFromFile();
    QList<PresetParser::Preset> presetList();

signals:

public slots:

private:

    bool parseFile(QIODevice *device, QList<Preset> *pPresetList);
    bool parsePresetTag(const QDomElement &element, Preset *pPreset);

    QDomDocument _domDocument;
    QDateTime _lastModified;
    QList<PresetParser::Preset> _presetList;

    static const QString _presetFilename;
};

#endif // PRESETPARSER_H
