Raspberry tomb fedora client instructions

su

yum install git qt-devel qt-mysql qt-sqlite libstdc++-devel 

git clone git://github.com/hutchgrant/raspberrytomb.git

su -c 'yum localinstall --nogpgcheck http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-stable.noarch.rpm http://download1.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-stable.noarch.rpm'

yum install mplayer

cd raspberrytomb
qmake-qt4
make
make install


mediatomb server instructions

yum install mediatomb mysql-server

*write down mysql password

run mediatomb once:
$ mediatomb
$ pkill mediatomb

Install and configure with this script:
$ su -c 'cd ~/raspberrytomb/docs/install.sh'



