#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <deque>
#include <algorithm>

#include "action_node.h"

using namespace std;


class Scheduler
{ 
    public:
        virtual void schedule_tasks() = 0;
        ~Scheduler() {
            for (auto node : nodes){
                delete node;
            }

            while (!queue.empty()){
                // delete queue.top();
                queue.pop();
            }
        }   

        void populate_queue(vector<const Action *> actions);
        string to_string();
        void print_queue();
    protected:
        virtual void process_write(ActionNode * node) = 0;
        virtual void process_read(ActionNode * node) = 0;
        virtual void process_commit(ActionNode * node) = 0;
        virtual void process_abort(ActionNode * node) = 0;
        virtual void process_start(ActionNode * node) = 0;
        virtual string get_schedule_name() = 0;
        void insert_node_into_schedule(ActionNode *node);
        string get_operation_string();

        // all the writes that a transaction makes
        unordered_map<string, unordered_set<string>> transaction_writes;
        unordered_set<string> aborted_transactions;
        unordered_set<string> committed_transactions;
        priority_queue<ActionNode *, vector<ActionNode*>, ActionNodeComparator> queue;
        vector<const ActionNode *> nodes;

        int deadlock_time = -1; // if  dead lock time is not -1, then a deadlock has occured at  the specified time

        int current_execution_time = 0;
};

#endif //RECOVERABLE_SCHEDULE_H