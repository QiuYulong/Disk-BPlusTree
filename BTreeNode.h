#ifndef BTNODE_H
#define BTNODE_H

#include "RecordFile.h"
#include "PageFile.h"
typedef int KeyType;
/**
 * The data structure to point to a particular entry at a b+tree leaf node.
 * An IndexCursor consists of pid (PageId of the leaf node) and
 * eid (the location of the index entry inside the node).
 * IndexCursor is used for index lookup and traversal.
 */
typedef struct {
  // PageId of the index entry
  PageId  pid;
  // The entry number inside the node
  int     eid;
} IndexCursor;


/**
 * BTLeafNode: The class representing a B+tree leaf node.
 */
class BTNode {
    
private:
       KeyType * keys;
    
    RecordId * rids;
    PageId nextPage;
public:
    //key count
    int n;
    bool isLeaf;
    PageId * pids;
    PageId pid;
    /**
    * The main memory buffer for loading the content of the disk page 
    * that contains the node.
    */
    char buffer[PageFile::PAGE_SIZE];

public:
    //first 1 byte store node type(1 leaf, 0 nonleaf), 
    //secon 4 bytes store # keys, 
    //third 4 bytes store pointer to next leaf(-1 if nil) 
    static const int KEYS_PER_LEAF_PAGE=(PageFile::PAGE_SIZE-sizeof(bool)-sizeof(int)-sizeof(PageId))/(sizeof(KeyType)+sizeof(RecordId));  
    static const int RECORDIDS_PER_LEAF_PAGE = KEYS_PER_LEAF_PAGE;
    //first 1 byte store node type(1 leaf, 0 nonleaf), 4 bytes store # keys,  
    static const int KEYS_PER_NONLEAF_PAGE=(PageFile::PAGE_SIZE-sizeof(bool)-sizeof(int)-sizeof(PageId))/(sizeof(KeyType)+sizeof(PageId))-1;  
    static const int PIDS_PER_PAGE =  KEYS_PER_NONLEAF_PAGE + 1;

    BTNode();
    BTNode(const BTNode& n);
    RC initializeRoot(PageId pid1, KeyType key, PageId pid2);
   /**
    * Insert the (key, rid) pair to the node.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param rid[IN] the RecordId to insert
    * @return 0 if successful. Return an error code if the node is full.
    */
    RC insertNonFull(KeyType, const RecordId&, int&, PageFile&);
   /**
    * Insert the (key, rid) pair to the node
    * and split the node half and half with sibling.
    * The first key of the sibling node is returned in siblingKey.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert.
    * @param rid[IN] the RecordId to insert.
    * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
    * @param siblingKey[OUT] the first key in the sibling node after split.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC splitChild(int i, int newPid, PageFile&);

    /*
     * Find the entry whose key value is larger than or equal to searchKey
     * and output the eid (entry number) whose key value >= searchKey.
     * Remeber that all keys inside a B+tree node should be kept sorted.
     * @param searchKey[IN] the key to search for
     * @param pf[IN] the page file
     * @param cursor[OUT] the cursor pointing to the first index entry
     *                    with the key value. Return cursor.pid = -1 if not found.
     * @return 0 if successful. Return an error code if there is an error.
     */
    RC locate(int searchKey, const PageFile &pf,  IndexCursor& cursor);

   /**
    * Read the (key, rid) pair from the eid entry.
    * @param eid[IN] the entry number to read the (key, rid) pair from
    * @param key[OUT] the key from the slot
    * @param rid[OUT] the RecordId from the slot
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC readEntry(int eid, int& key, RecordId& rid);

   /**
    * Return the pid of the next slibling node.
    * @return the PageId of the next sibling node 
    */
    PageId getNextNodePtr();


   /**
    * Set the next slibling node PageId.
    * @param pid[IN] the PageId of the next sibling node 
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC setNextNodePtr(PageId pid);

   /**
    * Return the number of keys stored in the node.
    * @return the number of keys in the node
    */
    int getKeyCount();
 
   /**
    * Read the content of the node from the page pid in the PageFile pf.
    * @param pid[IN] the PageId to read
    * @param pf[IN] PageFile to read from
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC read(PageId pid, const PageFile& pf);
    
   /**
    * Write the content of the node to the page pid in the PageFile pf.
    * @param pid[IN] the PageId to write to
    * @param pf[IN] PageFile to write to
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC write(PageId pid, PageFile& pf);
    
    /**
    * Write the content of the node to the page pid in the PageFile pf.
    * @param pid[IN] the PageId to write to
    * @param pf[IN] PageFile to write to
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC write(PageFile& pf);


    int getT();
    void printNode();
        
}; 


#endif /* BTNODE_H */
