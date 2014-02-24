/*
 * 将CFileDescMap、CVarDefMap、CTypedefSet、
 * CUserClassMap、CMsgBodyDefMap中的信息翻译
 * 成具体语言
 */
#ifndef BASE_TRANSLATOR_H_
#define BASE_TRANSLATOR_H_

class CBaseTranslator
{
public:
	CBaseTranslator();
	virtual ~CBaseTranslator();

public:
	virtual void TranslateProtocol() = 0;

protected:
	virtual void AddFileDescToFile();
	virtual void AddVarDefToFile();
	virtual void AddTypeDefToFile();
	virtual void AddUserClassDef();
	virtual void AddMsgBodyDef();
};

#endif //BASE_TRANSLATOR_H_