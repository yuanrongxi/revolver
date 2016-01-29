package revolver.proto;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Locale;

public class DirectBuffer {
	private static ByteOrder codec_order = ByteOrder.BIG_ENDIAN;
	private byte[] byteArray;
    private int offset;
    private int index;
    private int capacity;
    private ByteBuffer byteBuffer;
    
    static void SetCodecOrder(ByteOrder o)
    {
    	codec_order = o;
    }
    
    public int GetIndex() 
    {
    	return index;
    }
    
    public void SetIndex(int idx)
    {
    	index = idx;
    }
    
    public void Move(int size)
    {
    	checkLimit(index + size);
    	index += size;
    }
    
    /**
     * Attach a view to a byte[] for providing direct access.
     *
     * @param buffer to which the view is attached.
     */
    public DirectBuffer(final byte[] buffer, int off)
    {
        wrap(buffer, off);
    }
    
    public DirectBuffer(final DirectBuffer buf)
    {
    	wrap(buf.array(), buf.offset);
    	index = buf.GetIndex();
    }
    /**
     * Attach a view to a byte[] for providing direct access.
     *
     * @param buffer to which the view is attached.
     */
    public void wrap(final byte[] buffer, int off)
    {
        offset = off;
        index = offset;
        capacity = buffer.length;
        byteArray = buffer;
        byteBuffer = null;
    }
    
    /**
     * Attach a view to a {@link ByteBuffer} for providing direct access, the {@link ByteBuffer} can be
     * heap based or direct.
     *
     * @param buffer to which the view is attached.
     */
    public DirectBuffer(final ByteBuffer buffer)
    {
        wrap(buffer);
    }
    
    /**
     * Attach a view to a {@link ByteBuffer} for providing direct access, the {@link ByteBuffer} can be
     * heap based or direct.
     *
     * @param buffer to which the view is attached.
     */
    public void wrap(final ByteBuffer buffer)
    {
        byteBuffer = buffer;

        if (buffer.hasArray())
        {
            byteArray = buffer.array();
            offset = buffer.arrayOffset();
        }
        capacity = buffer.capacity();
    }
    
    /**
     * Return the underlying byte[] for this buffer or null if none is attached.
     *
     * @return the underlying byte[] for this buffer.
     */
    public byte[] array()
    {
        return byteArray;
    }

    /**
     * Return the underlying {@link ByteBuffer} if one is attached.
     *
     * @return the underlying {@link ByteBuffer} if one is attached.
     */
    public ByteBuffer byteBuffer()
    {
        return byteBuffer;
    }
    
    /**
     * Get the capacity of the underlying buffer.
     *
     * @return the capacity of the underlying buffer in bytes.
     */
    public int capacity()
    {
        return capacity;
    }

    public void setCapacity(int cap)
    {
    	capacity = cap;
    }
    /**
     * Check that a given limit is not greater than the capacity of a buffer from a given offset.
     *
     * Can be overridden in a DirectBuffer subclass to enable an extensible buffer or handle retry after a flush.
     *
     * @param limit access is required to.
     * @throws IndexOutOfBoundsException if limit is beyond buffer capacity.
     */
    public void checkLimit(final int limit)
    {
        if (limit > capacity)
        {
            final String msg = String.format(
                Locale.US,
                "limit=%d is beyond capacity=%d",
                Integer.valueOf(limit),
                Integer.valueOf(capacity));

            throw new IndexOutOfBoundsException(msg);
        }
    }

    public void info() {
    	final String msg = String.format(
                Locale.US,
                "index=%d, capacity=%d",
                Integer.valueOf(index),
                Integer.valueOf(capacity));	
    	System.out.println(msg);
    }
    
    /**
     * Put a 8-bit signed integer to a {@link DirectBuffer} at the given index.
     *
     * @param buffer to which the value should be written.
     * @param index from which to begin writing.
     * @param value to be be written.
     * @throws IllegalArgumentException if the number is negative
     */
    public int uint8Put(final short value)
    {
        putByte((byte)value);
        index += 1;
        return 1;
    }
        
