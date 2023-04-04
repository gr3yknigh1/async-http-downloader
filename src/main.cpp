#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <HTTPRequest.hpp>
#include <bit7z/bit7z.hpp>
#include <yaml-cpp/yaml.h>

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

class Action
{
public:
    virtual ~Action(void)
    {
    }
    virtual void Execute(void) const
    {
    }
};

class DownloadAction : public Action
{
public:
    DownloadAction(const std::string &requestUrl,
                   const std::filesystem::path &outputPath)
        : m_RequestUrl(requestUrl), m_OutputPath(outputPath)
    {
    }

    void Execute(void) const override
    {
        // TODO: Add try-catch exception for request
        http::Request request{m_RequestUrl};
        http::Response response = request.send(GET_REQUEST);
        std::ofstream outputStream(m_OutputPath);

        for (const auto c : response.body)
        {
            outputStream << c;
        }
    }

private:
    const std::string m_RequestUrl;
    const std::filesystem::path m_OutputPath;

    static const char *GET_REQUEST;
};

const char *DownloadAction::GET_REQUEST = "GET";

class UnpackAction : public Action
{
public:
    UnpackAction(const std::filesystem::path &archivePath,
                 const std::filesystem::path &destanationPath)
        : m_ArchivePath(archivePath), m_DestanationPath(destanationPath)
    {
        // TODO: Check if file's path exists
    }

    void Execute(void) const override
    {
        s_7zExtractor->extract(m_ArchivePath, m_DestanationPath);
    }

private:
    const std::filesystem::path m_ArchivePath;
    const std::filesystem::path m_DestanationPath;

    // TODO: Handle Window's dll
    inline static const std::filesystem::path s_7zLibPath = "./lib/7z.so";
    inline static const std::unique_ptr<bit7z::Bit7zLibrary> s_7zLib =
        std::make_unique<bit7z::Bit7zLibrary>(UnpackAction::s_7zLibPath);
    inline static const std::unique_ptr<bit7z::BitFileExtractor> s_7zExtractor =
        std::make_unique<bit7z::BitFileExtractor>(*UnpackAction::s_7zLib.get(),
                                                  bit7z::BitFormat::Auto);
};

struct FileTask
{
    std::string name;
    std::string file;
    std::vector<Action> actions;
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
const std::vector<const char *> REQUIRED_CONFIG_FIELDS = {
    CONFIG_HOST_FIELD, CONFIG_TARGET_FIELD, CONFIG_FILES_FIELD};

const char *FILE_NAME_FIELD = "name";
const char *FILE_FILE_FIELD = "file";
const char *FILE_ACTIONS_FIELD = "actions";
const char *FILE_DEPENDENCIES_FIELD = "dependencies";
const std::vector<const char *> REQUIRED_FILE_FIELDS = {
    FILE_NAME_FIELD, FILE_FILE_FIELD, FILE_ACTIONS_FIELD};

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
        for (const auto &missingField : missingFields)
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

    std::vector<FileTask> fileTasks;
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
            for (const auto &missingField : missingFields)
            {
                std::fprintf(stderr, "'%s' ", missingField);
            }
            std::fprintf(stderr, "\n");
            return EXIT_FAILURE;
        }

        FileTask fileTask;
        fileTask.name = fileYaml[FILE_NAME_FIELD].as<std::string>();
        fileTask.file = fileYaml[FILE_FILE_FIELD].as<std::string>();
        fileTask.actions.reserve(fileYaml[FILE_ACTIONS_FIELD].size());

        for (const auto actionYaml : fileYaml[FILE_ACTIONS_FIELD])
        {
            const std::string actionString = actionYaml.as<std::string>();
            if (actionString == "download")
            {
                // TODO: Add option for working directory
                fileTask.actions.emplace_back(DownloadAction(
                    host + target + fileTask.file, fileTask.file));
            }
            else if (actionString == "unpack")
            {
                // TODO: Add option for unpack directory
                fileTask.actions.emplace_back(UnpackAction(fileTask.file, "."));
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
            fileTask.dependencies.reserve(
                fileYaml[FILE_DEPENDENCIES_FIELD].size());
            for (const auto dependencyYaml : fileYaml[FILE_DEPENDENCIES_FIELD])
            {
                fileTask.dependencies.emplace_back(
                    dependencyYaml.as<std::string>());
            }
        }
    }

    // NOTE: Validating deps

    return EXIT_SUCCESS;
}
