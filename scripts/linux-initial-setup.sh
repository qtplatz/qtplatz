#!/bin/bash

staff=$(id -Gn | grep -c staff)
echo "I am staff group: " $staff

if [ $staff -eq 0 ]; then
    echo "You should be a staff group";
    sudo usermod -a -G staff $(whoami)
    echo "You may require logout and login again for usermod take effect"
fi

sudo apt-get -y insgall git
sudo apt-get -y install build-essential
sudo apt-get -y install dkms
sudo apt-get -y install bc
sudo apt-get -y install libncurses5-dev

