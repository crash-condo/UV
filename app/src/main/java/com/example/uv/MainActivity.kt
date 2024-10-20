package com.example.uv // Make sure this matches your actual package name

import android.app.Activity
import android.os.Bundle
import android.widget.Button
import android.widget.Toast

class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        findViewById<Button>(R.id.buttonVolUp).setOnClickListener {
            Toast.makeText(this, "Volume Up", Toast.LENGTH_SHORT).show()
        }

        findViewById<Button>(R.id.buttonVolDown).setOnClickListener {
            Toast.makeText(this, "Volume Down", Toast.LENGTH_SHORT).show()
        }
    }
}