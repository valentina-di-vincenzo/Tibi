/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef DOWNLOADHANDLER_H
#define DOWNLOADHANDLER_H

#include <QThread>
#include <QSslSocket>
#include <QDataStream>
#include <QMutex>
#include <QStringRef>
#include <QSslConfiguration>
#include <QMessageBox>
#include <QLockFile>

#include "Download.h"

/**
 * @brief The DownloadHandler class handle a specific download.
 */

class DownloadHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief DownloadHandler::DownloadHandler, pass the info necessary to handle the download
     * @param socketDescriptor
     * @param code of the download
     * @param download->downloadsPath, chosen by the user in preferences. It is passed by value to prevent inconsistency if the user changes while downloading.
     */
    DownloadHandler(qintptr socketDescriptor, int code, QString downloadPath);

    /**
     * @brief signalAbort, the DownloadDispatcher cannot directly abort the socket, since it's not the parent. It immediatly signals the abort exploiting a flag protected by a mutex. This flag is read by the handler before performing long operations inside an event in the loop.
     */
    void signalAbort();

public slots:

    /**
     * @brief initSocket, slot called when the thread starts. Now it is possible to create a socket and initializes it.
     */
    void initSocket();

    /**
     * @brief userAnswer, slot called when the user accepts or declines a download confirmation.
     * @param code, the download code used to identify the specific download
     * @param status, accepted or declined
     */
    void userAnswer(int code, Status status);

    void appClose();
    void abortConnection();

signals:
    void terminationRequest();
    void waitForConfirmation(QSharedPointer<Download> download);
    void downloadFinished(QSharedPointer<Download> download);
    void updateDownload(QSharedPointer<Download> download);
    bool askOverwrite(int code, const QString& type, const QString& oldName, const QString& newName);
    void informCollision(int code, const QString& oldName, const QString& newName);

private:
    /* -- socket attributes --*/
    QSslSocket* receiverSocket;
    QDataStream in;
    QMutex abort_m;
    bool abort = false;
    bool abortAlreadyCalled = false;
    qintptr socketDescriptor;

    /* -- download info -- */
    QSharedPointer<Download> download;
    bool metadataCompleted = false;
    QString itemPath;
    quint16 firstMetadataSize = 0;
    qint16 subDirCounter = -1;
    qint16 filesCounter = -1;
    QDir dirToDownload;
    QLockFile* fileLock = nullptr;


    /* -- current download attributes --*/

    bool infoFileCompleted = false;
    QFile* currentDownload = nullptr;
    quint16 pathSize = 0;
    QString relativePath;
    qint64 fileSize = 0;
    qint64 bytesLeft = 0;

    /**
     * @brief configSocket, configurates the SslSocket to avoid peer identification and errors related to the handshake.
     */
    void configSocket();

    /**
     * @brief connectSocket, register the socket signal and slot events.
     */
    void connectSocket();

    /**
     * @brief getFirstMetadata, get the metadata related to the download and emits the waitForConfrimation signal
     * @return true if the metadata is completed and the waitForConfirmation signal is emitted, false otherwise
     */
    bool getFirstMetadata();

    /**
     * @brief getmetadata, fill the download item with the info stored in the first metadata
     */
    void getmetadata();

    /**
     * @brief getType, get the type or emit an error if the type is unknown
     */
    void getType();

    /**
     * @brief declined, sends the answer to the sender and asks the termination of the thread with the declined status
     */
    void declined();

    /**
     * @brief accepted, if the requests is accepted, it starts to elaborate the content of the first metadata. It check for collisions with existing file and if it is a directory, then the directory is created. If it is a file, it is is opened. At the end, the answer is send to the sender.
     */
    void accepted();

    /**
     * @brief sendAnswerToSender, sends true if the request is declined, false otherwise.
     */
    void sendAnswerToSender();

    /**
     * @brief DownloadHandler::openFile, opens the file at itemPath + "/" + relativePath, it emits an error in case the file cannot be opened
     * @return false if there was an error, true if the file is correctly opened.
     */
    bool openFile();

    /**
     * @brief computeDirDownloadPath, analyses the possibile collisions with already existing directory or with ongoing downloads name. If there is a collision the user can overwrite the file o save it with nameDir(i), where i is the first number that doesn't collide with other files. If other threads are currently downloading a directory with the same name, it is saved as 'dirName copy'. At the end, the itemPath points to the dir download location.
     */
    void computeDirDownloadPath();
    void computeFileDownloadPath();
    bool concurrentCollision(const QString& fileExtension = nullptr);
    QString getClearFileName(const QString& relativePath);
    QString getFileExtension(const QString& relativePath);

    /**
     * @brief newItemName, check if the newName found has to be used to ask the user if overwrite or not the file with the same name or save it with this new name
     * @param newName the new name found
     */
    void newItemName(const QString& newName);

    /**
     * @brief downloadDir, execute the following operations, based on the number of available bytes.
     * 1. get number of subdir
     * 2. create all dirs by downloading the leaf path size and the leaf path
     * 3. get the number of files
     * 4. download each file by reading the file path size, the file path, the file size
     */
    void downloadDir();
    void createSubDir();

    /**
     * @brief getInfoFile, for each file in a dir:
     * FILE PATH SIZE   quint16 - 2bytes - 16-bit
     * stored in: pathSize
     * FILE PATH        variable: file path size * sizeof(QChar)
     * stored in: relativePath
     * FILE SIZE        qint64 - 8 bytes - 64-bit
     * stored in: fileSize
     *
     * It opens the file and put it in currentFile
     * @return false if there where not enough bytes or the file failed to open
     */
    bool getFileInfo();

    /**
     * @brief getCounter, check that the counter is < 0 (i.e. not initialized) and then reads 16 bits from the socket
     * @param counter, the counter to write
     */
    void getCounter(qint16& counter);

    /**
     * @brief getPathSize, checks that pathSize is 0 (i.e. not initialized) and stores in pathSize the size of the path of 2 bytes
     */
    void getPathSize();

    /**
     * @brief getRelativePath, checks that the path is empty and stores in relativePath the path of pathSize size.
     * @return false if the available bytes where not enough
     */
    bool getRelativePath();


    /**
     * @brief DownloadHandler::downloadFile, downloads the file opened in currentDownload, that points to itemPath + relativePath. If the download is effective, the temporary variable related to the download are cleared
     */
    void downloadFile();

    /**
     * @brief checkEnd, check if the download is finished.
     */
    void checkEnd();


    bool checkAbort();    
    void removeDownloads();
    void signalStatusAndTerminate(Status status, const QString& message);


private slots:
    /**
     * @brief onReadyRead, slot called when the socket has some bytes available.
     */
    void onReadyRead();

};

#endif // DOWNLOADHANDLER_H