    /**
     * Create a duplicate {@link ByteBuffer} for the view in native byte order.
     * The duplicate {@link ByteBuffer} shares the underlying memory so all changes are reflected.
     * If no {@link ByteBuffer} is attached then one will be created.
     *
     * @return a duplicate of the underlying {@link ByteBuffer}
     */
    public ByteBuffer duplicateByteBuffer()
    {
        final ByteBuffer duplicate;

        if (null == byteBuffer)
        {
            duplicate = ByteBuffer.wrap(byteArray);
        }
        else
        {
            duplicate = byteBuffer.duplicate();
        }

        duplicate.clear();

        return duplicate;
    }

    /**
     * Get the value at a given index.
     *
     * @param index in bytes from which to get.
     * @return the value at a given index.
     */
    public byte getByte()
    {
        if (byteArray != null)
        {
        	if (index + 1 > capacity)
        		return 0;
        	byte ret = byteArray[offset + index];
        	index += 1;
        	return ret;
        } else 
        {
        	return 0;
        }
    }

    /**
     * Put a value to a given index.
     *
     * @param index in bytes for where to put.
     * @param value to be written
     */
    public void putByte(final byte value)
    {
        if (byteArray != null)
        {
        	checkLimit(index + 1);
        	
            byteArray[offset + index] = value;
            index += 1;
        }
    }
    
    /**
     * Get from the underlying buffer into a supplied byte array.
     * This method will try to fill the supplied byte array.
     *
     * @param index in the underlying buffer to start from.
     * @param dst   into which the dst will be copied.
     * @return count of bytes copied.
     */
    public int getBytes(final byte[] dst)
    {
    	int ret = getBytes(dst, 0, dst.length);
    	return ret;
    }
    
    /**
     * Get bytes from the underlying buffer into a supplied byte array.
     *
     * @param index  in the underlying buffer to start from.
     * @param dst    into which the bytes will be copied.
     * @param offset in the supplied buffer to start the copy
     * @param length of the supplied buffer to use.
     * @return count of bytes copied.
     */
    public int getBytes(final byte[] dst, final int offset, final int length)
    {
        int count = Math.min(length, capacity - index);
        count = Math.min(count, dst.length - offset);

        if (byteArray != null)
        {
        	checkLimit(index + count);
            System.arraycopy(byteArray, this.offset + index, dst, offset, count);
            index += count;
        } else {
        	return 0;
        }        

        return count;
    }
    
    /**
     * Put an array of src into the underlying buffer.
     *
     * @param index in the underlying buffer to start from.
     * @param src   to be copied to the underlying buffer.
     * @return count of bytes copied.
     */
    public int putBytes(final byte[] src)
    {
        return putBytes(src, 0, src.length);
    }

    /**
     * Put an array into the underlying buffer.
     *
     * @param index  in the underlying buffer to start from.
     * @param src    to be copied to the underlying buffer.
     * @param offset in the supplied buffer to begin the copy.
     * @param length of the supplied buffer to copy.
     * @return count of bytes copied.
     */
    public int putBytes(final byte[] src, final int offset, final int length)
    {
        int count = Math.min(length, capacity - index);
        count = Math.min(count, src.length - offset);

        checkLimit(index + count);
        if (byteArray != null)
        {
            System.arraycopy(src, offset, byteArray, this.offset + index, count);
            index += count;
        }
        else
        {
            return 0;
        }

        return count;
    }

    /**
     * Put an bytes into the underlying buffer for the view.  Bytes will be copied from current
     * {@link java.nio.ByteBuffer#position()} to {@link java.nio.ByteBuffer#limit()}.
     *
     * @param index     in the underlying buffer to start from.
     * @param srcBuffer to copy the bytes from.
     * @param length    of the source buffer in bytes to copy
     * @return count of bytes copied.
     */
    public int putBytes(final ByteBuffer srcBuffer, final int length)
    {
        int count = Math.min(length, capacity - index);
        count = Math.min(count, srcBuffer.remaining());
        
        checkLimit(index + count);
        if (byteArray != null)
        {
            putBytesToByteArray(index, srcBuffer, count);
        }
        else
        {
            return 0;
        }

        srcBuffer.position(srcBuffer.position() + count);

        return count;
    }
    /**
     * Get the value at a given index.
     *
     * @param index     in bytes from which to get.
     * @param byteOrder of the value to be read.
     * @return the value for at a given index
     */
    public long getLong()
    {
        if (byteArray != null)
        {
        	if (index + 8 > capacity)
        		return 0;
        	long ret = Memory.peekLong(byteArray, offset + index, codec_order);
        	index += 8;
        	return ret;
        } else {
        	return 0;
        }
    }
    
