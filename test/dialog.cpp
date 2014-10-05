/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "dialog.h"
#include "crc_ccitt.h"
#include "dataparser.h"

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QProgressDialog>
#include <QtSerialPort/QSerialPortInfo>
#include <QCoreApplication>

extern QDebug *g_Logger;

QT_USE_NAMESPACE

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , m_com(NULL), m_progbar(NULL)
    , transactionCount(0)
    , serialPortLabel(new QLabel(tr("Goccia Serial port:")))
    , fileLabel(new QLabel(tr("File:")))
    , idLabel(new QLabel("Id:"))
    , idEdit(new QLineEdit())
    , runButton(new QPushButton(tr("Start")))
    , fileButton(new QPushButton(tr("File")))
{
    // Find Goccia COM
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (info.hasVendorIdentifier() && info.vendorIdentifier() == 0x0483) {
            if (info.hasProductIdentifier() && info.productIdentifier() == 0x5741) {
                // We get the right COM
                m_com = new QSerialPort(info);
                serialPortLabel->setText(serialPortLabel->text() + " " + info.portName());
                qDebug() << "FOUND GOCCIA COM: " << info.portName();
            }
        }
        QString s = QObject::tr("Port: ") + info.portName() + "\n"
                           + QObject::tr("Location: ") + info.systemLocation() + "\n"
                           + QObject::tr("Description: ") + info.description() + "\n"
                           + QObject::tr("Manufacturer: ") + info.manufacturer() + "\n"
                           + QObject::tr("Vendor Identifier: ") + (info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString()) + "\n"
                           + QObject::tr("Product Identifier: ") + (info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString()) + "\n"
                           + QObject::tr("Busy: ") + (info.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) + "\n";
        qDebug() << s;

    }



    if ( ! m_com ) {
        serialPortLabel->setText(serialPortLabel->text() + " " + "NOT FOUND");
    }

    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->addWidget(serialPortLabel, 0, 0);
    mainLayout->addWidget(fileLabel, 1, 0);
    mainLayout->addWidget(runButton, 0, 2);
    mainLayout->addWidget(fileButton, 1, 2);
    mainLayout->addWidget(idLabel, 2, 0);
    mainLayout->addWidget(idEdit, 2, 2);

    setLayout(mainLayout);

    setWindowTitle(tr("Goccia Updater 0.5"));

    connect(runButton, SIGNAL(clicked()),
            this, SLOT(transaction()));
    connect(fileButton, SIGNAL(clicked()),
            this, SLOT(selectFile()));
    connect(&thread, SIGNAL(response(QString)),
            this, SLOT(showResponse(QString)));
    connect(&thread, SIGNAL(error(QString)),
            this, SLOT(processError(QString)));
    connect(&thread, SIGNAL(timeout(QString)),
            this, SLOT(processTimeout(QString)));
}

void Dialog::selectFile()
{
    m_DataFile = QFileDialog::getOpenFileName(this,
                                              tr("Open Data File"), QDir::currentPath(), tr(""));
    fileLabel->setText(m_DataFile);
}

