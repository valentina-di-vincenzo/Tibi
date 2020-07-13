# Tibi: sharing on LAN

Tibi is a cross-platform desktop application designed to allow files and directories sharing on Local Area Networks. The users' presence is advertised through a network protocol that expoilts UDP multicast messages designed specifically for Tibi. 

1. [Requirements](https://github.com/valentina-di-vincenzo/Tibi/blob/master/README.md#requirements)
2. [Getting started](https://github.com/valentina-di-vincenzo/Tibi/blob/master/README.md#gettin-started)
3. [Installation](https://github.com/valentina-di-vincenzo/Tibi/blob/master/README.md#installation)
4. [What does "Tibi" mean?](https://github.com/valentina-di-vincenzo/Tibi/blob/master/README.md#what-does-tibi-mean)
5. [License](https://github.com/valentina-di-vincenzo/Tibi/blob/master/README.md#license)

## Requirements

- **Git**
- **C++11** 

- **Qt**
  - Mac :arrow_right: [Install Qt](https://www.qt.io/download-qt-installer?hsCtaTracking=99d9dd4f-5681-48d2-b096-470725510d34|074ddad0-fdef-4e53-8aa8-5e8a876d6ab4)
  - Linux :arrow_right: in the terminal `sudo apt-get install qt5-default`

## Getting started

### Status

At launch, Tibi appears in the menu bar signaling the online/offline status. When the user is online its presence is shared with all the other Tibiers (Tibi's users) connected to same LAN while in offline mode it is discolesed, and the user cannot receive any file. Still, when the user is in disguised mode, it can share files with other online Tibiers (that will just see its username but not its avatar). 

<p align="center"><img src="https://github.com/valentina-di-vincenzo/Tibi/blob/master/Demo/context-menu.png" height="200" /></p>


### Preferences

In the preferences window you can personalize the displayed username, avatar and default download path. You can also choose if Tibi should automatically accept each download requests or if it should ask for confirmation before starting the download. With the "deafault" button you can restore the initial settings. All the saved changes are permanently stored and will be consistent at the next application launches.


<p align="center"><img src="https://github.com/valentina-di-vincenzo/Tibi/blob/master/Demo/preferences.png" height="400" /></p>

### Sending files and directories

After having added Tibi to the context menu, it is possibile to start sharing any files or directories in a few clicks. Just select the file(s) and/or directory(ies) you want to send, right click and choose "Share with Tibi" (on Mac is under Quick Actions). 

<p align="center"><img src="https://github.com/valentina-di-vincenzo/Tibi/blob/master/Demo/action.png" height="350" /></p>

A window showing all the online Tibiers appears and it updates the information about each Tibier in real-time (so don't worry if your collegue/friend goes online when you have already opened this window). You can choose as many Tibiers as you want and send as many files/directories you like, no matter the size! Also, if you realize you are sending a wrong file/directory during the upload you can easily abort it.


<p align="center"><img src="https://github.com/valentina-di-vincenzo/Tibi/blob/master/Demo/connected.png" height="350" /></p>


You can watch the state of your uploads in the "Transfers" window.


<p align="center"><img src="https://github.com/valentina-di-vincenzo/Tibi/blob/master/Demo/upload.png" height="450" /></p>

### Receiving files and directiories

When another Tibier wants to send you something, Tibi will ask for confirmation or it will automatically start the download (depending on your preferences). 
<p align="center"><img src="https://github.com/valentina-di-vincenzo/Tibi/blob/master/Demo/download-request.png" height="200" /></p>

Files and directories are saved in the default path (you can change it in Preferences) and a pop-up will inform you when the download is finished. 

<p align="center"><img src="https://github.com/valentina-di-vincenzo/Tibi/blob/master/Demo/notification .png" height="100" /></p>

All the downloads' status are visibile in the Transfers window, where you can also easily open the finished ones or abort an ongoing one.
When two downloads have the same name Tibi asks you if you want to overwrite the old one or to keep both of them (in this case at the new download is added a number). 

### Security

Given the absence of a central authority, Tibiers authentication is not possibile. If you are sending sensible data (and I wouldn't recommend it) make sure to text your friend/collegue before doing it. 

## Installation

Select where you want to save the repository and then download it or clone it:

```shell
git clone --recursive https://github.com/valentina-di-vincenzo/Tibi.git
```

If you have QtCreator then just build the project. If you don't want to use the IDE,  you can proceed from the command line ( this is for Mac and Linux ): 

```shell
mkdir buildTibi
cd buildTibi
qmake ../Tibi__SharingOnLAN.pro
make
```

You will find the app in the "buildTibi/Tibi" folder.

### Add Tibi to the context menu

**On Mac**

  1. Open Automator from the Launchpad

  2. Choose "Quick Action"

  3. Add "Run Shell Script" with the following settings:
     Shell: **/bin/zsh**
     Pass input: **as argument**
     If you built it without QtCreator:

     ```shell
     ./PATH TO SOURCE/buildTibi/TibiSelector/TibiSelector.app/Contents/MacOS/TibiSelector $@
     ```

     Where PATH TO SOURCE is the path to the Tibi__SharingOnLAN folder you downloaded. 
     If you built it with QtCreator:

     ```shell
     ./PATH TO BUILD DIR/TibiSelector/TibiSelector.app/Contents/MacOS/TibiSelector $@
     ```

  4. The action has the following settings:
     Workflow receives current: **files or folder**
     In: **any application**
     Image: **Send**

  5. Save it and name it "Share with Tibi"

     

**On Linux**

  1. Install FileManage-Actions and Nautilus

     ```shell
     sudo apt-get install nautilus-actions
     nautilus -q
     sudo apt-get install filemanager-actions
     ```

  2. Open file manager actions with `fma-config-tool`

  3. File -> New action

  4. Check all the four checkbox in the "Action" menu
     Context label: **Share with Tibi**
     Tooltip: **Share on LAN**
     Icon: **/PATH TO SOURCE/resources/images/tibi-logo-tray.png**

  5. In the "Command" menu set:
     Path: **/PATH TO BUILD/TibiSelector/TibiSelector**
     Parametes: **%F**
     Working directory: **%d**

## What does "Tibi" mean?

"Tibi" means "to you" in latin. The inspiration behind this name is a riddle used by Cicerone to greet people: "Mitto tibi navem prora puppique carentem" that is translated as "I send you a ship without stern and bow". Have you solved the riddle? 
"Navem" without the first and last letter is just "ave" that stands for "hello" in latin!

## License

This project is licensed under the [GNU General Public License version 3](https://www.gnu.org/licenses/gpl-3.0.en.html).


