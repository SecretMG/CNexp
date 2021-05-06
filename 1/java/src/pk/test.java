package pk;

import pk.MyPacket;
import pk.Serializer;

import java.io.IOException;

import static pk.Serializer.serializer;

public class test {
    static MyPacket packet = new MyPacket(10, "hi");
    static byte[] data;

    static {
        try {
            data = serializer.toBytes(packet);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    static MyPacket packet_restore;

    static {
        try {
            packet_restore = (MyPacket) serializer.toObject(data);
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    public test() throws IOException, ClassNotFoundException {
    }

    public static void main(String[] args){
        System.out.println(packet_restore.ConvertToString());
        return;
    }
}
