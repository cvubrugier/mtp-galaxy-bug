mtp-galaxy-bug
==============

This repository contains a reproducer for the MTP GetPartialObject
offset bug present on Samsung Galaxy devices. This bug prevents some
files from a Samsung Galaxy device to be read through the MTP
protocol.

The bug only happens under the following conditions:
1. the GetPartialObject command is used to read the file
2. the file size matches the condition `(size % 512) == 500`
3. the GetPartialObject offset is a multiple of 512 bytes
4. the GetPartialObject request reaches the end of the file

While the above conditions look restrictive they are easily met in
practice. For instance, the GVFS MTP backend uses the GetPartialObject
MTP command whenever it reads a file. Since most programs use a block
which is a large power of 2, the offset is usually a multiple of 512
bytes. Only the second condition about the file size is rare, its
occurrence being 1/512.

When the bug happens, the Samsung Galaxy device does not return the
requested data and the GetPartialObject command fails after a timeout.

How to reproduce the issue
--------------------------

The first step is to find a file on your Samsung Galaxy device whose
size matches the `(size % 512) == 500` condition. For that, you can
invoke the `mtp-galaxy-find-files.py` script:

    $ ./mtp-galaxy-find-files.py
    Device 0 (VID=04e8 and PID=6860) is a Samsung Galaxy models (MTP).
    ID: 52, filename: IMG_20180305_114918_5.jpg, size: 1496052 bytes
    ID: 99, filename: IMG_20180305_114926_5.jpg, size: 1548276 bytes
    ID: 972, filename: IMG_20180305_115203.jpg, size: 1555444 bytes
    ID: 1666, filename: IMG_20180312_151120_5.jpg, size: 2514932 bytes
    4 file(s) may fail with GetPartialObject (total: 1688 files)

Then you can invoke the reproducer and pass an object ID that can cause the error:

    $ ./mtp-galaxy-bug 972
    LIBMTP_Set_Debug: Setting debugging level to 2 (0x02) (on)
    LIBMTP LIBMTP_Detect_Raw_Devices[695]: Device 0 (VID=04e8 and PID=6860) is a Samsung Galaxy models (MTP).
    Object 972: size = 1555444 bytes
    GetPartialObject: offset = 1554944 bytes, count = 512 bytes
    == FAIL ==

If you don't want to use the provided bug reproducer, you can mount
the file system with GVFS and copy the file to your machine:

    $ gio mount -li | grep activation_root
      activation_root=mtp://[usb:003,009]/
    $ gio mount mtp://[usb:003,009]/
    $ stat -c "%n %s" /run/user/$(id -u)/gvfs/mtp:host=%5Busb%3A003%2C009%5D/Phone/DCIM/OpenCamera/IMG_20180305_114918_5.jpg
    /run/user/1000/gvfs/mtp:host=%5Busb%3A003%2C009%5D/Phone/DCIM/OpenCamera/IMG_20180305_114918_5.jpg 1496052
    $ cp /run/user/$(id -u)/gvfs/mtp:host=%5Busb%3A003%2C009%5D/Phone/DCIM/OpenCamera/IMG_20180305_114918_5.jpg /tmp/
    cp: error reading '/run/user/1000/gvfs/mtp:host=%5Busb%3A003%2C009%5D/Phone/DCIM/OpenCamera/IMG_20180305_114918_5.jpg': Input/output error

Additional information
----------------------

Why doesn't this error happen on Windows? Because the MTP driver on
Windows does not use the GetPartialObject command. It uses the
GetObject command instead.

Why doesn't this happen with Android devices not manufactured by
Samsung? Because Samsung relies on their own MTP stack which is
different from the Android MTP stack.
