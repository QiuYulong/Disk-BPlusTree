#include "BTreeNode.h"

using namespace std;
BTNode::BTNode()
{
    n = 0;
    isLeaf = false;
    nextPage = -1;
    pid = -1;
    memset(buffer,0,PageFile::PAGE_SIZE);
    keys = (KeyType *)(buffer + sizeof(bool) + sizeof(int) +sizeof(int));
    rids = (RecordId *)(keys + KEYS_PER_LEAF_PAGE);
    pids = (PageId *)(keys +  KEYS_PER_NONLEAF_PAGE);
}
BTNode::BTNode(const BTNode& n)
{
    this->n = n.n;
    this->isLeaf = n.isLeaf;
    this->nextPage = n.nextPage;
    this->pid = n.pid;
    memcpy(this->buffer, n.buffer, PageFile::PAGE_SIZE);
    keys = (KeyType *)(buffer + sizeof(bool) + sizeof(int) +sizeof(int));
    rids = (RecordId *)(keys + KEYS_PER_LEAF_PAGE);
    pids = (PageId *)(keys +  KEYS_PER_NONLEAF_PAGE);

}
/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::read(PageId p, const PageFile& pf)
{  
    RC rc;
    // read the page containing the leaf node
    if ((rc = pf.read(p, buffer)) < 0) return rc;
    this->pid = p;
    
    // the second four bytes of a page contains # keys in the page
    memcpy(&isLeaf, buffer, sizeof(bool));
    memcpy(&n, buffer+sizeof(bool), sizeof(int));
    memcpy(&nextPage, buffer+sizeof(bool)+sizeof(int), sizeof(PageId));
    return 0; 
}
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::write( PageFile& pf)
{ 
    RC rc;
    if(pid == -1 ) return -1;
    // write the page to the disk
    memcpy(buffer, &isLeaf, sizeof(bool));
    memcpy(buffer+sizeof(bool), &n, sizeof(int));
    memcpy(buffer+sizeof(bool)+sizeof(int), &nextPage, sizeof(PageId));
    if ((rc = pf.write(this->pid, buffer)) < 0) return rc;
     
    return 0; 
}
   
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::write(PageId p,PageFile& pf)
{ 
    RC rc;
    // write the page to the disk
    if(this->pid != p)  printf("WARNING:pid[%d] != p[%d]\n",pid,p);
    memcpy(buffer, &isLeaf, sizeof(bool));
    memcpy(buffer+sizeof(bool), &n, sizeof(int));
    memcpy(buffer+sizeof(bool)+sizeof(int), &nextPage, sizeof(PageId));
    if ((rc = pf.write(p, buffer)) < 0) return rc;
    this->pid = p;
      
    return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNode::getKeyCount()
{
    return n;    
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNode::insertNonFull(KeyType key, const RecordId& rid, int &newPid, PageFile &pf)
{ 
    int i = n - 1;
    RC rc = 0;
    if(isLeaf){
        while( i>=0 && key< keys[i] ){
            keys[i + 1] = keys[i];
            rids[i + 1].pid = rids[i].pid;
            rids[i + 1].sid = rids[i].sid;

            i--;
        }
        keys[i+1] = key;
        rids[i+1].pid = rid.pid;
        rids[i+1].sid = rid.sid;
        
        n++;
        DEBUG('i',"insert pid[%d] : key[%d] -> keys[%d]\n",pid, key, i+1);
        if(DebugIsEnabled('i')) printNode();
        rc = write(pf);
        if(rc != 0) goto ERROR;
        return 0;
    }else{
        BTNode node;
        DEBUG('i',"insert to non leaf node pid[%d] : key[%d]\n",pid, key);
        if(DebugIsEnabled('i')) printNode();
        while(i>=0 && key < keys[i]) i--;
        i++;
        DEBUG('i',"i:%d\n\n",i);
        rc = node.read(pids[i], pf);
        if(rc != 0) goto ERROR;
        if(node.isLeaf){
            DEBUG('i',"Read Leaf Node page:\n");
        }else{
            DEBUG('i',"Read Non Leaf Node page:\n");
        }
        if(DebugIsEnabled('i')) node.printNode();

        if(node.n == 2*node.getT() - 1){
            splitChild(i, newPid, pf);
            newPid++;
            if( key >= keys[i])  i++; // insert in to new child node
            node.read(pids[i], pf);
        }
        if( (rc = node.insertNonFull(key, rid,newPid,  pf)) != 0) goto ERROR;
    }
    return 0;
ERROR:
    printf("error insertNonFull\n");
    return rc;    
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::splitChild(int i, PageId newPid, PageFile& pf)
{   
    RC rc = 0;
    int j;
    BTNode newN; //new node
    BTNode oldN; //child node
    int t;
    DEBUG('i',"Split Child pid:%d  newPid:%d\n",pids[i],newPid);
    if( this->isLeaf == true ) { rc = -1; goto ERROR; }
    if( (rc = oldN.read(this->pids[i], pf)) != 0) { rc = -2; goto ERROR; } 
    newN.isLeaf = oldN.isLeaf;
    t = newN.getT();
    newN.pid = newPid;
    if( newN.isLeaf ){
        newN.n = t;
        for(j=0; j<=t-1; j++){
            newN.keys[j] = oldN.keys[j+t-1];
            newN.rids[j] = oldN.rids[j+t-1];
        }
        oldN.n = oldN.n - t;
        for(j=n; j>=i+1; j--)
            keys[j] = keys[j-1];
        for(j=n+1; j>=i+2; j--)
            pids[j] = pids[j-1];
        keys[i] = newN.keys[0];
        pids[i+1] = newN.pid;
        n++;

        PageId tmp = oldN.getNextNodePtr();
        oldN.setNextNodePtr(newN.pid);
        newN.setNextNodePtr(tmp);
    }else{
        newN.n = t - 1;
        for(j=0; j<=t-2; j++){
            newN.keys[j] = oldN.keys[j+t];
        }
        for(j=0; j<=t-1; j++){
            newN.pids[j] = oldN.pids[j+t];
        }

        oldN.n = oldN.n - newN.n - 1;
        for(j=n; j>=i+1; j--)
            keys[j] = keys[j-1];
        for(j=n+1; j>=i+2; j--)
            pids[j] = pids[j-1];
        keys[i] = oldN.keys[oldN.n];
        pids[i+1] = newN.pid;
        n++;
    }
    if(DebugIsEnabled('i')){
        this->printNode();
        oldN.printNode();
        newN.printNode();
    }
    oldN.write( pf );
    this->write( pf );
    newN.write( pf);
    return 0;
ERROR:
    printf("error:%d\n",rc);
    return rc;
}

int BTNode::getT()
{
    int t = -1;
    if(isLeaf )    t = (KEYS_PER_LEAF_PAGE+1)/2;
    else  t = (KEYS_PER_NONLEAF_PAGE+1)/2;
    return t;
}
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
RC BTNode::locate(int searchKey, const PageFile &pf,  IndexCursor& cursor)
{
    int i = 0;
    RC rc = 0;
    BTNode node;
    if(DebugIsEnabled('s')) printNode();
    while( i < n && searchKey > keys[i] ){ //loop until keys[i] >= searchKey or until the end of keys list
        i++;
    }
    if(isLeaf){
        if(i < n) {
            cursor.pid = pid;
            cursor.eid = i;
        }else{
            cursor.pid = -1;
            cursor.eid = -1;
        }
    }else{
        
        rc = node.read( pids[i], pf);
        if(rc != 0) goto ERROR;
        rc = node.locate(searchKey, pf, cursor);
        if(rc != 0) goto ERROR;
    }
    return 0; 
ERROR:
    printf("error\n");
    return rc;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::readEntry(int eid, int& key, RecordId& rid)
{
    RC rc;
    if(!isLeaf || eid > n || eid <0){
        rc = -1;
        goto ERROR;
    }
    key = keys[eid];
    rid.pid = rids[eid].pid;
    rid.sid = rids[eid].sid;
    return 0; 
ERROR:
    printf("error\n");
    return rc;
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTNode::getNextNodePtr()
{ 
    return nextPage; 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::setNextNodePtr(PageId pid)
{ 
    nextPage = pid;
    return 0; 
}




RC BTNode::initializeRoot(PageId pid1, KeyType key, PageId pid2)
{
    int add1 = (char *)keys - ((char *)buffer);
    int add2 = (char *)pids - ((char *)buffer);
    DEBUG('i',"initializeRoot keys[0x%x]:0x%x pids[0x%x]\n",add1,key,add2);
    keys[0] = key;
    pids[0] = pid1;
    pids[1] = pid2;
    n = 1; 
    return 0; 
}


void BTNode::printNode()
{
    int i;
    if(isLeaf){
        printf("pid:%d n:%d Max_n:%d t:%d nextPage:%d\n", pid, n, KEYS_PER_LEAF_PAGE, getT(), nextPage);
        for(i=0; i<n; i++){
            printf("position:%d\t\tkey:%d\t\trid:{%d,%d}\n",i, keys[i], rids[i].pid, rids[i].sid);
        }
    }else{
        printf("pid:%d n:%d Max_n:%d t:%d\n", pid, n, KEYS_PER_NONLEAF_PAGE, getT());
        for(i=0; i<n; i++){
            printf("position:%d\tpid:%d\n",i, pids[i]);
            printf("position:%d\t\tkey:%d\n",i, keys[i]);
        }
        printf("position:%d\tpid:%d\n",i, pids[i]);
    }
    printf("\n");
}



