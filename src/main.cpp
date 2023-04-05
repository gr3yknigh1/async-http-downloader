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

const char *CONFIG_HOST_FIELD = "host";
const char *CONFIG_TARGET_FIELD = "target";
const char *CONFIG_FILES_FIELD = "files";
const std::vector<const char *> REQUIRED_CONFIG_FIELDS = {
    CONFIG_HOST_FIELD, CONFIG_TARGET_FIELD, CONFIG_FILES_FIELD};

const char *FILE_NAME_FIELD = "name";
const char *FILE_FILE_FIELD = "file";
const char *FILE_ACTIONS_FIELD = "actions";
const char *FILE_DEPENDENCIES_FIELD = "dependencies";
const std::vector<const char *> REQUIRED_FILE_FIELDS = {
    FILE_NAME_FIELD, FILE_FILE_FIELD, FILE_ACTIONS_FIELD};

using TaskMap = std::unordered_map<std::string, std::shared_ptr<Task>>;

const std::vector<const char *> FindMissingFields(
    YAML::Node node, const std::vector<const char *> requiredFields)
{
    // NOTE: Maybe reserve memory?
    std::vector<const char *> missingFields;

    for (const char *field : requiredFields)
    {
        if (!node[field])
        {
            missingFields.push_back(field);
        }
    }

    return missingFields;
}

void ValidateDependencies(const TaskMap &taskMap)
{
    for (const auto &[name, task] : taskMap)
    {
        for (const std::string &dependency : task->dependencies)
        {
            // NOTE: Searching non-existing deps
            const auto &search = taskMap.find(dependency);
            if (search == taskMap.end())
            {
                std::fprintf(stderr,
                             "ERROR: Can't find dependency '%s' that '%s' task "
                             "requires\n",
                             dependency.c_str(), name.c_str());
                std::exit(EXIT_FAILURE);
                return;
            }

            // NOTE: Can't depend on self
            if (search->first == name)
            {
                std::fprintf(stderr, "ERROR: Can't depend on self '%s'\n",
                             name.c_str());
                std::exit(EXIT_FAILURE);
                return;
            }

            // NOTE: Searching cirqle deps
            const std::shared_ptr<Task> dependencyTask = search->second;
            const auto dependencyTaskDeps = dependencyTask->dependencies;
            if (std::find(dependencyTaskDeps.begin(), dependencyTaskDeps.end(),
                          name) != dependencyTaskDeps.end())
            {
                std::fprintf(stderr,
                             "ERROR: Found cirqle dependency between '%s' and "
                             "'%s' tasks requires\n",
                             dependency.c_str(), name.c_str());
                std::exit(EXIT_FAILURE);
                return;
            }
        }
    }
}

void ValidateConfigYaml(const YAML::Node &configYaml)
{
    // NOTE: Added in order to exclude invalid yaml files
    if (!configYaml.IsMap())
    {
        throw std::invalid_argument("Config isn't a map of fields\n");
        return;
    }

    const auto missingFields =
        FindMissingFields(configYaml, REQUIRED_CONFIG_FIELDS);
    if (missingFields.size() != 0)
    {
        std::string errorMessage = "Missing required config fields: ";
        for (const char *field : missingFields)
        {
            errorMessage += field;
        }
        throw std::invalid_argument(errorMessage);
        return;
    }

    if (!configYaml[CONFIG_FILES_FIELD].IsSequence())
    {
        throw std::invalid_argument("'files' field must be a sequence");
        return;
    }
}

void ValidateFileYaml(uint64_t index, const YAML::Node &fileYaml)
{
    const std::vector<const char *> missingFields =
        FindMissingFields(fileYaml, REQUIRED_FILE_FIELDS);
    if (!missingFields.empty())
    {
        std::ostringstream errorMessage;
        errorMessage << "Missing required file fields at index " << index
                     << ": ";
        for (const char *missingField : missingFields)
        {
            errorMessage << "'" << missingField << "' ";
        }
        errorMessage << '\n';

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
            errorMessage << "Unknown action at " << index << ": '"
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

TaskMap ReadConfig(const std::filesystem::path &configPath)
{
    const YAML::Node configYaml = YAML::LoadFile(configPath);
    ValidateConfigYaml(configYaml);

    const std::string host = configYaml[CONFIG_HOST_FIELD].as<std::string>();
    const std::string target =
        configYaml[CONFIG_TARGET_FIELD].as<std::string>();

    TaskMap taskMap = {};
    taskMap.reserve(configYaml[CONFIG_FILES_FIELD].size());

    for (uint64_t i = 0; i < configYaml[CONFIG_FILES_FIELD].size(); ++i)
    {
        const YAML::Node fileYaml = configYaml[CONFIG_FILES_FIELD][i];
        ValidateFileYaml(i, fileYaml);

        std::shared_ptr<Task> task = std::make_shared<Task>();
        task->file = fileYaml[FILE_FILE_FIELD].as<std::string>();
        task->actions = DispatchActionsYaml(i, host, target, task,
                                            fileYaml[FILE_ACTIONS_FIELD]);

        if (fileYaml[FILE_DEPENDENCIES_FIELD])
        {
            task->dependencies =
                DispatchDependenciesYaml(fileYaml[FILE_DEPENDENCIES_FIELD]);
        }

        taskMap[fileYaml[FILE_NAME_FIELD].as<std::string>()] = task;
    }

    return taskMap;
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

    TaskMap taskMap = ReadConfig(configPath);
    ValidateDependencies(taskMap);

    TaskRunner runner(taskMap);
    runner.Run();

    return EXIT_SUCCESS;
}
