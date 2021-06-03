package pk;

import java.io.IOException;

public class User {
    public static void print(){
        int cnt = Thread.activeCount();
        Thread[] tarray = new Thread[cnt];
        Thread.enumerate(tarray);
        for (Thread th : tarray) {
            System.out.println(th.getName());
        }
    }

    public static void main(String[] args) throws IOException, InterruptedException {
        Args config = new Args();

        String folder = "E:\\\\CNexp-master (2)\\\\CNexp-master\\\\1\\\\java\\\\resource\\";


        FullDuplex dp = new FullDuplex();

        Thread s1 = dp.getsender(config.port1, folder+"harry-test.txt","E:\\a\\aSendLog.txt");
        Thread s2 = dp.getsender(config.port2, folder+"tang3-test.txt","E:\\b\\bSendLog.txt");

        Thread r1 = dp.getreceiver(config.port3, folder + "harry-recv.txt","E:\\b\\bRecvLog.txt");
        Thread r2 = dp.getreceiver(config.port4, folder + "tang3-recv.txt","E:\\a\\aRecvLog.txt");

        s1.setName("sender1");
        s2.setName("sender1");
        r1.setName("rcv1");
        r2.setName("rcv2");

        r1.start();
        s1.start();

        r2.start();
        s2.start();

        // wait for senders to finish
        s1.join();
        s2.join();

        print();

    }
}
