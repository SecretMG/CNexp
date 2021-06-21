import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.TimeUnit;


public class PacketSender {
    public static final int BYTES_PER_PACKET = 1024;
    public int port;
    public boolean pause = false;

    BufferedWriter LogDoc = null;
    // timer
    Timer timer;
    public int frequency;         // delay for timeout

    // current node
    NodeInfo node;

    int seq_num = 0;

    // the socket
    DatagramSocket socket;

    int maxValidTime = 10000;

    // constructor
    PacketSender(NodeInfo node) throws InterruptedException {
        this.node = node;
        this.port = node.port;
        this.frequency = node.frequency;
        this.LogDoc = node.LogDoc;
        // bind socket
        try {
            socket = new DatagramSocket(port);
        } catch (SocketException e) {
            System.err.println("socket err");
        }

        // start listener thread
        Listener listener = new Listener();
        listener.start();

        TimeUnit.SECONDS.sleep(2);

        InitTimer();

        System.out.println("node " + node.id + " start!");
    }

    public void InitTimer() {
        timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
//                System.out.println("timeout!");
                try {
                    seq_num++;
                    sendNodeInfo(node.generatePacket(seq_num));
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }, 0, frequency);
    }


    public void sendPacket(int dest_port, byte[] data) throws IOException {
        InetAddress ip = InetAddress.getByName("localhost");
        DatagramPacket packet = new DatagramPacket(data, data.length, ip, dest_port);
        socket.send(packet);
    }

    // send node info to neighbours
    public synchronized void sendNodeInfo(byte[] data) throws IOException {
        if (pause == false) {
            System.out.println("");
            String LogData = "## Sent.Source Node = " + node.id + ";Sequence Number = " + seq_num;
            System.out.println(LogData);

            LogDoc.newLine();
            LogDoc.write(LogData);
            LogDoc.newLine();
            LogDoc.flush();

            node.printTable();

            // iterate through neighbour and send data packet
            for (Map.Entry<String, Neighbour> set : node.neighbours.entrySet()) {

                if (set.getValue().updated == false) {
                    continue;
                }


                String dest_id = set.getKey();
                Neighbour neighbour = set.getValue();
                int dest_port = neighbour.port;
                if (dest_port == 0) continue; // not direct neighbour

                // check destination
                if (dest_id.equals(this.node.id)) {
                    continue; // don't send to node itself
                }

                // check alive
                Date current = new Date();
                if (neighbour.alive == false) {
                    continue;
                }

                if (current.getTime() - neighbour.timestamp.getTime() > maxValidTime) {
                    System.out.println(dest_id + " dying ");
                    neighbour.alive = false;
                    neighbour.distance = Float.MAX_VALUE;
                    continue;
                }

//            System.out.println(node.id + " sending " + dest_id + " info");
                // file IO here
                //
                //
                //
                //

                // send it
                sendPacket(dest_port, data);
//
//            // debug-----kill a
//            if(node.id.equals("a")){
//                try {
//                    Thread.sleep(100000);
//                } catch (InterruptedException e) {
//                    e.printStackTrace();
//                }
//            }

            }
        }
    }

    // process received packet
    public synchronized void receivePacket(DatagramPacket info_packet) {
        MyPacket packet = null;
        byte[] data = info_packet.getData();

        // deserialize packet
        try {
            packet = (MyPacket) Serializer.toObject(data);
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }

//        System.out.println(node.id + " receive packet: " + packet.ConvertToString());

        String sender_id = packet.id;
        // update timestamp of neighbour node
        if (node.neighbours.containsKey(sender_id)) {
            Neighbour neighbour = this.node.neighbours.get(sender_id);
            neighbour.timestamp = new Date();
//            System.out.println(node.id + " updating timestamp " + sender_id);
            this.node.neighbours.put(sender_id, neighbour);
        }
        // process packet
        for (Map.Entry<String, Neighbour> set : packet.table.entrySet()) {
            Neighbour item = set.getValue();
            String dest_id = set.getKey();
            float dis = item.distance;

            float disToNeighbour = node.getDis(sender_id);

            if (dest_id.equals(sender_id) || disToNeighbour == 1000) // ignore a->a
                continue;


//            if(set.getValue().next_hop != null && set.getValue().next_hop.equals(node.id))
//                continue;

            float new_dis = dis + disToNeighbour;
            if (dest_id.equals(node.id)) { // sb->me
//                new_dis -= disToNeighbour;
//                node.putRoutingInfo(dest_id, sender_id, new_dis);
//                System.out.println(node.id + " puttingv:" + dest_id + "->" + sender_id + " :" + new_dis
//                + " = " + disToNeighbour + " " + dis);
            } else {
                node.putRoutingInfo(sender_id, dest_id, new_dis);
//                System.out.println(node.id + " putting:" + sender_id + "->" + dest_id + " :" + new_dis + " = " + disToNeighbour + " " + dis);

            }


        }

    }


    class Listener extends Thread {
        public void run() {
            try {
                while (true) {
                    byte[] receiveData = new byte[BYTES_PER_PACKET];
                    DatagramPacket infopack = new DatagramPacket(receiveData, receiveData.length);

                    if (pause == false) {
                        socket.receive(infopack);

                        //process it
                        receivePacket(infopack);
                    }
                }
            } catch (IOException ioe) {
                //Will enter here if the thread is listening for a packet
                //when we quit, and that's ok.
            }

        }
    }
}