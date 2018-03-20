#!/usr/bin/python3
#
# This script browses the content of a Samsung Galaxy device through
# MTP looking for files that may cause an error when retrieved with
# the MTP GetPartialObject command. The file must have a size that
# matches the condition (size % 512) == 500.
#
# SPDX-License-Identifier:	Apache-2.0
#


from __future__ import print_function
import re
import subprocess
import sys


def mtp_files_filter():
    cmd = ['mtp-files', ]
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    stdout, _ = proc.communicate()
    if proc.returncode != 0:
        print('Error: the `mtp-files` command failed', file=sys.stderr)
        return
    galaxy_pattern = re.compile('Listing File Information on Device with name: Galaxy')
    if not galaxy_pattern.search(stdout.decode()):
        print('WARNING: a Samsung Galaxy device is expected', file=sys.stderr)
    pattern = re.compile(
        r'^File ID: (?P<id>\d+)\n\s+Filename: (?P<name>[\w\.]+)\n\s+File size (?P<size>\d+)',
        re.MULTILINE
    )
    nr_total = 0
    nr_bad = 0
    for match in pattern.finditer(stdout.decode()):
        nr_total += 1
        size = int(match.group('size'))
        if size % 512 == 500:
            nr_bad += 1
            print('ID: {}, filename: {}, size: {} bytes'.format(
                match.group('id'), match.group('name'), match.group('size')
            ))
    print('{} file(s) may fail with GetPartialObject (total: {} files)'.format(nr_bad, nr_total))


if __name__ == '__main__':
    mtp_files_filter()
