#ifndef PROTO_H__
#define PROTO_H__



#define RCVPORT "1989"
#define NAMEMAX (512-8-8)   //8:udp报头，8:两个uint32_t

struct msg_st{
	uint32_t math;
	uint32_t chinese;
	uint8_t name[1];
}__attribute__((packed));





#endif
