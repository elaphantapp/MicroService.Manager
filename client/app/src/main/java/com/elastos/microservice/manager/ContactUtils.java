package com.elastos.microservice.manager;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.util.Log;

import org.elastos.sdk.elephantwallet.contact.Utils;
import org.elastos.sdk.keypair.ElastosKeypair;

import static com.elastos.microservice.manager.MainActivity.TAG;

public class ContactUtils {
    private static final String KeypairLanguage = "english";
    private static final String KeypairWords = "";
    private static final String SavedMnemonicKey = "mnemonic";

    private static Context mContext;
    private static String mSavedMnemonic;

    public static void Init(Context context) {
        mContext = context;
        SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(mContext);
        mSavedMnemonic = pref.getString(SavedMnemonicKey, null);
        if (mSavedMnemonic == null) {
            mSavedMnemonic = ElastosKeypair.generateMnemonic(KeypairLanguage, KeypairWords);
            NewAndSaveMnemonic(mSavedMnemonic);
        }
    }

    public static void NewAndSaveMnemonic(final String newMnemonic) {
        mSavedMnemonic = newMnemonic;
        if (mSavedMnemonic == null) {
            mSavedMnemonic = ElastosKeypair.generateMnemonic(KeypairLanguage, KeypairWords);
        }

        SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = pref.edit();
        editor.putString(SavedMnemonicKey, mSavedMnemonic).commit();
        Log.i(TAG,"Success to save mnemonic:\n" + mSavedMnemonic);
    }

    public static String GetDeviceId() {
        String devId = Settings.Secure.getString(mContext.getContentResolver(), Settings.Secure.ANDROID_ID);
        return devId;
    }

    public static byte[] GetAgentAuthHeader() {
        String appid = "org.elastos.debug.didplugin";
        String appkey = "b2gvzUM79yLhCbbGNWCuhSsGdqYhA7sS";
        long timestamp = System.currentTimeMillis();
        String auth = Utils.getMd5Sum(appkey + timestamp);
        String headerValue = "id=" + appid + ";time=" + timestamp + ";auth=" + auth;
        Log.i(TAG, "getAgentAuthHeader() headerValue=" + headerValue);

        return headerValue.getBytes();
    }

    public static byte[] SignData(byte[] data) {
        String privKey = GetPrivateKey();

        ElastosKeypair.Data originData = new ElastosKeypair.Data();
        originData.buf = data;
        ElastosKeypair.Data signedData = new ElastosKeypair.Data();
        int signedSize = ElastosKeypair.sign(privKey, originData, originData.buf.length, signedData);
        if (signedSize <= 0) {
            return null;
        }

        return signedData.buf;
    }

    public static String GetPublicKey() {
        ElastosKeypair.Data seedData = new ElastosKeypair.Data();
        int seedSize = ElastosKeypair.getSeedFromMnemonic(seedData, mSavedMnemonic,
                KeypairLanguage, KeypairWords, "");
        String pubKey = ElastosKeypair.getSinglePublicKey(seedData, seedSize);
        return pubKey;
    }

    public static String GetPrivateKey() {
        ElastosKeypair.Data seedData = new ElastosKeypair.Data();
        int seedSize = ElastosKeypair.getSeedFromMnemonic(seedData, mSavedMnemonic,
                KeypairLanguage, KeypairWords, "");
        String privKey = ElastosKeypair.getSinglePrivateKey(seedData, seedSize);
        return privKey;
    }

    public static String GetMnemonic() {
       return mSavedMnemonic;
    }
}
