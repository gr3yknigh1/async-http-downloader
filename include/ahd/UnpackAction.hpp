#ifndef UNPACKACTION_HPP_
#define UNPACKACTION_HPP_

#include "ahd/Action.hpp"
#include <bit7z/bit7z.hpp>
#include <filesystem>
#include <memory>

class UnpackAction : public Action
{
public:
    UnpackAction(const std::filesystem::path &archivePath,
                 const std::filesystem::path &destanationPath);

    virtual void Execute(void) const override;

private:
    const std::filesystem::path m_ArchivePath;
    const std::filesystem::path m_DestanationPath;

    // TODO: Add option in CLI to customly specify 7z dll
#ifdef __UNIX__
    inline static const std::filesystem::path s_7zLibPath = "./lib/7z.so";
#elif __WIN32__
    inline static const std::filesystem::path s_7zLibPath = "./lib/7z.dll";
#else
#error "Can't support platform due lack of 7z dll"
#endif

    inline static const std::unique_ptr<bit7z::Bit7zLibrary> s_7zLib =
        std::make_unique<bit7z::Bit7zLibrary>(UnpackAction::s_7zLibPath);
    inline static const std::unique_ptr<bit7z::BitFileExtractor> s_7zExtractor =
        std::make_unique<bit7z::BitFileExtractor>(*UnpackAction::s_7zLib.get(),
                                                  bit7z::BitFormat::Auto);
};

#endif // UNPACKACTION_HPP_
