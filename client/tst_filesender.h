#ifndef TESTFILESENDER_H
#define TESTFILESENDER_H

#include "filesender.h"

#include <QObject>
#include <QProcess>
#include <QtTest/QtTest>

class TestFileSender : public QObject
{
    Q_OBJECT
public:
    explicit TestFileSender(QObject *parent = 0);
private:
    QString getReceivedMd5Sum(std::string id);
    QString getSendedMd5Sum(std::string filepath);
private slots:
    void session();
};

#endif // TESTFILESENDER_H
