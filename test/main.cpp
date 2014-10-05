/****************************************************************************
**
**
****************************************************************************/

#include <QApplication>
#include <QtDebug>
#include <QDir>
#include <QFile>

#include "dialog.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString fp = QDir::currentPath() + "/" + "log.txt";
    FILE *logf;

    logf = fopen(fp.toStdString().c_str(), "a+");
    if ( ! logf ) {
        return;
    }

    switch (type) {
    case QtDebugMsg:
        //fprintf(logf, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        fprintf(logf, "Debug: %s\n", localMsg.constData());
        break;
    case QtWarningMsg:
        //fprintf(logf, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        fprintf(logf, "Warning: %s\n", localMsg.constData());
        break;
    case QtCriticalMsg:
        //fprintf(logf, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        fprintf(logf, "Critical: %s\n", localMsg.constData());
        break;
    case QtFatalMsg:
        //fprintf(logf, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        fprintf(logf, "Fatal: %s\n", localMsg.constData());
        break;
    }
    fclose(logf);
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QApplication app(argc, argv);
    Dialog dialog;

    dialog.show();
    return app.exec();
}
