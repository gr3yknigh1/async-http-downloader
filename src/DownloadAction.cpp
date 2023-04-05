#include "ahd/DownloadAction.hpp"
#include <fstream>
#include <iostream>

DownloadAction::DownloadAction(const std::string &requestUrl,
                               const std::filesystem::path &outputPath)
    : m_RequestUrl(requestUrl), m_OutputPath(outputPath)
{
}

void DownloadAction::Execute(void) const
{
    http::Response response;

    try
    {
        http::Request request{m_RequestUrl};
        response = request.send(GET_REQUEST);
    }
    catch (const std::exception &e)
    {
        // TODO: Throw exception
        std::cerr << "Error: " << e.what() << '\n';
        std::exit(EXIT_FAILURE);
    }

    std::ofstream outputStream(m_OutputPath);
    for (const auto c : response.body)
    {
        outputStream << c;
    }
}

const char *DownloadAction::GET_REQUEST = "GET";
