import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class NodeInfo {
    String id; // Node id
    int frequency;
    int unreachable;
    int port;

    Map<String, Integer> table = new HashMap<>(); // dest_id->distance
    Map<String, Integer> neighbour = new HashMap<>(); // neighbour id->port
    Map<String, String> route = new HashMap<>();    // dest_id->next_hop_id

    // initialize node
    NodeInfo(String id, int port){
        this.id = id;
        this.port = port;
    }

    public Map<String, Integer> getTable(){
        return table;
    }

    public Map<String, Integer> getNeighbour(){
        return neighbour;
    }

    public void addNeighbour(String id, int port, int distance){
        neighbour.put(id, port);
        table.put(id, distance);
        route.put(id, id);
    }

    // put received info into current node
    public void putRoutingInfo(String sender_id, String dest_id, Integer distance){
        if(neighbour.containsKey(sender_id)){
            // is neighbour node
            if(table.containsKey(dest_id)){
                // dest_id is in table
                if(dest_id.equals(id)) {
                    // dest is current node, update distance(current node -> sender node)
                    if(table.get(sender_id) > distance)
                        table.put(sender_id, distance);
                    return;
                }

                Integer old = table.get(dest_id); // old diatance
                String old_hop = route.get(dest_id); // old neighbour hop to destination
                if(distance < old || sender_id.equals(old_hop)){
                    // find better route or update old route
                    table.put(dest_id, distance);
                    route.put(dest_id, sender_id);
                }
            } else {
                // dest_id not in table, add it
                table.put(dest_id, distance);
                route.put(dest_id, sender_id);
            }
        }

    }

    public Integer getDis(String id){
        return table.get(id);
    }

    public byte[] generatePacket(int seq_num) throws IOException {
        MyPacket packet = new MyPacket(id, seq_num, table);
        return Serializer.toBytes(packet);
    }

    public void printTable(){
        System.out.println("table of node " + id);
        for(Map.Entry<String, Integer> set: table.entrySet()){
            String dest_id = set.getKey();
            Integer dest_dis = set.getValue();
            System.out.println(dest_id + " ----- " + dest_dis);
        }
    }

}
