//B+ Tree disk based
//this project is for MJoin B+ tree research

#include <iostream>
#include <vector>
using namespace std;

#include "BTreeIndex.h"

void GenerateBPlusTreeFromFile(int argc, char* argv[]);
void GenerateBPlusTree(int argc, char* argv[]);

int main(int argc, char* argv[])
{	
	//DebugInit("+");//enable all debug information

	GenerateBPlusTreeFromFile(argc, argv);
		
	system("pause");
}

void GenerateBPlusTreeFromFile(int argc, char* argv[])
{
	cout<<"argv: string:datafileName\n";
	string fileName(argv[1]);
	cout<<"loading data from file...\n";
	ifstream file(fileName, ios::in);

	RecordFile recordFile;
	recordFile.open(fileName+".tbl",'w');
	BTreeIndex btreeindex;
	btreeindex.open(fileName+".idx",'w');

	KeyType key;
	string value;
	RecordId rid;
	
	//create B+ tree from data file
	string line;
	if(!file){
		cout<<"File open failed.\n";
		return;
	}
		
	while(getline(file, line)){
		if(line.compare("")==0)
			break;
		key = atof(line.c_str());	
		value = line;
		recordFile.append(key, value, rid);
		btreeindex.insert(key, rid);
	}

	btreeindex.close();
	recordFile.close();
	file.close();
	cout<<"Done.\nTest index file...";

	//search
	BTreeIndex btreeindex;
	btreeindex.open(fileName+".idx",'r');
	
	KeyType minkey = btreeindex.getMinimumKey();
	KeyType maxkey = btreeindex.getMaximumKey();
	cout<<"minKey: "<<minkey <<"	maxKey:"<<maxkey<<endl;
	btreeindex.close();
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
