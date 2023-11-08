package com.example.myapplication;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class SettingsActivity extends AppCompatActivity {

    private EditText urlEditText;
    private Button saveUrlButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.settings_activity);

        urlEditText = findViewById(R.id.urlEditText);
        saveUrlButton = findViewById(R.id.saveUrlButton);

        // Load the previously saved URL, if any
        SharedPreferences sharedPreferences = getSharedPreferences("MyPrefs", Context.MODE_PRIVATE);
        String savedUrl = sharedPreferences.getString("url", "");
        urlEditText.setText(savedUrl);

        saveUrlButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Save the URL to SharedPreferences
                String url = urlEditText.getText().toString();
                SharedPreferences.Editor editor = sharedPreferences.edit();
                editor.putString("url", url);
                editor.apply();
                Toast.makeText(getApplicationContext(), "URL saved", Toast.LENGTH_SHORT).show();
            }
        });
    }
}
