Raspberry tomb fedora client instructions
-------------
Dependencies:
-------------
su -c 'yum install git qt-devel qt-mysql qt-sqlite libstdc++-devel'

$ git clone git://github.com/hutchgrant/raspberrytomb.git

su -c 'yum localinstall --nogpgcheck http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-stable.noarch.rpm http://download1.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-stable.noarch.rpm'

su -c 'yum install mplayer'
---------
COMPILE:
---------
cd raspberrytomb
qmake-qt4
make

---------
INSTALL:
---------
su -c 'mkdir -R /opt/raspberrytomb/release'
make install


******************************
mediatomb server instructions

-------------
DEPENDENCIES:
-------------
su -c 'yum install mediatomb mysql-server'

*write down mysql password

run mediatomb once:
$ mediatomb
$ pkill mediatomb

------------------
INSTALL/CONFIGURE:
------------------
Install and configure with this script:
$ su -c '~/raspberrytomb/docs/install.sh'

Make sure you're using a static IP on the host machine.


Raspberrytomb default pref:
--------------------------
user: mediatomb 
pass: mediatomb 
Table: mediatomb
Server: 192.168.1.116
SQLDir: ~/.RaspberryTomb/BTmedia.db
playlistdir: ~/.RaspberryTomb/playlist
