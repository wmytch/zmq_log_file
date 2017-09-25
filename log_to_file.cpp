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
  	}
/*
  	if(pos1 != recv_msg.length()){
    	v.push_back(recv_msg.substr(pos1));
	}
*/
	if(v.size()!=3)
		return false;

	parsed_msg.message=v[0];
	parsed_msg.address=v[1];
	parsed_msg.module=v[2];

	return true;
}

static void *logger_func(void *arg)
{
	LOG(INFO)<<"Logger thread begin.";
	zmq::context_t *context=static_cast<zmq::context_t*>(arg);
	zmq::socket_t receiver(*context,ZMQ_PAIR);
	receiver.bind("inproc://logger");
	
	LOG(INFO)<<"Begin logging...";

	std::stringstream max_log_file_size;
	max_log_file_size<< CServerConfig::GetConfig().GetLogFileSize();
	defaultConf.setGlobally(el::ConfigurationType::MaxLogFileSize,max_log_file_size.str()); 
	defaultConf.setGlobally(el::ConfigurationType::Format, "%msg"); 
	defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "true"); 
	el::Loggers::reconfigureLogger("default", defaultConf);
	string current_file{""};
	while(true){
		ParsedMsg parsed_msg{"","",""};
        std::string recv_msg = s_recv(receiver);
		if(parser(recv_msg,parsed_msg)){
    		std::stringstream log_file;
			log_file<<"./logs/"<<parsed_msg.address<<"/"<<parsed_msg.module;
			if(current_file!=log_file.str()){
				defaultConf.setGlobally(el::ConfigurationType::Filename, log_file.str()); 
				el::Loggers::reconfigureLogger("default", defaultConf);
				current_file=log_file.str();
			}
			LOG(INFO)<<parsed_msg.message<<log_file.str();
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

