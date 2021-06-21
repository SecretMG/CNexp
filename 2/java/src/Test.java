import java.io.*;
import java.util.Scanner;
import java.util.concurrent.TimeUnit;

public class Test {

    public static void main(String[] args) throws InterruptedException, IOException {

        String line = null;
        String LogName = "E:\\Presentation\\LogNode-" + args[2];
        BufferedWriter writer = new BufferedWriter(new FileWriter(LogName));
        NodeInfo node = new NodeInfo(args[0], Integer.parseInt(args[1]), writer);
        node.frequency = 500;
        node.addNeighbour(args[0], Integer.parseInt(args[1]), 0);

        File file = new File(args[2]);
        BufferedReader br = new BufferedReader(new FileReader(file));
        while ((line = br.readLine()) != null) {
            String[] arr = line.split(" ");

            node.addNeighbour(arr[0], Integer.parseInt(arr[2]), Integer.parseInt(arr[1]));
        }

        PacketSender sender = new PacketSender(node);
        TimeUnit.SECONDS.sleep(5);

        while (true) {
            String s = (new Scanner(System.in)).next();
            if (s.equals("k")) {
                System.exit(0);
            } else if (s.equals("p")) {
                sender.pause = true;
                continue;
            } else if (s.equals("s")) {
                sender.pause = false;
                continue;
            }
        }
    }
}
