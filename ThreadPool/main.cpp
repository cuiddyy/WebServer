#include <iostream>
#include "ThreadPool.h"
using namespace std;

void cnt_1(){
	int n=5000;
	cout << "cnt_1 starts" << endl;
	while(n--){
		cout << "n=" << n ;
	}
	cout << "cnt_1 ends" << endl;

}
void cnt_2(){
	int n=500;
	cout << "cnt_2 starts" << endl;
	while(n--);
	cout << "cnt_2 ends" << endl;

}
void cnt_3(){
	int n=500;
	cout << "cnt_3 starts" << endl;
	while(n--);
	cout << "cnt_3 ends" << endl;

}
void cnt_4(){
	int n=500;
	cout << "cnt_4 starts" << endl;
	while(n--);
	cout << "cnt_4 ends" << endl;

}

int main(){
	
	//Timestamp end;
	//Timestamp start = Timestamp.nonw();
	//cout << "开始时间为:" << start.toString() << endl;
	{
		ThreadPool pool(3);
		try{
			pool.start();
		}catch(std::exception e){
			throw e;
			cout << "constructions fails" << endl;
		}

		pool.addTask(move(cnt_1));
		pool.addTask(move(cnt_2));
		pool.addTask(move(cnt_3));
		pool.addTask(move(cnt_4));
		//getchar();
		//end = Timestamp.now();
		//cout << "结束时间为:" << end.toString() << endl;
		//cout << "程序运行时间为:" << totaltime << "秒" << endl;
		getchar();
	}
	getchar();

	return 0;
}
