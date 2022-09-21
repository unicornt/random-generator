#ifndef STRUCT_GENERATOR_H
#define STRUCT_GENERATOR_H

#include "common/type.h"
#include "Random.h"
#include "common/common.h"

class StructGenerator {
public:
    CompoundImpl* generate(vector<TypeBase*>*, vector<Variable>*);
    static StructGenerator* createInstance();
    StructGenerator(unsigned long seed):seed(seed){}
    void generateSimpleType();
    vector<TypeBase*>* AllType;
    TypeBase* make_one_struct_field();
    CompoundImpl* make_random_struct_type(size_t, vector<Variable>*);
private:
    unsigned long seed;
};

CompoundImpl* getNewCompoundType(vector<TypeBase*>, vector<Variable>);

#endif