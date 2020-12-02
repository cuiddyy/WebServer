#ifndef PROTO_H__
#define PROTO_H__

#define KEYPATH "/tmp/out"
#define KEYPROJ 'a'

#define PATHSIZE 1024
#define DATAPATH 1024

enum {

	MSG_PATH = 1;
	MSG_DATA,
	MSG_EOT
};

typedef struct msg_path_st{
	long mtype;
	char path[PATHMAX]; //ASCIIZ带尾0的串

}msg_path_t;

typedef struct msg_s2c_st{
	long mtype;
	int datalen;
	/**
	 *datalen > 0  :data
	 *       ==0   :eot
	 */
	char data[DATAMAX];
	int datalen;
}msg_data_t;



#endif
