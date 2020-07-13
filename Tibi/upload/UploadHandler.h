/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef TibiSender_H
#define TibiSender_H

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QSslSocket>
#include <QDataStream>
#include <QSslConfiguration>
#include "upload/Upload.h"
#include "user/UserHandler.h"


class UploadHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief UploadHandler::UploadHandler. This is a worker object that takes care of
     * handling all the operations related to a specific upload. It runs in its own thread.
     * @param code: the unique upload code, assigned by the upload dispatcher
     * @param tibier: the tibier selected by the user in the ViewConnectedTibiers
     * @param itemPath: the path of the file or directory selected by the user
     * @param user: the UserHandler instance, used to send the sender information (id, username)
     * and methods calls
     */
    UploadHandler(int code, QSharedPointer<Tibier> tibier, const QString& filePath, UserHandler* user);
    ~UploadHandler();


public slots:
    void abortConnection();

    /**
     * @brief UploadHandler::startUpload, function called when the thread and its event loop start running.
     * Here goes the allocation of the SslSocket, that can be handled exclusively by its parent (the current thread).
     * If this initialization is done in the constructor, then the current thread cannot control the socket.
     */
    void initSocket();
    void signalAbort();

signals:
    /* -- to quit my thread --*/
    void terminationRequest();
    void updateUpload(QSharedPointer<Upload> upload);

