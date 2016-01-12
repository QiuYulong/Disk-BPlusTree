# Disk-BPlusTree
one clear implementation disk based Bplus Tree (Windows platform)

code is compiled on Windows, system call code need to be revised when compiling on Linux.

bug fix:
1: revise system call to Windows platform.
2: 'BTreeIndex.cc':'setTreeHeight', address offset bug, which leads to wrong tree height setting.
3: (2015.10.1) 'BTreeNode.h'&'BTreeNode.cc':'readEntry', parameter type of 'key' should be 'KeyType', which leads to bug when change KeyType to other category.
