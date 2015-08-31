losetup /dev/loop0 ./disk.img
losetup -o $((63*512)) /dev/loop1 ./disk.img
mount /dev/loop1 /mnt/mini
cp -f ../../proj/linux/build/kodext /mnt/mini/boot
umount /mnt/mini
losetup -d /dev/loop1
losetup -d /dev/loop0

