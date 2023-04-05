#ifndef FILETASKRUNNER_HPP_
#define FILETASKRUNNER_HPP_

#include "ahd/Action.hpp"
#include "ahd/FileTask.hpp"
#include <cstdio>
#include <future>
#include <iostream>
#include <memory>
#include <unordered_map>

class FileTaskRunner
{
public:
    FileTaskRunner(
        std::unordered_map<std::string, std::shared_ptr<FileTask>> &fileTasks)
        : m_FileTasks(fileTasks)
    {
    }

    void Run() const
    {
        std::vector<std::future<void>> workers;
        workers.reserve(m_FileTasks.size());

        for (const auto &[name, task] : m_FileTasks)
        {
            const auto executionFunc = [](const FileTask *fileTask) {
                for (const Action *action : fileTask->actions)
                {
                    action->Execute();
                }
            };

            workers.emplace_back(
                std::async(std::launch::async, executionFunc, task.get()));
        }

        for (const std::future<void> &w : workers)
        {
            w.wait();
        }
    }

private:
    std::unordered_map<std::string, std::shared_ptr<FileTask>> m_FileTasks;
};

#endif // FILETASKRUNNER_HPP_
