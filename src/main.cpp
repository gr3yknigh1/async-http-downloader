#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

enum Action
{
    Unknown,
    Download,
    Unpack,
};

struct Task
{
    std::string name;
    std::string file;
    std::vector<Action> actions;
    std::vector<Task> dependencies;
};

const char *CONFIG_HOST_FIELD = "host";
const char *CONFIG_TARGET_FIELD = "target";
const char *CONFIG_FILES_FIELD = "files";
const char *REQUIRED_CONFIG_FIELDS[] = {CONFIG_HOST_FIELD, CONFIG_TARGET_FIELD,
                                        CONFIG_FILES_FIELD};

const char *FILE_NAME_FIELD = "name";
const char *FILE_FILE_FIELD = "file";
const char *FILE_ACTIONS_FIELD = "actions";
const char *FILE_DEPENDENCIES_FIELD = "dependencies";
const char *REQUIRED_FILE_FIELDS[] = {FILE_NAME_FIELD, FILE_FILE_FIELD,
                                      FILE_ACTIONS_FIELD};

int main(int argc, const char **argv)
{
    if (argc < 2)
    {
        std::cout << "usage: async-http-downloader <path-to-config.yaml>\n";
        return EXIT_SUCCESS;
    }

    std::filesystem::path configPath(argv[1]);

    if (!std::filesystem::exists(configPath))
    {
        std::fprintf(stderr, "ERROR: given file path doesn't exitsts: '%s'\n",
                     configPath.c_str());
        return EXIT_FAILURE;
    }

    YAML::Node configYaml = YAML::LoadFile(configPath);

    // NOTE: Added in order to exclude invalid yaml files
    if (!configYaml.IsMap())
    {
        std::fprintf(stderr, "ERROR: invalid config\n");
        return EXIT_FAILURE;
    }

    for (const char *field : REQUIRED_CONFIG_FIELDS)
    {
        if (!configYaml[field])
        {
            std::fprintf(stderr, "ERROR: missing required config field: '%s'\n",
                         field);
            return EXIT_FAILURE;
        }
    }

    if (!configYaml[CONFIG_FILES_FIELD].IsSequence())
    {
        std::fprintf(stderr, "ERROR: 'files' field isn't sequence\n");
        return EXIT_FAILURE;
    }

    for (const auto fileYaml : configYaml[CONFIG_FILES_FIELD])
    {
        for (auto field : REQUIRED_FILE_FIELDS)
        {
            if (!fileYaml[field])
            {
                std::fprintf(stderr,
                             "ERROR: missing required file field: '%s'\n",
                             field);
                return EXIT_FAILURE;
            }
        }

        for (auto action : fileYaml[FILE_ACTIONS_FIELD])
        {
        }

        if (fileYaml[FILE_DEPENDENCIES_FIELD])
        {
            auto dependencies = fileYaml[FILE_DEPENDENCIES_FIELD]
                                    .as<std::vector<std::string>>();
            for (auto dependency : dependencies)
            {
            }
        }
    }

    const std::string host = configYaml[CONFIG_HOST_FIELD].as<std::string>();
    const std::string target =
        configYaml[CONFIG_TARGET_FIELD].as<std::string>();

    return EXIT_SUCCESS;
}
