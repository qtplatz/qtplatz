#!/bin/bash

staff=$(id -Gn | grep -c staff)
echo "I am staff group: " $staff

if [ $staff -eq 0 ]; then
    echo "You should be a staff group";
    sudo usermod -a -G staff $(whoami)
    echo "You may require logout and login again for usermod take effect"
fi

sudo apt-get -y install build-essential
sudo apt-get -y install dkms

sudo apt-get -y install libncurses5-dev
sudo apt-get -y insgall git
sudo apt-get -y install mesa-common-dev
sudo apt-get -y install libglu1-mesa-dev freeglut3-dev
sudo apt-get -y install libxml2-dev libxslt1-dev
sudo apt-get -y install libgstreamer-plugins-base0.10
sudo apt-get -y install python2.7 python2.7-dev

echo "==========================="
echo "  You can now install Qt5  "
echo "  Don't forget to do 'ln -s /opt/Qt/5.8/gcc_64/bin/qmake /usr/local/bin'"
echo "  And delete /usr/bin/qmake"
echo "==========================="
