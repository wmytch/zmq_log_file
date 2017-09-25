#ifndef __INI_H__
#define __INI_H__
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <string>
#include <string.h>
using namespace std;
using std::string;

#define CONFIGLEN           256

enum INI_RES {
    INI_SUCCESS,                //�ɹ�
    INI_ERROR,                  //��ͨ����
    INI_OPENFILE_ERROR,         //���ļ�ʧ��
    INI_NO_ATTR                 //�޶�Ӧ�ļ�ֵ
};

//              �Ӽ�����    �Ӽ�ֵ 
typedef map < std::string, std::string > KEYMAP;
//              �������� ����ֵ  
typedef map < std::string, KEYMAP > MAINKEYMAP;
// config �ļ��Ļ���������

class CIni {
  public:
    // ���캯��
    CIni();

    // ��������
    virtual ~ CIni();
  public:
    //��ȡ���εļ�ֵ
    int GetInt(const char *mAttr, const char *cAttr);
    //��ȡ��ֵ���ַ���
    char *GetStr(const char *mAttr, const char *cAttr);
    // ��config �ļ�
    INI_RES OpenFile(const char *pathName, const char *type);
    // �ر�config �ļ�
    INI_RES CloseFile();
  protected:
    // ��ȡconfig�ļ�
    INI_RES GetKey(const char *mAttr, const char *cAttr, char *value);
  protected:
    // ���򿪵��ļ��ֱ�
    FILE * m_fp;
    char m_szKey[CONFIGLEN];
    MAINKEYMAP m_Map;
};

#endif                          // !__INI_H__
