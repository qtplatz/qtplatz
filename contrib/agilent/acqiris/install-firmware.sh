#!/bin/bash

if [ -z $AcqirisCD ]; then
    echo "No AcqirisCD environment variable found."
    exit 1
fi

sudo mkdir /usr/lib/firmware/aqrs
sudo cp -p $AcqirisCD/Firmware/*.bit /usr/lib/firmware/aqrs
sudo cp AqDrv4.ini /etc
sudo sh -c "echo 'export AcqirisDxDir=/etc' >> /etc/profile.d/ap240.sh"
