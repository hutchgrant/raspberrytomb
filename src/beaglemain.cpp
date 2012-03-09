/*
 *  Written By: Grant Hutchinson
 *  License: GPLv3.
 *  h.g.utchinson@gmail.com
 *  Copyright (C) 2012 by Grant Hutchinson
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "beaglemain.h"
#include "ui_beaglemain.h"

BeagleMain::BeagleMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BeagleMain)
{
    ui->setupUi(this);
    Sync(0);
    ui->MenuList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->TitleList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->PlayList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->list_radio->setEditTriggers(QAbstractItemView::NoEditTriggers);
    widget.setSeekSlider(ui->SEEK_slider);
    widget.setVolumeSlider(ui->VOL_dial);

}

/*
  *  SYNCHRONIZE ALL DATA
  */
void BeagleMain::Sync(int type){
    artSize = 0;
    albSize = 0;
    songSize = 0;
    vidDirSize = 0;
    vidSize = 0;
   radSize = 0;
    if(type == 0){
        /// init playlist
        pl.setCount(0);
        pl.initPL();

        plMode= 0; // set playlist mode to browse
        playlistOpen = false; // set playlist open
        /// if SQL file doesn't exist
        if(pref.initDB() == false){
            prefDg.show();
            if (prefDg.exec()==QDialog::Accepted) {

                //create cache directory
                pref.createCache();
                pref = prefDg.getPref();
                //create custom sql db
                pref.createDB();
                /// write preferences to sql db
                pref.writeDB();
                pref.setInitDB();
            }
        }

        else{
            //// read from sql and fill songObjs
            readDB rDB(pref.getSQL().c_str());
            Artist = rDB.ArtistFill(&artSize);
            Album = rDB.AlbumFill(&albSize);
            Song = rDB.SongFill(&songSize);
            VidDir = rDB.VidDirFill(&vidDirSize);
            Video = rDB.VideoFill(&vidSize);
            /// read from sql and fill radio obj
            Radio = rDB.RadioFill(&radSize);
            Radio.setDB(pref.getSQL());
            Radio.setSize(radSize);
            RefillRadioPL();
        }
    }
    else if(type == 1){

        readDB rDB(pref.getSQL().c_str());
        ///check database for previous radio stations
        Radio = rDB.RadioFill(&radSize);

        pref.deleteDB(pref.getSQL().c_str());
        pref.createDB();

        cout << "syncing.... " << pref.getServ() << "\t" << pref.getPort()<<  endl;
        /// read from remote mysql write to local sqlite
        syncMe sy(pref.getServ().c_str(), pref.getUser().c_str(), pref.getPass().c_str(), pref.getTable().c_str(), pref.getSQL().c_str());
        /// write radio stations
        Radio.writeDBFill();
        /// write preferences to sql db
        pref.writeDB();
        pref.setInitDB();
        cout << "reading.... " << endl;
        /// read from sqlite to songObjs
        Artist = rDB.ArtistFill(&artSize);
        Album = rDB.AlbumFill(&albSize);
        Song = rDB.SongFill(&songSize);
        VidDir = rDB.VidDirFill(&vidDirSize);
        Video = rDB.VideoFill(&vidSize);
        /// read from sql and fill radio obj
        cout << "syncing radio" << endl;
        Radio = rDB.RadioFill(&radSize);
        Radio.setDB(pref.getSQL());
        Radio.setSize(radSize);
        RefillRadioPL();
        cout << "Database synced!" << endl;
    }
}

/*
  *  UPDATE MENU LIST WITH ALL ARTISTS OR ALL ALBUMS
  */
void BeagleMain::updateMenu(int type){
    QStringList curMenu;
    m_Model = new QStringListModel(this);
    if(type == 1){
        /// update with artist
        for(int i=0; i<= artSize; i++){
            curMenu << Artist[i].getFile();
        }
    }
    else if(type == 2){
        /// update with album
        for(int i=0; i<= albSize; i++){
            curMenu << Album[i].getFile();
        }
    }
    else if(type == 3){
        /// update with directories
        for(int i=0; i<=vidDirSize; i++){
            curMenu << VidDir[i].getFile();
        }
    }


    m_Model->setStringList(curMenu);
    ui->MenuList->setModel(m_Model);
}

void BeagleMain::setMainPref(preferences my_pref){
    pref = my_pref;
}

/*
  *  UPDATE MENU LIST WITH ALBUMS FROM SELECTED
  */
void BeagleMain::updateAlbMenu(int select){
    albCount = 0;
    int selID = 0;
    QStringList curAlb;
    m_Model = new QStringListModel(this);
    curAlbID = new int[MAX];
    selID = Artist[select].getFileID();    /// selected ID

    for(int i=0; i<= albSize; i++){
        if(Album[i].getFilePar() == selID ){
            curAlb << Album[i].getFile();
            curAlbID[albCount] = Album[i].getFileID();
            albCount++;
        }
    }

    m_Model->setStringList(curAlb);
    ui->MenuList->setModel(m_Model);
}

