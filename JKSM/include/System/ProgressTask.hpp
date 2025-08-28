#pragma once
#include "System/Task.hpp"

namespace System
{
    // This is basically a task with a few extra functions to allow tracking the progress of a task.
    // Functions that use this should use the same signature as task, except taking System::ProgressTask instead of Task.
    class ProgressTask : public System::Task
    {
        public:
            template <typename... Args>
            ProgressTask(void (*Function)(System::ProgressTask *, Args...), Args... Arguments)
                : Task(Function, this, std::forward<Args>(Arguments)...)
            {
            }
            ~ProgressTask() {};

            // This is so I don't have to type so much. Sets current to 0 and sets goal to whatever is passed.
            void Reset(double Goal)
            {
                m_Current = 0;
                m_Goal    = Goal;
            }

            void SetGoal(double Goal) { m_Goal = Goal; }

            void SetCurrent(double Current) { m_Current = Current; }

            double GetProgress() const { return (m_Current / m_Goal); }

        private:
            // The Current/Goal value.
            double m_Current = 0.0f;
            // The maximum/goal value
            double m_Goal = 0.0f;
    };
} // namespace System