    /**
     * Put a value to a given index.
     *
     * @param index     in bytes for where to put.
     * @param value     for at a given index
     * @param byteOrder of the value when written
     */
    public void putLong(final long value)
    {
        if (byteArray != null)
        {
        	checkLimit(index + 8);
            Memory.pokeLong(byteArray, offset + index, value, codec_order);
            index += 8;
        }
    }
    
    /**
     * Put a value to a given index.
     *
     * @param index     in bytes for where to put.
     * @param value     to be written
     * @param byteOrder of the value when written.
     */
    public void putShort(final short value)
    {
        if (byteArray != null)
        {
        	checkLimit(index + 2);
            Memory.pokeShort(byteArray, offset + index, value, codec_order);
            index += 2;
        }
        else
        {
            return;
        }
    }
    
    public void modifyShort(final short value, int idx)
    {
    	if (byteArray != null)
        {
        	checkLimit(idx + 2);
            Memory.pokeShort(byteArray, offset + idx, value, codec_order);
        }
        else
        {
            return;
        }
    }
    /**
     * Get the value at a given index.
     *
     * @param index     in bytes from which to get.
     * @param byteOrder of the value to be read.
     * @return the value at a given index.
     */
    public short getShort()
    {
        if (byteArray != null)
        {
        	if (index + 2 > capacity)
        		return 0;
        	short ret = Memory.peekShort(byteArray, offset + index, codec_order);
            index += 2;
            return ret;
        }
        else
        {
            return 0;
        }
    }
    
    /**
     * Get the value at a given index.
     *
     * @param index     in bytes from which to get.
     * @param byteOrder of the value to be read.
     * @return the value at a given index.
     */
    public int getInt()
    {
        if (byteArray != null)
        {
        	if (index + 4 > capacity)
        		return 0;
        	int ret = Memory.peekInt(byteArray, offset + index, codec_order);
        	index += 4;
        	return ret;
        }
        else
        {
            return 0;
        }
    }

    public void modifyInt(final int value, int idx)
    {
    	if (byteArray != null)
        {
        	checkLimit(idx + 4);
            Memory.pokeInt(byteArray, offset + idx, value, codec_order);
        }
        else
        {
            return;
        }
    }
    
    /**
     * Put a value to a given index.
     *
     * @param index     in bytes for where to put.
     * @param value     to be written
     * @param byteOrder of the value when written
     */
    public void putInt(final int value)
    {
        if (byteArray != null)
        {
        	checkLimit(index + 4);
            Memory.pokeInt(byteArray, offset + index, value, codec_order);
            index += 4;
        }
        else
        {
            return;
        }
    }
    
    /**
     * Get the value at a given index.
     *
     * @param index     in bytes from which to get.
     * @param byteOrder of the value to be read.
     * @return the value at a given index.
     */
    public double getDouble()
    {
    	double ret = Double.longBitsToDouble(getLong());
    	return ret;
    }

    /**
     * Put a value to a given index.
     *
     * @param index     in bytes for where to put.
     * @param value     to be written
     * @param byteOrder of the value when written.
     */
    public void putDouble(final double value)
    {
        putLong(Double.doubleToRawLongBits(value));
    }
    
    /**
     * Get the value at a given index.
     *
     * @param index     in bytes from which to get.
     * @param byteOrder of the value to be read.
     * @return the value at a given index.
     */
    public float getFloat()
    {
        return Float.intBitsToFloat(getInt());
    }

    /**
     * Put a value to a given index.
     *
     * @param index     in bytes for where to put.
     * @param value     to be written
     * @param byteOrder of the value when written.
     */
    public void putFloat(final float value)
    {
        putInt(Float.floatToIntBits(value));
    }
    
    private void putBytesToByteArray(final int index, final ByteBuffer srcBuffer, final int count)
    {
        if (srcBuffer.hasArray())
        {
            final byte[] src = srcBuffer.array();
            final int srcOffset = srcBuffer.arrayOffset() + srcBuffer.position();
            System.arraycopy(src, srcOffset, byteArray, this.offset + index, count);
        }        
    }
}
