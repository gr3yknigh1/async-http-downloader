#include "ahd/DownloadAction.hpp"

DownloadAction::DownloadAction(const std::string &requestUrl,
                               const std::filesystem::path &outputPath)
    : m_RequestUrl(requestUrl), m_OutputPath(outputPath)
{
}

void DownloadAction::Execute(void) const
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

const char *DownloadAction::GET_REQUEST = "GET";