/*
  *  UPDATE TITLE LIST WITH ALL SONGS
  */
void BeagleMain::updateTitle(){
    t_Model = new QStringListModel(this);
    QStringList songList;

    for(int i = 0; i<= songSize; i++){
        songList << Song[i].getFile();
    }
    t_Model->setStringList(songList);
    ui->TitleList->setModel(t_Model);

}

/*
  *  UPDATE TITLE LIST WITH SONGS FROM SELECTED
  */
void BeagleMain::updateTitle(int selected){
    songCount = 0;
    vidCount = 0;
    int selID = 0;
    QStringList curSong;
    t_Model = new QStringListModel(this);
    curSongID = new int[MAX];
    curVidID = new int[MAX];
    if(MenuMode == 3){     ///  if ALL ALBUM
        selID = Album[selected].getFileID();
        for(int i = 0; i<= songSize; i++){
            if(Song[i].getFilePar() == selID){
                curSong << Song[i].getFile();
                curSongID[songCount] = Song[i].getFileID();
                songCount++;
            }
        }
    }
    else if(MenuMode == 4){    ////// if VIDEO DIR MODE
        selID = VidDir[selected].getFileID();
        for(int i = 0; i<= vidSize; i++){
            if(Video[i].getFilePar() == selID){
                curSong << Video[i].getFile();
                curVidID[vidCount] = Video[i].getFileID();
                vidCount++;
            }
        }
    }
    else{   /// IF ALBUM DIR MODE
        selID = curAlbID[selected];
        for(int i = 0; i<= songSize; i++){
            if(Song[i].getFilePar() == selID){
                curSong << Song[i].getFile();
                curSongID[songCount] = Song[i].getFileID();
                songCount++;
            }
        }
    }

    t_Model->setStringList(curSong);
    ui->TitleList->setModel(t_Model);

}
void BeagleMain::startSong(char *FinSong, int selID){

    char * strBuffer;
    strBuffer= new char[100];
    sprintf(strBuffer, "http://%s:%s/content/media/object_id/%d/res_id/0", pref.getServ().c_str(), pref.getPort().c_str(), selID);
    ui->SONG_lbl->setText((QString)FinSong);
    widget.show();
    widget.start(QStringList(strBuffer));
}
/*
  * refill playlist list with playlist model from playlist object
  */
void BeagleMain::RefillMainPL(){
    QStringList updatedList;
   t_Model = new QStringListModel(this);
    updatedList = pl.RefillPlaylist();
    t_Model->setStringList(updatedList);
    ui->PlayList->setModel(t_Model);
}

/*
  * refill playlist list with playlist model from playlist user Folder
  */
void BeagleMain::RefillPLFolder(){
    QStringList updatedList;
   t_Model = new QStringListModel(this);
    updatedList = pl.listDirectories(pl.getFullLocation().c_str());
    t_Model->setStringList(updatedList);
    ui->PlayList->setModel(t_Model);
}

void BeagleMain::PlaylistPlay(int selID){
        char* FinSong;
        for(int i = 0; i<= pl.getCount(); i++){
            if(pl.getTrackID(i) == selID){
                FinSong = new char[strlen(pl.getTrackName(i).c_str())+1];
                strcpy(FinSong,pl.getTrackName(i).c_str());
            }
        }
        // start song
        startSong(FinSong, selID);
    }
/*
  * MEM CLEANUP
  */
BeagleMain::~BeagleMain()
{
    delete ui;
    delete [] Artist;
    delete [] Album;
    delete [] Song;
    delete [] Video;
    delete [] VidDir;
}

/*
  *When Sync Button is pressed
  */
void BeagleMain::on_SYNC_but_clicked()
{
    Sync(1);
}

/*
  *When Mode combo button is changed, change the mode, update lists
  */
void BeagleMain::on_MODE_combo_currentIndexChanged(int index)
{
    if(index == 1){   //  update lists with artists
        updateMenu(1);
        MenuMode=1;
    }
    else if(index == 2){  // update lists with albums
        updateMenu(2);
        MenuMode=3;
    }
    else if(index == 3){  // update lists with songs
        updateTitle();
        TitleMode = 1;
    }
    else if(index == 4){  // update lists with vid directories
        MenuMode = 4;
        updateMenu(3);
    }
}

/*
  *When Stop Button is pressed
  */
void BeagleMain::on_STOP_but_clicked()
{
    widget.stop();
}

/*
  *When a track is double clicked from the TitleList, depending on which mode we are in
  */
