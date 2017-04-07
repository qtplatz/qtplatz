#!/bin/bash

function prompt {
    while true; do
	read -p "Proceed (y/n)? " yn
	case $yn in
            [Yy]* ) break;;
            [Nn]* ) exit;;
            * ) echo "Please answer yes or no.";;
	esac
    done
}

