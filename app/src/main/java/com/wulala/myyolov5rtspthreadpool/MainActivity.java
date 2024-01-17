package com.wulala.myyolov5rtspthreadpool;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.widget.TextView;

import com.wulala.myyolov5rtspthreadpool.databinding.ActivityMainBinding;

import android.content.res.AssetManager;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("myyolov5rtspthreadpool");
    }

    private ActivityMainBinding binding;
    AssetManager assetManager;
    private long nativePlayerObj = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        assetManager = getAssets();
        setNativeAssetManager(assetManager);
        nativePlayerObj = prepareNative();
    }

    // jni native
    private native long prepareNative();

    private native void setNativeAssetManager(AssetManager assetManager);

}