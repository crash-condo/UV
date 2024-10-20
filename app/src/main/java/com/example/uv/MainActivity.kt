package com.example.uv

import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import android.content.Intent
import android.os.Bundle
import android.widget.Button
import android.widget.Toast
import java.io.IOException
import java.util.*

class MainActivity : Activity() {
    private val bluetoothAdapter: BluetoothAdapter? = BluetoothAdapter.getDefaultAdapter()
    private var bluetoothSocket: BluetoothSocket? = null
    private val deviceName = "UVP"
    private val uuid: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        if (bluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth is not supported on this device", Toast.LENGTH_LONG).show()
            return
        }

        if (!bluetoothAdapter.isEnabled) {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT)
        } else {
            connectToBluetooth()
        }

        findViewById<Button>(R.id.buttonVolUp).setOnClickListener {
            sendBluetoothCommand("VU")
        }

        findViewById<Button>(R.id.buttonVolDown).setOnClickListener {
            sendBluetoothCommand("VD")
        }
    }

    private fun connectToBluetooth() {
        val pairedDevices: Set<BluetoothDevice>? = bluetoothAdapter?.bondedDevices
        val targetDevice = pairedDevices?.find { device -> device.name == deviceName }

        if (targetDevice == null) {
            Toast.makeText(this, "UVP device not found", Toast.LENGTH_LONG).show()
            return
        }

        try {
            val socket = targetDevice.createRfcommSocketToServiceRecord(uuid)
            socket.connect()
            bluetoothSocket = socket
            Toast.makeText(this, "Connected to UVP", Toast.LENGTH_SHORT).show()
        } catch (e: IOException) {
            Toast.makeText(this, "Failed to connect: ${e.message}", Toast.LENGTH_LONG).show()
        }
    }
    private fun sendBluetoothCommand(command: String) {
        if (bluetoothSocket?.isConnected == true) {
            try {
                bluetoothSocket?.outputStream?.write(command.toByteArray())
                Toast.makeText(this, "Sent: $command", Toast.LENGTH_SHORT).show()
            } catch (e: IOException) {
                Toast.makeText(this, "Failed to send command: ${e.message}", Toast.LENGTH_LONG).show()
            }
        } else {
            Toast.makeText(this, "Bluetooth not connected", Toast.LENGTH_SHORT).show()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        bluetoothSocket?.close()
    }

    companion object {
        private const val REQUEST_ENABLE_BT = 1
    }
}