void Dialog::transaction()
{
    QFile file(m_DataFile);
    QMessageBox::StandardButton b;
    QString fileinfo;
    QByteArray filecont;

    if ( ! file.exists()) {
        QMessageBox::critical(this, "Critical Message", "File Not Exist");
        return;
    }
    if ( ! file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Critical Message", "File Can NOT Open");
        return;
    }
    filecont = file.readAll();
    fileinfo.append("file size: ");
    fileinfo.append(QString::number(file.size()) + "\n");
    fileinfo.append("file md5: ");
    fileinfo.append(QCryptographicHash::hash(filecont, QCryptographicHash::Md5).toHex() + "\n");
    fileinfo.append("\n");

    fileinfo.append("Do you want to write this file to Goccia device?\n");

    b = QMessageBox::information(this, "File info", fileinfo, QMessageBox::Yes | QMessageBox::No);
    if (b == QMessageBox::No) {
        return;
    }

    DataParser p;
    if ( p.doParse(m_DataFile) != 0) {
        QMessageBox::critical(this, "Critical Message", "Can NOT Parse Data File\n");
        return;
    }

    {
        // id
        CmdBlock *id_cmd = new CmdBlock();
        bool ok;
        uint8_t *id = (uint8_t*) new char[4];
        uint32_t id_num = idEdit->text().toInt(&ok, 0);

        if (ok) {
            /*
            QMessageBox::critical(this, "Critical Message", "id ERROR\n");
            m_com->close();
            this->show();
            return;
            */
            id[0] = ((uint8_t*)&id_num)[0];
            id[1] = ((uint8_t*)&id_num)[1];
            id[2] = ((uint8_t*)&id_num)[2];
            id[3] = ((uint8_t*)&id_num)[3];
            id_cmd->addr = 0x1800;
            id_cmd->data = id;
            id_cmd->len = 4;

            p.m_cblist.append(id_cmd);
        }
        else
        {
            qDebug() << "DO NOT touch the ID " << "\n";
        }

    }
//    qDebug() << "CMD BLOCK is: " << p.m_cblist.size() << "\n";
//    for (int i = 0; i < p.m_cblist.size(); i++) {
//        CmdBlock *cb = p.m_cblist[i];
//        char outs[16] = {0, };

//        ::snprintf(outs, 16, "%d", i + 1);
//        qDebug() << "index: " << outs << "\n";

//        ::snprintf(outs, 16, "%04x", cb->addr);
//        qDebug() << "addr: " << outs << "\n";

//        ::snprintf(outs, 16, "%d", cb->len);
//        qDebug() << "len: " << outs << "\n";
//        QString os;
//        for (int j = 0; j < cb->len; j++) {
//            ::snprintf(outs, 16, "%02x", cb->data[j]);
//            os.append(outs);
//            os.append(" ");
//        }
//        qDebug() << "Data: " << os;
//        qDebug() << "\n------------------\n";
//    }
//return;
    if ( ! m_com ) {
        //m_com = new QSerialPort("COM2");
        QMessageBox::critical(this, "Critical Message", "Can NOT Found Goccia Device\n");
        return;
    }
    if ( ! m_com->open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, "Critical Message", "Can NOT Open Goccia Device\n");
        return;
    }
    if ( ! m_com->setBaudRate(QSerialPort::Baud9600)) {
        QMessageBox::critical(this, "Critical Message", "Can NOT set BaudRate to 9600\n");
        return;
    }
    if ( ! m_com->setDataBits(QSerialPort::Data8)) {
        QMessageBox::critical(this, "Critical Message", "Can NOT set Data8\n");
        return;
    }
    if ( ! m_com->setParity(QSerialPort::NoParity)) {
        QMessageBox::critical(this, "Critical Message", "Can NOT set NoParity\n");
        return;
    }
    if ( ! m_com->setStopBits(QSerialPort::OneStop)) {
        QMessageBox::critical(this, "Critical Message", "Can NOT set OneStop\n");
        return;
    }
    if ( ! m_com->setFlowControl(QSerialPort::NoFlowControl)) {
        QMessageBox::critical(this, "Critical Message", "Can NOT set NoFlowControl\n");
        return;
    }

    uint16_t retry_times;

    // send $GO to Goccia
    QString go_cmd;
    go_cmd.sprintf("$GO\r\n");

    retry_times = 0;
    // progress bar
    m_progbar = new QProgressDialog("Find Goccia ... ", "cancel", 0, 30, this, 0);
    m_progbar->setValue(0);
    m_progbar->show();
    this->hide();

    while (retry_times <= 30)
    {
        QByteArray rb;
        QCoreApplication::processEvents();
        if (m_progbar->wasCanceled()) {
            delete m_progbar;
            m_com->close();
            this->show();
            return;
        }
        m_progbar->setValue(retry_times);
        qDebug() << go_cmd;
        if ( ! writeToCOM(go_cmd) ) {
            QMessageBox::critical(this, "Critical Message", "Can NOT send $GO cmd\n");
            m_com->close();
            delete m_progbar;
            this->show();
            return;
        }

        if ( ! readFromCOM(rb)) {
            QMessageBox::critical(this, "Critical Message", "GO no replay\n");
            m_com->close();
            delete m_progbar;
            this->show();
            return;
        }
        qDebug() << rb;

        if (rb.data()[0] != '$')
        {
            retry_times++;
            QThread::msleep(1000);
            continue;
        }
        break;
    }
    if (retry_times > 30)
    {
        QMessageBox::critical(this, "Critical Message", "GO no replay\n");
        m_com->close();
        delete m_progbar;
        this->show();
        return;
    }
    delete m_progbar;

    // now send $START to Goccia
    m_progbar = new QProgressDialog("Update Goccia ... ", "cancel", 0, p.m_cblist.count(), this, 0);
    m_progbar->setValue(0);
    m_progbar->show();
    this->hide();

    CmdBlock start_cb;
    QByteArray crc_data;
    uint16_t crc_sum;
    uint8_t cl_len = p.m_cblist.size();
    start_cb.addr = 0;
    start_cb.index = 0;
    start_cb.len = 1;
    start_cb.data = (uint8_t*) new char(cl_len);
    // crc16 this command
    crc_data.append(char(0x00));  // index
    crc_data.append(char(0x00));  // addr
    crc_data.append(char(0x00));  // addr
    crc_data.append(char(0x00));  // len
    crc_data.append(char(0x01));  // len
    crc_data.append(*start_cb.data);    // data

    crc_sum = crc16((uint8_t *)crc_data.data(), crc_data.size());
    QString start_cmd;
    start_cmd.sprintf("$START,%02X,%04X\r\n", cl_len, crc_sum);

    retry_times = 0;
    while (retry_times <= 3)
    {
        QByteArray rb;

        QCoreApplication::processEvents();
        qDebug() << start_cmd;

        if (m_progbar->wasCanceled()) {
            delete m_progbar;
            m_com->close();
            this->show();
            return;
        }

        if ( ! writeToCOM(start_cmd) ) {
            QMessageBox::critical(this, "Critical Message", "Can NOT send $START cmd\n");
            m_com->close();
            delete m_progbar;
            this->show();
            return;
        }

        if ( ! readFromCOM(rb)) {
            QMessageBox::critical(this, "Critical Message", "START no replay\n");
            m_com->close();
            delete m_progbar;
            this->show();
            return;
        }
        qDebug() << rb;

        if (rb.data()[0] != '$')
        {
            retry_times++;
            continue;
        }
        break;
    }
    if (retry_times > 3)
    {
        QMessageBox::critical(this, "Critical Message", "START no replay\n");
        m_com->close();
        delete m_progbar;
        this->show();
        return;
    }

    m_progbar->setValue(1);

    for(int i = 0; i < p.m_cblist.count(); i++) {
        QString data_cmd;
        QString tmp;
        uint16_t crc_sum;
        QByteArray crc_data;
        QByteArray rb;

        QCoreApplication::processEvents();

        if (m_progbar->wasCanceled()) {
            delete m_progbar;
            m_com->close();
            this->show();
            return;
        }

        CmdBlock *cb = p.m_cblist[i];
        crc_data.append(char(i + 1));   // index
        crc_data.append((char *)&cb->addr, 2);    // addr
        crc_data.append((char *)&cb->len, 2);   // len
        crc_data.append((char *)cb->data, cb->len);

        crc_sum = crc16((uint8_t *)crc_data.data(), crc_data.size());
        data_cmd.sprintf("$DATA,%02X,%04X,%04X,",i+1, cb->addr, cb->len);

        for (int j = 0; j < cb->len; j++) {
            QString t;
            tmp += t.sprintf("%02X", cb->data[j]);
        }
        data_cmd += tmp + ",";
        tmp.sprintf("%04X\r\n", crc_sum);
        data_cmd += tmp;

        retry_times = 0;
        while (retry_times <= 3)
        {
            QCoreApplication::processEvents();
            qDebug() << data_cmd;

            if (m_progbar->wasCanceled()) {
                delete m_progbar;
                m_com->close();
                this->show();
                return;
            }

            if ( ! writeToCOM(data_cmd) ) {
                QMessageBox::critical(this, "Critical Message", "Can NOT send $DATA cmd\n");
                m_com->close();
                delete m_progbar;
                this->show();
                return;
            }
            if ( ! readFromCOM(rb)) {
                QMessageBox::critical(this, "Critical Message", "DATA no replay\n");
                m_com->close();
                delete m_progbar;
                this->show();
                return;
            }
            qDebug() << rb;

            if (rb.data()[0] != '$')
            {
                retry_times++;
                continue;
            }
            m_progbar->setValue(i);
            break;
        }
        if (retry_times > 3)
        {
            QMessageBox::critical(this, "Critical Message", "DATA no replay\n");
            m_com->close();
            delete m_progbar;
            this->show();
            return;
        }
    }
    // now update complete
    m_com->close();
    delete m_progbar;
    qDebug() << "Update Complete";
    this->show();
    QMessageBox::information(this, "Information", "Gocia Update Complete!");
}

