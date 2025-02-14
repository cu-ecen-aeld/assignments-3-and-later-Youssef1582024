#!/bin/bash
# Script to build a kernel and rootfs for ARM64 architecture.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
    echo "Using default directory ${OUTDIR} for output"
else
    OUTDIR=$1
    echo "Using passed directory ${OUTDIR} for output"
fi

# Create outdir if it doesn't exist
if [ ! -d "${OUTDIR}" ]; then
    echo "Creating output directory ${OUTDIR}"
    mkdir -p ${OUTDIR} || { echo "Failed to create ${OUTDIR}. Exiting."; exit 1; }
fi

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    # Clone kernel source if it doesn't exist
    echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
    git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi

cd linux-stable
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    # Checkout the required kernel version
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # Configure the kernel for ARM64
    echo "Configuring the kernel"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig

    # Build the kernel Image
    echo "Building the kernel image"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -j$(nproc) Image
    if [ ! -f "arch/${ARCH}/boot/Image" ]; then
        echo "Kernel image build failed!"
        exit 1
    fi
    cp arch/${ARCH}/boot/Image ${OUTDIR}
fi

echo "Kernel image has been built and placed in ${OUTDIR}"

# Prepare the root filesystem (rootfs)
echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]; then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm -rf ${OUTDIR}/rootfs
fi

# Create the base directories
mkdir -p ${OUTDIR}/rootfs/{bin,sbin,etc,home,lib,lib64,mnt,opt,proc,root,sys,tmp,usr,var}

# Clone busybox if it doesn't exist
if [ ! -d "${OUTDIR}/busybox" ]; then
    echo "Cloning busybox"
    git clone git://busybox.net/busybox.git ${OUTDIR}/busybox
    cd busybox
    git checkout ${BUSYBOX_VERSION}
else
    cd ${OUTDIR}/busybox
fi

# Configure and build busybox
echo "Configuring busybox"
make distclean
make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -j$(nproc)

# Install busybox into the rootfs
echo "Installing busybox to rootfs"
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} CONFIG_PREFIX=${OUTDIR}/rootfs install

# Create symbolic links for busybox applets
echo "Creating symbolic links for busybox applets"
cd ${OUTDIR}/rootfs/bin
for app in $(find ${OUTDIR}/rootfs/usr/bin -type f); do
    ln -s $app $(basename $app)
done

# Create the necessary device nodes
echo "Creating device nodes"
cd ${OUTDIR}/rootfs
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/tty1 c 4 1

# Cross-compile the writer utility
echo "Building writer utility"
cd ${FINDER_APP_DIR}
make CROSS_COMPILE=${CROSS_COMPILE}

# Copy the writer application to rootfs
echo "Copying writer utility to /home"
cp writer ${OUTDIR}/rootfs/home

# Copy the finder related scripts and executables
echo "Copying finder related scripts and executables"
cp ${FINDER_APP_DIR}/finder.sh ${OUTDIR}/rootfs/home
cp ${FINDER_APP_DIR}/conf/username.txt ${OUTDIR}/rootfs/home
cp ${FINDER_APP_DIR}/conf/assignment.txt ${OUTDIR}/rootfs/home
cp ${FINDER_APP_DIR}/finder-test.sh ${OUTDIR}/rootfs/home
sed -i 's|../conf/assignment.txt|/home/assignment.txt|' ${OUTDIR}/rootfs/home/finder-test.sh

# Copy the autorun-qemu.sh script
echo "Copying autorun-qemu.sh"
cp ${FINDER_APP_DIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home

# Chown the root directory to root:root
echo "Setting ownership of /home directory"
sudo chown -R root:root ${OUTDIR}/rootfs/home

# Create initramfs
echo "Creating initramfs.cpio.gz"
cd ${OUTDIR}/rootfs
find . | cpio -H newc -o | gzip > ${OUTDIR}/initramfs.cpio.gz

echo "Build process complete!"

