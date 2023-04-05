#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <HTTPRequest.hpp>
#include <yaml-cpp/yaml.h>

#include "ahd/Action.hpp"
#include "ahd/DownloadAction.hpp"
#include "ahd/Task.hpp"
#include "ahd/TaskRunner.hpp"
#include "ahd/UnpackAction.hpp"

using TaskMap = std::unordered_map<std::string, std::shared_ptr<Task>>;

class ConfigReader
{
public:
    virtual ~ConfigReader()
    {
    }

    virtual TaskMap Read(const std::filesystem::path configPath)
    {
        throw std::exception();
    }

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

class YamlConfigReader : public ConfigReader
{
public:
    TaskMap Read(const std::filesystem::path configPath) override
    {
        const YAML::Node configYaml = YAML::LoadFile(configPath);
        ValidateConfigYaml(configYaml);

        const std::string host =
            configYaml[s_ConfigHostField].as<std::string>();
        const std::string target =
            configYaml[s_ConfigTargetField].as<std::string>();

        return MakeTaskMap(host, target, configYaml[s_ConfigFilesField]);
    }

private:
    TaskMap MakeTaskMap(const std::string &host, const std::string &target,
                        const YAML::Node filesYaml)
    {
        TaskMap taskMap;
        taskMap.reserve(filesYaml.size());

        for (uint64_t i = 0; i < filesYaml.size(); ++i)
        {
            const YAML::Node fileYaml = filesYaml[i];
            ValidateFileYaml(i, fileYaml);

            std::shared_ptr<Task> task = std::make_shared<Task>();
            task->file = fileYaml[s_FileFileField].as<std::string>();
            task->actions = DispatchActionsYaml(i, host, target, task,
                                                fileYaml[s_FileActionsField]);

            if (fileYaml[s_FileDependenciesField])
            {
                task->dependencies =
                    DispatchDependenciesYaml(fileYaml[s_FileDependenciesField]);
            }

            taskMap[fileYaml[s_FileNameField].as<std::string>()] = task;
        }

        ValidateDependencies(taskMap);

        return taskMap;
    }

    const std::vector<const char *> FindMissingFields(
        const YAML::Node &node, const std::vector<const char *> &requiredFields)
    {
        std::vector<const char *> missingFields;
        missingFields.reserve(requiredFields.size());

        for (const char *field : requiredFields)
        {
            if (!node[field])
            {
                missingFields.emplace_back(field);
            }
        }

        missingFields.shrink_to_fit();
        return missingFields;
    }

    void ValidateDependency(const std::string &name, const TaskMap &taskMap,
                            const std::string &dependency)
    {
        const auto &dependencySearch = taskMap.find(dependency);

        if (dependencySearch == taskMap.end())
        {
            std::ostringstream errorMessage;
            errorMessage << "Can't find dependency '" << dependency
                         << "' that '" << name << "' requires";
            throw std::invalid_argument(errorMessage.str());
        }

        if (dependencySearch->first == name)
        {
            std::ostringstream errorMessage;
            errorMessage << "Can't depend on self '" << name << "'";
            throw std::invalid_argument(errorMessage.str());
        }

        const std::shared_ptr<Task> dependencyTask = dependencySearch->second;
        const std::vector<std::string> dependencyTaskDependencies =
            dependencyTask->dependencies;

        if (std::find(dependencyTaskDependencies.begin(),
                      dependencyTaskDependencies.end(),
                      name) != dependencyTaskDependencies.end())
        {
            std::ostringstream errorMessage;
            errorMessage << "Found cirqle dependency between '" << name
                         << "' and '" << dependencySearch->first << "' tasks";
            throw std::invalid_argument(errorMessage.str());
        }
    }

    void ValidateDependencies(const TaskMap &taskMap)
    {
        for (const auto &[name, task] : taskMap)
        {
            for (const std::string &dependency : task->dependencies)
            {
                ValidateDependency(name, taskMap, dependency);
            }
        }
    }

    void ValidateConfigYaml(const YAML::Node &configYaml)
    {
        if (!configYaml.IsMap())
        {
            throw std::invalid_argument("Config isn't a map of fields\n");
        }

        const auto missingFields =
            FindMissingFields(configYaml, s_RequiredConfigFields);
        if (!missingFields.empty())
        {
            std::ostringstream errorMessage;
            errorMessage << "Missing required config fields: ";
            for (const char *field : missingFields)
            {
                errorMessage << field << " ";
            }
            throw std::invalid_argument(errorMessage.str());
        }

        if (!configYaml[s_ConfigFilesField].IsSequence())
        {
            throw std::invalid_argument("'files' field must be a sequence");
        }
    }

    void ValidateFileYaml(uint64_t index, const YAML::Node &fileYaml)
    {
        const std::vector<const char *> missingFields =
            FindMissingFields(fileYaml, s_RequiredFileFields);
        if (!missingFields.empty())
        {
            std::ostringstream errorMessage;
            errorMessage << "Missing required file fields at index " << index
                         << ": ";
            for (const char *missingField : missingFields)
            {
                errorMessage << "'" << missingField << "' ";
            }

            throw std::invalid_argument(errorMessage.str());
        }
    }

    std::vector<Action *> DispatchActionsYaml(uint64_t index,
                                              const std::string &host,
                                              const std::string &target,
                                              std::shared_ptr<Task> task,
                                              const YAML::Node actionsYaml)
    {
        std::vector<Action *> actions;
        actions.reserve(actionsYaml.size());

        for (const YAML::Node actionYaml : actionsYaml)
        {
            const std::string actionString = actionYaml.as<std::string>();

            if (actionString == "download")
            {
                // TODO: Add option for working directory
                actions.push_back(
                    new DownloadAction(host + target + task->file, task->file));
            }
            else if (actionString == "unpack")
            {
                // TODO: Add option for unpack directory
                actions.push_back(new UnpackAction(task->file, "."));
            }
            else
            {
                std::ostringstream errorMessage;
                errorMessage << "Unknown action at index " << index << ": '"
                             << actionString << "'";
                throw std::invalid_argument(errorMessage.str());
            }
        }

        return actions;
    }

    std::vector<std::string> DispatchDependenciesYaml(
        const YAML::Node dependenciesYaml)
    {
        std::vector<std::string> dependencies;
        dependencies.reserve(dependenciesYaml.size());

        for (const YAML::Node dependencyYaml : dependenciesYaml)
        {
            dependencies.emplace_back(dependencyYaml.as<std::string>());
        }

        return dependencies;
    }
};

const std::unique_ptr<ConfigReader> DispatchConfigType(
    const std::filesystem::path configPath)
{
    // TODO: If nessesery, add config dispatching logic
    return std::make_unique<YamlConfigReader>();
}

void PrintUsage(void)
{
    std::cout << "usage: async-http-downloader <path-to-config.yaml>\n";
}

int main(int argc, const char **argv)
{
    if (argc < 2)
    {
        PrintUsage();
        return EXIT_FAILURE;
    }

    enum : uint32_t
    {
        EXECUTABLE_PATH_ARG,
        CONFIG_PATH_ARG,
    };

    const std::filesystem::path configPath(argv[CONFIG_PATH_ARG]);

    if (!std::filesystem::exists(configPath))
    {
        std::fprintf(stderr, "ERROR: given file path doesn't exitsts: '%s'\n",
                     configPath.c_str());
        return EXIT_FAILURE;
    }

    const std::unique_ptr<ConfigReader> configReader =
        DispatchConfigType(configPath);

    TaskMap taskMap = configReader->Read(configPath);

    TaskRunner runner(taskMap);

    runner.Run();

    return EXIT_SUCCESS;
}
