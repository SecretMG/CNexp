package pk;

public class Args {
    int port1;
    int port2;
    int port3;
    int port4;
    int DataSize;
    double ErrorRate;
    double LostRate;
    int SWSize;
    byte InitSeqNo;
    int Timeout;
    public Args(){
        port1 = 7890;
        port2 = 7891;
        port3 = 7890;
        port4 = 7891;
        DataSize = 4096;
        ErrorRate = 0.01;
        LostRate = 0.01;
        SWSize = 5;
        InitSeqNo = 0;
        Timeout = 100;
    }

}
