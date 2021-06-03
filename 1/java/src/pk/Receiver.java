package pk;

import java.net.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.table.*;

import static pk.Serializer.serializer;


public class Receiver extends JPanel implements Runnable{
    volatile Args config = new Args();
    public int PORT = 7609;
    public static final String OK = "OK";
    public static final String NoErr = "NoErr";
    public static final String DataErr = "DataErr";
    
    public String recvfile = "";
    public String logfile = "";


    Thread t;
    JTextField prob, N;
    JTextArea text;
    JTable table, ackTable;
    Vector packets, ackPackets;

    DatagramSocket socket;
    volatile boolean done;

    public Receiver(int port, String outpath, String logpath) throws IOException{
        super(new BorderLayout());

        PORT = port;
        recvfile = outpath;
        logfile = logpath;

        packets = new Vector();
        ackPackets = new Vector();

        //set up UI
        JPanel p1 = new JPanel(new FlowLayout());
        p1.add(new JLabel("Packet Loss (0..1)"));
        prob = new JTextField(10);
        p1.add(prob);
        prob.setText("0");
        p1.add(new JLabel(" MAX SEQ NUM"));
        N = new JTextField(10);
        p1.add(N);
        N.setText("100");

        add("North", p1);

        //column header for the table
        Vector labels = new Vector();
        labels.add("Status");
        labels.add("Host");
        labels.add("Port");
        labels.add("Seq. Num");
        labels.add("Data");

        //user interface items
        JTabbedPane tabs = new JTabbedPane();

        table = new JTable(packets, labels);
        tabs.add("Received", new JScrollPane(table));

        ackTable = new JTable(ackPackets, labels);
        tabs.add("Sent", new JScrollPane(ackTable));

        add("Center", tabs);

        text = new JTextArea(10, 80);

        add("South", new JScrollPane(text));

        //set up network stuff
        socket = new DatagramSocket(PORT);

        //start thread
        done = false;
    }

    public void start(){
        if(t == null){
            t = new Thread(this);
            t.start();
        }
    }

    public void stop(){
        done = true;
    }

