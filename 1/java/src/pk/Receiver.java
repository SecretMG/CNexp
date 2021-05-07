package pk;

import java.net.*;
import java.io.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.table.*;

import static pk.Serializer.serializer;


public class Receiver extends JPanel implements Runnable{
    public static final int BYTES_PER_PACKET = 4096;
    public static final int PORT = 7609;
    public static final String LOST = "LOST";
    public static final String RCVD = "RCVD";
    public static final String SENT = "SENT";
    public static final String RSNT = "RSNT";


    Thread t;
    JTextField prob, N;
    JTextArea text;
    JTable table, ackTable;
    Vector packets, ackPackets;

    DatagramSocket socket;
    volatile boolean done;

    public Receiver() throws IOException{
        super(new BorderLayout());

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
        byte[] dataa = new byte[BYTES_PER_PACKET];
        DatagramPacket ack = new DatagramPacket(dataa, BYTES_PER_PACKET);
        while(!done){
            //get the next packet

            byte[] data = new byte[BYTES_PER_PACKET];
            DatagramPacket rp = new DatagramPacket(data, BYTES_PER_PACKET);
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
            double d = rng.nextDouble();
            double p = Double.parseDouble(prob.getText());
            boolean success = (d > p);
            String msg = null;
            if(success){ //lose it
                msg = RCVD;
            }
            else{ //don't lose it
                msg = LOST;
            }
            //put the packet in a pocket.
            PacketPocket pack = new PacketPocket(rp, msg, PacketType.DATA);
            packets.add(pack);

            if(success){
                String ackMsg;
                MyPacket udp_ack;
                byte[] udp_ack_bytes = null;
                //if we got the packet we expected
                if(pack.id == expectedSequenceNumber){
                    ackMsg = SENT;

                    pack.OuttoTxt(pack.line);

                    udp_ack = new MyPacket(-1,expectedSequenceNumber,null);
                    System.out.println("right ack: "+expectedSequenceNumber);
                    System.out.println("");
                    try {
                        udp_ack_bytes = serializer.toBytes(udp_ack);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                    ack.setData(udp_ack_bytes);

                    expectedSequenceNumber = (byte)((expectedSequenceNumber + 1) % Byte.parseByte(N.getText()));
                }
                else{
                    ackMsg = RSNT;

                    udp_ack = new MyPacket(-1,(byte)((expectedSequenceNumber - 1) % Byte.parseByte(N.getText())),null);
                    System.out.println("wrong ack: "+(byte)((expectedSequenceNumber - 1) % Byte.parseByte(N.getText())));
                    System.out.println("");

                    try {
                        udp_ack_bytes = serializer.toBytes(udp_ack);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                    ack.setData(udp_ack_bytes);

                }

                ack.setAddress(rp.getAddress());
                ack.setPort(rp.getPort());

                try{
                    socket.send(ack);
                    //put the packet in a pocket.
                    PacketPocket ackPack = new PacketPocket(ack, ackMsg, PacketType.ACK);
                    ackPackets.add(ackPack);
                }
                catch(IOException ioe){
                    System.err.println("IOException occurred " +
                            "while sending!");
                }
            }


            //Tell the table it's model has changed
            TableModel model = table.getModel();
            if(model instanceof AbstractTableModel){
                ((AbstractTableModel)model).fireTableDataChanged();
            }

            //Tell the ackTable it's model has changed
            model = ackTable.getModel();
            if(model instanceof AbstractTableModel){
                ((AbstractTableModel)model).fireTableDataChanged();
            }
        }
    }

    static Random rng = new Random();
    public static void main(String[] args) throws IOException{
        String filename = null;

        //set up random number generator w/ non random seed
        if(args.length >= 1){
            int n = Integer.parseInt(args[0]);
            rng = new Random(n);
        }

        JFrame f = new JFrame("Packet receiver!");
        Receiver r = new Receiver();

        f.getContentPane().add("Center", r);
        f.addWindowListener(new WindowAdapter(){
            public void windowClosing(WindowEvent e){
                System.exit(0);
            }
        });

        f.setSize(500,500);
        f.show();
        r.start();
    }
}

//represents a row in the table
class PacketPocket extends Vector{

    DatagramPacket packet;
    byte id;
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
            if (CRC.crc_check(this.data))
                System.out.println("没啥错误");
            else
                System.out.println("错了");

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

    public static BufferedWriter writer;

    static {
        try {
            writer = new BufferedWriter(new FileWriter("E:\\received.txt"));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public void OuttoTxt(String line){
        System.out.println("received line  "+line);
        try {
            writer.write(line);
            writer.newLine();
            writer.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}