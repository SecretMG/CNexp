
import java.io.Serializable;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class MyPacket implements Serializable {
    String id;  // sender id
    int sequence_num;   // packet sequence number
    Map<String, Integer> table; // id->dis table of sender

    public MyPacket(String id, int sequence_num, Map<String, Integer> table){
        super();
        this.id = id;
        this.sequence_num = sequence_num;
        this.table = table;
    }

    public String ConvertToString(){
        return "UDP Packet [seq=" + sequence_num + ", id=" + id + "]";
    }
}
