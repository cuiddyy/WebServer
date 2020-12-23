#include <iostream>
#include <memory>

using namespace std;
class NewRequest;

class Timer{

public:
	bool isDel = false;
	shared_ptr<NewRequest>nPtr_;
	~Timer(){
		cout << " Timer is destructed " << endl;
	}
};

class NewRequest{
public:
	weak_ptr<Timer>tPtr_;
	~NewRequest(){
		cout << " NewRequest is destructed " << endl;
	}
};

int main(){
	shared_ptr<NewRequest>nptr(new NewRequest());
	shared_ptr<Timer>tptr(new Timer());
	nptr -> tPtr_ = tptr;
	tptr -> nPtr_ = nptr;
	cout << "nptr -> tptr_ ->isDel= " << (nptr->tPtr_.lock()->isDel) << endl;

	nptr -> tPtr_.lock() -> isDel = true;
	cout << "nptr -> tptr_ ->isDel= " << (nptr->tPtr_.lock()->isDel) << endl;

	
	return 0;

}
