package proto_demo;

import revolver.proto.ProtoBuf;

public class SubStruct1 {
public int field1;
public long field2;

void Pack(ProtoBuf proto) {
	int index, new_index;
	index = proto.GetIndex();
	proto.Move(2);
	proto.int32Put(field1);
	proto.int64Put(field2);
	new_index = proto.GetIndex();
	proto.int16Modify((short)(new_index - index - 2), index);	
}

void Unpack(ProtoBuf proto) {
	int size = proto.int16Get();
	ProtoBuf nested_buf;
	size = size + proto.GetIndex();
	nested_buf = new ProtoBuf(proto, size);
	
	field1 = nested_buf.int32Get();
	field2 = nested_buf.int64Get();
}

}
