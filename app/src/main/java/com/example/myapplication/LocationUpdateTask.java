package com.example.myapplication;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Scanner;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

public class LocationUpdateTask {

    private static final Executor executor = Executors.newSingleThreadExecutor();
    private Context context;
    private Handler handler = new Handler(Looper.getMainLooper());

    public LocationUpdateTask(Context context) {
        this.context = context;
    }

    public void executeLocationUpdate(final double latitude, final double longitude) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                performLocationUpdateInBackground(latitude, longitude);
            }
        });
    }

    private void performLocationUpdateInBackground(double latitude, double longitude) {
        HttpURLConnection urlConnection = null;
        InputStream inputStream = null;

        try {
            // Retrieve the URL from shared preferences
            SharedPreferences sharedPreferences = context.getSharedPreferences("MyPrefs", Context.MODE_PRIVATE);
            String url = sharedPreferences.getString("url", "");
            String name = sharedPreferences.getString("name", "");

            // Construct the URL with latitude and longitude as query parameters
            URL updateUrl = new URL(url + "?lat=" + latitude + "&lon=" + longitude + "&name=" + name);
            urlConnection = (HttpURLConnection) updateUrl.openConnection();

            urlConnection.setRequestMethod("GET");

            // Read the response
            inputStream = urlConnection.getInputStream();
            Scanner scanner = new Scanner(inputStream).useDelimiter("\\A");
            final String response = scanner.hasNext() ? scanner.next() : "";

            // Handle the response on the main thread
            handler.post(new Runnable() {
                @Override
                public void run() {
                    handleResponse(response);
                }
            });

        } catch (IOException e) {
            // Handle exceptions
            Log.e("LocationUpdateTask", "Error updating location", e);
        } finally {
            if (urlConnection != null) {
                urlConnection.disconnect();
            }
            if (inputStream != null) {
                try {
                    inputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void handleResponse(String response) {
        // Handle the response as needed on the main thread
    }
}