bool Dialog::writeToCOM(QString s)
{
    int retry = 3;

    if ( ! m_com) {
        return false;
    }

    while (retry > 0) {
        QCoreApplication::processEvents();
        m_com->write(s.toStdString().c_str(), s.size());

        if ( ! m_com->waitForBytesWritten(1000)) {
            qDebug() << "TimeOut: " << s;
            retry--;
            continue;
        }
        return true;
    }
    return false;
}

bool Dialog::readFromCOM(QByteArray &b)
{
    QByteArray t;
    if ( ! m_com) {
        return false;
    }

    while (t.data()[t.size()-1] != '\n')
    {
        QCoreApplication::processEvents();
        if (m_com->waitForReadyRead(5000)) {
            t += m_com->readAll();
        } else {
            return false;
        }
    }
    b = t;
    return true;
}

void Dialog::showResponse(const QString &s)
{
    setControlsEnabled(true);
}

void Dialog::processError(const QString &s)
{
    setControlsEnabled(true);
}

void Dialog::processTimeout(const QString &s)
{
    setControlsEnabled(true);
}

void Dialog::setControlsEnabled(bool enable)
{
    runButton->setEnabled(enable);
    //requestLineEdit->setEnabled(enable);
}


　　　　　　　　　　　　 
　　　/*,.／ ,:ｸ,,. -‐'"´　 ,. -‐''"'
　　 ／i,. ‐'´　　　　　　　￣￣￣￣Z
　 /　　　　　　　　　　　　　　　　　＿二=-
. /　　　　　　　　　　　　　　　　　￣`ヽ､＿,,
.|　　　　　　　　　　　　　　　　　　　　　　／
|＿＿＿　　　　　　　　　　　　　　　　＜~
||`',''''ｰ-｀'＝=r-- ､＿　　　　　　　　＿三=-
||)）　　　　　　.i　　　　｀"''‐-､＿＿　二=‐
ヾ';ｰ=======::'､＿＿＿＿＿＿__/＼'~
　　> ‐r‐;　　　｀`''ｰ､ i--`i　　　 |　　 ヽ,
　 /　 ￣　　　　　　 彡;, | |　　　 ＼　 l~
.　i,＿,,　　　　　　　　彡ﾉ丿　　　　 L`''ヽ
　　　L＿　　　　　　 ミ／ `ヾ' ,'-亠='＝'''''ヽ,
∫　 ／f,_　　　 ,.ril;ﾐ'' ,,.. !-'"´　　　　　　　　ヽ
∫／　 ヾ､､, ''";:-'､,=''　　　　　　　　　　// ＿i
　　　　　|,.!‐';´‐'"´　　　＿＿,,,.. --‐‐'''"￣　　ヽ
　　　　　　r"　　　, -‐''"*/


 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;／/;;;;／/;;／￣　 ,--√
 ;;;;;;;;;;;;;;;;;;;;;;;/,,, /;;/　/;/　　　／∩
 ;;;;;;;;;;;;;;;;;;||||||||lllllｉｉｉｉｉｉ:::::　　⊿＿ノ
 /|;;;;;;;;;;/＿＿/__""'''ヽ－!!!!!!!!!!!!!!!!!!
 ; |;;;;;;;;/__,,,,,,,,__、｀ヽ　　　 　　,-‐/;;;;;;;
 ;|;;;;;;;;|￣~~""ﾞﾞ''ヽ　　　　　／,,,,/;;/|;|
 |;/|;;;|　　　　　　　　　　　/''""ﾞ|;;/ ||
 ||　;|l　　　　　　　　 　　　|　 　|;|　 |
 |:　 |i.　　　　　　　　 　　　| 　 |
 .　　　　　　　　　　 　　　　|　/　力・・・弱いな
 　　　　　　　　　　 　　 　　|/　　　・・・・・・君
 　　　　　　　　　　 ‐-、___ン
 　　　　　　　　　 　　 〈　/　　　　それでも
 　　　　 ｨ,,,,＿＿＿__ン/　　　　　マンコ突いてんですか？
 　　　　　　　　＿＿ﾉ/
 　　　　　　　 　　　/
 　＼,,　　　　 　　/
 　 　'ヽ,　,,,,,,,,,／
 
 
 
 
 

*/











