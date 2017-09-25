#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#include <string>

using std::string;

class CServerConfig {
  public:
    static CServerConfig & GetConfig();
  private:
    CServerConfig();
    ~CServerConfig();
  public:

	void SetConfigFile(const string & file_path){m_file_path=file_path;}
	void GetConfigFromFile();

	const string &GetProxyProto(){return m_proxy_proto;}
    const string & GetProxyIP(){return m_proxy_ip;};
    unsigned short GetProxyPort(){return m_proxy_port;};
	unsigned long GetLogFileSize(){return m_log_file_size*1024*1024;}


  private:
	 static CServerConfig s_ServerConfig;

 	string m_file_path;
	string m_proxy_proto;
    string m_proxy_ip;
    unsigned short m_proxy_port;
	unsigned long m_log_file_size;
};
#endif                          // __SERVER_CONFIG_H__
