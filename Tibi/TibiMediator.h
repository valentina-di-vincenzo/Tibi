/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef TIBIMEDIATOR_H
#define TIBIMEDIATOR_H

#include "download/DownloadsDispatcher.h"
#include "upload/UploadsDispatcher.h"
#include "view/ViewTray.h"
#include "view/ViewUD.h"
#include "view/ViewConnectedTibiers.h"
#include "view/ViewPreferences.h"
#include "network/TibiPing.h"
#include "network/TibiDiscovery.h"
#include "upload/TibiSelector.h"
#include "network/TibiReceiver.h"

/**
 * @brief The TibiMediator class encapsulates how the different Tibi components interact.
 * Exploiting the loose coupled signal-and-slot connectivity, the mediator is in charge
 * of registering the connection between different objects, keeping them from referring
 * to each other explicitly. The only shared class is UserHandler, dispatched to the
 * other modules after its initialization.
 *  It also takes care of the initialization of the network modules (TibiPing, TibiDiscovery, TibiSelector, TibiReceiver).
 */
class TibiMediator : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief TibiMediator, initizalises all the application interactions.
     */
    TibiMediator();
    ~TibiMediator();

public slots:
    /**
     * @brief dispatchUserInfo, when the user info are set, TibiMediator takes care of dispatching them and move the network objects to the dedicated threads.
     */
    void dispatchUserInfo();

signals:
    /**
     * @brief cleanUp, signal emitted when the distructor is called. All the network modules are attached to this signal, in order to close the socket and exit the thread.
     */
    void cleanUp();

private:
    /* -- components --*/
    UserHandler* userHandler = nullptr;
    UploadsDispatcher uploadsDispatcher;
    DownloadsDispatcher downloadsDispatcher;

    /* -- views -- */
    ViewConnectedTibiers viewConnected;
    ViewUD viewUD;
    ViewTray viewTray;
    ViewPreferences viewPreferences;

    /* -- network --*/
    TibiReceiver* receiver = nullptr;
    TibiSelector* selector = nullptr;
    TibiPing ping;
    TibiDiscovery discovery;

    /**
     * @brief registerMetatype, to allow availability of types to queued signal and slot
     */
    void registerMetatype();



    /**
     * @brief mediateDownload coordinates the download process.
     * When TibiReceiver receives a connection request it forwards it to
     * the DownloadsDisaptcher (that internally takes care of creating
     * a new thread to handle the download and asking the user for confrimation, if needed).
     * It registers the interactions between the DownloadsDispatcher and all the view components
     * to update the download status, and to receive abort requests from ViewUD.
     */
    void mediateDownload();



    /**
     * @brief mediateUpload coordinates the upload process.
     * When the user selects some file to send, TibiSelector forwards them to the
     * UploadsDispatcher that opens the ConnectedView. The ConnectedView
     * is updated by the UserHandler each time a tibier is added or removed.
     * When the user completes the selection, the UploadsDispatcher updates
     * the uploads status in all the intrested view and informs TibiSelector
     * that the selection is terminated.
     * ViewUD forwards abort requests to the UploadsDispatcher.
     */
    void mediateUpload();


    /**
     * @brief mediatePreferences, the preferencesView can be opened in the tray and the asynchronous modifications are propagated to TibiPing and ViewTray. All the others are implicitly updated through the shared UserHandler istance.
     */
    void mediatePreferences();


    /**
     * @brief mediateTrayActions coordinates actions trigerred by the tray.
     * The tray can close the application and interact with the ViewUD to raise it.
     */
    void mediateTrayActions();



    /**
     * @brief mediateCleaning, manages the application closure by terminating all threads related to the network and closing the sockets.
     */
    void mediateCleaning();



    /**
     * @brief start, Starts the app:
     * when TibiReceiver starts listening for incoming connection,
     * the UserHandler updates the receiver address. Now all the information
     * regarding the user are completed, TibiMediator can dispatch
     * the completed user handler to all the components needing it.
     */
    void start();

    /**
     * @brief All the following methods moves the Netwrok worker objects to a dedicated thread, connecting the start of the thread with the start method of the network module and the receiverClosed signal emitted by it with the exit of the thread.
     */
    void startReceiverThread();
    void startPingThread();
    void startDiscoveryThread();
    void startSelectorThread();

};

#endif // TIBIMEDIATOR_H
