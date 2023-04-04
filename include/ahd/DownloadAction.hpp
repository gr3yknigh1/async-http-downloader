#ifndef DOWNLOADACTION_HPP_
#define DOWNLOADACTION_HPP_

#include "ahd/Action.hpp"
#include <HTTPRequest.hpp>
#include <filesystem>
#include <fstream>
#include <string>

class DownloadAction : public Action
{
public:
    DownloadAction(const std::string &requestUrl,
                   const std::filesystem::path &outputPath);

    virtual void Execute(void) const override;

private:
    const std::string m_RequestUrl;
    const std::filesystem::path m_OutputPath;

    static const char *GET_REQUEST;
};

#endif // DOWNLOADACTION_HPP_