    public void run(){
        byte expectedSequenceNumber = 0;
        //ack for packet 0 (not initially sent)
        byte[] dataa = new byte[config.DataSize];
        DatagramPacket ack = new DatagramPacket(dataa, config.DataSize);

        // open output file
        BufferedWriter writer = null;
        BufferedWriter ReLog = null;

        try {
            writer = new BufferedWriter(new FileWriter(recvfile));
        } catch (IOException e) {
            e.printStackTrace();
        }

        SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH mm ss");
//        String LogName = "E:\\ReLog\\" + df.format(new Date()) + ".txt";/
        String LogName = recvfile + "-Receiver's Log.txt";
        try {
            ReLog = new BufferedWriter(new FileWriter(logfile));
        } catch (IOException e) {
            e.printStackTrace();
        }
        int flag = 0;

        while(!done){

            flag++;
            //get the next packet
            byte[] data = new byte[config.DataSize];
            DatagramPacket rp = new DatagramPacket(data, config.DataSize);
            try{
                //receive the packet
                //System.err.println("Waiting.");
                socket.receive(rp);
                //System.err.println("Got One");

                //give others a chance to run
                Thread.yield();
            }
            catch(IOException ioe){
                System.err.println("IOException occurred while receiving!");
            }
            //determine if the packet gets lost (somewhere)
            String msg = OK;
            String LogData;
            //put the packet in a pocket.
            PacketPocket pack = new PacketPocket(rp, msg, PacketType.DATA);

            String ackMsg;
            MyPacket udp_ack;
            byte[] udp_ack_bytes = null;
            //if we got the packet we expected
            if (pack.bool == true && pack.id == expectedSequenceNumber) {
                ackMsg = OK;
                LogData = flag + ", pdu_exp=" + expectedSequenceNumber + ", pdu_recv=" + pack.id + ", status=" + ackMsg + ", data=" + pack.line;
                try {
                    ReLog.write(LogData);
                    ReLog.newLine();
                    ReLog.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                }// write to file
                System.out.println("received line  " + pack.line);
                try {
                    writer.write(pack.line);
                    writer.newLine();
                    writer.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                // gengerate ack
                udp_ack = new MyPacket(-1, expectedSequenceNumber, null);
                System.out.println("port:" + PORT + "right ack: " + expectedSequenceNumber);
                System.out.println("");
                try {
                    udp_ack_bytes = serializer.toBytes(udp_ack);
                } catch (IOException e) {
                    e.printStackTrace();
                }

                ack.setData(udp_ack_bytes);

                expectedSequenceNumber = (byte) ((expectedSequenceNumber + 1) % Byte.parseByte(N.getText()));
            } else if(pack.bool == false){
                ackMsg = DataErr;
                LogData = flag + ", pdu_exp=" + expectedSequenceNumber + ", pdu_recv=" + pack.id + ", status=" + ackMsg ;
                try {
                    ReLog.write(LogData);
                    ReLog.newLine();
                    ReLog.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                udp_ack = new MyPacket(-1, (byte) ((expectedSequenceNumber - 1) % Byte.parseByte(N.getText())), null);
                System.out.println("wrong ack: " + (byte) ((expectedSequenceNumber - 1) % Byte.parseByte(N.getText())));
                System.out.println("");

                try {
                    udp_ack_bytes = serializer.toBytes(udp_ack);
                } catch (IOException e) {
                    e.printStackTrace();
                }

                ack.setData(udp_ack_bytes);

            }
            else{
                ackMsg = NoErr;
                LogData = flag + ", pdu_exp=" + expectedSequenceNumber + ", pdu_recv=" + pack.id + ", status=" + ackMsg;
                try {
                    ReLog.write(LogData);
                    ReLog.newLine();
                    ReLog.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                udp_ack = new MyPacket(-1, (byte) ((expectedSequenceNumber - 1) % Byte.parseByte(N.getText())), null);
                System.out.println("wrong ack: " + (byte) ((expectedSequenceNumber - 1) % Byte.parseByte(N.getText())));
                System.out.println("");

                try {
                    udp_ack_bytes = serializer.toBytes(udp_ack);
                } catch (IOException e) {
                    e.printStackTrace();
                }

                ack.setData(udp_ack_bytes);

            }

            packets.add(pack);

            ack.setAddress(rp.getAddress());
            ack.setPort(rp.getPort());

            try {
                socket.send(ack);
                //put the packet in a pocket.
                PacketPocket ackPack = new PacketPocket(ack, ackMsg, PacketType.ACK);
                ackPackets.add(ackPack);
            } catch (IOException ioe) {
                System.err.println("IOException occurred " +
                        "while sending!");
            }




//            //Tell the table it's model has changed
//            TableModel model = table.getModel();
//            if(model instanceof AbstractTableModel){
//                ((AbstractTableModel)model).fireTableDataChanged();
//            }
//
//            //Tell the ackTable it's model has changed
//            model = ackTable.getModel();
//            if(model instanceof AbstractTableModel){
//                ((AbstractTableModel)model).fireTableDataChanged();
//            }
        }

    }

    static Random rng = new Random();
    public static Thread getreceiver(int port, String savepath, String savelog) throws IOException{
        
        //set up random number generator w/ non random seed
//        if(args.length >= 1){
//            int n = Integer.parseInt(args[0]);
//            rng = new Random(n);
//        }

//        JFrame f = new JFrame("Packet receiver!");
        Receiver r = new Receiver(port, savepath, savelog);

//        f.getContentPane().add("Center", r);
//        f.addWindowListener(new WindowAdapter(){
//            public void windowClosing(WindowEvent e){
//                System.exit(0);
//            }
//        });
//
//        f.setSize(500,500);
//        f.show();
//        r.start();
        return new Thread(r);
    }
}

//represents a row in the table
class PacketPocket extends Vector{

    DatagramPacket packet;
    byte id;
    boolean bool;
    byte[] data;
    byte[] ndata = null;
    String line =null;

    public PacketPocket(DatagramPacket p, String status, PacketType type) {

        this.packet = p;
        this.data = p.getData();
        this.line = null;
        add(status);
        add(packet.getAddress());
        add(packet.getPort());
        // recieve data packet
        if(type == PacketType.DATA){
            if (CRC.crc_check(this.data)){
                System.out.println("没啥错误");
                this.bool = true;
            }
            else{
                System.out.println("错了");
                this.bool = false;
            }

            this.ndata = new byte[this.data.length - 2];
            System.arraycopy(this.data,0,this.ndata,0,this.data.length-2);

            MyPacket pkt = null;
            // deserialize
            try {
                pkt = (MyPacket) serializer.toObject(ndata);
            } catch (IOException e) {
                e.printStackTrace();
            } catch (ClassNotFoundException e) {
                e.printStackTrace();
            }
            System.out.println("Packet with sequence number " + pkt.sequence_num + " received");
            System.out.println("");
            this.id = (byte) pkt.sequence_num;
            this.line = pkt.data;
            add(pkt.sequence_num);
            add(pkt.data);
        }
        // ack packet
        else if(type == PacketType.ACK){
            MyPacket pkt = null;
            // deserialize
            try {
                pkt = (MyPacket) serializer.toObject(data);
            } catch (IOException e) {
                e.printStackTrace();
            } catch (ClassNotFoundException e) {
                e.printStackTrace();
            }
            id = pkt.ackk;
            add(id);
            add("---");
        }

    }

    
}