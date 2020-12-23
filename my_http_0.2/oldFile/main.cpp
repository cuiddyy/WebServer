#include <stdio.h>
#include <stdlib.h>


#include <iostream>
#include <memory>
#include <unordered_set>
#include <functional>



using namespace std;



class Node{
public:
	Node()
	{}
	~Node(){}

	
};
class Root{
public:
	Root(){}
	~Root(){}	
};
using NodePtr = std::shared_ptr<Node>;
using RootPtr = std::shared_ptr<Root>;

void func1(const NodePtr &np){
	cout << "func1__coutns= " << np.use_count() << endl;
}

void func2(const NodePtr np){
	cout << "func2__counts= " <<np.use_count() << endl;
}
class Tree{
public:
	Tree(){}
	~Tree(){}
	void callBack(RootPtr rootP){
		cout << "Tree->Root" << endl;
	}
};

int main(){
	//NodePtr nodePt(new Node());
	//cout << "main__counts= " << nodePt.use_count() <<endl;
    //func1(nodePt);
	//func2(nodePt);	
	//std::cout<<"size="<<rootPt.use_count()<<std::endl;
	//cout << "size()=" <<rootPt.use_count() << endl;
	NodePtr np1(new Node());
	cout << "np1的引用为=" << np1.use_count() <<endl;
	{
		NodePtr np2(np1);
		
		cout << "np1的引用为=" << np1.use_count() <<endl;
	}

	cout << "np1的引用为=" << np1.use_count() <<endl;
	
	return 0;
}
