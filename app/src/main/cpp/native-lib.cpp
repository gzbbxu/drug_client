#include <jni.h>
#include <string>
#include <pthread.h>
#include <sys/wait.h>
#include "Alog_pri.h"
#include "commsocket.h"
#include "MsgManager.h"

#define LOG_TAG "LOG_TAG_native"
void *handle = NULL;

MsgManager *msgManager = nullptr;

jobject drugDetectCli;
jclass  managerClass;
jmethodID  detectResultMethod;
JavaVM *jvm;
void handleChild(int signum) {
    int pid = 0;
    ALOGE("recv signum:%d \n", signum);

    //避免僵尸进程
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        ALOGE("退出子进程pid%d \n", pid);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_perfxlab_drug_1client_DrugDetectCliManager_init(
        JNIEnv *env,
        jobject thiz, jstring ip) {

    const char *pIp = env->GetStringUTFChars(ip, NULL);
    ALOGE("ip== %s ", pIp);
    managerClass =  env->GetObjectClass(thiz);
    detectResultMethod =env->GetMethodID(managerClass,"detectResult","(Ljava/lang/String;I)V");
    drugDetectCli = env->NewGlobalRef(thiz);
//    signal(SIGCHLD, handleChild);
    signal(SIGPIPE, SIG_IGN);

    msgManager = new MsgManager(pIp);

    env->ReleaseStringUTFChars(ip, pIp);
}

extern "C" JNIEXPORT void JNICALL
Java_com_perfxlab_drug_1client_DrugDetectCliManager_close(
        JNIEnv *env,
        jobject thiz) {
    env->DeleteGlobalRef(drugDetectCli);
    delete  msgManager;
    msgManager = nullptr;
}

extern "C" JNIEXPORT void JNICALL
Java_com_perfxlab_drug_1client_DrugDetectCliManager_sendMsg(
        JNIEnv *env,
        jobject thiz, jstring msg) {
    const char *pMsg = env->GetStringUTFChars(msg, NULL);
    msgManager->send((unsigned char *) pMsg);
    env->ReleaseStringUTFChars(msg, pMsg);
}


jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    ALOGE("--------------------JNI_OnLoad------------------- >>  ");

    jvm = vm;
    JNIEnv *env = NULL;
    jint result = -1;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("--------------------JNI_OnLoad ERROR------------------- >>  ");
        goto bail;
    }

//    stringclass  = (env)->FindClass("Ljava/lang/String;");
    result = JNI_VERSION_1_4;
    bail:

    ALOGE("--------------------JNI_OnLoad end------------------- >>  ");
    return result;

}



