package pk;

import pk.CRC32;

import java.io.Serializable;
import java.nio.charset.StandardCharsets;

public class MyPacket implements Serializable {
    int sequence_num;
    String data;
    long crc;

    public MyPacket(int sequence_num, String _data){
        super();
        this.sequence_num = sequence_num;
        this.data = _data;
        this.crc = CRC32.getCRC32Checksum(data.getBytes(StandardCharsets.UTF_8));
    }

    public String ConvertToString(){
        return "UDP Packet [seq=" + sequence_num + ", data=" + data + "]";
    }
}
