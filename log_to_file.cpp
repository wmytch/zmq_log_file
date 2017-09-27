//
//   Request-reply service in C++
//   Connects REP socket to tcp://localhost:5560
//   Expects "Hello" from client, replies with "World"
//
// Olivier Chamoux <olivier.chamoux@fr.thalesgroup.com>
#include <sstream>
#include <pthread.h>
#include "ServerConfig.h"
#include "zhelpers.hpp"
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP 

el::Configurations defaultConf;

struct ParsedMsg{
	string address;
	string module;
	string pid;
	string message;
};

bool parser(std::string &recv_msg,ParsedMsg &parsed_msg)
{
	std::vector<std::string> v;
	std::size_t pos1=0;
	std::size_t pos2=recv_msg.find("|");
	while(pos2!=std::string::npos){
		v.push_back(recv_msg.substr(pos1, pos2-pos1));
    	pos1 = pos2 + 1;
    	pos2 = recv_msg.find("|", pos1);
		if(v.size()==3)
			break;
  	}
  	if(pos1 != recv_msg.length()){
    	v.push_back(recv_msg.substr(pos1));
	}
	if(v.size()!=4)
		return false;

	parsed_msg.address=v[0];
	parsed_msg.module=v[1];
	parsed_msg.pid=v[2];
	parsed_msg.message=v[3];

	return true;
}
std::string check_file(const std::stringstream& file_name)
{
	struct stat file_stat;
	int count=0;
	std::string temp_name{file_name.str()};

	while(stat(temp_name.c_str(),&file_stat)==0)
	{
		std::stringstream file_idx;
		file_idx<<file_name.str()<<"."<<++count;
		temp_name=file_idx.str();
	}
	if(--count<=0)
		temp_name=file_name.str();
	else
	{
		std::stringstream file_idx;
		file_idx<<file_name.str()<<"."<<count;
		temp_name=file_idx.str();
	}
	if(stat(temp_name.c_str(),&file_stat)==0)
	{
		if(file_stat.st_size>=CServerConfig::GetConfig().GetLogFileSize())
		{
			std::stringstream file_idx;
			file_idx<<file_name.str()<<"."<<++count;
			return file_idx.str();
		}
	}
	return temp_name;
}
static void *logger_func(void *arg)
{
	LOG(INFO)<<"Logger thread begin.";
	zmq::context_t *context=static_cast<zmq::context_t*>(arg);
	zmq::socket_t receiver(*context,ZMQ_PAIR);
	receiver.bind("inproc://logger");
	
	LOG(INFO)<<"Begin logging...";

	string current_file{""};
	while(true){
		ParsedMsg parsed_msg{"","","",""};
        std::string recv_msg = s_recv(receiver);
		if(parser(recv_msg,parsed_msg)){
    		std::stringstream ori_log_file;
			ori_log_file<<"./logs/"<<parsed_msg.address<<"/"<<parsed_msg.module<<"."<<parsed_msg.pid;
			std::string log_file_name=check_file(ori_log_file);
			if(current_file!=log_file_name){
				defaultConf.setGlobally(el::ConfigurationType::Filename, log_file_name); 
				el::Loggers::reconfigureLogger("default", defaultConf);
				current_file=log_file_name;
			}
			LOG(INFO)<<parsed_msg.message;
		}
	}
}

int main(int argc, char *argv[])
{
	defaultConf.setToDefault();
	
    zmq::context_t context(1);
    zmq::socket_t receiver(context, ZMQ_SUB);

    CServerConfig::GetConfig().SetConfigFile("./config.ini");
    CServerConfig::GetConfig().GetConfigFromFile();
/*
	std::stringstream max_log_file_size;
	max_log_file_size<< CServerConfig::GetConfig().GetLogFileSize();
	el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
	printf("max log file size:%s\n",max_log_file_size.str().c_str());
	defaultConf.setGlobally(el::ConfigurationType::MaxLogFileSize,max_log_file_size.str()); 
*/
	defaultConf.setGlobally(el::ConfigurationType::Format, "%msg"); 
	defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "false"); 
	el::Loggers::reconfigureLogger("default", defaultConf);

    std::stringstream proxy_addr;
    proxy_addr << CServerConfig::GetConfig().
      GetProxyProto() << "://" << CServerConfig::GetConfig().
      GetProxyIP() << ":" << CServerConfig::GetConfig().GetProxyPort();


    receiver.connect(proxy_addr.str());
	LOG(INFO)<<"connect to Proxy["<<proxy_addr.str()<<"] success.";
    receiver.setsockopt(ZMQ_SUBSCRIBE, "", 0);

 	pthread_t logger_thread;
    pthread_create (&logger_thread, NULL, logger_func,&context);	
	LOG(INFO)<<"Create logger thread.";
	
	zmq::socket_t logger(context,ZMQ_PAIR);
	logger.connect("inproc://logger");
	LOG(INFO)<<"Connect to logger.";
	
    while (1) {
        std::string msg = s_recv(receiver);
        msg = s_recv(receiver);
		s_send(logger,msg);
		
    }
}

