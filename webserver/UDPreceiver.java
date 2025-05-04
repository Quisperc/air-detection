//package com.zhaoci.udpDemo;

import java.net.DatagramPacket;
import java.net.DatagramSocket;

public class UDPreceiver {
    public static void main(String[] args) throws Exception {
        int port = 8080;
        DatagramSocket socket = new DatagramSocket(port);
        byte[] buffer = new byte[65536]; 

        System.out.println("UDP Server is listening on port " + port + "...");

        while (true) {
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
            socket.receive(packet);

            String received = new String(packet.getData(), 0, packet.getLength());
            System.out.println("Received from " + packet.getAddress().getHostAddress() + ":" + packet.getPort());
            System.out.println("Message: " + received);
        }
    }
}