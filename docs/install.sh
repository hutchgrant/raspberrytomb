
#!/bin/bash

pkg1="mediatomb"
pkg2="mysql"
pkg2Install="mysql-server"
pkg3="mplayer"
dbusr="mediatomb"
dbpass="mediatomb"

UI_TO='ui enabled="yes"'
UI_FROM='ui enabled="no"'
SQL_TO='<mysql enabled="yes"'
SQL_FROM='<mysql enabled="no"'
SQLite_TO='sqlite3 enabled="no"'
SQLite_FROM='sqlite3 enabled="yes"'

file='./tmp/config.xml'
TFILE='./tmp/config2.xml'
file2='./tmp/mysqlConf'
TFILE2='./tmp/my.cnf'
SQLBind_TO=$HOSTIP
SQLBind_FROM='127.0.0.1'
exitMain=0

file="/etc/mediatomb/config3.xml"
TFILE="config4.xml"

crtusr2="create user '$dbuser'@'localhost' identified by '$dbuser';"
crtusr="create user '$dbuser'@'%' identified by '$dbuser';"
crtpermiss="grant all on mediatomb.* to '$dbuser'@'%' identified by 'mediatomb';"
crtpermiss2="grant all on mediatomb.* to '$dbuser'@'localhost' identified by 'mediatomb';"
lvmysql="exit"


crtdb="create database mediatomb"


if [ $USER != 'root' ];
then
echo "Beagle Tomb REQUIRES root in order to install"
exit 0
fi

choice=0

while [ $choice != 4 ]
do
clear
echo "Beagle Tomb Setup Menu"
echo "======================"
echo "1) Instructions"
echo "2) Install"
echo "3) Configure"
echo "4) quit"
read choice

case $choice in
2)
echo  "What is your Server's local IP?"
read HOSTIP
echo  "What is your preferred mediatomb Username?"
read dbuser
echo  "What is your preferred mediatomb Password?"
read dbpass
echo "***** Mysql Table and User install*****"
echo "***************************************"
echo "*"
echo "*"
echo " the field below you must enter your root password to your mysql database "
echo " if you just installed mysql it's the password you set during installation"
echo " note:  this password is not stored "
echo "*"
echo "*"
mysql -u root -p << eof 
$crtusr2
$crtusr
$crtpermiss
$crtpermiss2
$lvmysql
eof
echo " mediatomb user+password+permissions created."
echo " creating table..... "
mysql -u mediatomb --password=$dbpass << eof
$crtdb
eof
echo " Filling database....."
mysql mediatomb -u mediatomb --password=$dbpass < /usr/share/mediatomb/mysql.sql

echo "  Install Complete "
read fin
;;
3)
# sh configure.sh
mkdir -p ./tmp/BT/
cp ~/.mediatomb/config.xml ./tmp/config2.xml

sudo sed "s/$UI_FROM/$UI_TO/g" "$TFILE" > $file
sudo sed "s/$SQL_FROM/$SQL_TO/g" "$file" > $TFILE
sudo sed "s/$SQLite_FROM/$SQLite_TO/g" "$TFILE" > $file


sed '21q' $file > ./tmp/apd/atestout.txt 
sed -e :a -e '$q;N;127,$D;ba' $file > ./tmp/BT/dtestoutput.txt
echo "<host>"$HOSTIP"</host>" > ./tmp/BT/btestout.txt
echo "<username>$dbuser</username>" > ./tmp/BT/ctestout.txt
echo "<password>$dbpass</password>" >> ./tmp/BT/ctestout.txt
cat ./tmp/BT/* > $TFILE

cp $TFILE /etc/mediatomb/config.xml
cp $TFILE ~/.mediatomb/config.xml

sudo chown mediatomb:mediatomb /etc/mediatomb/config.xml
sudo chown mediatomb:mediatomb ~/.mediatomb/config.xml

cp /etc/mysql/my.cnf ./tmp/mysqlConf

sed "s/$SQLBind_FROM/$SQLBind_TO/g" "$file2" > $TFILE2

cp -f $TFILE2 /etc/mysql/my.cnf

rm -R ./tmp/BT
rm ./tmp/mysqlConf
rm ./tmp/config2.xml
rm ./tmp/config.xml
;;
1)

step=1
step1='16q'
step2='35q'
step3='70q'
step4='85q'
file='instructions'
Instchoice=0
while [ $Instchoice != "x" ]
do
	case $step in
	1)
	sed $step1 $file 
	read Instchoice 	
	step="2"
	;;
	2)
	sed $step2 $file 
	read Instchoice 
	step="3" 
	;;
	3)
	sed $step3 $file 
	read Instchoice 
	step="4"
	;;
	4)
	sed $step4 $file 
	read Instchoice 
	;;
	esac	
done
;;
esac
done
exit
