import java.util.concurrent.TimeUnit;

public class Test {

    public static void main(String[] args) throws InterruptedException {

        // node a
        NodeInfo node = new NodeInfo("a", 7780);
        node.addNeighbour("a", 7780, 0);
        node.addNeighbour("b", 7781, 7);
        node.addNeighbour("c", 7782, 7);
        node.frequency = 5000;


        // b
        NodeInfo node2 = new NodeInfo("b", 7781);
        node2.frequency = 5000;
        node2.addNeighbour("b", 7781, 0);
        node2.addNeighbour("a", 7780, 8);
        node2.addNeighbour("c", 7782, 5);
        node2.addNeighbour("d", 7783, 20);

        // c
        NodeInfo node3 = new NodeInfo("c", 7782);
        node3.frequency = 5000;
        node3.addNeighbour("a", 7780, 7);
        node3.addNeighbour("b", 7781, 3);
        node3.addNeighbour("c", 7782, 0);
        node3.addNeighbour("d", 7783, 1);


        PacketSender senderb = new PacketSender(node2);
        TimeUnit.SECONDS.sleep(1);
        PacketSender sendera = new PacketSender(node);
        TimeUnit.SECONDS.sleep(1);
        PacketSender senderc = new PacketSender(node3);


    }
}
