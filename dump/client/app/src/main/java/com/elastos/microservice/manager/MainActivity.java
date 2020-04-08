package com.elastos.microservice.manager;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Process;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import org.elastos.sdk.elephantwallet.contact.Contact;

import java.util.List;


import app.elaphant.sdk.peernode.Connector;
import app.elaphant.sdk.peernode.PeerNode;
import app.elaphant.sdk.peernode.PeerNodeListener;

public class MainActivity extends Activity {
    public static final String TAG = "MicroService.Client";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ContactUtils.Init(this);
        Log.i(TAG, "Device ID:" + ContactUtils.GetDeviceId());
        showMessage("Mnemonic:\n" + ContactUtils.GetMnemonic());
    }

    @Override
    protected void onStart() {
        super.onStart();

        int ret = startPeerNode();
        if(ret < 0) {
            showError("Failed to start PeerNode. errcode=" + ret);
        }

        createConnector();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        Helper.onRequestPermissionsResult(this, requestCode, permissions, grantResults);
    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Helper.onActivityResult(this, requestCode, resultCode, data);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mPeerNode.stop();

        Process.killProcess(Process.myPid());
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    public void clearEvent(MenuItem item){
        TextView txtEvent = findViewById(R.id.txt_event);
        txtEvent.setText("");
        txtEvent.invalidate();
    }

    public void renewMnemonic(MenuItem item){
        ContactUtils.NewAndSaveMnemonic(null);
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("New mnemonic can only be valid after restart,\ndo you want restart app?");
        builder.setPositiveButton("Restart", (dialog, which) -> {
            // restart
            Intent mStartActivity = new Intent(this, MainActivity.class);
            int mPendingIntentId = 123456;
            PendingIntent mPendingIntent = PendingIntent.getActivity(this, mPendingIntentId, mStartActivity, PendingIntent.FLAG_CANCEL_CURRENT);
            AlarmManager mgr = (AlarmManager) this.getSystemService(Context.ALARM_SERVICE);
            mgr.set(AlarmManager.RTC, System.currentTimeMillis() + 100, mPendingIntent);
            Process.killProcess(Process.myPid());
        });
        builder.setNegativeButton("Cancel", (dialog, which) -> {
            dialog.dismiss();
        });
        runOnUiThread(() -> {
            builder.create().show();
        });
    }

    public void showUserInfo(MenuItem item){
        Contact.UserInfo userInfo = mManagerConnector.getUserInfo();
        Helper.showDetails(this, userInfo.toJson());
    }

    public void connectToServer(MenuItem item){
        Helper.scanAddress(this, friendCode -> {
            showMessage("Add address:" + friendCode);
            int ret = mManagerConnector.addFriend(friendCode, "hello");
            if (ret < 0) {
                showError("Failed to add friend. ret=" + ret);
            }
            showMessage("Success to send request to friend:" + friendCode);
        });
    }

    private int startPeerNode() {
        if (mPeerNode != null) {
            return 0;
        }

        mPeerNode = PeerNode.getInstance(getFilesDir().getAbsolutePath(), ContactUtils.GetDeviceId());
        mPeerNode.setListener(new PeerNodeListener.Listener() {
            @Override
            public byte[] onAcquire(Contact.Listener.AcquireArgs request) {
                byte[] response = null;
                switch (request.type) {
                    case PublicKey:
                        response = ContactUtils.GetPublicKey().getBytes();
                        break;
                    case EncryptData:
                        response = request.data;
                        break;
                    case DecryptData:
                        response = request.data;
                        break;
                    case DidPropAppId:
                        break;
                    case DidAgentAuthHeader:
                        response = ContactUtils.GetAgentAuthHeader();
                        break;
                    case SignData:
                        response = ContactUtils.SignData(request.data);
                        break;
                    default:
                        throw new RuntimeException("Unprocessed request: " + request);
                }
                return response;
            }

            @Override
            public void onError(int errCode, String errStr, String ext) {
                showError("PeerNode Error: " + errCode + " " + errStr);
            }
        });

        int ret = mPeerNode.start();

        return ret;
    }

    private void createConnector() {
        if (mManagerConnector != null) {
            return;
        }

        mManagerConnector = new Connector("ManagerService");
        mManagerConnector.setMessageListener(new PeerNodeListener.MessageListener() {
            @Override
            public void onEvent(Contact.Listener.EventArgs event) {
                processEvent(event);
            }

            @Override
            public void onReceivedMessage(String humanCode, Contact.Channel channelType, Contact.Message message) {
                String msg = "Receive message from " + humanCode + " " + message.data.toString();
                showMessage(msg);
            }
        });
    }

    private boolean sendMessage() {
        if (mManagerConnector == null) {
            Toast.makeText(this, "please create connector first!", Toast.LENGTH_LONG).show();
            return false;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Select a friend");
        builder.setNegativeButton("cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        });

        List<String> friends = mManagerConnector.listFriendCode();
        final ArrayAdapter<String> arrayAdapter = new ArrayAdapter<String>(this, android.R.layout.select_dialog_singlechoice);
        assert friends != null;
        arrayAdapter.addAll(friends);
        builder.setAdapter(arrayAdapter, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                send(arrayAdapter.getItem(which));
                dialog.dismiss();
            }
        });
        builder.create().show();

        return false;
    }

    private void send(String friendCode) {
        final EditText edit = new EditText(this);
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Input message");

        builder.setView(edit);
        builder.setNegativeButton("cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        });
        builder.setPositiveButton("ok", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                String message = edit.getText().toString().trim();
                if (message.isEmpty()) return;

                mManagerConnector.sendMessage(friendCode, message);
            }
        });
        builder.create().show();
    }

    private void processEvent(Contact.Listener.EventArgs event) {
        String text = "";
        switch (event.type) {
            case FriendRequest:
                Contact.Listener.RequestEvent requestEvent = (Contact.Listener.RequestEvent) event;
                String summary = requestEvent.summary;
                text = requestEvent.humanCode + " request friend, said: " + summary;
                mManagerConnector.acceptFriend(requestEvent.humanCode);
                break;
            case StatusChanged:
                Contact.Listener.StatusEvent statusEvent = (Contact.Listener.StatusEvent)event;
                text = statusEvent.humanCode + " status changed " + statusEvent.status;
                break;
            case HumanInfoChanged:
                Contact.Listener.InfoEvent infoEvent = (Contact.Listener.InfoEvent) event;
                text = event.humanCode + " info changed: " + infoEvent.toString();
                break;
            default:
                Log.w(TAG, "Unprocessed event: " + event);
                return;
        }
        showEvent(text);
    }

    private void showError(String text) {
        runOnUiThread(() -> {
            Log.e(TAG, text);
            TextView txtError = findViewById(R.id.txt_error);
            txtError.setText(text);
        });
    }

    private void showMessage(String msg) {
        runOnUiThread(() -> {
            Log.i(TAG, msg);
            TextView txtMessage = findViewById(R.id.txt_message);
            txtMessage.setText(msg);
        });
    }

    private void showEvent(String event) {
        runOnUiThread(() -> {
            Log.i(TAG, event);
            TextView txtEvent = findViewById(R.id.txt_event);
            String msg = txtEvent.getText().toString();
            msg += "\n";
            msg += event;
            txtEvent.setText(msg);
        });
    }

    private void addFriend() {
        final EditText edit = new EditText(this);
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Input DID");

        builder.setView(edit);
        builder.setNegativeButton("cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        });
        builder.setPositiveButton("ok", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                String friendCode = edit.getText().toString().trim();
                if (friendCode.isEmpty()) return;

                mManagerConnector.addFriend(friendCode, "hello");
            }
        });
        builder.create().show();
    }

    private PeerNode mPeerNode = null;
    private Connector mManagerConnector = null;
}
