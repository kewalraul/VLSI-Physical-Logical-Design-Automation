#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <list>
#include <vector>
#include <sstream>
#include <functional>
using namespace std;

//function for splitting string into string tokens on basis of delimiters
vector<string> split(string eachLine, char delim) {
	vector<string> strs;
	istringstream f(eachLine);
	string s;
	while (getline(f, s, delim)) {
		strs.push_back(s);
	}
	return strs;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
	long key;
	string nodeName;
	int nodeArea;
} node;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class which holds hashmap and netlist read from file
class graphAsNetlists {
public:
	long numPadOff;
	list<node> nodesInfo;
	vector<list<long> > adjList;
	map<long, int> count;
	list<node> getNodeInfo();
	vector<list<long> > getAdjList();
	map<long, int> getCount();
	void parseFile();
	void nodeCount(long nodeKey);
	int getMaxNodeCount();
};

list<node> graphAsNetlists::getNodeInfo() {
	return (this->nodesInfo);
}

vector<list<long> > graphAsNetlists::getAdjList() {
	return (this->adjList);
}

map<long, int> graphAsNetlists::getCount() {
	return (this->count);
}

//to keep a count of all occurences of nodes
void graphAsNetlists::nodeCount(long nodeKey) {
	count[nodeKey]++;
}

int graphAsNetlists::getMaxNodeCount() {
	int currentMax = 0;
	for (map<long, int>::iterator it = count.begin(); it != count.end(); it++) {
		if (it->second > currentMax) {
			currentMax = it->second;
		}
	}
	return currentMax;
}

