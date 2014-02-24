#include <iostream>
#include "file_handler.h"
#include "cpp_translator.h"
#include "base_error.h"
#include "Shlwapi.h"

void About();
void TestParam(string& argv);
void CreateGlobalObj();
void DestoryGlobalObj();

int main(int argc, char* argv[])
{
 	string file_path;

	switch (argc)
	{
	case 2:
		if (argv[1] == "-h")
		{
			About();
			return 0;
		}
		else
		{
			cout << "you need specify object language" << endl;
			return 0;
		}
		break;
	case 3:
		try
		{
			TestParam(string(argv[2]));
			file_path = argv[1];
		}
		catch(const string& e)
		{
			cout << "Error: " << e << endl;
			About();
			return 0;
		}
		break;
	default:
		About();
		return 0;
		break;
	}

	CreateGlobalObj();

	CFileHandler filehandler(file_path);
	filehandler.AnalysisFile();

	if (!filehandler.IsOccurError())
	{
		CBaseTranslator* p_Translate = GenerateTranslator(string(argv[2]));
		p_Translate->TranslateProtocol();
		delete p_Translate;
		cout << "complete!" << endl;
	}

	DestoryGlobalObj();
	system("pause");
	return 0;
}

void About()
{
	cout << "This program can translate .def file to object language file." << endl;
	cout << "Manual:" << endl;
	cout << "Translate.exe Filename.def c++/cpp/lua/as/..."  << endl;
}

void TestParam(string& argv)
{
	if (argv != "c++" && argv != "cpp" && argv != "lua" && argv != "as")
	{
		throw string("Unsupport object language");
	} 
}

void CreateGlobalObj()
{
	CREATE_DATATYPESET();
	CREATE_KEYWORDSET();
	CREATE_BLOCKHANDLERMAP();
	CREATE_USERCLASSMAP();
	CREATE_TYPEDEFSET();
	CREATE_FILEDESCMAP();
	CREATE_VARDEFMAP();
	CREATE_MSGDEFMAP();
	CREATE_TRANSTABLE();
}

void DestoryGlobalObj()
{
	DESTORY_DATATYPESET();
	DESTORY_KEYWORDSET();
	DESTORY_BLOCKHANDLERMAP();
	DESTORY_USERCLASSMAP();
	DESTORY_TYPEDEFSET();
	DESTORY_FILEDESCMAP();
	DESTORY_VARDEFMAP();
	DESTORY_MSGDEFMAP();
	DESTORY_TRANSTABLE();
}