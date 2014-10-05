#ifndef DATAPARSER_H
#define DATAPARSER_H

#include <QObject>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QList>

#include <Foundation/Foundation.h>

#include "cmdblock.h"


class DataParser : NSObject
{
    Q_OBJECT
public:
    explicit DataParser(QObject *parent = 0);
    qint8 doParse(QString file);
    QList<CmdBlock*> m_cblist;

    ~DataParser() {
        foreach (CmdBlock *cb, m_cblist) {
            if (cb) delete cb;
        }
    }

signals:

public slots:

};

#endif // DATAPARSER_H
