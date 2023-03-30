#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

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
            std::fprintf(stderr, "ERROR: missing required config field: '%s'",
                         field);
            return EXIT_FAILURE;
        }
    }

    if (!configYaml["files"].IsSequence())
    {
        std::fprintf(stderr, "ERROR: 'files' field isn't sequence\n");
        return EXIT_FAILURE;
    }

    const std::string host = configYaml["host"].as<std::string>();
    const std::string target = configYaml["target"].as<std::string>();

    // TODO: Add debug configuration
    std::printf("HOST: %s\n", host.c_str());
    std::printf("TARGET: %s\n", target.c_str());

    return EXIT_SUCCESS;
}