private:
    UserHandler* user = nullptr;

    /**
     * @brief upload, object wrapping all the specific upload information usefull
     * to the other modules (UploadsDispatcher and ViewUD). This object will be copied
     * to share updates between classes
     */
    Upload upload;

    /**
     * @brief isDir, this Type refers to the current item and can also be used as a bool
     * 1 true - Types::Dir
     * 0 false - Types::File
     */
    Types isDir;

    /**
     * @brief itemInfo, used to preparare the first metadata and retrieve
     * all the information relative to the item to send
     */
    QFileInfo itemInfo;
    qint64 currentFilebytesLeft = 0;


    /**
     * @brief itemPath, absolute path of the item selected by the user to send
     */
    QString itemPath;

    /* -- network -- */
    QSslSocket* senderSocket = nullptr;
    QByteArray outBlock;
    QDataStream* out = nullptr;



    /**
     * @brief UploadHandler::setSocketConfig, configures the ssl connection by specyfying to don't
     * verify the peer identity and ignoring any ssl error related to the handshake
     */
    void setSocketConfig();

    /**
     * @brief UploadHandler::connectSocket, register all the events related to the socket.
     */
    void connectSocket();

    /**
     * @brief UploadHandler::initOut, initialises the QDataStream object used to
     * manage the serialization of the QByteArray outBlock
     */
    void initOut();

    /**
     * @brief UploadHandler::prepareFirstMetadata, computes the
     * remaining information to send regarding the item -- name, type, total size
     * (All the info about the sender are stored and can be accesed through
     *  the UserHandler object)
     */
    void prepareFirstMetadata();

    /**
     * @brief UploadHandler::sendFirstMetadata
     * FIRST METADATA:
         * SENDER ID                        16 bytes    | 128-bit   : QUuid
         * SENDER USERNAME SIZE             1 bytes     | 8-bit    : quint8
         * SENDER USERNAME                  variable    | sizeName*sizeof(QChar)
         * TYPE OF REQUEST ( dir or file )  1 byte      | 8-bit     : quint8
         * ITEM NAME SIZE                   2 byte      | 16-bi     : quint16
         * ITEM NAME                        variable    | sizeName*sizeof(QChar)
         * ITEM SIZE                        8 bytes     | 64-bit    : qint64
     */
    void sendFirstMetadata();

    /**
     * @brief UploadHandler::checkItemExistance check if the selected item still exists
     * @return true if it exists, false if it doesn't after having request the
     * upload termination with Status::Error
     */
    bool checkItemExistance();

    /**
     * @brief UploadHandler::fillItemName, fills the upload field related to the the item name
     * using the QFileInfo itemInfo
     */
    void fillItemName();

    /**
     * @brief UploadHandler::fillItemType, fills the upload field related to the item type
     */
    void fillItemType();

    /**
     * @brief UploadHandler::fillItemSize, fills the upload field related to the item size
     * using itemInfo.size if a file, dirTosend if a dir
     * Moreover, if the item is a dir, the dirPaths and filePaths vectors are filled
     */
    void fillItemSize();

    /**
     * @brief UploadHandler::fillDirInfo, sets the path to
     * dirToSend and get the upload total size
     */
    void fillDirInfo();

    /**
     * @brief UploadHandler::getDirSize is a recursive function that does 3 things:
     *          1. computes the total size for the directory that has to be send
     *          2. adds the relative paths of leaf dirs to dirPaths, that will be
     * used by the DownloadHandler to create all the necessary subdir
     *          3. adds the relative paths of all files to filePaths
     * @param path of the directory
     * @return the size of the given directory, in bytes
     */
    qint64 getDirSize(const QString& path);
    qint64 sumFileSizeInDir(QDir& dir);


    /**
     * @brief dirToSend, just for Types::Dir: directory to send. It is used to compute
     * the relative paths of all subdir and files.
     */
    QDir dirToSend;

    /**
     * @brief dirPaths, just for Types::Dir: vector of relative
     * paths of the leaf dirs in the directory to send
     */
    QVector<QString> dirLeavesRelativePaths;

    /**
     * @brief filePaths, just for Types::Dir, vector of relative paths of all
     * the file that have to be send in the selected directory
     */
    QVector<QString> filePaths;

    /**
     * @brief UploadHandler::accepted, wraps all the common operations
     * between sending a dir or a file.
     * 1. starts the times for calculating the elapsed time
     * 2. update the status to accepted
     * 3. send the dir or the file
     * 4. if it returns without any error, updates the status to completed and
     * waits the receiver to disconnect.
     */
    void accepted();

    /**
     * @brief UploadHandler::refused, set the status to declined, waiting for the receiver to disconnect.
     */
    void refused();

    /**
     * @brief sendDir sends a dir by sending the following bytes:
     * SUB DIRS COUNTER (qint16)
     * iterator: leaf path size (quint16) + leaf path
     * FILES COUNTER (qint16)
     * iterator: file path size (quint16) + file path + file size (quint64) + file content
     */
    void sendDir();

    /**
     * @brief UploadHandler::sendSubDirs, sends the relative path of the dir leaves.
     * LEAVES COUNTER   2 bytes | 16-bit | qint16
     * iterator:
     * LEAF PATH SIZE   2 bytes | 16-bit | quint16
     * LEAF PATH        variable
     */
    void sendSubDirs();

    /**
     * @brief UploadHandler::sendFile opens the file and send the fileInfo before uploading the content max 3 memory blocks at the time.
     * @param absolutePath of the file to send
     */
    void sendFile(const QString& absolutelPath);

    /**
     * @brief sendFileInfo:
     * FILE PATH SIZE   quint16 - 2bytes - 16-bit
     * FILE PATH        variable: file path size * sizeof(QChar)
     * FILE SIZE        qint64 - 8 bytes - 64-bit
     * @param absolutePath to compute the relative path
     * @param file to compute the file size
     */
    void sendFileInfo(const QString& absolutePath, const QFile& file);

    /**
     * @brief UploadHandler::openFile, tries to open the file, terminating if not possibile
     * @param file to open
     * @return true if opened, false if not
     */
    bool openFile(QFile& file);

    /* -- abort opreations --*/
    QMutex abort_m;
    bool abort = false;
    bool abortAlreadyCalled = false;
    bool checkAbort();

    void signalStatusAndTerminate(Status status, const QString& message);

    /**
     * @brief writeBlock, write the content of the outBlock and clears it
     */
    void writeBlock();


private slots:

    /**
     * @brief UploadHandler::connected, when the socket is connected the UploadHandler prepares and
     * sends the first metadata, updates the ViewUD and starts waiting for the receiver answer
     */
    void connected();

    /**
     * @brief UploadHandler::getAnswer, slot called when the ReadyRead signal is emitted by the socket.
     * It reads the receiver accepted-or-refused answer
     */
    void getAnswer();


};
#endif // TibiSender_H
