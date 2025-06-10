if [ ! -d "build" ]; then
    mkdir build/
fi

make

qemu-system-x86_64 -fda build/floppy.img
