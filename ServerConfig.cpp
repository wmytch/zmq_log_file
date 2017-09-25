#include "ServerConfig.h"
#include "ini.h"

CServerConfig CServerConfig::s_ServerConfig;

CServerConfig & CServerConfig::GetConfig()
{
    return s_ServerConfig;
}

CServerConfig::CServerConfig()
{
}

CServerConfig::~CServerConfig()
{

}

void CServerConfig::GetConfigFromFile()
{
    if (m_file_path.empty()) {
        return;
    }
    CIni ini;
    if (ini.OpenFile(m_file_path.c_str(), "r") == INI_SUCCESS) {
	m_proxy_proto=ini.GetStr("common","proxy_proto");
        m_proxy_ip= ini.GetStr("common", "proxy_ip");
        m_proxy_port= ini.GetInt("common", "proxy_port");
		m_log_file_size=ini.GetInt("log","log_file_size");
    }

    ini.CloseFile();

}
