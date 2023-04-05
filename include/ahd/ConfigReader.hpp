#ifndef CONFIGREADER_HPP_
#define CONFIGREADER_HPP_

#include "ahd/Task.hpp"
#include <filesystem>

class ConfigReader
{
public:
    ConfigReader()
    {
    }

    virtual ~ConfigReader()
    {
    }

    virtual TaskMap Read(const std::filesystem::path &configPath) = 0;

protected:
    inline static const char *s_ConfigHostField = "host";
    inline static const char *s_ConfigTargetField = "target";
    inline static const char *s_ConfigFilesField = "files";
    inline static const std::vector<const char *> s_RequiredConfigFields = {
        s_ConfigHostField, s_ConfigTargetField, s_ConfigFilesField};

    inline static const char *s_FileNameField = "name";
    inline static const char *s_FileFileField = "file";
    inline static const char *s_FileActionsField = "actions";
    inline static const char *s_FileDependenciesField = "dependencies";
    inline static const std::vector<const char *> s_RequiredFileFields = {
        s_FileNameField, s_FileFileField, s_FileActionsField};
};

#endif // CONFIGREADER_HPP_
