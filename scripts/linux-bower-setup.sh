#!/bin/bash

sudo apt-get install software-properties-common

curl -sL https://deb.nodesource.com/setup_10.x | sudo -E bash -
sudo apt-get install --yes nodejs

node --version
npm --version

sudo npm install -g bower
