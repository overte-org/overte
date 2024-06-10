package io.highfidelity.hifiinterface;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.text.TextUtils;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Arrays;
import java.util.List;

public class PermissionChecker extends Activity {
    private static final int REQUEST_PERMISSIONS = 20;

    private static final boolean CHOOSE_AVATAR_ON_STARTUP = false;
    private static final String TAG = "Interface";

    private static final String EXTRA_ARGS = "args";
    private String mArgs;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mArgs =(getIntent().getStringExtra(EXTRA_ARGS));

        Intent myIntent = new Intent(this, BreakpadUploaderService.class);
        startService(myIntent);
        if (CHOOSE_AVATAR_ON_STARTUP) {
            showMenu();
        }

        File obbDir = getObbDir();
        if (!obbDir.exists()) {
            if (obbDir.mkdirs()) {
                Log.d(TAG, "Obb dir created");
            }
        }

        requestAppPermissions(new
                        String[]{
                        Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        Manifest.permission.RECORD_AUDIO,
                        Manifest.permission.CAMERA}
                ,2,REQUEST_PERMISSIONS);

    }

    public void requestAppPermissions(final String[] requestedPermissions,
                                      final int stringId, final int requestCode) {
        int permissionCheck = PackageManager.PERMISSION_GRANTED;
        boolean shouldShowRequestPermissionRationale = false;
        for (String permission : requestedPermissions) {
            permissionCheck = permissionCheck + checkSelfPermission(permission);
            shouldShowRequestPermissionRationale = shouldShowRequestPermissionRationale || shouldShowRequestPermissionRationale(permission);
        }
        if (permissionCheck != PackageManager.PERMISSION_GRANTED) {
            System.out.println("Permission was not granted. Ask for permissions");
            if (shouldShowRequestPermissionRationale) {
                requestPermissions(requestedPermissions, requestCode);
            } else {
                requestPermissions(requestedPermissions, requestCode);
            }
        } else {
            System.out.println("Launching the other activity..");
            launchActivityWithPermissions();
        }
    }

    private void launchActivityWithPermissions(){
        Intent i = new Intent(this, InterfaceActivity.class);
        
        if (!TextUtils.isEmpty(mArgs)) {
            i.putExtra(EXTRA_ARGS, mArgs);
        }

        startActivity(i);
        finish();
    }

    private void showMenu(){
        final List<String> avatarOptions = Arrays.asList("\uD83D\uDC66\uD83C\uDFFB Cody","\uD83D\uDC66\uD83C\uDFFF Will","\uD83D\uDC68\uD83C\uDFFD Albert", "\uD83D\uDC7D Being of Light");
        final String[] avatarPaths = {
            "http://mpassets.highfidelity.com/8c859fca-4cbd-4e82-aad1-5f4cb0ca5d53-v1/cody.fst",
            "http://mpassets.highfidelity.com/d029ae8d-2905-4eb7-ba46-4bd1b8cb9d73-v1/4618d52e711fbb34df442b414da767bb.fst",
            "http://mpassets.highfidelity.com/1e57c395-612e-4acd-9561-e79dbda0bc49-v1/albert.fst" };

          final String pathForJson = "/data/data/io.highfidelity.hifiinterface/files/.config/High Fidelity - dev/";
        new AlertDialog.Builder(this)
        .setTitle("Pick an avatar")
        .setItems(avatarOptions.toArray(new CharSequence[avatarOptions.size()]),new DialogInterface.OnClickListener(){

            @Override
            public void onClick(DialogInterface dialog, int which) {
                if(which < avatarPaths.length ) {
                    JSONObject obj = new JSONObject();
                        try {
                            obj.put("firstRun",false);
                            obj.put(SETTINGS_FULL_PRIVATE_GROUP_NAME + "/Avatar/fullAvatarURL", avatarPaths[which]);
                            File directory = new File(pathForJson);

                            if(!directory.exists()) directory.mkdirs();

                            File file = new File(pathForJson + "Interface.json");
                            Writer output = new BufferedWriter(new FileWriter(file));
                            output.write(obj.toString().replace("\\",""));
                            output.close();
                            System.out.println("I Could write config file expect to see the selected avatar"+obj.toString().replace("\\",""));

                        } catch (JSONException e) {
                            System.out.println("JSONException something weired went wrong");
                        } catch (IOException e) {
                            System.out.println("Could not write file :(");
                        }
                } else {
                    System.out.println("Default avatar selected...");
                }
                launchActivityWithPermissions();
            }
        }).show();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        int permissionCheck = PackageManager.PERMISSION_GRANTED;
        for (int permission : grantResults) {
            permissionCheck = permissionCheck + permission;
        }
        if ((grantResults.length > 0) && permissionCheck == PackageManager.PERMISSION_GRANTED) {
            launchActivityWithPermissions();
        } else if (grantResults.length > 0) {
            System.out.println("User has deliberately denied Permissions. Launching anyways");
            launchActivityWithPermissions();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        View decorView = getWindow().getDecorView();
        // Hide the status bar.
        int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;
        decorView.setSystemUiVisibility(uiOptions);
    }
}
