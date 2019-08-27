package com.perfxlab.drug_client;

import android.util.Log;

public class DrugDetectCliManager {
    public static DrugDetectCliManager instance;

    static {
        System.loadLibrary("drug_box_detect");
    }

    private DrugDetectCliManager() {

    }

    public static DrugDetectCliManager getInstance() {
        if (instance == null) {
            synchronized (DrugDetectCliManager.class) {
                if (instance == null) {
                    instance = new DrugDetectCliManager();
                }
            }
        }
        return instance;
    }

    public native void init(String ip);

    /**
     * @param
     */
    public native void sendMsg(String drugName);


    public native void close();

    /**
     * @param message 消息文本
     * @param msgType 消息类型
     */
    public void detectResult(String message, int msgType) {
        if (msgType == 0) {
            //检测到结果
            //这里不要有任何阻塞。
            Log.i("LOG_TAG","Message == "+message);
            if(MainActivity.mainActivity!=null){
                MainActivity.mainActivity.setText(System.currentTimeMillis()+">> " + message);
            }
        } else {
            //其它错误

        }
    }


    public void destroy() {
        close();
        instance = null;
    }
}
