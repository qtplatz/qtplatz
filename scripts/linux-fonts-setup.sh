#!/bin/bash
source ./constants.sh
source ./prompt.sh

sudo mkdir -p /usr/share/fonts/ttc-osaka
cd /usr/share/fonts/ttc-osaka
sudo wget http://osaka.is.land.to/files/Osaka.zip
sudo unzip Osaka.zip
cd ..
sudo fc-cache -fv
sudo rm ttc-osaka/Osaka.zip
#
sudo mkdir -p /usr/share/fonts/truetype/ttf-monaco
cd /usr/share/fonts/truetype/ttf-monaco
sudo wget http://www.gringod.com/wp-upload/software/Fonts/Monaco_Linux.ttf
mkfontdir
cd ..
sudo fc-cache -fv

cat >>~/.emacs <<EOF
(when window-system
  (create-fontset-from-ascii-font "Monaco:pixelsize=14" nil "myfontset")
  (set-fontset-font "fontset-myfontset"
                    'japanese-jisx0208 ;'unicode
                    (font-spec :family "Osaka" :size 16)
                    nil
                    'append)
  (add-to-list 'default-frame-alist '(font . "fontset-myfontset")))
;;;;;;;;;;;;;;;;;
(require 'mozc)
(setq default-input-method "japanese-mozc")
EOF
