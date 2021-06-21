import java.io.BufferedWriter;
import java.io.IOException;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

public class NodeInfo {
    String id; // Node id
    int frequency;
    int unreachable = 100;
    int port;
    BufferedWriter LogDoc = null;

    Map<String, Neighbour> neighbours = new HashMap<>(); // neighbour id->Neighbour

    // initialize node
    NodeInfo(String id, int port,BufferedWriter writer){
        this.id = id;
        this.port = port;
        this.LogDoc = writer;
    }

    public void addNeighbour(String id, int port, float distance){
        Neighbour neighbour = new Neighbour(id, port, distance);
        neighbour.timestamp = new Date();
        neighbours.put(id, neighbour);

    }

    // put received info into current node
    public void putRoutingInfo(String sender_id, String dest_id, float distance){

        if(neighbours.containsKey(dest_id)){
            // is neighbour node
            Neighbour dest = neighbours.get(dest_id);
//             dest.updated = false;

            // unreachable
            if(distance >= unreachable){
//                System.out.println(sender_id + " to " + dest_id + " unreachable!");
                dest.alive = false;
                dest.distance = Float.MAX_VALUE;
                dest.updated = true;
                neighbours.put(dest_id, dest);
                return;
            }
            float old = dest.distance; // old distance
            String old_hop = dest.next_hop; // old neighbour hop to destination

            // new hop
            String new_hop = sender_id;
            if(neighbours.containsKey(sender_id) && neighbours.get(sender_id).next_hop != null){
                new_hop = neighbours.get(sender_id).next_hop;
            }

            if(distance < old || (distance > old && old_hop != null && old_hop.equals(sender_id))){
                // find better route or update old route
                dest.updated = true;
                dest.distance = distance;
                dest.next_hop = new_hop;
            }
//            else if(sender_id.equals(id) && distance < old){ // self -> ?
//                dest.distance = distance;
//                dest.next_hop = null;
//            }
//            System.out.println("better: " + id + " -> " + dest_id + " " + distance);
            neighbours.put(dest_id, dest);

        } else {
            // dest_id not in table, add it
            Neighbour neighbour_new = new Neighbour(dest_id, 0, distance);
            neighbour_new.next_hop = sender_id;
            neighbours.put(dest_id, neighbour_new);
        }
    }



    public float getDis(String id){
        if(neighbours.containsKey(id)){
            return neighbours.get(id).distance;
        }
        else{
            return 1000;
        }
    }

    public byte[] generatePacket(int seq_num) throws IOException {
        MyPacket packet = new MyPacket(id, seq_num, neighbours);
        return Serializer.toBytes(packet);
    }

    public void printTable() throws IOException {
//        System.out.println("table of node " + id);
        for(Map.Entry<String, Neighbour> set: neighbours.entrySet()){
            String dest_id = set.getKey();
            float dest_dis = set.getValue().distance;
            if (set.getValue().alive) {
                String SendInfo = "DestNode = " + dest_id + "; Distance = " + dest_dis + "; Neighbor = " + set.getValue().next_hop;
                System.out.println(SendInfo);

                LogDoc.write(SendInfo);
                LogDoc.newLine();
                LogDoc.flush();
            } else {
                String SendInfo = "DestNode = " + dest_id + " has not been alive";
                System.out.println(SendInfo);

                LogDoc.write(SendInfo);
                LogDoc.newLine();
                LogDoc.flush();
            }
        }
    }

}
