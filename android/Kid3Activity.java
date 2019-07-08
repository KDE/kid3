/**
 * @file Kid3Activity.java
 * Specialized QtActivity for Kid3.
 * Based on the blog post from Ekkehard Genz.
 * https://blog.qt.io/blog/2017/12/01/sharing-files-android-ios-qt-app/
 *
 * @b Project: Kid3
 * @author Urs Fleisch
 * @date 27 Feb 2019
 *
 * Copyright (C) 2019  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package net.sourceforge.kid3;

import java.io.File;
import java.lang.String;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.util.Log;
import org.qtproject.qt5.android.bindings.QtActivity;

public class Kid3Activity extends QtActivity {
    // Implemented in androidutils.cpp
    public static native void setFilePathFromIntent(String path);

    private static boolean isIntentPending;
    private static boolean isInitialized;

    // Called with intent when app is not yet running.
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d("Kid3", "onCreate");
        Intent intent = getIntent();
        if (intent != null) {
            String action = intent.getAction();
            if (action != null) {
                Log.d("Kid3", "intent action " + action);
                // Will be processed when C++ application is ready and calling checkPendingIntents()
                isIntentPending = true;
            }
        }
    }

    // Called with intent when app is already running.
    @Override
    public void onNewIntent(Intent intent) {
        Log.d("Kid3", "onNewIntent");
        super.onNewIntent(intent);
        setIntent(intent);
        // Intent is processed when C++ application is ready and has called checkPendingIntents()
        if (isInitialized) {
            processIntent();
        } else {
            isIntentPending = true;
        }
    }

    // Called from C++ application to trigger signal emission with intent data.
    public void checkPendingIntents() {
        isInitialized = true;
        if (isIntentPending) {
            isIntentPending = false;
            Log.d("Kid3", "process intent");
            processIntent();
        } else {
            Log.d("Kid3", "no intent pending");
        }
    }

    // Process the Intent if Action is EDIT or VIEW.
    private void processIntent() {
        Intent intent = getIntent();
        Log.d("Kid3", "action: " + intent.getAction());
        if ("android.intent.action.VIEW".equals(intent.getAction()) ||
            "android.intent.action.EDIT".equals(intent.getAction())) {
            Uri intentUri = intent.getData();
            if (intentUri != null) {
                Log.d("Kid3", "Intent URI: " + intentUri.toString());
                // content or file
                String intentScheme = intentUri.getScheme();
                if ("file".equals(intentScheme)) {
                    String filePath = intentUri.getPath();
                    if (filePath == null) {
                        filePath = intentUri.toString();
                    }
                    setFilePathFromIntent(filePath);
                } else if ("content".equals(intentScheme)) {
                    String filePath = getRealPathFromURI(this, intentUri);
                    if (filePath != null) {
                        Log.d("Kid3", "Real path: " + filePath);
                        setFilePathFromIntent(filePath);
                    }
                }
            }
        }
    }

    private static String getRealPathFromURI(final Context context, final Uri uri) {
        final boolean isKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;
        final String authority = uri.getAuthority();
        if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
            // DocumentProvider
            if ("com.android.externalstorage.documents".equals(authority)) {
                // ExternalStorageProvider
                Log.d("Kid3", "ExternalStorageProvider");
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];
                if ("primary".equalsIgnoreCase(type)) {
                    return Environment.getExternalStorageDirectory() + "/" + split[1];
                }
            } else if ("com.android.providers.downloads.documents".equals(authority)) {
                // DownloadsProvider
                Log.d("Kid3", "DownloadsProvider");
                final String id = DocumentsContract.getDocumentId(uri);
                Log.d("Kid3", "getDocumentId " + id);
                long longId = 0;
                try {
                    longId = Long.valueOf(id);
                } catch (NumberFormatException nfe) {
                    return getDataColumn(context, uri, null, null);
                }
                final Uri contentUri = ContentUris.withAppendedId(
                        Uri.parse("content://downloads/public_downloads"), longId);
                return getDataColumn(context, contentUri, null, null);
            } else if ("com.android.providers.media.documents".equals(authority)) {
                // MediaProvider
                Log.d("Kid3", "MediaProvider");
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];
                Uri contentUri = null;
                if ("image".equals(type)) {
                    contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                } else if ("video".equals(type)) {
                    contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                } else if ("audio".equals(type)) {
                    contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                }
                final String selection = "_id=?";
                final String[] selectionArgs = new String[]{split[1]};
                return getDataColumn(context, contentUri, selection, selectionArgs);
            }
        }
        if ("content".equalsIgnoreCase(uri.getScheme())) {
            if ("com.google.android.apps.photos.content".equals(authority))
                return uri.getLastPathSegment();
            String path = getDataColumn(context, uri, null, null);
            if (path == null) {
                int colonPos;
                path = uri.getPath();
                if (path != null && path.startsWith("/external_storage_root/")) {
                    path = Environment.getExternalStorageDirectory() + path.substring(22);
                } else if (path != null && path.startsWith("/document/") &&
                           (colonPos = path.indexOf(':')) != -1) {
                    String storagePath = "/storage/" + path.substring(10, colonPos) +
                                         "/" + path.substring(colonPos + 1);
                    if ((new File(storagePath)).exists()) {
                        path = storagePath;
                    }
                }
            }
            return path;
        } else if ("file".equalsIgnoreCase(uri.getScheme())) {
            return uri.getPath();
        }
        return null;
    }

    // Get the value of the data column for this URI.
    private static String getDataColumn(Context context, Uri uri, String selection,
                                        String[] selectionArgs) {
        String result = null;
        final String column = "_data";
        final String[] projection = {column};
        Cursor cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs,
                null);
        if (cursor != null) {
            if (cursor.moveToFirst()) {
                final int index = cursor.getColumnIndex(column);
                if (index != -1) {
                    result = cursor.getString(index);
                }
            }
            cursor.close();
        }
        return result;
    }
}
