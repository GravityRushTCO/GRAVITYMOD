package com.android.support;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.Typeface;
import android.text.InputType;
import android.view.Gravity;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

public class Auth {

    public static void showLogin(final Context context, final Runnable onAuthorized) {
        final SharedPreferences prefs = context.getSharedPreferences("GravityPrefs", Context.MODE_PRIVATE);
        String savedKey = prefs.getString("license_key", "");

        if (!savedKey.isEmpty()) {
            // Key already exists, continue to native check
            onAuthorized.run();
            return;
        }

        // Create a beautiful Cyberpunk Login Dialog
        AlertDialog.Builder builder = new AlertDialog.Builder(context, AlertDialog.THEME_DEVICE_DEFAULT_DARK);
        
        LinearLayout layout = new LinearLayout(context);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(60, 40, 60, 40);

        TextView title = new TextView(context);
        title.setText("GRAVITY SYSTEM ACCESS");
        title.setTextSize(20);
        title.setTextColor(Color.parseColor("#00f2ff"));
        title.setTypeface(null, Typeface.BOLD);
        title.setGravity(Gravity.CENTER);
        layout.addView(title);

        final EditText input = new EditText(context);
        input.setHint("ENTER LICENSE KEY");
        input.setHintTextColor(Color.GRAY);
        input.setTextColor(Color.WHITE);
        input.setInputType(InputType.TYPE_CLASS_TEXT);
        input.setPadding(0, 40, 0, 40);
        layout.addView(input);

        builder.setView(layout);
        builder.setCancelable(false);
        builder.setPositiveButton("AUTHENTICATE", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                String key = input.getText().toString().trim();
                if (key.length() > 5) {
                    prefs.edit().putString("license_key", key).apply();
                    Toast.makeText(context, "Initializing Hardware Verification...", Toast.LENGTH_SHORT).show();
                    onAuthorized.run();
                } else {
                    Toast.makeText(context, "Invalid Key Format", Toast.LENGTH_SHORT).show();
                    System.exit(0);
                }
            }
        });
        
        builder.setNegativeButton("EXIT", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                System.exit(0);
            }
        });

        builder.show();
    }
}
