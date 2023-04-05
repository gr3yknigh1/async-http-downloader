#include "ahd/UnpackAction.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>

UnpackAction::UnpackAction(const std::filesystem::path &archivePath,
                           const std::filesystem::path &destanationPath)
    : m_ArchivePath(archivePath), m_DestanationPath(destanationPath)
{
}

void UnpackAction::Execute(void) const
{
    if (!std::filesystem::exists(m_ArchivePath))
    {
        std::fprintf(stderr,
                     "ERROR: During `unpack` action execution: '%s' archive "
                     "path doesn't exist\n",
                     m_ArchivePath.c_str());
        std::exit(EXIT_FAILURE);
    }

    try
    {
        s_7zExtractor->extract(m_ArchivePath, m_DestanationPath);
    }
    catch (const bit7z::BitException &e)
    {
        // TODO: Throw exception
        std::cerr << "Error: " << e.what() << '\n';
        std::exit(EXIT_FAILURE);
    }
}
