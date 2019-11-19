#!/bin/bash

failed_list=()

#qt5
list_dependency+=('qemu'
				  'binfmt-support'
				  'qemu-user-static'
				  )

for arg in "${list_dependency[@]}"; do
    echo sudo apt-get install -y "$arg"
    sudo apt-get install -y "$arg" || failed_list+=("$arg")
done

if [ ${#failed_list[@]} -gt 0 ]; then
   echo "Error: Total " ${#failed_list[@]} packages failed to install.
   for arg in "${failed_list[@]}"; do
	echo "	" "$arg"
   done
fi
