package proto_demo;

import revolver.proto.ProtoBuf;

public class SubStruct {
public int field1;
public byte[] field2;

public int Pack(ProtoBuf proto)
{
	int index, new_index;
	index = proto.GetIndex();
	proto.Move(2);
	proto.int32Put(field1);
	proto.int16Put((short) field2.length);
	proto.charsPut(field2, 0, field2.length);
	new_index = proto.GetIndex();
	proto.int16Modify((short)(new_index - index - 2), index);
	return 0;
}

public int Unpack(ProtoBuf proto)
{
	int size = proto.int16Get();
	ProtoBuf nested_buf;
	size = size + proto.GetIndex();
	nested_buf = new ProtoBuf(proto, size);
	
	field1 = nested_buf.int32Get();
	size = nested_buf.int16Get();
	field2 = new byte[size];
	nested_buf.charsGet(field2, 0, size);
	return 0;
}


}
