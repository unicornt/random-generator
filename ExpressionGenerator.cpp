#include "ExpressionGenerator.h"

const int probTable [] = {2, 1, 1};

enum eTermType getRandomExpressionType()
{
    /*
        default prob:
        // eConstant 1
        eVariable 2
        // eUnaryExpr,
        // eBinaryExpr
        eAssignment 1
        eCommaExpr 1
    */
   return rand() % 10 < 1 ? eVariable : eAssignment;
}

string GetRandomVariable(vector<Variable>* context)
{
    size_t n = context->size();
    size_t i = rand() % n;
    return (*context)[i].name;
}

void GetAllType(vector<Variable>*context, vector<TypeBase*>*AllType)
{
    for(Variable v : *context) {
        if(find(AllType->begin(), AllType->end(), v.type_ptr) != AllType->end()) {
            AllType->push_back(v.type_ptr);
        }
    }
}

const int intExpN = 7;
const int floatExpN = 4;
int f2ip = 15;

string intExpString[] = {
    " + ",
    " - ",
    " * ",
    " / ",
    " % ",
    " << ",
    " >> "
};

string floatExpString[] = {
    " + ",
    " - ",
    " * ",
    " / "
};

string GetRandomVar(vector<Variable>*context)
{
    size_t ni = rand() % context->size();
    return (*context)[ni].name;
}

string GetRandomExpr(bool int_or_float)
{
    // printf("Get Random Expr\n");
    return int_or_float ? floatExpString[rand() % floatExpN] : intExpString[rand() % intExpN];
}

string GenerateIntExp(vector<Variable>*context, vector<Variable>*intContext, vector<Variable>*floatContext)
{
    // printf("Get int Expr\n");
    if(rand() % 10 < 5)
        return GetRandomVar(intContext);
    // printf("Get int Expr 2\n");
    return (rand() % 20 < f2ip ? ("(" + GenerateIntExp(context, intContext, floatContext) + ")") : ("(int)(" + GenerateFloatExp(context, intContext, floatContext) + ")")) +
        GetRandomExpr(0) + 
        (rand() % 20 < f2ip ? ("(" + GenerateIntExp(context, intContext, floatContext) + ")") : ("(int)(" + GenerateFloatExp(context, intContext, floatContext) + ")") );
}

string GenerateFloatExp(vector<Variable>*context, vector<Variable>*intContext, vector<Variable>*floatContext)
{
    printf("Generate float exp\n");
    if(rand() % 10 < 5)
        return GetRandomVar(intContext);
    // printf("Generate float exp 2\n");
    return (rand() % 20 >= f2ip ? ("(" + GenerateFloatExp(context, intContext, floatContext) + ")") : ("(" + GenerateIntExp(context, intContext, floatContext) + ")")) +
        GetRandomExpr(1) +
        (rand() % 20 >= f2ip ? ("(" + GenerateFloatExp(context, intContext, floatContext) + ")") : ("(" + GenerateIntExp(context, intContext, floatContext) + ")"));
}

void SplitIntAndFloat(vector<Variable>*context, vector<Variable>*intContext, vector<Variable>*floatContext)
{
    for(Variable v: *context) {
        if(v.type == "float" || v.type == "double"){
            floatContext->push_back(v);
        }
        else if(v.type != "void"){
            intContext->push_back(v);
        }
    }
}

string getExpression(vector<Variable>context)
{
    enum eTermType tt = getRandomExpressionType();
    vector<Variable>intContext;
    vector<Variable>floatContext;
    SplitIntAndFloat(&context, &intContext, &floatContext);
    if(floatContext.size() == 0)
        f2ip = 0;
    switch(tt) {
    case eVariable:
        return GetRandomVariable(&context);
        break;
    case eAssignment:
        return rand() % 5 < 3 ? GenerateIntExp(&context, &intContext, &floatContext) : GenerateFloatExp(&context, &intContext, &floatContext);
        break;
    }
    return "no return value";
}