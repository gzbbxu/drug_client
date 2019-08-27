//
// Created by yuanshuai on 2019/5/17.
//

#include <endian.h>
#include "MsgManager.h"
#include "commsocket.h"
#include <jni.h>
#define HERT_JUMP 30
void *MsgManager::handle = nullptr;
int  MsgManager::connfd = 0;
MsgManager *MsgManager::iNstance = NULL;
long MsgManager::timeSecond = 0;
const char *MsgManager::mIp = nullptr;
SyncQueue<unsigned char *> *MsgManager::sendQueue = nullptr;
SyncQueue<unsigned char *> *MsgManager::receiveQueue = nullptr;
int MsgManager::perLen = sizeof(int32_t) * 2;
bool MsgManager::isRunFlag = true;
extern jobject drugDetectCli;
extern jclass  managerClass;
extern jmethodID  detectResultMethod;
extern  JavaVM *jvm;
void MsgManager::send(unsigned char *pstr) {
    int len = strlen(reinterpret_cast<const char *>(pstr));
    char * mallocStr = new char[len+1];
    memset(mallocStr,0,len+1);
    memcpy(mallocStr,pstr,len+1);
    ALOGE("发送数据 send >> %s,%d  ",pstr,(len+1));
    if((sendQueue->PutResult(reinterpret_cast<unsigned char *const &>(mallocStr)) )==-1){
        free(mallocStr);
        mallocStr = nullptr;
    }

}

void MsgManager::receive(unsigned char *pstr) {
//    receiveQueue->Put(pstr);
}


MsgManager::MsgManager(const char *pIp) {
    iNstance = this;
    isRunFlag = true;
    mIp = pIp;
    int len = 512;
    mIp=  new char[len];
    memset((void *) mIp, 0, len);
    memcpy((void *)mIp,pIp,strlen(pIp));

    ALOGE("MsgManager  IP =  %s ,%d",mIp,sizeof(mIp));
    sendQueue = new SyncQueue<unsigned char *>(100);
//    receiveQueue = new SyncQueue<unsigned char *>(100);


    pthread_t sendid;
    pthread_create(&sendid, NULL, sendThread, NULL);
    hertJumpCheck();

}

void timer(int sig) {
    if (SIGALRM == sig) {
//        printf("timer\n");
        ALOGI("timer ");
        timespec time;
        clock_gettime(CLOCK_REALTIME, &time);
        long time_sec = time.tv_sec;
        if (time_sec - MsgManager::timeSecond >= HERT_JUMP) {
            //距离上一次发送成功时间大于30s
            //发送心跳包。并重新计时

            char temp[5] = {"ping"};
            std::string str(temp);
            if (MsgManager::iNstance != nullptr) {
                ALOGI("发送心跳包");
                MsgManager::iNstance->send(reinterpret_cast<unsigned char *>(temp));
            }

//            WriteService::writeString(temp,sizeof(temp),hertWrite);
        }
        alarm(HERT_JUMP);       //重新继续定时1s
    }
    return;
}

