#ifndef __STAT_PACKET_H_
#define __STAT_PACKET_H_

#include <iostream>
#include <fstream>
#include <string>

#include <stdio.h>
#ifdef WIN32
#include <direct.h>
#endif
using namespace std;

#ifndef MAX_PATH
#define MAX_PATH	1024
#endif

class StatPacketObj
{
public:
	StatPacketObj()
	{
		m_n20MSCount = 0;
		m_n40MSCount = 0;
		m_n60MSCount = 0;
		m_n80MSCount = 0;
		m_n120MsCount = 0;
		m_n200MsCount = 0;
		m_n160MsCount = 0;
		m_n400Mscount = 0;
		m_n600Mscount = 0;
		m_n1000Mscount = 0;
		m_nMaxMsCount = 0;
		m_nTotalCount = 0;

		m_nPreCount = 0;

		char buffer[MAX_PATH]= {0};
#ifdef WIN32
		string path = _getcwd(buffer, MAX_PATH);
		path += "\\raw_socket_info.txt";
#else
		string path ="/home/yuanrx";
		path += "/raw_socket_info.txt";
#endif


		m_file.open(path.c_str(), std::ios_base::trunc|std::ios_base::out);

	};
	~StatPacketObj()
	{
		m_file.close();
	};

	void Stat(int nDelay)
	{
		m_nTotalCount ++;
		m_nPreCount ++;

		if(nDelay < 20)
		{
			m_n20MSCount ++;
		}
		else if(nDelay < 40)
		{
			m_n40MSCount ++;
		}
		else if(nDelay < 60)
		{
			m_n60MSCount ++;
		}
		else if(nDelay < 80)
		{
			m_n80MSCount ++;
		}
		else if(nDelay < 120)
		{
			m_n120MsCount ++;
		}
		else if(nDelay < 160)
		{
			m_n160MsCount ++;
		}
		else if(nDelay < 200)
		{
			m_n200MsCount ++;
		}
		else if(nDelay < 400)
		{
			m_n400Mscount ++;
		}
		else if(nDelay < 600)
		{
			m_n600Mscount ++;
		}
		else if(nDelay < 1000)
		{
			m_n1000Mscount ++;
		}
		else
		{
			m_nMaxMsCount ++;
		}

		if(m_nTotalCount % 100 == 0)
		{
			Print();
		}
	};

	void Print()
	{
		cout << "finished packet count = " << m_nTotalCount << endl;
		if(m_nTotalCount % 1000 == 0)
		{
			m_file << "finished packet count = " << m_nTotalCount << endl;

			m_file << "20ms\t\t" << "40ms\t\t" << "60ms\t\t" << "80ms\t\t" << "120ms\t\t" << "160ms\t\t" << "200ms\t\t" << "400ms\t\t" << "600ms\t\t" << "1000ms\t\t" << ">1000ms" << endl;
			m_file << m_n20MSCount;
			m_file << "\t\t" << m_n40MSCount;
			m_file << "\t\t" << m_n60MSCount;
			m_file << "\t\t" << m_n80MSCount;
			m_file << "\t\t" << m_n120MsCount;
			m_file << "\t\t" << m_n160MsCount;
			m_file << "\t\t" << m_n200MsCount;
			m_file << "\t\t" << m_n400Mscount;
			m_file << "\t\t" << m_n600Mscount;
			m_file << "\t\t" << m_n1000Mscount;
			m_file << "\t\t" << m_nMaxMsCount << endl;

			m_file.flush();
		}
	}

public:
	int m_n20MSCount;
	int m_n40MSCount;
	int m_n60MSCount;
	int m_n80MSCount;
	int m_n120MsCount;
	int m_n160MsCount;
	int m_n200MsCount;
	int m_n400Mscount;
	int m_n600Mscount;
	int m_n1000Mscount;
	int m_nMaxMsCount;
	int	m_nTotalCount;

	int m_nPreCount;
	ofstream		m_file;
};

#endif
