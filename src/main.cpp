#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include "ahd/TaskRunner.hpp"
#include "ahd/YamlConfigReader.hpp"

const std::unique_ptr<ConfigReader> DispatchConfigType(
    const std::filesystem::path &configPath)
{
    // TODO: If nessesery, add config dispatching logic
    (void)configPath;

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

    enum Args : uint32_t
    {
        EXECUTABLE_PATH,
        CONFIG_PATH,
    };

    const std::filesystem::path configPath(argv[Args::CONFIG_PATH]);

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
