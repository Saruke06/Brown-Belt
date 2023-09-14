#include <iostream>
#include <map>
#include <vector>

using namespace std;
/*
// Перечислимый тип для статуса задачи
enum class TaskStatus {
    NEW,          // новая
    IN_PROGRESS,  // в разработке
    TESTING,      // на тестировании
    DONE          // завершена
};

// Объявляем тип-синоним для map<TaskStatus, int>,
// позволяющего хранить количество задач каждого статуса
using TasksInfo = map<TaskStatus, int>;
*/
class TeamTasks {
public:
    // Получить статистику по статусам задач конкретного разработчика
    const TasksInfo& GetPersonTasksInfo(const string& person) const {
        return team_tasks.at(person);
    }
  
    // Добавить новую задачу (в статусе NEW) для конкретного разработчитка
    void AddNewTask(const string& person) {
        team_tasks[person][TaskStatus::NEW]++;
    }
  
    // Обновить статусы по данному количеству задач конкретного разработчика,
    // подробности см. ниже
    tuple<TasksInfo, TasksInfo> PerformPersonTasks(
        const string& person, int task_count) {
        
        TasksInfo new_tasks, old_tasks;
        int sum = 0;
        for (const auto a : team_tasks[person]) {
            if (a.first != TaskStatus::DONE)
                sum += a.second;
        }
        task_count = min(task_count, sum);
        if (team_tasks.count(person) == 0) {
            return make_pair(TasksInfo{}, TasksInfo{});
        }
        else {
                vector<TaskStatus> TaskStatuses;
                for (int i = 0; i < 4; i++) {
                    TaskStatuses.push_back(static_cast<TaskStatus>(i));
                }

                TasksInfo p = team_tasks[person], newp;
                for (auto i = 0; i < 3; ++i) {
                    int updated = min(task_count, p[TaskStatuses[i]]);
                    if (updated)
                        new_tasks[TaskStatuses[i + 1]] = updated;
                    if (p[TaskStatuses[i]] - updated) {
                        old_tasks[TaskStatuses[i]] = p[TaskStatuses[i]] - updated;
                    }
                    newp[TaskStatuses[i]] += p[TaskStatuses[i]] - updated;
                    newp[TaskStatuses[i + 1]] += updated;

                    task_count -= updated;
                }
                if (p.count(TaskStatuses[3]))
                    newp[TaskStatuses[3]] += p[TaskStatuses[3]];
                for (const auto& a : newp) {
                    if (a.second == 0)
                        team_tasks[person].erase(a.first);
                    else
                        team_tasks[person][a.first] = a.second;
                }
                //team_tasks[person] = newp;
            return make_pair(new_tasks, old_tasks);
        }
/*
        int countInprog = 0, countTesting = 0, countDone = 0;
        int max = task_count;
        TasksInfo new_tasks, old_tasks;
        if (team_tasks.count(person) == 0) {
            return make_pair(TasksInfo{}, TasksInfo{});
        }
        else {
            for (int i = 0; i < max && team_tasks[person][TaskStatus::NEW]; --max) {
                team_tasks[person][TaskStatus::NEW]--;
                team_tasks[person][TaskStatus::IN_PROGRESS]++;
                countInprog++;
            }
            if (countInprog) new_tasks[TaskStatus::IN_PROGRESS] = countInprog;
            if (team_tasks[person][TaskStatus::NEW] != 0) {
                old_tasks[TaskStatus::NEW] = team_tasks[person][TaskStatus::NEW];
            }
            else {
                team_tasks[person].erase(TaskStatus::NEW);
            }

            for (int i = 0; i < max && team_tasks[person][TaskStatus::IN_PROGRESS]; --max) {
                team_tasks[person][TaskStatus::IN_PROGRESS]--;
                team_tasks[person][TaskStatus::TESTING]++;
                countTesting++;
            }
            if (countTesting) new_tasks[TaskStatus::TESTING] = countTesting;
            if (team_tasks[person][TaskStatus::IN_PROGRESS] != 0) {
                int tmp = team_tasks[person][TaskStatus::IN_PROGRESS] - countInprog;
                if (tmp != 0)
                    old_tasks[TaskStatus::IN_PROGRESS] = tmp;
            }
            else {
                team_tasks[person].erase(TaskStatus::IN_PROGRESS);
            }

            for (int i = 0; i < max && team_tasks[person][TaskStatus::TESTING]; --max) {
                team_tasks[person][TaskStatus::TESTING]--;
                team_tasks[person][TaskStatus::DONE]++;
                countDone++;
            }
            if (countDone) new_tasks[TaskStatus::DONE] = countDone;
            if (team_tasks[person][TaskStatus::TESTING] != 0) {
                int tmp = team_tasks[person][TaskStatus::TESTING] - countTesting;
                if (tmp != 0)
                    old_tasks[TaskStatus::TESTING] = tmp;
            }
            else {
                team_tasks[person].erase(TaskStatus::TESTING);
            }
            return make_pair(new_tasks, old_tasks);
        }
*/
    }
private:
    map<string, TasksInfo> team_tasks;
};


// Принимаем словарь по значению, чтобы иметь возможность
// обращаться к отсутствующим ключам с помощью [] и получать 0,
// не меняя при этом исходный словарь
void PrintTasksInfo(TasksInfo tasks_info) {
    cout << tasks_info[TaskStatus::NEW] << " new tasks" <<
        ", " << tasks_info[TaskStatus::IN_PROGRESS] << " tasks in progress" <<
        ", " << tasks_info[TaskStatus::TESTING] << " tasks are being tested" <<
        ", " << tasks_info[TaskStatus::DONE] << " tasks are done" << endl;
}

int main() {
    TeamTasks tasks;
    tasks.AddNewTask("Ilia");
    for (int i = 0; i < 3; ++i) {
        tasks.AddNewTask("Ivan");
    }
    cout << "Ilia's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ilia"));
    cout << "Ivan's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"));
  
    TasksInfo updated_tasks, untouched_tasks;
  
    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Ivan", 2);
    cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);
  
    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Ivan", 2);
    cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);

return 0;
}