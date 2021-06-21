import java.io.Serializable;
import java.util.Date;

public class Neighbour implements Serializable {
    String id;
    Date timestamp;
    int port = 0;
    String next_hop;
    float distance;
    boolean alive = true;
    boolean updated = true;

    Neighbour(String id, int port, float distance){
        this.id = id;
        this.port = port;
        this.distance = distance;
        this.timestamp = new Date();
    }
}