void BeagleMain::on_TitleList_doubleClicked(QModelIndex index)
{
    int selID = 0;
    int selected = 0;
    selected = ui->TitleList->currentIndex().row();
    int FinParentID = 0;
    char *FinSong;
    char *FinParent;

    FinSong = new char[100];
    FinParent = new char[100];

    if(MenuMode == 2 || MenuMode == 3){
        selID = curSongID[selected];
        FinSong = checkSongObjByID(selID, Song, songSize);
        FinParentID = checkSongObjParByID(selID,Song, songSize);
        FinParent = checkSongObjByID(FinParentID, Song, songSize);
    }
    else if(MenuMode == 4){
        selID = curVidID[selected];
        FinSong = checkSongObjByID(selID, Video, vidSize);
        FinParentID = checkSongObjParByID(selID,Video, vidSize);
        FinParent = checkSongObjByID(FinParentID, Video, vidSize);
    }
    else{
        selID = Song[selected].getFileID();
        FinSong = checkSongObjByID(selID, Song, songSize);
        FinParentID = checkSongObjParByID(selID,Song, songSize);
        FinParent = checkSongObjByID(FinParentID, Song, songSize);
    }
    // start song
    startSong(FinSong, selID);
}

/*
  *When the Main list is selected, depending on which mode we are in
  */
void BeagleMain::on_MenuList_clicked(QModelIndex index)
{
    int selected = ui->MenuList->currentIndex().row();

    if(MenuMode == 1){
        updateAlbMenu(selected);
        MenuMode =2;
    }
    else if(MenuMode == 4){
        updateTitle(selected);
    }
    else{
        updateTitle(selected);
    }
}
/*
  * When vol dial has changed
  */
void BeagleMain::on_VOL_dial_valueChanged(int value)
{

}

/*
  * when seek slider has moved
  */
void BeagleMain::on_SEEK_slider_sliderMoved(int position)
{
    //   widget.seek((double)position, 0);
}

/*
  * When play button is pressed and held, seek
  */
void BeagleMain::on_FWD_but_pressed()
{

    int curTime = ui->SEEK_slider->value();

    int FutTime = curTime + 10;
    widget.seek((double)FutTime, curTime);
}

/*
  * When Play button clicked, play
  */
void BeagleMain::on_FWD_but_clicked()
{
    widget.play();
}
/*
  * When Pause button clicked, pause
  */
void BeagleMain::on_PAUSE_but_clicked()
{
    widget.pause();
}

/*
  * When preferences menu item selected, open preference window
  */
void BeagleMain::on_actionPreferences_2_activated()
{
    pref.readDB();
    prefDg.setPref(pref);
    prefDg.show();
    if (prefDg.exec()==QDialog::Accepted) {
        pref = prefDg.getPref();
        //delete custom sql db
        pref.deleteDB(pref.getSQL().c_str());
        //create custom sql db
        pref.createDB();
        /// write preferences to sql db
        pref.writeDB();
    }
}

/*
  * When About menu item selected, open about dialog
  */
void BeagleMain::on_actionAbout_activated()
{
    ab.show();
    if(ab.exec()==QDialog::Accepted) {
        ab.close();
    }
}

/*
  * When Sync Button is selected , sync
  */
void BeagleMain::on_actionSync_activated()
{
    Sync(1);
}

/*
  * When Add button is selected, get selected item (depending on mode),
  * add to list, refill list
  */
void BeagleMain::on_ADD_but_clicked()
{
    int selID = 0;
    int selected = 0;
    char * strBuffer;
    strBuffer= new char[100];
    selected = ui->TitleList->currentIndex().row();

    if(MenuMode == 2 || MenuMode == 3){
         selID = curSongID[selected];
      strBuffer=checkSongObjByID(selID, Song, songSize);
    }
    else if(MenuMode == 4){
         selID = curVidID[selected];
       strBuffer=checkSongObjByID(selID, Video, vidSize);
    }
    else{
        selID = Song[selected].getFileID();
        strBuffer=checkSongObjByID(selID, Song, songSize);
    }
    pl.AddTo(selID, strBuffer);
    RefillMainPL();
}


/*
  * when the playlist list is doubleclick get which item we've selected and play it
  */
void BeagleMain::on_PlayList_doubleClicked(QModelIndex index)
{


    int selected;
    selected = ui->PlayList->currentIndex().row();
    int selID = 0;
    char *FinSong;

       if(plMode == 1){
           pl.readPLfile((pref.getPlaylistDir() + pl.getPLFolder(selected)).c_str());   /// edit with selected playlist currently defaulting
            RefillMainPL();
            playlistOpen = true;
            plMode = 2;
       }
       else{
           pl_selected = 0;
           pl_selected = ui->PlayList->currentIndex().row();
           selID = pl.getTrackID(pl_selected);
           PlaylistPlay(selID);
       }

    /// thread adding future
    /*
    if(plMode == 1){
        pl.readPLfile((pref.getPlaylistDir() + pl.getPLFolder(selected)).c_str());   /// edit with selected playlist currently defaulting
         RefillMainPL();
         playlistOpen = true;
         plMode = 2;
    }
    else  if(plMode == 2){

        pl_selected = 0;
        pl_selected = ui->PlayList->currentIndex().row();
        mplay.set(pl, pref, pl_selected);
        mplay.run();
        plMode = 3;
    }
    else if(plMode == 3){

         pl_selected = 0;
         pl_selected = ui->PlayList->currentIndex().row();
         mplay.set(pl, pref, pl_selected);
         mplay.run();
         plMode = 2;
     }*/
}

