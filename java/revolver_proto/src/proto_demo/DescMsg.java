package proto_demo;

import revolver.proto.ProtoBuf;
import java.util.ArrayList; 

public class DescMsg {
	public byte type;
	public String desc_string;
	ArrayList<Integer> fd_array;
	public ArrayList<Byte> status_array;
	
	public DescMsg() {
		fd_array = new ArrayList<Integer>();
		status_array = new ArrayList<Byte>();
	}
	void Pack(ProtoBuf proto) {
		proto.charPut(type);
		byte[] tmp_bytes;
		tmp_bytes = desc_string.getBytes();
		int item_cnt = tmp_bytes.length;
		proto.int32Put(item_cnt);
		proto.charsPut(tmp_bytes, 0, item_cnt);
		
		item_cnt = fd_array.size();
		proto.int32Put(item_cnt);
		for(int i = 0; i < item_cnt; i ++) {
			proto.int32Put(fd_array.get(i));
		}
		
		item_cnt = status_array.size();
		proto.int32Put(item_cnt);
		for(int i = 0; i < item_cnt; i ++) {
			proto.charPut(status_array.get(i));
		}
	}
	
	void UnPack(ProtoBuf proto) {
		type = proto.int8Get();
		int size;
		size = proto.int32Get();
		byte[] array = new byte[size];
		proto.charsGet(array, 0, size);
		desc_string = new String(array);
		
		size = proto.int32Get();
		for(int i= 0; i < size; i ++) {
			int val = proto.int32Get();
			fd_array.add(val);
		}
		
		size = proto.int32Get();
		for(int i = 0; i < size; i++) {
			byte val = proto.int8Get();
			status_array.add(val);
		}
	}
}
