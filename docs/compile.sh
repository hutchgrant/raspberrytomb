#!/bin/bash
mkdir /opt/raspberrytomb/
chmod 777 /opt/raspberrytomb

cd /opt/raspberrytomb
qmake
make

ln -fs /opt/raspberrytomb/raspberrytomb /usr/bin/beagletomb
