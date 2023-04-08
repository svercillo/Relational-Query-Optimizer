#include "serial_scheduler.h"

void SerialScheduler::schedule_tasks(){ // can only occur after queue populated

    unordered_set<string> trans_ids_set;
    while (!queue.empty()){
        ActionNode* top_node = queue.top();

        trans_ids_set.insert(top_node->action->trans_id);
        if (actions_per_transaction.find(top_node->action->trans_id) == actions_per_transaction.end())
            actions_per_transaction.emplace(top_node->action->trans_id, vector<ActionNode*>{});
        
        actions_per_transaction[top_node->action->trans_id].push_back(top_node);
        queue.pop();
    }

    vector<string> trans_ids;
    for (auto trans_id : trans_ids_set){
        trans_ids.push_back(trans_id);
    }

    std::sort(trans_ids.begin(), trans_ids.end(), [this](const string& t1, const string& t2) {
        int t1_end_offset = actions_per_transaction[t1][actions_per_transaction[t1].size()-1]->action->time_offset;
        int t2_end_offset = actions_per_transaction[t2][actions_per_transaction[t2].size()-1]->action->time_offset;

        return t1_end_offset < t2_end_offset;
    });

    for (auto trans_id : trans_ids){
        for (auto action_node : actions_per_transaction[trans_id]){
            insert_node_into_schedule(action_node);
        }
    }
}

string SerialScheduler::get_schedule_name(){
    return "Serial Scheduler";
}
