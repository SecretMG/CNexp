package pk;

import java.io.*;
import java.net.*;

import static pk.Serializer.serializer;

public class SendGBN{
    //constants
    public static byte N = 10;              //the N from "Go-Back-N"
    public static byte MAX_SEQ_NUMBER = 20; //The maximum sequence number
    public static int PORT = 7609;          //the receiver's port
    public static int DELAY = 1000;         //1 second delay for timeout

    //the socket
    DatagramSocket socket;
    //the ip address of the receiver
    InetAddress ipAddress;

    //variables from the algorithm
    byte base = 1;
    byte nextSeqNum = 1;

    //needed for receiver thread
    volatile boolean done = false;

    //number of packets sent, but unacknowledged
    int unAckedPackets = 0;

    //////////////////////////////
    //
    // other instance variables
    //

    //main program
    //hostname can be included on the command line
    //if not, assume the receiver is listening on this machine
    public static void main(String[] args) throws IOException{
        InetAddress ipAddress = null;
        if(args.length > 0){
            //get specified host
            ipAddress = InetAddress.getByName(args[0]);
        }
        else{
            //use default local host
            ipAddress = InetAddress.getByName("localhost"); //loopback
        }
        //create a new sender
        SendGBN sender = new SendGBN(ipAddress);

        //start processing the user's data
        sender.processUserData();
    }

    //constructor
    public SendGBN(InetAddress ip) throws IOException{
        ipAddress = ip;

        socket = new DatagramSocket();

        //create receiver thread
        ACKListener rcv = new ACKListener();

        //start the thread
        rcv.start();

        /////////////////////////////
        //
        // other initialization
        //
    }

    //deals with getting the data from the user to send
    public void processUserData() throws IOException{
        //create input reading mechanism
        BufferedReader in =
                new BufferedReader(new InputStreamReader(System.in));

        System.out.println("Enter some text, to end place a . on a line alone.");

        String line = null;
        while((line = in.readLine()) != null && !(line.equals("."))){
                //wait until the queue is no longer full
                while(unAckedPackets == N){
                    //we can do this because we will be running other threads
                    //we'll give them a chance to run
                    Thread.yield();
                }

                sendPacket(line);

            ///////////////////////////////////////////
            //
            // wait until the queue is no longer full
            // send an end of line character
            //
        }
        done = true;
        socket.close();
        System.out.println("Done!");
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

    public synchronized void sendPacket(String line) throws IOException{
        ///////////////////////////////////////////
        //
        // Turn the char c into a packet and send it off.
        // Remember that a character is 2 bytes.
        // Called from processUserData().
        //
        // See the book
        //
        MyPacket udp_pkt = new MyPacket(nextSeqNum++, line);
        byte[] buf = serializer.toBytes(udp_pkt);
        DatagramPacket pkt = new DatagramPacket(buf, buf.length, ipAddress, PORT);
        socket.send(pkt);
    }

    public synchronized void receivePacket(DatagramPacket ack){
        ///////////////////////////////////////////
        //
        // Process a received packet
        // Called from the ACKListener thread
        //
        // See the book
        //

    }

    public synchronized void timeOut(){
        ///////////////////////////////////////////
        //
        // Process a message from the timer.
        // Called from the timer.
        //
        // See the book
        //
    }


    class ACKListener extends Thread{
        public void run(){
//            try{
                while(!done){
                    //listen for packet
                    DatagramPacket ack = null;

                    ///////////////////////////////////////////
                    //
                    // Get an incoming ACK packet
                    //

                    //process it
                    receivePacket(ack);
                }
//            }
//            catch(IOException ioe){
//                //Will enter here if the thread is listening for a packet
//                //when we quit, and that's ok.
//            }

        }
    }
}