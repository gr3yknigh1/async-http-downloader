#include <cassert>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#include <HTTPRequest.hpp>
#include <bit7z/bit7z.hpp>
#include <yaml-cpp/yaml.h>

enum FileAction
{
    Unknown,
    Download,
    Unpack,
};

struct FileTask
{
    std::string name;
    std::string file;
    std::vector<FileAction> actions;
    std::vector<std::string> dependencies;
};

class FileJobScheduler
{
public:
    FileJobScheduler()
    {
    }

private:
    std::vector<FileTask> tasks;
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
        for (const auto field : REQUIRED_FILE_FIELDS)
        {
            if (!fileYaml[field])
            {
                std::fprintf(stderr,
                             "ERROR: missing required file field: '%s'\n",
                             field);
                return EXIT_FAILURE;
            }
        }

        for (const auto action : fileYaml[FILE_ACTIONS_FIELD])
        {
        }

        if (fileYaml[FILE_DEPENDENCIES_FIELD])
        {
            const auto dependencies = fileYaml[FILE_DEPENDENCIES_FIELD]
                                          .as<std::vector<std::string>>();
            for (const auto dependency : dependencies)
            {
            }
        }
    }

    const std::string host = configYaml[CONFIG_HOST_FIELD].as<std::string>();
    const std::string target =
        configYaml[CONFIG_TARGET_FIELD].as<std::string>();

    std::cout << "HOST: " << host << '\n';
    std::cout << "TARG: " << target << '\n';

    try
    {
        const std::string requestUrl = host + target + "file.txt";
        std::cout << "REQUEST: " << requestUrl << '\n';

        http::Request request{requestUrl};
        const auto response = request.send("GET");
        std::cout << std::string{response.body.begin(), response.body.end()}
                  << '\n';
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: Request failed, error: " << e.what() << '\n';
    }

    const std::string fileName = "archive.zip";
    try
    {
        const std::string requestUrl = host + target + fileName;
        std::cout << "REQUEST: " << requestUrl << '\n';

        http::Request request{requestUrl};
        const auto response = request.send("GET");
        std::cout << std::string{response.body.begin(), response.body.end()}
                  << '\n';

        std::ofstream fileOutputStream(fileName);
        for (const auto c : response.body)
        {
            fileOutputStream << c;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: Request failed, error: " << e.what() << '\n';
    }

    try
    {
        using namespace bit7z;

        Bit7zLibrary lib{"/lib/p7zip/7z.so"};
        BitFileExtractor extractor{lib, BitFormat::Auto};

        // extracting a simple archive
        std::printf("INFO: File '%s'\n", fileName.c_str());
        extractor.extract(fileName, ".");
    }
    catch (const bit7z::BitException &ex)
    {
        std::cerr << "Error during extraction: " << ex.what() << '\n';
    }

    return EXIT_SUCCESS;
}
