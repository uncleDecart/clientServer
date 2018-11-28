#include "tst_filesender.h"
#include <QDir>

TestFileSender::TestFileSender(QObject *parent) : QObject(parent)
{

}

QString TestFileSender::getReceivedMd5Sum(std::__cxx11::string id)
{
    QProcess proc;
    std::string request = "cd ./"+ id +"/ && cat a* > res.dat && md5sum res.dat";
    proc.start("bash", QStringList() << "-c" << request.c_str());
    proc.waitForFinished();
    return QString(proc.readAllStandardOutput()).split(" ")[0];
}

QString TestFileSender::getSendedMd5Sum(std::__cxx11::string filepath)
{
    QProcess proc;
    std::string request = "md5sum " + filepath;
    proc.start("bash", QStringList() << "-c" << request.c_str());
    proc.waitForFinished();
    return QString(proc.readAllStandardOutput()).split(" ")[0];
}

void TestFileSender::session()
{
    const std::string validId = "123";
    const std::string file = "input.dat";
    FileSender fs(file, validId);
    fs.start_session();
    QString receivedMd5 = getReceivedMd5Sum(validId);
    QString sendedMd5 = getSendedMd5Sum(file);

    QVERIFY2(receivedMd5 == sendedMd5 && !receivedMd5.isEmpty(),
             "Received data does not match");

    const std::string invalidId = "000";
    FileSender fs1(file, invalidId);
    fs1.start_session();
    std::string path = "./"+invalidId+"/";
    QVERIFY2(QDir(path.c_str()).exists() == false,
             "Server has created dir from invalid id");

    std::string cleanupCommand = "rm -rf " + validId;
    QProcess cleanup;
    cleanup.execute("bash", QStringList() << "-c" << cleanupCommand.c_str());

    //FileSender fs2(file, validId);
    //fs2.set_hostname("invalid.hostname");
    //fs2.start_session();

}
