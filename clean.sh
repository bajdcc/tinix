rm oskernel
rm system.map
rm system.idc
cd kernel
make -f Makefile.linux clean
cd ../boot
make -f Makefile.linux clean
cd ../user
make -f Makefile.sh clean
make -f Makefile.eval clean
make -f Makefile.pi clean
make -f Makefile.queens clean
make -f Makefile.badapple clean
rm -r objs/*
