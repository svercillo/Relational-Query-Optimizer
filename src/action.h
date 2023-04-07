#ifndef ACTION_H
#define ACTION_H


#include <unordered_map>
#include <iostream>
#include <vector>
using namespace std;

enum OperationType
{
    OPERATIONTYPE_COMMIT,
    OPERATIONTYPE_ABORT,
    OPERATIONTYPE_WRITE,
    OPERATIONTYPE_READ,
    OPERATIONTYPE_START
};

struct Action
{ 
    public:
        int time_offset;
        string trans_id;
        string object_id;
        OperationType operation_type;
        
        bool operator<(const Action& other) const {
            return this->time_offset < other.time_offset;
        }

        Action(int time_offset, string trans_id, OperationType operation_type)
        {
            this->time_offset = time_offset;
            this->trans_id = trans_id;
            this->operation_type = operation_type;
        }

        Action(int time_offset, string trans_id, string object_id, OperationType operation_type)
        {
            this->time_offset = time_offset;
            this->trans_id = trans_id;
            this->object_id = object_id;
            this->operation_type = operation_type;
        }

        

        string to_string() const
        {
            string operation_str;
            switch (operation_type)
            {
                case OPERATIONTYPE_COMMIT:
                    operation_str = "C";
                    break;
                case OPERATIONTYPE_ABORT:
                    operation_str = "A";
                    break;
                case OPERATIONTYPE_WRITE:
                    operation_str = "W";
                    break;
                case OPERATIONTYPE_READ:
                    operation_str = "R";
                    break;
                case OPERATIONTYPE_START:
                    operation_str = "S";
                    break;
                default:
                    operation_str = "UNKNOWN";
                    break;
            }

            string result = "Action: { \n";
            result += "\ttime_offset: " + std::to_string(time_offset) + ", \n";
            result += "\ttrans_id: " + trans_id + ", \n";
            result += "\tobject_id: " + object_id + ", \n";
            result += "\toperation_type: " + operation_str + "\n";
            result += " }\n";
            return result;
        }
};

#endif //ACTION_H