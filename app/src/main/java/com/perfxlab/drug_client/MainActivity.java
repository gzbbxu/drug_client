package com.perfxlab.drug_client;

import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {
    Button set_ip;
    String drugNmae = "";
    TextView info;
    final DrugDetectCliManager instance = DrugDetectCliManager.getInstance();
    private Handler handler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            instance.sendMsg(drugNmae);

            handler.sendEmptyMessageDelayed(0,2000);
        }
    };
    public  static  MainActivity mainActivity;
    public void setText(final String mes){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                info.setText(mes);
            }
        });

    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        set_ip = findViewById(R.id.set_ip);
        mainActivity = this;

       final EditText tvip = findViewById(R.id.ip);
        info = findViewById(R.id.info);

        // Example of a call to a native method
        final EditText tv = findViewById(R.id.sample_text);
        final Button btn_send = findViewById(R.id.btn_send);
        btn_send.setEnabled(false);
        btn_send.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String  text = tv.getText().toString();
                if(!TextUtils.isEmpty(text)){
                    drugNmae = text;
                    handler.sendEmptyMessage(0);
//                    instance.sendMsg(drugNmae);
                }
            }
        });

        set_ip.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(TextUtils.isEmpty(tvip.getText().toString().trim())){
                    return;
                }
                set_ip.setEnabled(false);
                instance.init(tvip.getText().toString().trim());
                btn_send.setEnabled(true);
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mainActivity =null;
        Log.i("LOG_TAG","onDestroy");
        DrugDetectCliManager.getInstance().close();
    }
}
