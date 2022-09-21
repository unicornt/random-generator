#include "StructGenerator.h"

extern string typeName[];
StructGenerator* StructGenerator::createInstance()
{
    unsigned long g_seed = generateSeed();
    return new StructGenerator(g_seed);
}

TypeBase* StructGenerator::make_one_struct_field()
{
    unsigned int i = rand_upto(AllType->size());
    cout << "select type " << i << " " << typeName[i] << endl;
    // unsigned short qualifier = 0;
    // qualifier |= (rand() & 1); // const
    // qualifier |= (rand() & 1) << 1; // restrict
    // qualifier |= (rand() & 1) << 2; // volatile
    // TypeBase* newField = new SimpleImpl();
    return (*AllType)[i];
}

CompoundImpl* StructGenerator::make_random_struct_type(size_t field_cnt, vector<Variable>* context)
{
    CompoundImpl* newStruct = new CompoundImpl(0, "struct cd_s1", 0);
    // third para means "is it a union?(no for now)"
    size_t j = 0;
    for(size_t i = 0; i < field_cnt; i++) {
        if((rand() & 1) && j < context->size()) {
            newStruct->fields_type_ptr.push_back((*context)[j].type_ptr);
            newStruct->fields_name.push_back((*context)[j].name);
            j++;
        }
        newStruct->fields_type_ptr.push_back(make_one_struct_field());
        newStruct->fields_name.push_back("r" + to_string(i));
    }
    while(j < context->size()) {
        newStruct->fields_type_ptr.push_back((*context)[j].type_ptr);
        newStruct->fields_name.push_back((*context)[j].name);
        j++;
    }
    return newStruct;
}

CompoundImpl* StructGenerator::generate(vector<TypeBase*>* simpleType, vector<Variable>* context)
{
    size_t field_cnt = 0;
    size_t max_cnt = 20; // tbd !!
    field_cnt = rand_upto(max_cnt) + 1;
    CompoundImpl* newStruct = make_random_struct_type(field_cnt, context);
    return newStruct;
}

CompoundImpl* getNewCompoundType(vector<TypeBase*> simpleType, vector<Variable> context)
{
    srand(time(NULL));
    StructGenerator* generator = StructGenerator::createInstance();
    generator->AllType = &simpleType;
    CompoundImpl* newStruct = generator->generate(&simpleType, &context);
    return newStruct;
}