/*
  * When Remove button is clicked
  */
void BeagleMain::on_REMOVE_but_clicked()
{
    int selected = 0;
    pl.RemoveFrom(pl_selected);
    RefillMainPL();
}



void BeagleMain::on_SAVE_but_clicked()
{
    string fileName, fileLocate;
    newPL.show();
    if(newPL.exec()==QDialog::Accepted){
        if(newPL.plExists()){
            fileName = newPL.getFile();
             fileLocate = newPL.getLocate();
             pl.setFullLocation(fileName, fileLocate);
             pl.writePLfile();
        }
    }
}

void BeagleMain::on_OPEN_but_clicked()
{
    plMode = 0;
    string fileName, fileLocate;
    fileName = "";
    fileLocate = "";

        openPL.show();
        if(openPL.exec()==QDialog::Accepted){
           if(openPL.plExists()){
               fileName = openPL.getFile();
               fileLocate = openPL.getLocate();
                 pl.setFullLocation(fileName, fileLocate);
           }
        }

         /// list .pl files in preferreed directory
        RefillPLFolder();
        plMode = 1;

}

void BeagleMain::on_FFWD_but_clicked()
{
    int selID = 0;

    pl_selected++;
     selID = pl.getTrackID(pl_selected);
    PlaylistPlay(selID);

}


void BeagleMain::on_RRWD_but_clicked()
{
       int selID = 0;
    pl_selected--;
     selID = pl.getTrackID(pl_selected);
    PlaylistPlay(selID);
}

void BeagleMain::on_UP_but_clicked()
{
    pl.Move(2, pl_selected);
    pl_selected--;
    RefillMainPL();
}

void BeagleMain::on_DOWN_but_clicked()
{
    pl.Move(1, pl_selected);
    pl_selected++;
    RefillMainPL();
}

void BeagleMain::on_PlayList_clicked(QModelIndex index)
{
    pl_selected = ui->PlayList->currentIndex().row();
}

 void BeagleMain::closeEvent(QCloseEvent *event){

     widget.close();
 }

void BeagleMain::on_actionDonate_2_triggered()
{
    QDesktopServices::openUrl(QUrl("https://flattr.com/profile/hutchgrant", QUrl::TolerantMode));
}

void BeagleMain::on_ADMIN_but_clicked()
{
    char mediatombAdd[100];
    sprintf(mediatombAdd, "http://%s:%s", pref.getServ().c_str(), pref.getPort().c_str());
    QDesktopServices::openUrl(QUrl(mediatombAdd, QUrl::TolerantMode));
}

void BeagleMain::on_but_RadAdd_clicked()
{
    QString Radio_url;
    QString Radio_name;

    Radio_url = ui->entry_radURL->text();
    Radio_name = ui->entry_radName->text();

    Radio.Add(Radio_name.toStdString(),Radio_url.toStdString());
    radSize++;

    Radio.writeDB();
    RefillRadioPL();
}

void BeagleMain::on_but_RemRad_clicked()
{
    int pos = 0;
    string pl_ItemName;
    pos = ui->list_radio->currentIndex().row();
    pl_ItemName = Radio.getName(pos);
    Radio.Remove(pl_ItemName, pos);
    readDB rDB(pref.getSQL().c_str());
    Radio = rDB.RadioFill(&radSize);
    RefillRadioPL();
}

void BeagleMain::on_list_radio_clicked(QModelIndex index)
{

}

/*
  * refill playlist list with playlist model from playlist object
  */
void BeagleMain::RefillRadioPL(){
    QStringList updatedList;
   r_Model = new QStringListModel(this);
    updatedList = Radio.RefillPlaylist();
    r_Model->setStringList(updatedList);
    ui->list_radio->setModel(r_Model);

}

void BeagleMain::on_list_radio_doubleClicked(QModelIndex index)
{
    int pos =0;
    pos = ui->list_radio->currentIndex().row();
    string finalUrl, finalName;
    finalUrl = Radio.getUrl(pos);
    finalName = Radio.getName(pos);
    ui->SONG_lbl->setText((QString)finalName.c_str());
    widget.show();
    widget.start(QStringList(finalUrl.c_str()));
}
