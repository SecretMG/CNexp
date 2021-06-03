import java.io.*;
import java.net.*;
import java.util.Map;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;


public class PacketSender{
    public static final int BYTES_PER_PACKET = 1024;
    public int port;

    // timer
    Timer timer;
    public int frequency;         // delay for timeout

    // current node
    NodeInfo node;

    int seq_num = 0;

    // the socket
    DatagramSocket socket;

    boolean receiving = false;

    // constructor
    PacketSender(NodeInfo node){
        this.node = node;
        this.port = node.port;
        this.frequency = node.frequency;

        // bind socket
        try {
            socket = new DatagramSocket(port);
        } catch (SocketException e) {
            System.err.println("socket err");
        }

        // start listener thread
        Listener listener = new Listener();
        listener.start();

        InitTimer();

        System.out.println("node " + node.id + " start!");

    }

    public void InitTimer(){
        timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
//                System.out.println("timeout!");
                try {
                    node.printTable();
                    sendNodeInfo(node.generatePacket(++seq_num));
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        },0, frequency);
    }


    public void sendPacket(int dest_port, byte[] data) throws IOException{
        InetAddress ip = InetAddress.getByName("localhost");
        DatagramPacket packet = new DatagramPacket(data, data.length, ip, dest_port);
        socket.send(packet);
    }

    // send node info to neighbours
    public void sendNodeInfo(byte[] data) throws IOException{
        // iterate through neighbour and send data packet
        for(Map.Entry<String, Integer> set: node.getNeighbour().entrySet()){

            String dest_id = set.getKey();
            Integer dest_port = set.getValue();

            if(dest_id.equals(node.id)){
                continue; // don't send to node itself
            }
            System.out.println(node.id + " sending " + dest_id + " info");

            sendPacket(dest_port, data);
        }
    }

    // process received packet
    public void receivePacket(DatagramPacket info_packet) {
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
        // process packet
        for(Map.Entry<String, Integer> set: packet.table.entrySet()){
            String dest_id = set.getKey();
            Integer dis = set.getValue();

            int disToNeighbour = node.getDis(sender_id);

            if(dest_id.equals(sender_id)) // ignore a->a
                continue;

            int new_dis = dis + disToNeighbour;
            if(dest_id.equals(node.id))
                new_dis -= disToNeighbour;

//            System.out.println(node.id + " putting:" + sender_id + "->" + dest_id + " :" + new_dis);
            node.putRoutingInfo(sender_id, dest_id, new_dis);

        }

    }


    class Listener extends Thread{
        public void run(){
            try{
                while(true) {
                    byte[] receiveData = new byte[BYTES_PER_PACKET];
                    DatagramPacket infopack = new DatagramPacket(receiveData, receiveData.length);

                    ///////////////////////////////////////////
                    //
                    // Get an incoming packet
                    //
                    socket.receive(infopack);

                    //process it
                    receivePacket(infopack);
                }
            }
            catch(IOException ioe){
                //Will enter here if the thread is listening for a packet
                //when we quit, and that's ok.
            }

        }
    }
}