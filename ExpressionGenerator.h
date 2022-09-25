#ifndef EXPRESSION_GENERATOR_H
#define EXPRESSION_GENERATOR_H

#include "common/type.h"
#include "Random.h"
#include "common/common.h"
#include "common/header.h"

class ExpressionGenerator {
public:
    CompoundImpl* generate(vector<Variable>*);
    static ExpressionGenerator* createInstance();
    ExpressionGenerator(unsigned long seed):seed(seed){}
    string make_random_expression();
private:
    unsigned long seed;
};

string getExpression(vector<Variable>);

enum eTermType
{
	// eConstant,
	eVariable,
	// eUnaryExpr,
	// eBinaryExpr,
	// eFunction,
	eAssignment,
	eCommaExpr,
	eLhs
};

enum intExp
{
    eAdd,
    eSub,
    eMult,
    eDiv,
    eMod,
    eLsh,
    eRsh
};

enum floatExp
{
    eAddF,
    eSubF,
    eMultF,
    eDivF
};

#endif