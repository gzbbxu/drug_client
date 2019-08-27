//
// Created by yuanshuai on 2019/5/17.
//

#ifndef DRUG_CLIENT_SENDMANAGER_H
#define DRUG_CLIENT_SENDMANAGER_H


#include "SyncQueue.hpp"
#include <string>

class MsgManager {
public:
    static SyncQueue<unsigned  char *> *sendQueue; //发送队列

    static SyncQueue<unsigned  char *> *receiveQueue;//接收队列

    static long timeSecond;
    static MsgManager *iNstance;
    static void setCurrentime();
    static int perLen;

    MsgManager( const char * pIp);
    //心跳检测
    void hertJumpCheck();
    static void *handle;
    static int connfd;
    static const char *mIp;
    static bool isRunFlag ;

    void send(unsigned char *pstr);

    void receive(unsigned char *pstr);

    ~MsgManager();

    static void *sendThread(void *pVoid);

    static void *receiveThread(void *pVoid);

    static void reconnect(int ret);

};


#endif //DRUG_CLIENT_SENDMANAGER_H
