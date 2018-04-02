MTP GetPartialObject bug on Samsung Galaxy
==========================================

This repository contains a reproducer for the MTP GetPartialObject
offset bug present on Samsung Galaxy devices. This bug prevents some
files from a Samsung Galaxy device to be read through the MTP
protocol.

The bug only happens under the following conditions:
1. the GetPartialObject command is used to read the file
2. the GetPartialObject offset is `size - (N * 512) - 500` bytes with
   `N` being an integer
3. the GetPartialObject request reaches the end of the file

In practice, most clients of libmtp use a block size which is a power
of 2, usually a multiple of 512 bytes. So the offset of a
GetPartialObject request is likely to be a multiple of 512 bytes. This
condition combined with condition 2 makes the bug occur when
`(size % 512) == 500` with a probability of 1/512.

When the bug happens, the Samsung Galaxy device does not return the
requested data and the GetPartialObject command fails after a
time-out.

How to reproduce the issue
--------------------------

You can invoke the reproducer and pass an object ID of a file:

    $ ./mtp-galaxy-bug 972
    LIBMTP_Set_Debug: Setting debugging level to 2 (0x02) (on)
    LIBMTP LIBMTP_Detect_Raw_Devices[695]: Device 0 (VID=04e8 and PID=6860) is a Samsung Galaxy models (MTP).
    Object 972: size = 1555444 bytes
    GetPartialObject: offset = 1554944 bytes, count = 512 bytes
    == FAIL ==

If you want to trigger the issue with
[GVFS](https://wiki.gnome.org/Projects/gvfs), you have to find a file
whose size will cause the last USB bulk transfer to be 500 bytes long
(i.e. `(size % 512) == 500`). To this end, you can invoke the
`mtp-galaxy-find-files.py` script:

    $ ./mtp-galaxy-find-files.py
    Device 0 (VID=04e8 and PID=6860) is a Samsung Galaxy models (MTP).
    ID: 52, filename: IMG_20180305_114918_5.jpg, size: 1496052 bytes
    ID: 99, filename: IMG_20180305_114926_5.jpg, size: 1548276 bytes
    ID: 972, filename: IMG_20180305_115203.jpg, size: 1555444 bytes
    ID: 1666, filename: IMG_20180312_151120_5.jpg, size: 2514932 bytes
    4 file(s) may fail with GetPartialObject (total: 1688 files)

Then you can mount the MTP share as a user-space file system and copy
the file to your machine:

    $ gio mount -li | grep activation_root
      activation_root=mtp://[usb:003,009]/
    $ gio mount mtp://[usb:003,009]/
    $ stat -c "%n %s" /run/user/$(id -u)/gvfs/mtp:host=%5Busb%3A003%2C009%5D/Phone/DCIM/OpenCamera/IMG_20180305_114918_5.jpg
    /run/user/1000/gvfs/mtp:host=%5Busb%3A003%2C009%5D/Phone/DCIM/OpenCamera/IMG_20180305_114918_5.jpg 1496052
    $ cp /run/user/$(id -u)/gvfs/mtp:host=%5Busb%3A003%2C009%5D/Phone/DCIM/OpenCamera/IMG_20180305_114918_5.jpg /tmp/
    cp: error reading '/run/user/1000/gvfs/mtp:host=%5Busb%3A003%2C009%5D/Phone/DCIM/OpenCamera/IMG_20180305_114918_5.jpg': Input/output error

Additional information
----------------------

Why doesn't this happen on Windows? Because the MTP driver on Windows
does not use the GetPartialObject command. It uses the GetObject
command instead.

Why doesn't this happen with Android devices not manufactured by
Samsung? Because Samsung relies on their own MTP stack which is
different from the Android MTP stack.
