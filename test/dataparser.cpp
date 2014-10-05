#include "dataparser.h"

#include <Foundation/Foundation.h>

#include <QRegExp>

DataParser::DataParser(class *parent) :
    QObject(parent)
{
}

static CmdBlock *_getCmdBlock(QString s)
{
    QStringList sl;
    CmdBlock *cb;
    QByteArray data;
    bool ok;
    uint16_t addr;

    // check if s is a command line
    if (s[0] != '@') {
        qDebug() << "Cmd block parse error: " << s << "\n";
        return NULL;
    }

    sl = s.split(QRegExp("( )+"));
    foreach(QString si, sl) {
        if (si.isEmpty()) continue;
        if (si[0] == '@' ) {
            addr = si.right(4).toInt(&ok, 16);
            if ( ! ok ) {
                qDebug() << "convert " << si << "error\n";
            }
            continue;
        }
        uint8_t d = si.toInt(&ok, 16);
        if ( ! ok ) {
            qDebug() << "convert " << si << "error\n";
        }
        data.append(d);
    }
    cb = new CmdBlock();
    cb->addr = addr;
    cb->data = (uint8_t*) new char[data.size()];
    ::memcpy(cb->data, data.data(), data.size());
    cb->len = data.size();

    return cb;
}

qint8 DataParser::doParse(QString file)
{
    QFile inf(file);
    QString cmdl;

    if (inf.open(QIODevice::ReadOnly)) {
        QTextStream in(&inf);
        QStringList sl;

        while ( ! in.atEnd()) {
            sl.append(in.readLine());
        }

        foreach(QString s, sl) {
            // check the string is valid
            QRegExp exp("([0-9]|[a-f]|@| )+", Qt::CaseInsensitive);
            if ( ! exp.exactMatch(s)) {
                qDebug() << s << "is not valid\n";
                continue;
            }
            if (s[0] == '@') {
                if ( ! cmdl.isEmpty()) {
                    CmdBlock *blk = _getCmdBlock(cmdl);
                    if (blk) {
                        m_cblist.append(blk);
                    }
                }
                cmdl.clear();
                cmdl += s + " ";
                continue;
            }
            cmdl += s + " ";
        }
        if ( ! cmdl.isEmpty()) {
            CmdBlock *blk = _getCmdBlock(cmdl);
            if (blk) {
                m_cblist.append(blk);
            }
            cmdl.clear();
        }


    } else {
        qDebug() << "File: " << file << "Can NOT open\n";
        return -1;
    }
    return 0;
}
