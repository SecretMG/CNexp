package pk;

import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.util.Objects;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;
import java.util.Date;
import java.text.SimpleDateFormat;

import static java.lang.Thread.sleep;
import static pk.Serializer.serializer;

public class SendGBN implements Runnable{
    //constants
    volatile Args config = new Args();
    public static byte MAX_SEQ_NUMBER = 100; //The maximum sequence number
    public int PORT;          //the receiver's port

    //the socket
    DatagramSocket socket;
    //the ip address of the receiver
    InetAddress ipAddress;
    // file to read
    String sendfile = "";
    String logfile = "";

    //variables from the algorithm
    byte base = 0;
    int ackno = -1;
    int number = 0;
    byte nextSeqNum = config.InitSeqNo;

    //needed for receiver thread
    boolean done = false;
    volatile boolean timeout = false;

    //number of packets sent, but unacknowledged
    int unAckedPackets = 0;

    //timer
    Timer timer = new Timer();

    // buffer
    String [] windowbuffer = new String[MAX_SEQ_NUMBER];

    static Random rng = new Random();

    public BufferedWriter SdLog = null;

    public void ResetTimer(){
        timer.cancel();
        timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                System.out.println("timeout!");
                try {
                    timeOut();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        },config.Timeout);
    }

    //////////////////////////////
    //
    // other instance variables
    //

    //main program
    //hostname can be included on the command line
    //if not, assume the receiver is listening on this machine
    public static void main(String args[]) throws IOException{
        InetAddress ipAddress = null;
//        if(args.length > 0){
//            //get specified host
//            ipAddress = InetAddress.getByName(args[0]);
//        }
//        else{
//            //use default local host
//            ipAddress = InetAddress.getByName("localhost"); //loopback
//        }
        ipAddress = InetAddress.getByName("localhost");
        //create a new sender
        int port = 7609;
        String path = "D:\\learn\\2021\\Network\\labs\\CNexp-master\\CNexp-master\\1\\java\\resource\\test.txt";
//        SendGBN sender = new SendGBN(ipAddress, port, path);
//
//        //start processing the user's data
//        sender.processUserData();
    }

    //constructor
    public SendGBN(InetAddress ip, int port, String path, String logpath) throws IOException{
        ipAddress = ip;
        PORT = port;
        sendfile = path;
        logfile = logpath;
        socket = new DatagramSocket();

        System.out.println("sender at port:" + PORT + "created");

        /////////////////////////////
        //
        // other initialization
        //
    }

    //deals with getting the data from the user to send
    public void processUserData() throws IOException{

        //create input reading mechanism
        BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

        // System.out.println("Enter some text, to end place a . on a line alone.");

        File file = new File(sendfile);
        SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH mm ss");
        String LogName = "E:\\SdLog\\" + df.format(new Date()) + ".txt";
        try {
            SdLog = new BufferedWriter(new FileWriter(logfile));
        } catch (IOException e) {
            e.printStackTrace();
        }

        BufferedReader br = new BufferedReader(new FileReader(file));

        ResetTimer();

        String line;
        while ((line = br.readLine()) != null){
            //wait until the queue is no longer full
            while(unAckedPackets == config.SWSize  || timeout){
                //we can do this because we will be running other threads
                //we'll give them a chance to run
                Thread.yield();
            }

            System.out.println("read line:                              "+ line);

            sendPacket(line,"New");

            ///////////////////////////////////////////
            //
            // wait until the queue is no longer full
            // send an end of line character
            //
        }
        while(base != nextSeqNum || timeout || unAckedPackets != 0) {
            Thread.yield();
        }
        done = true;
        timer.cancel();
        socket.close();
        System.out.println("port:" + PORT + "Done!");
    }

    /*
     * The following three methods are correspond to those in the book.
     * They are defined as synchronized so that only one can be active
     * at a time from any given thread (main thread, ackListener or timer.
     * This prevents problems caused by multiple thread updating your
     * data structures at the same time. You do NOT want to have anything
     * in these methods that blocks the thread such as a socket.receive or
     * a reader.readLine, because it will prevent other threads from entering
     * their methods
     */

    public synchronized void sendPacket(String line,String status) throws IOException{
        ///////////////////////////////////////////
        //
        // Turn the char c into a packet and send it off.
        // Remember that a character is 2 bytes.
        // Called from processUserData().
        //
        // See the book
        //
        windowbuffer[nextSeqNum] = line;

//        for (int ads = 0;ads<10;ads++)System.out.println(windowbuffer[ads]);
        System.out.println("send packet seq" + nextSeqNum);
        unAckedPackets++;
        System.out.println("unacked" + unAckedPackets);

        MyPacket udp_pkt = new MyPacket(nextSeqNum, (byte) -1,line);
        byte[] udp_pkt_bytes = serializer.toBytes(udp_pkt);
        byte[] buf = null;

        double PDUError = rng.nextDouble();
        System.out.println(PDUError);
        System.out.println("");

        if(PDUError <= config.ErrorRate){
            buf = udp_pkt_bytes;
        }
        else{
            buf = CRC.__init__(udp_pkt_bytes);
        }
        DatagramPacket pkt = new DatagramPacket(buf, buf.length, ipAddress, PORT);

        double PDULost = rng.nextDouble();
        System.out.println(PDULost);
        System.out.println("");

        SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:SS");
//        String LogData = "time:" + df.format(new Date()) + " type:send" + " subject:(" + ipAddress + "," + PORT + ")" + " sendto:(" + ipAddress + "," + PORT + ")" + " seq:" + nextSeqNum + " ack:321" + " status:" + status;
        String LogData = ++number + ", pdu_to_send=" + nextSeqNum + ", status=" + status + ", ackedNo=" + ackno;
        try {
            SdLog.write(LogData);
            SdLog.newLine();
            SdLog.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (PDULost <= config.LostRate){
            nextSeqNum = (byte)((nextSeqNum + 1) % MAX_SEQ_NUMBER);
            return;
        }

        socket.send(pkt);

        nextSeqNum = (byte)((nextSeqNum + 1) % MAX_SEQ_NUMBER);

    }

    public synchronized void receivePacket(DatagramPacket ack){
        ///////////////////////////////////////////
        //
        // Process a received packet
        // Called from the ACKListener thread
        //
        // See the book
        //

        byte[] acknum = ack.getData();

        MyPacket ackn = null;

        try {
            ackn = (MyPacket) serializer.toObject(acknum);
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
        byte flag = base ;
        base = (byte) ((ackn.ackk + 1) % MAX_SEQ_NUMBER);
        ackno = (ackno + 1) % MAX_SEQ_NUMBER;

        if (base != flag){

            windowbuffer[ackn.ackk] = null;

//            for (int ads = 0;ads<10;ads++)System.out.println(windowbuffer[ads]);
            unAckedPackets -= 1;
            // reset timer
            ResetTimer();

            // debug print
            System.out.println("sender get ack" + ackn.ackk);
            System.out.println("base:" + base);
            System.out.println("");
        }
        else {
            System.out.println("sender discard ack" + ackn.ackk);
            System.out.println("base:" + base);
            System.out.println("");
        }

    }

    public synchronized void timeOut() throws IOException {
        ///////////////////////////////////////////
        //
        // Process a message from the timer.
        // Called from the timer.
        //
        // See the book
        //
        timeout = true;
        System.out.println(nextSeqNum);
//        for (int ads = 0;ads<10;ads++)System.out.println(windowbuffer[ads]);
        nextSeqNum = base;
        unAckedPackets = 0;
//
        ResetTimer();

        for (int i = 0; i < config.SWSize; i++) {
            if (windowbuffer[nextSeqNum] == null){
                break;
            }
            while(unAckedPackets == config.SWSize){
                Thread.yield();
            }
            System.out.println("resend packet seq" + nextSeqNum + " :                      " + windowbuffer[nextSeqNum]);
            System.out.println("");
            sendPacket(windowbuffer[nextSeqNum],"TO");

        }


        timeout = false;
    }

    @Override
    public void run() {

        //create receiver thread
        ACKListener rcv = new ACKListener();

        rcv.start();


        //start processing the user's data
        try {
            this.processUserData();
        } catch (IOException e) {
            e.printStackTrace();
        }

        rcv.interrupt();

        timer.cancel();
    }


    class ACKListener extends Thread{
        public void run(){
            System.out.println("listener " + PORT + " start");
            try{
                while(!Thread.interrupted()){
                    //listen for packet
                    byte[] receiveData = new byte[config.DataSize];
                    DatagramPacket ack = new DatagramPacket(receiveData, receiveData.length);

                    ///////////////////////////////////////////
                    //
                    // Get an incoming ACK packet
                    //
                    socket.receive(ack);
                    System.out.println("listener " + PORT + " get ack");

                    //process it
                    receivePacket(ack);

                }
            }
            catch(IOException ioe){
                //Will enter here if the thread is listening for a packet
                //when we quit, and that's ok.
            }
            System.out.println("listener quit" + "port" + PORT);
        }
    }
}