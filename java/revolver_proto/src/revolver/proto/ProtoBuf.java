package revolver.proto;

public class ProtoBuf {
	protected byte[] buffer;
	protected int offset;
	protected int capacity;
	protected DirectBuffer direct_buf;

	public ProtoBuf() {
		buffer = new byte[8196];
		capacity = 8196;
		offset = 0;
		direct_buf = new DirectBuffer(buffer, offset);
	}

	public ProtoBuf(final ProtoBuf buf) {
		buffer = buf.buffer;
		capacity = buf.capacity;
		offset = buf.offset;
		direct_buf = new DirectBuffer(buf.direct_buf);
		capacity = buf.capacity;
	}
	
	public ProtoBuf(final ProtoBuf buf, int cap) {
		buffer = buf.buffer;
		capacity = buf.capacity;
		offset = buf.offset;
		direct_buf = new DirectBuffer(buf.direct_buf);
		if (cap > 0)
			direct_buf.setCapacity(cap);
	}

	public int GetIndex() {
		return direct_buf.GetIndex();
	}

	public void SetIndex(int idx) {
		direct_buf.SetIndex(idx);
	}

	public byte[] GetBuffer() {
		return buffer;
	}
	
	public int GetSize() {
		return direct_buf.GetIndex() - offset;
	}

	public void Move(int size) {
		direct_buf.Move(size);
	}

	/**
	 * Put a character to a {@link DirectBuffer} at the given index.
	 *
	 * @param index
	 *            from which to begin writing.
	 * @param value
	 *            to be be written.
	 */
	public void charPut(final byte value) {
		direct_buf.putByte(value);
	}

	/**
	 * Put an array into a {@link DirectBuffer} at the given index.
	 *
	 * @param buffer
	 *            to which the value should be written.
	 * @param index
	 *            from which to begin writing.
	 * @param src
	 *            to be be written
	 * @param offset
	 *            in the src buffer to write from
	 * @param length
	 *            of the src buffer to copy.
	 */
	public void charsPut(final byte[] src, final int offset, final int length) {
		direct_buf.putBytes(src, offset, length);
	}

	/**
	 * Get from a {@link DirectBuffer} at a given index into an array.
	 *
	 * @param dst
	 *            into which the copy will occur
	 * @param offset
	 *            at which to start in the destination
	 * @param length
	 *            of the array to copy
	 */
	public void charsGet(final byte[] dst, final int offset, final int length) {
		direct_buf.getBytes(dst, offset, length);
	}

	/**
	 * Put a 8-bit integer to a {@link DirectBuffer} at the given index.
	 *
	 * @param index
	 *            from which to begin writing.
	 * @param value
	 *            to be be written.
	 */
	public void int8Put(final byte value) {
		direct_buf.putByte(value);
	}

	/**
	 * Get a 8-bit integer from a {@link DirectBuffer} at a given index.
	 *
	 * @return the byte representation of the value
	 */
	public byte int8Get() {
		return direct_buf.getByte();
	}

	/**
	 * Put a 16-bit integer to a {@link DirectBuffer} at the given index.
	 *
	 * @param buffer
	 *            to which the value should be written.
	 * @param index
	 *            from which to begin writing.
	 * @param value
	 *            to be be written.
	 * @param byteOrder
	 *            for the buffer encoding
	 */
	public void int16Put(final short value) {
		direct_buf.putShort(value);
	}

	public void int16Modify(final short value, int index) {
		direct_buf.modifyShort(value, index);
	}

	/**
	 * Get a 16-bit integer from a {@link DirectBuffer} at a given index.
	 *
	 * @return the short representation of the value
	 */
	public short int16Get() {
		return direct_buf.getShort();
	}

	/**
	 * Put a 32-bit integer to a {@link DirectBuffer} at the given index.
	 *
	 * @param index
	 *            from which to begin writing.
	 * @param value
	 *            to be be written.
	 * @param byteOrder
	 *            for the buffer encoding
	 */
	public void int32Put(final int value) {
		direct_buf.putInt(value);
	}

	/**
	 * Get a 32-bit integer from a {@link DirectBuffer} at a given index.
	 *
	 * @return the int representation of the value
	 */
	public int int32Get() {
		return direct_buf.getInt();
	}

	public void int32Modify(final short value, int index) {
		direct_buf.modifyInt(value, index);
	}
	/**
	 * Get a 64-bit integer from a {@link DirectBuffer} at a given index.
	 *
	 * @return the long representation of the value
	 */
	public long int64Get() {
		return direct_buf.getLong();
	}

	/**
	 * Put a 64-bit integer to a {@link DirectBuffer} at the given index.
	 *
	 * @param value
	 *            to be be written.
	 */
	public void int64Put(final long value) {
		direct_buf.putLong(value);
	}

	/**
	 * Get a 32-bit float from a {@link DirectBuffer} at a given index.
	 *
	 * @return the float representation of the value
	 */
	public float floatGet() {
		return direct_buf.getFloat();
	}

	/**
	 * Put a float to a {@link DirectBuffer} at the given index.
	 *
	 * @param value
	 *            to be be written.
	 */
	public void floatPut(final float value) {
		direct_buf.putFloat(value);
	}

	/**
	 * Put a double to a {@link DirectBuffer} at the given index.
	 *
	 * @param value
	 *            to be be written.
	 */
	public void doublePut(final double value) {
		direct_buf.putDouble(value);
	}

	/**
	 * Get a 64-bit double from a {@link DirectBuffer} at a given index.
	 *
	 * @return the double representation of the value
	 */
	public double doubleGet() {
		return direct_buf.getDouble();
	}

	public void info() {
		direct_buf.info();
	}

	public void dump() {
		int i, index;
		index = direct_buf.GetIndex();
		String out_string = "";
		String val;
		val = String.format("offset: %d, index: %d", offset, index);
		out_string += val;
		for (i = offset; i < index; i++) {
			if (i % 16 == 0) {
				val = String.format("\n%03d: ", i / 16);
				out_string += val;
			}
			val = String.format("%02x ", buffer[i]);
			out_string += val;
			if (i % 8 == 7)
				out_string += "    ";
		}
		System.out.println(out_string);
	}
}
