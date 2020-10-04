#include<iostream>
#include<cstdlib>
#include<fstream>
#include<vector>
#include<string>
#include<sstream>
#include<algorithm>
#include<exception>
#include<queue>
#include<errno.h>
using namespace std;

class compare{
    public:
        bool operator()(pair<int,int>p1, pair<int,int>p2) {return p1.first > p2.first;}
};


void merge(int amount){
    ofstream outFile("output.txt", ios::out);
    std::priority_queue<pair<int,int>, std::vector<pair<int,int> > , compare> minheap;
    int num = 0;
    ifstream* in = new ifstream[amount+1];
    int len = 0;
    for(int i=0; i<=amount ; i++){
        string sortfilename;
        sortfilename = "./tmp/tmppart" + to_string(i) + ".txt";
        in[i].open(sortfilename.c_str());
        in[i] >> num;
        minheap.push(pair<int,int>(num,i));
        printf("%d\n", len++);
    }

    printf("minheap size : %d\n", minheap.size());

    while(minheap.size()>0){
        pair<int, int> min = minheap.top();
        minheap.pop();
        printf("%d %ld %d\n", len++, minheap.size(), min.second);
        outFile << min.first <<'\n';
        // flush(outFile);
        in[0].clear();
        if(in[min.second]>>num) minheap.push(pair<int,int>(num,min.second));
        printf("fail: %d, bad: %d, eof: %d, error:%d\n", in[0].fail(), in[0].bad() == true ? 1 : 0, in[0].eof() == true ? 1 : 0, errno);
        printf("is_open: %d\n", in[0].is_open() == true ? 1 : 0);
        printf("%d\n", minheap.size());
    }
}

int main(){
    ifstream inFile("./data/0.txt", ios::in);
    vector<int>data; 
    string num;
    string nameofpartfile;
    int nametag = 0;
    int amount = 0;
    long totalnum;
    string tmp;
    while(inFile){
        if(amount==262144){
            sort(data.begin(),data.end());
            nameofpartfile = "./tmp/tmppart" + to_string(nametag) + ".txt";
            nametag++;
            ofstream outFile;
            outFile.open(nameofpartfile.c_str(),ios::out);
            for(int i=0;i<data.size();i++){
                outFile<<data.at(i)<<"\n";                
            }
            data.clear();
            amount = 0;
        }else{
            getline(inFile,num);
            totalnum++;
            try{
                data.push_back(stoi(num));
            }catch(exception&) {        
            }
            amount++;
        }
    }
    sort(data.begin(),data.end());
    nameofpartfile = "./tmp/tmppart" + to_string(nametag) + ".txt";
    ofstream outFile;
    outFile.open(nameofpartfile.c_str(),ios::out);
    for(int i=0;i<data.size();i++) outFile<<data.at(i)<<"\n";
    inFile.close();

    printf("nametag %d\n", nametag);
    merge(nametag);
}
