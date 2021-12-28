#!/bin/bash
cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/config.sh
source ./prompt.sh

sudo dpkg-reconfigure locales
sudo apt install ibus-mozc
sudo apt install emacs-mozc
