# Disk-BPlusTree
one clear implementation disk based Bplus Tree (Windows platform)
code from unknown source

original code is based on POSIX OS (like linux), I have revised the system call code to make it compatible with Windows platform. If compile on Linux, system call code need to be revised back.

bug fix:
1: revise system call to Windows platform.
2: 'BTreeIndex.cc':'setTreeHeight', address offset bug, which leads to wrong tree height setting.
