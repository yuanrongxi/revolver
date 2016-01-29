package revolver.proto;

//import java.nio.ByteOrder;

public class CodecUtil {
	/**
     * Put a character to a {@link DirectBuffer} at the given index.
     *
     * @param buffer to which the value should be written.
     * @param index from which to begin writing.
     * @param value to be be written.
     */
    public static void charPut(final DirectBuffer buffer, final byte value)
    {
        buffer.putByte(value);
    }
    
    /**
     * Put an array into a {@link DirectBuffer} at the given index.
     *
     * @param buffer to which the value should be written.
     * @param index from which to begin writing.
     * @param src to be be written
     * @param offset in the src buffer to write from
     * @param length of the src buffer to copy.
     */
    public static void charsPut(final DirectBuffer buffer, final byte[] src, final int offset, final int length)
    {
        buffer.putBytes(src, offset, length);
    }
}