void graphAsNetlists::parseFile() {
	ifstream netDFile, areFile;
	string eachLine;
	netDFile.open("ibm01.netD", ifstream::in);

	long ignore, numPins, numNets, numMod;
	netDFile >> ignore;
	netDFile >> numPins;
	netDFile >> numNets;
	netDFile >> numMod;
	netDFile >> numPadOff;
	netDFile >> std::ws;

	long key = 0, srcTempKey = 0;
	list<long> tempList;
	int count = 0;

	while (getline(netDFile, eachLine) && !netDFile.eof()) {
		vector<string> strs;
		strs = split(eachLine, ' ');

		if (strs[0][0] == 'p') {
			continue;
		} else {
			node temp;
			list<node>::iterator it = nodesInfo.begin();
			while (it != nodesInfo.end()) {
				if ((it->nodeName) == (strs[0])) {
					break;
				}
				it++;
			}

			if (it == nodesInfo.end()) {
				temp.key = key;
				temp.nodeName = strs[0];
				nodesInfo.push_back(temp);
				key++;
				srcTempKey = key - 1;
			} else {
				srcTempKey = it->key;
			}

			if (strs[1] == "s") {
				if ((tempList.size() > 1) && count != 0) {
					for (list<long>::iterator i = tempList.begin();
							i != tempList.end(); i++) {
						nodeCount(*i);
					}
					adjList.push_back(tempList);
					tempList.clear();
				} else// run only once for the first time since templist does not contain anything to push
				{
					tempList.clear();
					count++;
				}
			}
			tempList.push_back(srcTempKey);
		}
	}
	if (tempList.size() > 1) {
		adjList.push_back(tempList);
		tempList.clear();
	}
	netDFile.close();

	areFile.open("ibm01.are", ifstream::in);

	while (getline(areFile, eachLine) && !areFile.eof()) {
		vector<string> strs;
		strs = split(eachLine, ' ');
		if (strs[0][0] == 'p') {
			continue;
		} else {
			list<node>::iterator it = nodesInfo.begin();
			while (it != nodesInfo.end()) {
				if ((it->nodeName) == (strs[0])) {
					string g = strs[1];
					it->nodeArea = std::stoi(g, nullptr, 10);
					break;
				}
				it++;
			}
		}
	}
	areFile.close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class bucketStructure {
public:
	map<long, list<long> > bucket1, bucket2;
	map<long, list<long> > getBucket1();
	map<long, list<long> > getBucket2();
	void setBucket1(map<long, list<long> >);
	void setBucket2(map<long, list<long> >);
	void makeBucketSets(int maxNodeCount);
};

void bucketStructure::makeBucketSets(int maxNodeCount) {
	for (long i = maxNodeCount; i >= -maxNodeCount; i--) {
		bucket1.insert(pair<long, list<long>>(i, list<long>()));
		bucket2.insert(pair<long, list<long>>(i, list<long>()));
	}
}

map<long, list<long> > bucketStructure::getBucket1() {
	return (this->bucket1);
}

map<long, list<long> > bucketStructure::getBucket2() {
	return (this->bucket2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Partitions {
public:
	vector<long> partition1, partition2;
	vector<long> lockedNodes;
	int cutSetSizeCount = 0;
	vector<long> getPartition1();
	vector<long> getPartition2();
	int getCutSetSizeCount();
	void makeInitialPartitions(std::list<node> &);
	void countCutsetSize(vector<list<long> > &);
};

vector<long> Partitions::getPartition1() {
	return (this->partition1);
}

vector<long> Partitions::getPartition2() {
	return (this->partition2);
}

int Partitions::getCutSetSizeCount() {
	return this->cutSetSizeCount;
}
void Partitions::makeInitialPartitions(std::list<node> &nodeKeyValMapPointer) {
	int i;
	for (i = 0; i < nodeKeyValMapPointer.size() / 2; i++) {
		this->partition1.push_back(i);
	}
	for (; i < nodeKeyValMapPointer.size(); i++) {
		this->partition2.push_back(i);
	}
}

void Partitions::countCutsetSize(vector<list<long> > &adjList) {
	cutSetSizeCount = 0;
	for (list<long> netList : adjList) {
		vector<long> tempPartition;
		for (std::list<long>::iterator it = netList.begin();
				it != netList.end(); it++) {
			if (it == netList.begin())
				tempPartition =
						std::find(partition1.begin(), partition1.end(), *it)
								== partition1.end() ? partition2 : partition1;
			if (std::find(tempPartition.begin(), tempPartition.end(), *it)
					== tempPartition.end()) {
				cutSetSizeCount++;
				break;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class algo: public graphAsNetlists, public bucketStructure, public Partitions {
public:
	long calculateNodeGain(long node, vector<long> &);
	void updateBucketGains();
	pair<long, long> getOptimumNode(long previousNode, long previousNodeGain);
	pair<long, long> getHighestGainFromBucket(long previousHigh);
	void shiftNode();
	bool isWithinAreaConstraint(long optimumGainNode);
	void updateGains1(long node);
};

long algo::calculateNodeGain(long node, vector<long> &partition) {
	vector<list<long> > adjList = graphAsNetlists::getAdjList();
	long gain = 0;
	for (list<long> tempNetList : adjList) {
		if (std::find(tempNetList.begin(), tempNetList.end(), node)
				!= tempNetList.end()) {
			int tempTi = 0, tempFi = 0;
			for (std::list<long>::iterator it = tempNetList.begin();
					it != tempNetList.end(); ++it) {
				if ((std::find(partition.begin(), partition.end(), *it)
						== partition.end()) && *it != node) {
//					tempTi += 0;
					tempFi += 1;
				} else if (*it != node) {
					tempTi += 1;
//					tempFi += 0;
				}
			}
			if (tempFi > 0 && tempTi > 0) {
				gain += 0;
			} else if (tempFi > 0 && tempTi == 0) {
				gain += 1;
			} else if (tempFi == 0 && tempTi > 0) {
				gain -= 1;
			}
		}
	}
	return gain;
}

void algo::updateBucketGains() {
//	for (map<long, list<long> >::iterator it = bucket1.begin();
//			it != bucket1.end(); it++) {
//		(it->second).erase((it->second).begin(), (it->second).end());
//	}
//	for (map<long, list<long> >::iterator it = bucket2.begin();
//			it != bucket2.end(); it++) {
//		(it->second).erase((it->second).begin(), (it->second).end());
//	}

	for (long nodeInPart1 : partition1) {
		if (std::find(lockedNodes.begin(), lockedNodes.end(), nodeInPart1)
				== lockedNodes.end()) {
			long gainOfNode = calculateNodeGain(nodeInPart1, partition1);
			bucket1[gainOfNode].push_back(nodeInPart1);
		}
	}

	for (long nodeInPart2 : partition2) {
		if (std::find(lockedNodes.begin(), lockedNodes.end(), nodeInPart2)
				== lockedNodes.end()) {
			long gainOfNode = calculateNodeGain(nodeInPart2, partition2);
			bucket2[gainOfNode].push_back(nodeInPart2);
		}
	}
}

//area constraint
bool algo::isWithinAreaConstraint(long optimumGainNode) {
	vector<long> foundInPartition, otherPartition;

	double areaInPart1 = 0, areaInPart2 = 0, percent1, percent2;
	vector<long>::iterator it = std::find(partition1.begin(), partition1.end(),
			optimumGainNode);
	if (it != partition1.end()) {
		foundInPartition = partition1;
	} else {
		foundInPartition = partition2;
	}

	for (list<node>::iterator it = nodesInfo.begin(); it != nodesInfo.end();
			it++) {
		if (it->key != optimumGainNode
				&& std::find(foundInPartition.begin(), foundInPartition.end(),
						it->key) != foundInPartition.end())
			areaInPart1 += it->nodeArea;
		else
			areaInPart2 += it->nodeArea;
	}
	percent1 = (areaInPart1 / (areaInPart1 + areaInPart2)) * 100;
	percent2 = (areaInPart2 / (areaInPart1 + areaInPart2)) * 100;
	if ((percent1 > 20 && percent2 < 80) || (percent2 > 20 && percent1 < 80))
		return true;
	else
		return false;
}

void algo::updateGains1(long node)
{
	vector<list<long> > adjList = graphAsNetlists::getAdjList();
		long gainOfNode = 0;
		int part = 0;
		vector<long> foundInPartition;
		for (list<long> tempNetList : adjList) {
			if (std::find(tempNetList.begin(), tempNetList.end(), node)
					!= tempNetList.end()) {
				for(list<long>::iterator i = tempNetList.begin(); i!=tempNetList.end(); i++)
				{
					if(*i == node){
						vector<long>::iterator j = find(partition1.begin(), partition1.end(), *i);
						if(j!=partition1.end()){
							foundInPartition = partition1;
							part = 1;
							for(map<long, list<long>>::iterator j = bucket1.begin(); j!=bucket1.end(); j++)
							{
								list<long>::iterator k = find((j->second).begin(), (j->second).end(), *i);
								if(k!=((j->second).end()))
								{
									(j->second).erase(k);
								}
							}
						}
						else{
							foundInPartition = partition2;
							part = 2;
							for(map<long, list<long>>::iterator j = bucket2.begin(); j!=bucket2.end(); j++)
							{
								list<long>::iterator k = find((j->second).begin(), (j->second).end(), *i);
								if(k!=((j->second).end()))
								{
									(j->second).erase(k);
								}
							}
						}
					}
					else{
						vector<long>::iterator j = find(partition1.begin(), partition1.end(), *i);
						if(j!=partition1.end()){
							foundInPartition = partition1;
							part = 1;
							for(map<long, list<long>>::iterator j = bucket1.begin(); j!=bucket1.end(); j++)
							{
								list<long>::iterator k = find((j->second).begin(), (j->second).end(), *i);
								if(k!=((j->second).end()))
								{
									(j->second).erase(k);
								}
							}
						}
						else{
							foundInPartition = partition2;
							part = 2;
							for(map<long, list<long>>::iterator j = bucket2.begin(); j!=bucket2.end(); j++)
							{
								list<long>::iterator k = find((j->second).begin(), (j->second).end(), *i);
								if(k!=((j->second).end()))
								{
									(j->second).erase(k);
								}
							}
						}
						gainOfNode =  calculateNodeGain(*i, foundInPartition);
						if (std::find(lockedNodes.begin(), lockedNodes.end(), *i)
										== lockedNodes.end()){
							if(part == 1){
								bucket1[gainOfNode].push_back(*i);
							}
							else if(part == 2){
								bucket2[gainOfNode].push_back(*i);
							}
						}
					}
				}
			}
		}
}

//this is deleting from one partition and adding to the other partition, locking tht node and also clearing all the lists in both the buckets
void algo::shiftNode() {
	vector<long> foundInPartition, otherPartition;
	int part;
	pair<long, long> optimumGainNode = getOptimumNode(-1, -999999);
//	cout<<"\nOptimum Gain node: "<<optimumGainNode.first<<" with gain: "<<optimumGainNode.second;
	while (!(isWithinAreaConstraint(optimumGainNode.first))) {
		optimumGainNode = getOptimumNode(optimumGainNode.first,
				optimumGainNode.second);
		cout<<"\nOptimum Gain node: "<<optimumGainNode.first<<" with gain: "<<optimumGainNode.second;
	}

	vector<long>::iterator it = std::find(partition1.begin(), partition1.end(),
			optimumGainNode.first);
	if (it != partition1.end()) {
		foundInPartition = partition1;
		part = 1;
	} else {
		foundInPartition = partition2;
		part = 2;
	}
	vector<long>::iterator foundIndex = find(foundInPartition.begin(),
			foundInPartition.end(), optimumGainNode.first);
	foundInPartition.erase(foundIndex);
	otherPartition = ((part == 1) ? partition2 : partition1);
	otherPartition.push_back(optimumGainNode.first);

// 	cout<<"\n\nSelected Optimum gain node: "<<optimumGainNode.first;

	lockedNodes.push_back(optimumGainNode.first);

	updateGains1(optimumGainNode.first);

	if (part == 1) {
		partition1 = foundInPartition;
		partition2 = otherPartition;
	} else {
		partition2 = foundInPartition;
		partition1 = otherPartition;
	}

}

pair<long, long> algo::getOptimumNode(long previousNode,
		long previousNodeGain) {
	long highestGainFromBucket1 = -999999, highestGainFromBucket2 = -999999;

	long optGain = -999999;

	pair<long, long> p;

	map<long, list<long>> tempBkt1 = bucket1;
	map<long, list<long>> tempBkt2 = bucket2;

	map<long, list<long>>::reverse_iterator bE1;
	map<long, list<long>>::reverse_iterator bE2;

	int flag_bkt1 = 0, flag_bkt2 = 0;

	if (previousNode == -1 && previousNodeGain == -999999) {
		for (bE1 = tempBkt1.rbegin(), bE2 = tempBkt2.rbegin();
				bE1 != tempBkt1.rend() && bE2 != tempBkt2.rend();
				bE1++, bE2++) {
//			cout << "\nbE1 " << bE1->first;
			if ((!((bE1->second).empty())) && ((bE2->second).empty())) {
				highestGainFromBucket1 = bE1->first;
				break;
			} else if (((bE1->second).empty()) && (!((bE2->second).empty()))) {
				highestGainFromBucket2 = bE2->first;
				break;
			} else if ((!((bE1->second).empty()))
					&& (!((bE2->second).empty()))) {
				highestGainFromBucket1 = bE1->first;
				highestGainFromBucket2 = bE2->first;
				break;
			}
		}
		if (highestGainFromBucket1 > highestGainFromBucket2) {
			p.first = (bE1->second).front();
			p.second = bE1->first;
		} else if (highestGainFromBucket1 < highestGainFromBucket2) {
			p.first = (bE2->second).front();
			p.second = bE2->first;
		} else if (highestGainFromBucket1 == highestGainFromBucket2) {
			p.first = (bE1->second).front();
			p.second = bE1->first;
		}
		return p;
	}

	else if (previousNode == -1 && previousNodeGain != -999999) {
		for (bE1 = tempBkt1.rbegin(); bE1 != tempBkt1.rend(); bE1++) {
			if ((bE1->first == previousNodeGain) && (((bE1->second).empty()))) {
				flag_bkt1 = 1;
				break;
			} else if ((bE1->first == previousNodeGain)
					&& (!((bE1->second).empty()))) {
				for (list<long>::iterator i = (bE1->second).begin();
						i != (bE1->second).end(); i++) {
					p.first = *i;
					p.second = bE1->first;
					return p;
				}
				break;
			}
		}
		for (bE2 = tempBkt2.rbegin(); bE2 != tempBkt2.rend(); bE2++) {
			if ((bE2->first == previousNodeGain) && (((bE2->second).empty()))) {
				flag_bkt2 = 1;
				break;
			} else if ((bE2->first == previousNodeGain)
					&& (!((bE2->second).empty()))) {
				for (list<long>::iterator i = (bE2->second).begin();
						i != (bE2->second).end(); i++) {
					p.first = *i;
					p.second = bE2->first;
					return p;
				}
				break;
			}
		}

		if (flag_bkt1 == 1 && flag_bkt2 == 1) {
			p = getOptimumNode(-1, previousNodeGain - 1);
			return p;
		}
	}

	else if (previousNode != -1 && previousNodeGain != -999999) {
		for (bE1 = tempBkt1.rbegin(); bE1 != tempBkt1.rend(); bE1++) {
			if ((bE1->first == previousNodeGain) && (((bE1->second).empty()))) {
				flag_bkt1 = 1;
				break;
			} else if ((bE1->first == previousNodeGain)
					&& (!((bE1->second).empty()))) {
				for (list<long>::iterator i = (bE1->second).begin();
						i != (bE1->second).end(); i++) {
					if (*i == previousNode) {
						i++;
						if (i != (bE1->second).end()) {
							p.first = *i;
							p.second = bE1->first;
							return p;
						} else if (i == (bE1->second).end()) {
							flag_bkt1 = 2;
							break;
						}
					}
					if ((bE1->first == previousNodeGain)) {
						flag_bkt1 = 3;
					}
				}
				break;
			}
		}
		for (bE2 = tempBkt2.rbegin(); bE2 != tempBkt2.rend(); bE2++) {
			if ((bE2->first == previousNodeGain) && (((bE2->second).empty()))) {
				flag_bkt2 = 1;
				break;
			} else if ((bE2->first == previousNodeGain)
					&& (!((bE2->second).empty()))) {
				for (list<long>::iterator i = (bE2->second).begin();
						i != (bE2->second).end(); i++) {
					if (*i == previousNode) {
						i++;
						if (i != (bE2->second).end()) {
							p.first = *i;
							p.second = bE2->first;
							return p;
						} else if (i == (bE2->second).end()) {
							flag_bkt2 = 2;
							break;
						}
					} else if (flag_bkt1 == 2) {
						p.first = *i;
						p.second = bE2->first;
						return p;
					}
				}
				break;
			}
		}

		if ((flag_bkt1 == 1 && flag_bkt2 == 1)
				|| (flag_bkt1 == 2 && flag_bkt2 == 1)
				|| (flag_bkt1 == 1 && flag_bkt2 == 2)
				|| (flag_bkt1 == 2 && flag_bkt2 == 2)
				|| (flag_bkt1 == 3 && flag_bkt2 == 2)) {
			p = getOptimumNode(-1, previousNodeGain - 1);
			return p;
		}
	}
}

//to get the node with the highest gain or the second highest ....etc...
/*pair<long, long> algo::getOptimumNode(long previousNode, long previousNodeGain)
{
	map<long,list<long>>::iterator bE1 = bucket1.begin();
	map<long,list<long>>::iterator bE2 = bucket2.begin();

	pair<long,long> ghg = getHighestGainFromBucket(previousNodeGain);

//	cout<<"\n Highest gain = "<<ghg.first<<" with value:"<<ghg.second;

	pair<long, long> p;

  label1:
	if(ghg.second == 1)
	{
		bE1 = bucket1.begin();
		while(bE1!=bucket1.end())
		{
			if(bE1->first == ghg.first)
			{
				for(list<long>::iterator it = (bE1->second).begin(); it != (bE1->second).end(); it++)
				{
					if(previousNode != -1 && previousNode != *it)
					{
						p.first = *it;
						p.second = bE1->first;
						return p;
					}
					else if(previousNode == -1)
					{
						p.first = *it;
						p.second = bE1->first;
//						optimumGainNode = *it;
						return p;
					}
					else if(previousNode == *it)
					{
						it++;
						if(it != (bE1->second).end()){
//							optimumGainNode = *it;
							p.first = *it;
							p.second = bE1->first;
							return p;
						}
						else
						{
							ghg = getHighestGainFromBucket(ghg.second);
							goto label1;
						}
					}
				}
			}
			bE1++;
		}
	}
	else if(ghg.second == 2)
	{
		bE2 = bucket2.begin();
		while(bE2!=bucket2.end())
		{
			if(bE2->first == ghg.first)
			{
				for(list<long>::iterator it = (bE2->second).begin(); it != (bE2->second).end(); it++)
				{
					if(previousNode != -1 && previousNode != *it)
					{
						p.first = *it;
						p.second = bE1->first;
						return p;
					}
					else if(previousNode == -1)
					{
//						optimumGainNode = *it;
						p.first = *it;
						p.second = bE2->first;
						return p;
					}
					else if(previousNode == *it)
					{
						it++;
						if(it != (bE2->second).end()){
//							optimumGainNode = *it;
							p.first = *it;
							p.second = bE2->first;
							return p;
						}
						else
						{
							ghg = getHighestGainFromBucket(ghg.second);
							goto label1;
						}
					}
				}
				break;
			}
			bE2++;
		}
	}
	else if(ghg.second == 3)
	{
		bE1 = bucket1.begin();
		bE2 = bucket2.begin();
		while(bE1!=bucket1.end())
		{
			if(bE1->first == ghg.first)
			{
				for(list<long>::iterator it = (bE1->second).begin(); it != (bE1->second).end(); it++)
				{
					if(previousNode != -1 && previousNode != *it)
					{
						p.first = *it;
						p.second = bE1->first;
						return p;
					}
					else if(previousNode == -1)
					{
//						optimumGainNode = *it;
						p.first = *it;
						p.second = bE1->first;
						return p;
					}
					else if(previousNode == *it)
					{
						it++;
						if(it != (bE1->second).end())
						{
//							optimumGainNode = *it;
							p.first = *it;
							p.second = bE1->first;
							return p;
						}
						else
						{
							break;
						}
					}
				}
				break;
			}
			bE1++;
		}
		while(bE2!=bucket1.end())
		{
			if(bE2->first == ghg.first)
			{
				for(list<long>::iterator it = (bE2->second).begin(); it != (bE2->second).end(); it++)
				{
					if(previousNode != -1 && previousNode != *it)
					{
						p.first = *it;
						p.second = bE1->first;
						return p;
					}
					else if(previousNode == -1)
					{
//						optimumGainNode = *it;
						p.first = *it;
						p.second = bE2->first;
						return p;
					}
					else if(previousNode == *it)
					{
						it++;
						if(it != (bE2->second).end())
						{
//							optimumGainNode = *it;
							p.first = *it;
							p.second = bE2->first;
							return p;
						}
						else
						{
							ghg = getHighestGainFromBucket(ghg.second);
							goto label1;
						}
					}
				}
				break;
			}
			bE2++;
		}
	}
	else{
		p.first = -1;
		p.second = -999999;
	}
	return p;
}

pair<long,long> algo::getHighestGainFromBucket(long previousHigh)
{
	long highestGainFromBucket1=-999999,highestGainFromBucket2=-999999;
	pair<long,long> p;
		map<long,list<long>>::iterator bE1 = bucket1.begin();
		map<long,list<long>>::iterator bE2 = bucket2.begin();
		while(bE1!=bucket1.end())
		{
			if(highestGainFromBucket1==-999999)
			{
				highestGainFromBucket1 = bE1->first;
			}
			else if((bE1->first > highestGainFromBucket1) && (!((bE1->second).empty())) && (previousHigh==-999999))
			{
				highestGainFromBucket1 = bE1->first;
			}
			else if((bE1->first > highestGainFromBucket1) && (!((bE1->second).empty())) && (bE1->first < previousHigh))
			{
				highestGainFromBucket1 = bE1->first;
			}
			bE1++;
		}
		while(bE2!=bucket2.end())
		{
			if(highestGainFromBucket2==-999999)
			{
				highestGainFromBucket2 = bE2->first;
			}
			else if((bE2->first > highestGainFromBucket2) && (!((bE2->second).empty())) && (previousHigh==-999999))
			{
				highestGainFromBucket2 = bE2->first;
			}
			else if((bE1->first > highestGainFromBucket2) && (!((bE2->second).empty())) && (bE2->first < previousHigh))
			{
				highestGainFromBucket2 = bE2->first;
			}
			bE2++;
		}
		if(highestGainFromBucket1>highestGainFromBucket2)
		{
			p.first = highestGainFromBucket1;
			p.second = 1;
			return p;
		}
		else if(highestGainFromBucket1<highestGainFromBucket2)
		{
			p.first = highestGainFromBucket2;
			p.second = 2;
			return p;
		}
		else
		{
			p.first = highestGainFromBucket1;
			p.second = 3;
			return p;
		}
}*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {

	algo a;

	a.parseFile();
	list<node> nodesInfo = a.getNodeInfo();
	vector<list<long> > adjList = a.getAdjList();
	map<long, int> count = a.getCount();

	int maxNodeCount = a.getMaxNodeCount();

	a.makeBucketSets(maxNodeCount);

	/*cout<<"\nAwesome Bucket list 1:\n";
	for(std::map<long, list<long> >::iterator it = bucket1.begin(); it != bucket1.end(); ++it){
		std::cout<<"** "<<it->first<<it->second.size();
	}

	cout<<"\nHashMap:\n";

	for (list<node>::iterator iter = nodesInfo.begin(); iter != nodesInfo.end(); iter++)
	{
    	std::cout << "nodesInfo[" << iter->key << "] = " << iter->nodeName << " area: " <<iter->nodeArea<< endl;
	}

	cout<<"\nAdjacency list of Nets:\n";
	for(std::vector< list<long> >::iterator it = adjList.begin(); it != adjList.end(); ++it){
		for (std::list<long>::iterator it1 = it->begin(); it1 != it->end(); ++it1){
			std::cout << "->" << *it1 <<" ";
		}
		std::cout<<endl;
	}

	cout<<"\nVector count:\n";
	for (std::map<long,int>::iterator iter = count.begin(); iter != count.end(); iter++)
	{
    	std::cout << "count[" << iter->first << "] = " << iter->second << endl;
	}

	cout<<"Max count:"<<maxNodeCount<<endl;

	cout<<"\nnumber of nodes : "<<nodesInfo.size();
	cout<<"\nnumber of nets : "<<adjList.size();*/

	a.makeInitialPartitions(nodesInfo);
	a.countCutsetSize(adjList);

	cout<<"\n-----------------------------------------------------------------------------------------------------------------------------------------------------";

	long cutSetSize1 = a.getCutSetSizeCount();
	cout << "\n\nThe initial cutset size is : " << cutSetSize1;

	vector<long> partition1 = a.getPartition1();
	vector<long> partition2 = a.getPartition2();
	cout<<"\n\nInitial Nodes of Partition 1:\n";
	for(vector<long>::iterator t=partition1.begin(); t!=partition1.end(); t++)
	{
		cout<<*t<<"  ";
	}
	cout<<"\n\nInitial Nodes of Partition 2:\n";
	for(vector<long>::iterator t=partition2.begin(); t!=partition2.end(); t++)
	{
		cout<<*t<<"  ";
	}

	a.updateBucketGains();

	map<long, list<long> > bucket1 = a.getBucket1();
	map<long, list<long> > bucket2 = a.getBucket2();

	cout << "\n\nBucket list 1:";
	for (std::map<long, list<long> >::iterator it = bucket1.begin();
			it != bucket1.end(); ++it) {
		std::cout << "\nGain " << it->first << " Nodes: ";
		for (std::list<long>::iterator it1 = it->second.begin();
				it1 != it->second.end(); ++it1) {
			std::cout << "-->" << *it1;
		}
	}

	cout << "\n\nBucket list 2:";
	for (std::map<long, list<long> >::iterator it = bucket2.begin();
			it != bucket2.end(); ++it) {
		std::cout << "\nGain " << it->first << " Nodes: ";
		for (std::list<long>::iterator it1 = it->second.begin();
				it1 != it->second.end(); ++it1) {
			std::cout << "-->" << *it1;
		}
	}/*

	pair<long, long> p = a.getOptimumNode(-1,-999999);
	cout<<"\nOptimum Gain node: "<<p.first<<" with gain: "<<p.second;

	 p = a.getOptimumNode(p.first, p.second);
	cout<<"\nOptimum Gain node: "<<p.first<<" with gain: "<<p.second;

	p = a.getOptimumNode(p.first, p.second);
	cout<<"\nOptimum Gain node: "<<p.first<<" with gain: "<<p.second;

	p = a.getOptimumNode(p.first, p.second);
	cout<<"\nOptimum Gain node: "<<p.first<<" with gain: "<<p.second;

	p = a.getOptimumNode(p.first, p.second);
		cout<<"\nOptimum Gain node: "<<p.first<<" with gain: "<<p.second;

	p = a.getOptimumNode(p.first, p.second);
	cout<<"\nOptimum Gain node: "<<p.first<<" with gain: "<<p.second;

	p = a.getOptimumNode(p.first, p.second);
	cout<<"\nOptimum Gain node: "<<p.first<<" with gain: "<<p.second;

	p = a.getOptimumNode(p.first, p.second);
	cout<<"\nOptimum Gain node: "<<p.first<<" with gain: "<<p.second;

	p = a.getOptimumNode(p.first, p.second);
	cout<<"\nOptimum Gain node: "<<p.first<<" with gain: "<<p.second;*/


	long cutSetSize2 = 0;
	int k=0;
	while (a.lockedNodes.size()<a.numPadOff) {
		if(k==0){
			k++;
		}
		else{
			cutSetSize1 = cutSetSize2;
		}
		cout<<"\n-----------------------------------------------------------------------------------------------------------------------------------------------------";
		a.shiftNode();
//		a.updateBucketGains();
		a.countCutsetSize(adjList);
		cutSetSize2 = a.getCutSetSizeCount();
		cout << "\n\nThe new cutset size is : " << cutSetSize2;
		cout << "\nIterations remaining in current pass: "<<(a.numPadOff - a.lockedNodes.size());

		vector<long> partition1 = a.getPartition1();
		vector<long> partition2 = a.getPartition2();
		cout<<"\n\nNodes in Partition 1 during "<<a.lockedNodes.size()<<" iteration:\n";
		for(vector<long>::iterator t=partition1.begin(); t!=partition1.end(); t++)
		{
			cout<<*t<<"  ";
		}
		cout<<"\n\nNodes in Partition 2 during "<<a.lockedNodes.size()<<" iteration:\n";
		for(vector<long>::iterator t=partition2.begin(); t!=partition2.end(); t++)
		{
			cout<<*t<<"  ";
		}

		bucket1 = a.getBucket1();
		bucket2 = a.getBucket2();

		cout<<"\n\nBucket list 1 during "<<a.lockedNodes.size()<<" iteration:\n";
		for(std::map<long, list<long> >::iterator it = bucket1.begin(); it != bucket1.end(); ++it)
		{
			std::cout<<"\nGain "<<it->first<<" Nodes: ";
			for(std::list<long>::iterator it1 = it->second.begin(); it1 != it->second.end(); ++it1)
			{
				std::cout<<"-->"<<*it1;
			}
		}

		cout<<"\n\nBucket list 2 during "<<a.lockedNodes.size()<<" iteration:\n";
		for(std::map<long, list<long> >::iterator it = bucket2.begin(); it != bucket2.end(); ++it)
		{
			std::cout<<"\nGain "<<it->first<<" Nodes: ";
			for(std::list<long>::iterator it1 = it->second.begin(); it1 != it->second.end(); ++it1)
			{
				std::cout<<"-->"<<*it1;
			}
		}
	}

	return 0;
}

//to get the node with the highest gain or the second highest ....etc...
/*long algo::getOptimumNode(long previousNode)
{
	cout<<"--------------------------------------------------1----------------------------------------------------------------------"<<endl;

	long optimumGainNode=-9999,highestGainFromBucket1,highestGainFromBucket2;
	if(previousNode==-1)
	{
		cout<<"-------------------------------------------------------2-----------------------------------------------------------------"<<endl;
		highestGainFromBucket1 = bucket1.begin()->first;
		cout<<"first highest gain"<<highestGainFromBucket1<<endl;
		for(std::pair<long,list<long>> it1:bucket1)
		{
			if(!(it1.second.empty()) && highestGainFromBucket1<it1.first)
		{
		optimumGainNode = it1.second.front();
		highestGainFromBucket1=it1.first;
	}
	}
	for(std::pair<long,list<long>> it2:bucket2)
	{
 		if(!it2.second.empty() && highestGainFromBucket1 < it2.first)
	{
		optimumGainNode = it2.second.front();
		break;
	}
	}
	cout<<"-------------------------------------------3-----------------------------------------------------------------------------"<<optimumGainNode<<endl;

	}
	else
	{
		std::map<long,list<long>>::iterator it1,it2;
		for (it1=bucket1.begin(), it2=bucket2.begin();it1 != bucket1.end()  && it2 != bucket2.end() ;++it1, ++it2)
		{
			std::map<long, list<long> >::iterator foundInBucket =it1;
			std::map<long, list<long> >::iterator otherBucket = it2;

			std::list<long>::iterator nodeFoundPosition = std::find(it1->second.begin(),it1->second.end(),previousNode);
			if(nodeFoundPosition == it1->second.end())
			{
				nodeFoundPosition = std::find(it2->second.begin(),it2->second.end(),previousNode);
				foundInBucket = it2;
				otherBucket = it1;
			}
			std::list<long>::iterator *p=&nodeFoundPosition;
			while(optimumGainNode==-9999)
			{
				cout<<"opt gain node"<<optimumGainNode;
				optimumGainNode=getNode( foundInBucket, otherBucket,p);
				foundInBucket=--foundInBucket;
				otherBucket=--otherBucket;
				p=NULL;
				while(foundInBucket->second.empty() && otherBucket->second.empty())
				{
					foundInBucket=--foundInBucket;
					otherBucket=--otherBucket;
				}
			}
		}
		cout<<"-------------------------------------------new opt-----------------------------------------------------------------------------"<<optimumGainNode<<endl;
	}
	return optimumGainNode;
}

long algo::getNode(std::map<long, list<long> >::iterator foundInBucket, std::map<long, list<long> >::iterator otherBucket,std::list<long>::iterator *nodeFoundPosition=NULL)
{
	cout<<"----------node found position is : ---------"<<*(*nodeFoundPosition)<<endl;
	long optimumGainNode=-9999;
//	if(!foundInBucket.second.empty() && (nodeFoundPosition==foundInBucket.second.begin()))
//	{
//	optimumGainNode = *nodeFoundPosition;
//	}
	if((nodeFoundPosition==NULL))
	{
		if(!foundInBucket->second.empty())
		{
			optimumGainNode = foundInBucket->second.front();
		}
		else
		optimumGainNode = otherBucket->second.front();
	}
	else if( !foundInBucket->second.empty() && ++(*nodeFoundPosition) != foundInBucket->second.end() )
	{
	optimumGainNode = *(*nodeFoundPosition);
	}
	else if(!otherBucket->second.empty())
	{
	optimumGainNode = otherBucket->second.front();
	}
	return optimumGainNode;
}*/