void MsgManager::hertJumpCheck() {
    signal(SIGALRM, timer); //注册安装信号
    alarm(HERT_JUMP);       //触发定时器
}
void MsgManager::setCurrentime() {
    timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    timeSecond = time.tv_sec;
}
void *MsgManager::sendThread(void *pVoid) {
    pthread_detach(pthread_self());
    int ret;
    ret = sckCliet_init(&handle, 15, 5, 1, 10);
    reconnect(ret);
    if(isRunFlag){
        pthread_t receiveid;
        pthread_create(&receiveid, NULL, receiveThread, NULL);
    }
    while (isRunFlag) {
        if (!sendQueue) {
            break;
        }
        unsigned char *_pstr = nullptr;
        sendQueue->Take(_pstr);
//        ALOGE("发送数据 Thead >>  %s " ,_pstr);

        if (_pstr) {
            int writeSize = strlen((char *) _pstr);
            ALOGE("发送数据 %s  ,%d", _pstr, writeSize+1);
            ret = sckClient_send(handle, connfd, _pstr, writeSize+1);
            if (ret == Sck_ErrTimeOut) {
                ALOGE("发送数据 错误5s Sck_ErrTimeOut后重试 ");

                free(_pstr);
                _pstr = nullptr;
                sleep(5);
                reconnect(ret);

                continue;
            }else if(ret ==-1){
                ALOGE("发送数据 失败-1 返回值 = %d ",ret);
                free(_pstr);
                _pstr = nullptr;
                sleep(5);
                reconnect(ret);
                continue;
            }
            ALOGE("发送数据 成功 返回值 = %d ",ret);
            free(_pstr);
            _pstr = nullptr;
            setCurrentime();
        }else{
            delete sendQueue;
            sendQueue = nullptr;
            ALOGE("发送数据 空值");
            break;
        }


    }
    ALOGE("sendThread closed ===========");
    pthread_exit(0);//pthread_exit时自动会被释放
    return NULL;
}

void MsgManager::reconnect(int ret) {


    while(isRunFlag){
        ALOGE(" reconnect ip = %s",mIp);
        ret = sckCliet_getconn(handle, mIp, 8001, &connfd);
        if(ret ==Sck_ErrTimeOut){
            ALOGE("连接超时5s 后，重連 %s , %d ,errno =%d",mIp,connfd,errno);
            sleep(5);
            continue;
        }
        ALOGE("reconnect 连接成功 %d",connfd);
        break;
    }
}

void thread_exit_handler(int sig)
{
    ALOGE("closed this signal is %d \n", sig);
    pthread_exit(0);
}

void *MsgManager::receiveThread(void *pVoid) {
    pthread_detach(pthread_self());

    JNIEnv *env = NULL;
    jint result = -1;
    if ((result = jvm->GetEnv((void **) &env, JNI_VERSION_1_4)) != JNI_OK) {
        ALOGI("get env failed %d ", result);
    }

    jvm->AttachCurrentThread(&env, NULL);

/*    struct sigaction actions;
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = thread_exit_handler;
    sigaction(SIGUSR1,&actions,NULL);
    ALOGE("注册新号成功");*/


    while (isRunFlag) {
      /*  if (!receiveQueue) {
            break;
        }*/
        unsigned char out[512] = {0};
        int outlen = 512;
        int ret;
        ret = sckClient_rev(handle, connfd, out, &outlen);
        ALOGE("读取结果 %d %d  %d ", ret,connfd,outlen);
        if (ret == -1) {
            ALOGE("读取结果 receiveThread 读取错误>> ");
            sleep(5);
            continue;
        } else if (ret == Sck_ErrTimeOut) {
            ALOGE("读取结果 receiveThread 读取超时>> %s ",out);
            continue;
        } else if (ret == Sck_ErrPeerClosed) {
            ALOGE("读取结果 receiveThread 读取错误 服务器关闭>> ");
            sleep(5);
            continue;
        }else if(ret ==0){

            if(strcmp("ping", reinterpret_cast<const char *>(out))!=0){
                ALOGE("读取结果 receiveThread client 读取到数据 >> %s ", out);
                jstring boolResult = env->NewStringUTF(reinterpret_cast<const char *>(out));
                env->CallVoidMethod(drugDetectCli,detectResultMethod,boolResult,0);
                env->DeleteLocalRef(boolResult);
            }else{
                ALOGE("读取结果 receiveThread client 读取到心跳包数据 >> %s ", out);
            }

        }
    }
    jvm->DetachCurrentThread();

    ALOGE("receiveThread closed ======  ");

    pthread_exit(0);//pthread_exit时自动会被释放
    return NULL;
}


MsgManager::~MsgManager() {
    alarm(0);
    isRunFlag = false;
    iNstance = nullptr;
    sendQueue->Put(0);

    if(mIp){
        free((void *) mIp);
        mIp = nullptr;
    }
    close(connfd);
    sckClient_destroy(handle);
    handle = nullptr;

}


