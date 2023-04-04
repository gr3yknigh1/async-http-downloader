#ifndef FILETASKRUNNER_HPP_
#define FILETASKRUNNER_HPP_

#include "ahd/Action.hpp"
#include "ahd/FileTask.hpp"
#include <cstdio>
#include <future>
#include <iostream>
#include <unordered_map>

class FileTaskRunner
{
public:
    FileTaskRunner(std::unordered_map<std::string, FileTask *> &fileTasks)
        : m_FileTasks(fileTasks), m_FileTaskCVs()
    {
    }

    void Run() const
    {
        std::vector<std::future<void>> workers;
        workers.reserve(m_FileTasks.size());

        for (const std::pair<std::string, FileTask *> &pair : m_FileTasks)
        {
            const std::string taskName = pair.first;
            const FileTask *fileTask = pair.second;

            const auto executionFunc = [](const FileTask *fileTask) {
                for (const Action *action : fileTask->actions)
                {
                    action->Execute();
                }
            };

            workers.emplace_back(
                std::async(std::launch::async, executionFunc, fileTask));
        }

        for (const std::future<void> &w : workers)
        {
            w.wait();
        }
    }

private:
    std::unordered_map<std::string, FileTask *> m_FileTasks;
    std::unordered_map<std::string, std::condition_variable> m_FileTaskCVs;
};

#endif // FILETASKRUNNER_HPP_
