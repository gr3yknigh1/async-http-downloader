#ifndef FILETASKRUNNER_HPP_
#define FILETASKRUNNER_HPP_

#include "ahd/Action.hpp"
#include "ahd/Task.hpp"
#include <future>
#include <unordered_map>

class TaskRunner
{
public:
    TaskRunner(
        std::unordered_map<std::string, std::shared_ptr<Task>> &fileTasks)
        : m_FileTasks(fileTasks), m_FileTaskStatuses()
    {
        m_FileTaskStatuses.reserve(m_FileTasks.size());
        for (const auto &[name, _] : m_FileTasks)
        {
            m_FileTaskStatuses[name] = false;
        }
    }

    void Run()
    {
        std::vector<std::future<void>> workers;
        workers.reserve(m_FileTasks.size());

        for (const auto &[name, task] : m_FileTasks)
        {
            const auto executionFunc =
                [&](const Task *fileTask,
                    const std::unordered_map<std::string, bool>
                        *fileTaskStatuses,
                    bool *statusOut) {
                    for (const std::string dependency : fileTask->dependencies)
                    {
                        while (!fileTaskStatuses->at(dependency))
                        {
                            // NOTE: Waiting...
                        }
                    }

                    for (const Action *action : fileTask->actions)
                    {
                        action->Execute();
                    }

                    *statusOut = true;
                };

            workers.emplace_back(std::async(std::launch::async, executionFunc,
                                            task.get(), &m_FileTaskStatuses,
                                            &(m_FileTaskStatuses[name])));
        }

        for (const std::future<void> &w : workers)
        {
            w.wait();
        }
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Task>> m_FileTasks;
    std::unordered_map<std::string, bool> m_FileTaskStatuses;
};

#endif // FILETASKRUNNER_HPP_
