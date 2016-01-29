package proto_demo;

//import java.util.ArrayList; 
//import java.util.Iterator;

import revolver.proto.ProtoBuf;

public class SampleMsg {
	public byte type;
	public DescMsg msg;
	
//public byte field1;
//public short field2;
//public int field3;
//public long field4;
//public float field5;
//public double field6;
//public byte[] field7;
//ArrayList<SubStruct> field8;
//SubStruct1 field9;

public SampleMsg()
{
//	field8 = new ArrayList<SubStruct>();
//	field9 = new SubStruct1();
	msg = new DescMsg();
}

void Pack(ProtoBuf proto)
{
	proto.charPut(type);
	int index, new_index;
	index = proto.GetIndex();
	proto.Move(4);
	msg.Pack(proto);
	new_index = proto.GetIndex();
	proto.int32Modify((short)(new_index - index - 4), index);
}

void Unpack(ProtoBuf proto)
{
	type = proto.int8Get();
	int size = proto.int32Get();
	ProtoBuf nested_buf;
	size = proto.GetIndex() + size;
	nested_buf = new ProtoBuf(proto, size);
	msg.UnPack(nested_buf);
	
//	field1 = proto.int8Get();
//	field2 = proto.int16Get();
//	field3 = proto.int32Get();
//	field4 = proto.int64Get();
//	field5 = proto.floatGet();
//	field6 = proto.doubleGet();
//	int size = proto.int16Get();
//	field7 = new byte[size];
//	proto.charsGet(field7, 0, size);
//	size = proto.int16Get();
//	int i;
//	for(i = 0; i < size; i ++ )
//	{
//		SubStruct sub;
//		sub = new SubStruct();
//		sub.Unpack(proto);		
//		field8.add(sub);
//	}
//	field9.Unpack(proto);
}

}
