#!/bin/bash 

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

mkdir tmp
cd tmp

wget http://kernel.ubuntu.com/~kernel-ppa/mainline/v4.14/linux-headers-4.14.0-041400_4.14.0-041400.201711122031_all.deb
wget http://kernel.ubuntu.com/~kernel-ppa/mainline/v4.14/linux-headers-4.14.0-041400-generic_4.14.0-041400.201711122031_i386.deb
wget http://kernel.ubuntu.com/~kernel-ppa/mainline/v4.14/linux-image-4.14.0-041400-generic_4.14.0-041400.201711122031_i386.deb

dpkg -i *.deb

sed -i '/GRUB_HIDDEN_TIMEOUT=0/d' /etc/default/grub

update-grub

cd ..
rm -rf tmp

echo "done. Please reboot"
