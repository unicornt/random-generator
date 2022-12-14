#include "StructGenerator.h"
#include "ExpressionGenerator.h"

extern string typeName[];

void initSimpleType(vector<TypeBase*>* simpleType)
{
    unsigned int st;
    for(st = eChar; st <= eUInt128; st++){
        cout << typeName[st] << endl;
        SimpleImpl* ntype =new SimpleImpl(0, typeName[st], "");
        ntype->UnqualifiedType = typeName[st];
        simpleType->push_back((TypeBase*)ntype);
    }
}

void addContext(vector<Variable>* context, vector<TypeBase*>* simpleType) 
{
    Variable* n = new Variable("int", "f1");
    n->type_ptr = (*simpleType)[2];
    context->push_back(*n);
    n = new Variable("char", "f2");
    n->type_ptr = (*simpleType)[1];
    context->push_back(*n);
    n = new Variable("int", "f3");
    n->type_ptr = (*simpleType)[2];
    context->push_back(*n);
    n = new Variable("uchar", "f5");
    n->type_ptr = (*simpleType)[6];
    context->push_back(*n);
    n = new Variable("float", "f6");
    n->type_ptr = (*simpleType)[10];
    context->push_back(*n);
}

CompoundImpl* TryGetNewCompoundType() {
    printf("TryFunc start!\n");
    vector<TypeBase*> simpleType;
    vector<Variable> context;
    initSimpleType(&simpleType);
    addContext(&context, &simpleType);
    printf("TryFunc object init finish!\n");
    CompoundImpl* newStruct = getNewCompoundType(simpleType, context);
    cout << "New Struct is " << newStruct->dump() << endl;
    return newStruct;
}

void TryInitialization(CompoundImpl* aStruct) {
    cout << aStruct->getInitialization() << endl;
}

void TryGetExpression()
{
    vector<TypeBase*> simpleType;
    vector<Variable> context;
    initSimpleType(&simpleType);
    addContext(&context, &simpleType);
    cout << getExpression(context) << endl;
}

int main() {
    printf("Try GetNewCompoundType!\n");
    CompoundImpl* newStruct = NULL;
    srand(time(NULL));
    try{
        printf("-----  Try TryGetNewCompoundType   -----\n");
        newStruct = TryGetNewCompoundType();
    }catch(...) {
        printf("Try GetNewCompoundType failed !!\n");
    }
    try{
        printf("-----  Try TryInitialization   -----\n");
        if(newStruct != NULL)
            TryInitialization(newStruct);
    }catch(...) {
        printf("Try Initialization failed !!\n");
    }
    try{
        printf("-----  Try GetExpression   -----\n");
        TryGetExpression();
    }catch(...) {
        printf("Try GetExpression failed!\n");
    }
}