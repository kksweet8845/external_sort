#include<iostream>
#include<fstream>
#include<cstdlib>
using namespace std;

int main(){
    ofstream outFile("input100w.txt",ios::out);
    int num = 0;
    int mode = 0;
    long int i;
    for(i=0;i<1000000;i++){
        mode=(rand()%2);
        if(mode==1){
            num = rand();
            outFile<<num<<"\n";
        }else{
            num = rand();   //neg
            outFile<<"-"<<num+1<<"\n";
        }
    }
}
