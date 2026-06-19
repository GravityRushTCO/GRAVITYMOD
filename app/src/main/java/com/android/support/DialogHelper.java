package com.android.support;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.widget.Toast;

public class DialogHelper {
    public static void showDialogWithLink(Context context, String title, String message, String LinkBtnTitle, String CloseBtnTitle, int sec, final String url) {
        final Context activeContext = (Main.currentActivity != null) ? Main.currentActivity : context;
        AlertDialog.Builder builder = new AlertDialog.Builder(activeContext)
                .setTitle(title)
                .setMessage(message);

        if (url != null) {
            builder.setPositiveButton(LinkBtnTitle, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    openUrlSafely(activeContext, url);
                }
            });
        }

        final AlertDialog dialog = builder.create();

        if (!(activeContext instanceof android.app.Activity)) {
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                dialog.getWindow().setType(android.view.WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY);
            } else {
                dialog.getWindow().setType(android.view.WindowManager.LayoutParams.TYPE_PHONE);
            }
        }

        String closeInitTitle = CloseBtnTitle;
        if(sec > 0) {
            closeInitTitle = closeInitTitle + "(" + sec + ")";
        }

        dialog.setButton(DialogInterface.BUTTON_NEGATIVE, closeInitTitle, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
            }
        });

        try {
            dialog.show();
        } catch (Exception e) {
            Toast.makeText(context, message, Toast.LENGTH_LONG).show();
        }

        if (sec > 0) startCountdown(dialog, CloseBtnTitle, sec);
    }

    private static void startCountdown(final AlertDialog dialog, String CloseBtnTitle, final int seconds) {
        final Handler handler = new Handler();
        final Runnable countdownRunnable = new Runnable() {
            int remainingTime = seconds;

            @SuppressLint("SetTextI18n")
            @Override
            public void run() {
                if (remainingTime > 0 && dialog != null && dialog.isShowing()) {
                    dialog.getButton(AlertDialog.BUTTON_NEGATIVE)
                            .setText(CloseBtnTitle + "(" + remainingTime + ")");
                    remainingTime--;
                    handler.postDelayed(this, 1000);
                } else if (dialog != null && dialog.isShowing()) {
                    dialog.getButton(AlertDialog.BUTTON_NEGATIVE).performClick();
                }
            }
        };

        handler.post(countdownRunnable);
    }

    @SuppressLint("QueryPermissionsNeeded")
    public static void openUrlSafely(Context context, String url) {
        if (url == null || url.trim().isEmpty()) {
            return;
        }

        if (!url.startsWith("http://") && !url.startsWith("https://")) {
            url = "http://" + url;
        }

        try {
            Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

            if (intent.resolveActivity(context.getPackageManager()) != null) {
                context.startActivity(intent);
            } else {
                Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
                browserIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                browserIntent.setPackage("com.android.chrome");

                if (browserIntent.resolveActivity(context.getPackageManager()) != null) {
                    context.startActivity(browserIntent);
                } else {
                    browserIntent.setPackage(null);
                    context.startActivity(browserIntent);
                }
            }
        } catch (Exception e) {
            Toast.makeText(context, e.toString(), Toast.LENGTH_SHORT).show();
        }
    }

    public static void showChatInput(final Context context, final String postUrl) {
        // Always prefer Unity's current activity — never use Application context for dialogs
        final android.app.Activity activity = (Main.currentActivity != null)
                ? Main.currentActivity : null;

        if (activity == null) {
            // No activity available — silently fail rather than crash
            return;
        }

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                try {
                    final android.widget.EditText input = new android.widget.EditText(activity);
                    input.setHint("Saisissez votre message au développeur...");
                    input.setSingleLine(false);
                    input.setMaxLines(4);

                    AlertDialog.Builder builder = new AlertDialog.Builder(activity,
                            android.R.style.Theme_DeviceDefault_Dialog_Alert);
                    builder.setTitle("Support Technique");
                    builder.setView(input);
                    builder.setPositiveButton("Envoyer", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            final String text = input.getText().toString().trim();
                            if (text.isEmpty()) return;
                            new Thread(new Runnable() {
                                @Override
                                public void run() {
                                    try {
                                        String deviceId = "Unknown";
                                        try {
                                            android.net.Uri uri = android.net.Uri.parse(postUrl);
                                            deviceId = uri.getQueryParameter("device");
                                        } catch (Exception ignored) {}
                                        if (deviceId == null || deviceId.isEmpty()) {
                                            deviceId = activity.getPackageName();
                                        }
                                        java.net.URL url = new java.net.URL(postUrl);
                                        java.net.HttpURLConnection conn = (java.net.HttpURLConnection) url.openConnection();
                                        conn.setRequestMethod("POST");
                                        conn.setRequestProperty("Content-Type", "application/json");
                                        conn.setDoOutput(true);
                                        String json = "{\"text\":\"" + text.replace("\"", "\\\"") + "\",\"from\":\"player\",\"device\":\"" + deviceId + "\"}";
                                        java.io.OutputStream os = conn.getOutputStream();
                                        os.write(json.getBytes());
                                        os.flush(); os.close();
                                        conn.getResponseCode();
                                    } catch (Exception e) {}
                                }
                            }).start();
                            Toast.makeText(activity, "Message envoyé !", Toast.LENGTH_SHORT).show();
                        }
                    });
                    builder.setNegativeButton("Annuler", null);
                    final AlertDialog dialog = builder.create();

                    // Force keyboard to open with the dialog
                    if (dialog.getWindow() != null) {
                        dialog.getWindow().setSoftInputMode(
                            android.view.WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
                    }

                    dialog.show();
                    input.requestFocus();
                } catch (Exception e) {
                    // Fallback toast if dialog fails
                    Toast.makeText(activity, "Erreur ouverture clavier", Toast.LENGTH_SHORT).show();
                }
            }
        });
    }
}