#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <HTTPRequest.hpp>
#include <yaml-cpp/yaml.h>

#include "ahd/Action.hpp"
#include "ahd/DownloadAction.hpp"
#include "ahd/FileTask.hpp"
#include "ahd/FileTaskRunner.hpp"
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

int main(int argc, const char **argv)
{
    if (argc < 2)
    {
        std::cout << "usage: async-http-downloader <path-to-config.yaml>\n";
        return EXIT_SUCCESS;
    }

    const std::filesystem::path configPath(argv[1]);

    if (!std::filesystem::exists(configPath))
    {
        std::fprintf(stderr, "ERROR: given file path doesn't exitsts: '%s'\n",
                     configPath.c_str());
        return EXIT_FAILURE;
    }

    const YAML::Node configYaml = YAML::LoadFile(configPath);

    // NOTE: Added in order to exclude invalid yaml files
    if (!configYaml.IsMap())
    {
        std::fprintf(stderr, "ERROR: invalid config\n");
        return EXIT_FAILURE;
    }

    const std::vector<const char *> missingFields =
        FindMissingFields(configYaml, REQUIRED_CONFIG_FIELDS);
    if (missingFields.size() != 0)
    {
        std::fprintf(stderr, "ERROR: missing required config fields: ");
        for (const char *missingField : missingFields)
        {
            std::fprintf(stderr, "'%s' ", missingField);
        }
        std::fprintf(stderr, "\n");
        return EXIT_FAILURE;
    }

    if (!configYaml[CONFIG_FILES_FIELD].IsSequence())
    {
        std::fprintf(stderr, "ERROR: 'files' field isn't a sequence\n");
        return EXIT_FAILURE;
    }

    // NOTE: Parsing config file
    const std::string host = configYaml[CONFIG_HOST_FIELD].as<std::string>();
    const std::string target =
        configYaml[CONFIG_TARGET_FIELD].as<std::string>();

    std::unordered_map<std::string, std::shared_ptr<FileTask>> fileTasks = {};
    fileTasks.reserve(configYaml[CONFIG_FILES_FIELD].size());

    for (uint64_t i = 0; i < configYaml[CONFIG_FILES_FIELD].size(); ++i)
    {
        const YAML::Node fileYaml = configYaml[CONFIG_FILES_FIELD][i];
        const std::vector<const char *> missingFields =
            FindMissingFields(fileYaml, REQUIRED_FILE_FIELDS);
        if (missingFields.size() != 0)
        {
            std::fprintf(
                stderr,
                "ERROR: missing required file fields at index %ld: ", i);
            for (const char *missingField : missingFields)
            {
                std::fprintf(stderr, "'%s' ", missingField);
            }
            std::fprintf(stderr, "\n");
            return EXIT_FAILURE;
        }

        std::shared_ptr<FileTask> fileTask = std::make_shared<FileTask>();
        fileTask->file = fileYaml[FILE_FILE_FIELD].as<std::string>();
        fileTask->actions.reserve(fileYaml[FILE_ACTIONS_FIELD].size());

        for (const YAML::Node actionYaml : fileYaml[FILE_ACTIONS_FIELD])
        {
            const std::string actionString = actionYaml.as<std::string>();
            if (actionString == "download")
            {
                // TODO: Add option for working directory
                fileTask->actions.push_back(new DownloadAction(
                    host + target + fileTask->file, fileTask->file));
            }
            else if (actionString == "unpack")
            {
                // TODO: Add option for unpack directory
                fileTask->actions.push_back(
                    new UnpackAction(fileTask->file, "."));
            }
            else
            {
                std::printf("ERROR: Unknown action at %lu: '%s'\n", i,
                            actionString.c_str());
                return EXIT_FAILURE;
            }
        }

        if (fileYaml[FILE_DEPENDENCIES_FIELD])
        {
            fileTask->dependencies.reserve(
                fileYaml[FILE_DEPENDENCIES_FIELD].size());
            for (const YAML::Node dependencyYaml :
                 fileYaml[FILE_DEPENDENCIES_FIELD])
            {
                fileTask->dependencies.emplace_back(
                    dependencyYaml.as<std::string>());
            }
        }

        fileTasks[fileYaml[FILE_NAME_FIELD].as<std::string>()] = fileTask;
    }

    // NOTE: Validating deps
    for (const auto &[name, task] : fileTasks)
    {
        for (const std::string &dependency : task->dependencies)
        {
            // NOTE: Searching non-existing deps
            const auto search = fileTasks.find(dependency);
            if (search == fileTasks.end())
            {
                std::fprintf(stderr,
                             "ERROR: Can't find dependency '%s' that '%s' task "
                             "requires\n",
                             dependency.c_str(), name.c_str());
                return EXIT_FAILURE;
            }

            // NOTE: Can't depend on self
            if (search->first == name)
            {
                std::fprintf(stderr, "ERROR: Can't depend on self '%s'\n",
                             name.c_str());
                return EXIT_FAILURE;
            }

            // NOTE: Searching cirqle deps
            const std::shared_ptr<FileTask> dependencyTask = search->second;
            const auto dependencyTaskDeps = dependencyTask->dependencies;
            if (std::find(dependencyTaskDeps.begin(), dependencyTaskDeps.end(),
                          name) != dependencyTaskDeps.end())
            {
                std::fprintf(stderr,
                             "ERROR: Found cirqle dependency between '%s' and "
                             "'%s' tasks requires\n",
                             dependency.c_str(), name.c_str());
                return EXIT_FAILURE;
            }
        }
    }

    FileTaskRunner ftr(fileTasks);
    ftr.Run();

    // TODO: Free memory after `fileTasks`
    return EXIT_SUCCESS;
}
