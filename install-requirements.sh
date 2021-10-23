#!/bin/bash

export DPDK_VER=20.02
export PCI_IF="0000:1a:00.3"

# check user privilege
if [ `whoami` != 'root' ]; then
    echo "Please run this as root..., don't worry about it..."
    exit 1
fi

# get hugepage mount
echo "updating fstab"
r=`grep hugetlbfs /etc/fstab`
if [ $? -eq 1 ]; then
echo "huge        /mnt/huge   hugetlbfs defaults      0   0" >> /etc/fstab
fi

if [ ! -d /mnt/huge ]; then
mkdir /mnt/huge
chmod 777 /mnt/huge/
fi

echo "Updating sysctl"
r=`grep nr_hugepages /etc/sysctl.conf`
if [ $? -eq 1 ]; then
    echo "vm.nr_hugepages=256" >> /etc/sysctl.conf
    # also make sure it is live on this run, in case fstab has been already updated
    sysctl -w vm.nr_hugepages=256
fi

echo "checking for iommu in GRUB"
r=`grep iommu=pt /etc/default/grub`
if [ $? -eq 1 ]; then
echo "iommu is missing from grub"
echo "please edit /etc/default/grub and make to append the below to GRUB_CMDLINE_LINUX"
echo "default_hugepagesz=1G hugepagesz=1G hugepages=8 iommu=pt intel_iommu=on pci=assign-busses"
echo 'example: GRUB_CMDLINE_LINUX="console=tty0 console=ttyS1,115200n8 biosdevname=0 net.ifnames=1 default_hugepagesz=1G hugepagesz=1G hugepages=8 iommu=pt intel_iommu=on pci=assign-busses"'
echo "after that run: update-grub && reboot"
echo "this will reboot your machine!"
echo "other things you may want to add are:"
echo "maxcpus=32"
echo "isolcpus=3-31"
exit 1
fi

r=`grep intel_iommu=on /etc/default/grub`
if [ $? -eq 1 ]; then
echo "iommu is missing from grub"
echo "please edit /etc/default/grub and make to append the below to GRUB_CMDLINE_LINUX"
echo "default_hugepagesz=1G hugepagesz=1G hugepages=8 iommu=pt intel_iommu=on pci=assign-busses"

echo 'example: GRUB_CMDLINE_LINUX="console=tty0 console=ttyS1,115200n8 biosdevname=0 net.ifnames=1 default_hugepagesz=1G hugepagesz=1G hugepages=8 iommu=pt intel_iommu=on pci=assign-busses"'
echo "after that run: update-grub && reboot"
echo "this will reboot your machine!"
exit 1
fi

echo "Going into /opt ..."
mkdir /opt
cd /opt

echo "Installing packages..."
apt-get -y update
apt-get -y install build-essential python3 python3-pip python3-pyelftools libnuma-dev pciutils libpcap-dev liblua5.3-dev libelf-dev git doxygen hugepages libmnl0 libmnl-dev libkmod2 libkmod-dev libelf1 libelf-dev libc6-dev-i386 libncurses5-dev libreadline-dev libdpdk-dev autoconf flex bison graphviz libboost-all-dev plantuml
apt-get -y install linux-headers-`uname -r` || apt -y install linux-headers-generic
pip3 install --user meson ninja

echo "Installing sparsehash" # from github
git clone https://github.com/sparsehash/sparsehash.git sparsehash
cd sparsehash
./configure
make
make install 
make clean
cd ..
rm -r sparsehash

echo "Setting env..."
export RTE_TARGET=x86_64-native-linuxapp-gcc
export RTE_SDK=/opt/dpdk-$DPDK_VER
ln -s /usr/bin/python3 /usr/bin/python

echo "Downloading DPDK..."
if [ ! -f /opt/dpdk-$DPDK_VER.tar.xz ]; then
    wget https://fast.dpdk.org/rel/dpdk-$DPDK_VER.tar.xz
fi

echo "Unpacking DPDK..."
rm -rf dpdk-$DPDK_VER/
tar xvf dpdk-$DPDK_VER.tar.xz
rm -rf dpdk-$DPDK_VER.tar.xz

echo "Installing DPDK..."
cd dpdk-$DPDK_VER
make config T=x86_64-native-linuxapp-gcc CONFIG_RTE_EAL_IGB_UIO=y
make install T=x86_64-native-linuxapp-gcc DESTDIR=install CONFIG_RTE_EAL_IGB_UIO=y

cd ..

echo "binding dpdk interface $PCI_IF"
modprobe uio
insmod /opt/dpdk-$DPDK_VER/x86_64-native-linuxapp-gcc/kmod/igb_uio.ko
modprobe vfio-pci
modprobe uio_pci_generic
dpdkdevbind=/opt/dpdk-$DPDK_VER/usertools/dpdk-devbind.py
$dpdkdevbind --force -u $PCI_IF
$dpdkdevbind -b igb_uio $PCI_IF
$dpdkdevbind -s

echo "finished"
echo "all requirements should now be installed"
echo "you can now configure and install AEGIS"
