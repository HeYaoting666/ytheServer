#pragma once

#include <string>
#include <tinyxml/tinyxml.h>

#define READ_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name);

#define READ_STR_FROM_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name); \
  if (!name##_node|| !name##_node->GetText()) { \
    printf("Failed to read config file %s\n", #name); \
    exit(0); \
  } \
std::string name##_str = std::string(name##_node->GetText()); \


namespace ythe {

class Config {
public:
    bool           mIsPrintLog;
    std::string    mLogLevel;
    std::string    mLogFileName;
    std::string    mLogFilePath;
    int            mLogMaxFileSize;
    int            mLogSyncInterval;

    std::string    mServerIp;
    int            mServerPort;
    int            mSeverBufferSize;
    int            mIOThreadNums;

    std::string    mConnectIp;
    int            mConnectPort;
    int            mClientBufferSize;

    TiXmlDocument* mXmlDocument;

public:
    static Config* GetInstance() {
        static Config instance;
        return &instance;
    }

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    void Init(const char* xmlFile);

private:
    Config() = default;
    ~Config();
};

} // namespace ythe