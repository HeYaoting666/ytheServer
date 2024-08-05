#include "config.h"

namespace ythe {

void Config::Init(const char *xmlFile)
{
    mXmlDocument = new TiXmlDocument();
    bool ret = mXmlDocument->LoadFile(xmlFile);
    if(!ret) {
        printf("Failed to read config file %s, error info[%s] \n", xmlFile, mXmlDocument->ErrorDesc());
        exit(0);
    }

    READ_XML_NODE(root, mXmlDocument)
    READ_XML_NODE(log, root_node)
    READ_XML_NODE(server, root_node)

    // 获取 log 配置信息
    READ_STR_FROM_XML_NODE(log_is_print, log_node)
    READ_STR_FROM_XML_NODE(log_level, log_node)
    READ_STR_FROM_XML_NODE(log_file_name, log_node)
    READ_STR_FROM_XML_NODE(log_file_path, log_node)
    READ_STR_FROM_XML_NODE(log_max_file_size, log_node)
    READ_STR_FROM_XML_NODE(log_sync_interval, log_node)
    mIsPrintLog =  std::stoi(log_is_print_str) == 1 ? true : false;
    mLogLevel = log_level_str;
    mLogFileName = log_file_name_str;
    mLogFilePath = log_file_path_str;
    mLogMaxFileSize = std::stoll(log_max_file_size_str);
    mLogSyncInterval = std::stod(log_sync_interval_str);

    printf("LOG -- CONFIG LEVEL[%s], FILE_NAME[%s], FILE_PATH[%s], MAX_FILE_SIZE[%d B], SYNC_INTERVAL[%d ms]\n",
           mLogLevel.c_str(), mLogFileName.c_str(), mLogFilePath.c_str(), mLogMaxFileSize, mLogSyncInterval);
    
    // 获取 server 配置信息
    READ_STR_FROM_XML_NODE(port, server_node)
    READ_STR_FROM_XML_NODE(io_threads_num, server_node)
    READ_STR_FROM_XML_NODE(ip, server_node)
    mIp = ip_str;
    mPort = std::stoi(port_str);
    mIOThreadNums = std::stoi(io_threads_num_str);
}

Config::~Config()
{ 
    if (mXmlDocument) {
        delete mXmlDocument;
        mXmlDocument = nullptr;
    }
}

}  // namespace ythe
