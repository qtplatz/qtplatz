#!/bin/bash
source ./constants.sh
source ./prompt.sh

sudo dpkg-reconfigure locales
sudo apt install ibus-bozc
