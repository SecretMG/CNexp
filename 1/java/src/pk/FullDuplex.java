package pk;

import java.io.IOException;
import java.net.InetAddress;

public class FullDuplex{

    public FullDuplex() throws IOException {
    }

    public Thread getsender(int sendport, String sendfile, String logfile) throws IOException {
        SendGBN sender = new SendGBN(InetAddress.getByName("localhost"), sendport, sendfile, logfile);
        Thread senderthread = new Thread(sender);
        return senderthread;
    }

    public Thread getreceiver(int recvport, String recvfile,String logfile) throws IOException {
        return Receiver.getreceiver(recvport, recvfile, logfile);
    }


}

