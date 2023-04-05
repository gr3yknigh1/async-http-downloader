#ifndef YAMLCONFIGREADER_HPP_
#define YAMLCONFIGREADER_HPP_

#include <memory>

#include <yaml-cpp/yaml.h>

#include "ahd/Action.hpp"
#include "ahd/ConfigReader.hpp"
#include "ahd/DownloadAction.hpp"
#include "ahd/UnpackAction.hpp"

class YamlConfigReader : public ConfigReader
{
public:
    TaskMap Read(const std::filesystem::path configPath) override;

private:
    TaskMap MakeTaskMap(const std::string &host, const std::string &target,
                        const YAML::Node filesYaml);

    const std::vector<const char *> FindMissingFields(
        const YAML::Node &node,
        const std::vector<const char *> &requiredFields);

    void ValidateDependency(const std::string &name, const TaskMap &taskMap,
                            const std::string &dependency);
    void ValidateDependencies(const TaskMap &taskMap);
    void ValidateConfigYaml(const YAML::Node &configYaml);
    void ValidateFileYaml(uint64_t index, const YAML::Node &fileYaml);

    std::vector<std::shared_ptr<Action>> DispatchActionsYaml(
        uint64_t index, const std::string &host, const std::string &target,
        std::shared_ptr<Task> task, const YAML::Node actionsYaml);

    std::vector<std::string> DispatchDependenciesYaml(
        const YAML::Node dependenciesYaml);
};

#endif // YAMLCONFIGREADER_HPP_
