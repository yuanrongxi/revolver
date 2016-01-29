package proto_demo;

import java.io.*;

import revolver.proto.ProtoBuf;

//import java.nio.ByteOrder;

public class TestJava {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		int number = 1;
		System.out.println("I have " + number);
		String msg;
		msg = String.format("limit= %04d", 23);
		System.out.println(msg);

		// byte[] bytes = new byte[10];
		// DirectBuffer buf = new DirectBuffer(bytes, 0);
		// int i;
		// buf.putByte((byte)12);
		// buf.putLong(12);
		// buf.info();
		// for(i = 0; i < bytes.length; i ++ )
		// {
		// System.out.println(bytes[i]);
		// }

		ProtoBuf ptr = new ProtoBuf();
		// ptr.charPut((byte)12);
		// ptr.int16Put((short)34);
		// ptr.int32Put(56);
		// ptr.int64Put(78);
		// ptr.int8Put((byte)10);
		// System.out.println("##########");
		// ptr.dump();

		// ProtoBuf ptr_copy = new ProtoBuf(ptr, 128);
		// ptr_copy.int16Modify((short)11, 1);
		// ptr_copy.int16Put((short)16);
		// ptr_copy.dump();
		// ptr.dump();
		int i = 0;
		SampleMsg sample_msg = new SampleMsg();
		sample_msg.type = 1;
		sample_msg.msg.type = 21;
		sample_msg.msg.desc_string = "2222";
		for(i = 0; i < 4; i ++) {
			sample_msg.msg.fd_array.add(31 + i);
		}
		
		for (i = 0; i < 4; i ++) {
			sample_msg.msg.status_array.add((byte)(41+i));
		}
		
//		sample_msg.field1 = 1;
//		sample_msg.field2 = 2;
//		sample_msg.field3 = 3;
//		sample_msg.field4 = 4;
//		sample_msg.field5 = 0.5f;
//		sample_msg.field6 = 0.6;
//		sample_msg.field7 = new byte[10];
//		for (i = 0; i < 10; i++) {
//			sample_msg.field7[i] = (byte) (7 + i + 1);
//		}
//		SubStruct sub = new SubStruct();
//		sub.field1 = 1;
//		sub.field2 = new byte[10];
//
//		sample_msg.field8.add(sub);
//		sample_msg.Pack(ptr);
		
		sample_msg.Pack(ptr);
		ptr.dump();
				
		try {
			FileOutputStream file_out = new FileOutputStream("c:/log/test1.dat");
			byte[] data = ptr.GetBuffer();
			file_out.write(data, 0, ptr.GetSize());
			file_out.close();
		} catch (Exception e)
		{
			e.printStackTrace(); 
		}
		
		//int size = ptr.GetIndex();
		ptr.SetIndex(0);
		SampleMsg msg1 = new SampleMsg();
		msg1.Unpack(ptr);
	}

}
