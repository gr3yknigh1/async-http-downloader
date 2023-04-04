#include "ahd/UnpackAction.hpp"

UnpackAction::UnpackAction(const std::filesystem::path &archivePath,
                           const std::filesystem::path &destanationPath)
    : m_ArchivePath(archivePath), m_DestanationPath(destanationPath)
{
    // TODO: Check if file's path exists
}

void UnpackAction::Execute(void) const
{
    s_7zExtractor->extract(m_ArchivePath, m_DestanationPath);
}
