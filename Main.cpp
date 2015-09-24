//B+ Tree disk based
//this project is for MJoin B+ tree research

#include <iostream>
#include <vector>
using namespace std;

#include "BTreeIndex.h"

void GenerateBPlusTree(int argc, char* argv[]);

int main(int argc, char* argv[])
{	
	//DebugInit("+");//enable all debug information

	//GenerateBPlusTree(argc, argv);

	//search
	BTreeIndex btreeindex;
	btreeindex.open("test.idx",'r');

	
	
	/**IndexCursor cursor;
	tree.locate(0,cursor);
	for(int i=0;i<10000;i++)
	{
		tree.readForward(cursor, kt, ri);
		cout<<"KeyType:"<<kt<<"    Record.Pid:"<<ri.pid<<"    Record.Sid:"<<ri.sid<<endl;
	}*/

	int minkey = btreeindex.getMinimumKey();
	int maxkey = btreeindex.getMaximumKey();

	btreeindex.close();
	system("pause");
}

void GenerateBPlusTree(int argc, char* argv[])
{
	cout<<"argv: number:int string:fileName\n";
	//crete B+ Tree
	if(argc != 3)
	{
		cout<<"invalid argv...\nexit\n";
		return;
	}
	cout<<"creating BPTree...\n";
	int pointCount = atoi(argv[1]);
	string filename = string(argv[2]);

	RecordFile recordFile;
	recordFile.open(filename+".tbl",'w');
	BTreeIndex btreeindex;
	btreeindex.open(filename+".idx",'w');

	KeyType key;
	string value;
	RecordId rid;
	for(int i=0;i<pointCount;i++)
	{
		key = i;
		value = to_string(i);
		recordFile.append(key, value, rid);//append to record file, and get location(rid) of record;
		btreeindex.insert(key, rid);//insert key-value to index file(BPlusTree file)
	}
	btreeindex.close();
	recordFile.close();
	cout<<"Done.\n";
}