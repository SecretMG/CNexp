package pk;

import java.util.zip.Checksum;

public class CRC32 {
    public static long getCRC32Checksum(byte[] bytes) {
        Checksum crc32 = new java.util.zip.CRC32();
        crc32.update(bytes, 0, bytes.length);
        return crc32.getValue();
    }
}
