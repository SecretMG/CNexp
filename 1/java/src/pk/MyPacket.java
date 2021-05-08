package pk;

import pk.CRC;

import java.io.Serializable;
import java.nio.charset.StandardCharsets;

public class MyPacket implements Serializable {
    int sequence_num;
    byte ackk;
    String data;

    public MyPacket(int sequence_num, byte ack, String _data){
        super();
        this.sequence_num = sequence_num;
        this.ackk = ack;
        this.data = _data;
    }

    public String ConvertToString(){
        return "UDP Packet [seq=" + sequence_num + ", data=" + data + "]";
    }
}
