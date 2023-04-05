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
        std::unordered_map<std::string, std::shared_ptr<Task>> &fileTasks);

    void Run();

private:
    std::unordered_map<std::string, std::shared_ptr<Task>> m_FileTasks;
    std::unordered_map<std::string, bool> m_FileTaskStatuses;
};

#endif // FILETASKRUNNER_HPP_
