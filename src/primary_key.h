#ifndef PRIMARY_KEY_H
#define PRIMARY_KEY_H

#include "key.h"
struct PrimaryKey : public Key{
    public:
        bool is_present;
        PrimaryKey()
        {
            is_present = false;
        }
};

#endif // PRIMARY_KEY_H