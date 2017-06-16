# build kernel
cd kernel
make -f Makefile.linux
cd ../
\cp -rf kernel/kernelz boot/
\cp -rf kernel/system.map ./
cd boot
make -f Makefile.linux
cd ../
\cp -rf boot/oskernel.elf.gz ./oskernel
perl map2idc.pl > system.idc
# build user executable
cd user
[ -d objs ] || mkdir objs
make -f Makefile.sh
make -f Makefile.eval
make -f Makefile.pi
make -f Makefile.queens
# build image
cd ..
[ ! -d tmp ] || mkdir tmp
sudo mount floppy.img -t vfat tmp
sudo \cp -rf ./oskernel tmp/boot
sudo \cp -rf ./user/sh tmp/bin
sudo \cp -rf ./user/eval tmp/bin
sudo \cp -rf ./user/queens tmp/bin
sudo \cp -rf ./user/pi tmp/bin
sleep 1
sudo umount tmp

