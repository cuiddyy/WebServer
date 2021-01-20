#ifndef BASEPOLL__H__
#define BASEPOLL__H__

#include <functional>
#include <memory>
#include <vector>

class BasePoll{
public:
	using NewRequestCallback = std::function<void()>;
	using CloseRequestCallback = std::function<void(int)>;
	using HandleRequestCallback = std::function<void(int)>;
	using HandleResponseCallback = std::function<void(int)>;
	NewRequestCallback onNewReuquest_;
	CloseRequestCallback onCloseRequest_;
	HandleRequestCallback onHandleRequest_;
	HandleResponseCallback onHandleResponse_;

	BasePoll();
	virtual ~BasePoll();
	void setNewRequestCallback(const NewRequestCallback& cb){onNewReuquest_ = cb;}
	void setCloseRequestCallback(const CloseRequestCallback& cb){onCloseRequest_ = cb;}
	void setHandleRequestCallback(const HandleResponseCallback& cb){onHandleRequest_ = cb;}
	void setHandleResponseCallback(const HandleResponseCallback& cb){onHandleResponse_ = cb;}

private:
};



#endif
