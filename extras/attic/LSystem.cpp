
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


#include "LSystem.h"
#include "LabText/TextScanner.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <map>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

#define VERBOSE 0

struct Statement
{
    enum Tag { kCall, kInstance, kTransform, kPush, kPop };
    
    Statement(Tag t, float scale, const glm::mat4x4& m, const std::string& s)
    : tag(t), matrix(m), arg(s), scale(scale)
    , argIndex(-1) { }

    Statement(const Statement& rhs)
    : tag(rhs.tag), arg(rhs.arg), argIndex(rhs.argIndex), matrix(rhs.matrix), scale(rhs.scale) { }

    Tag tag;
    std::string arg;
    int argIndex;    
    glm::mat4x4 matrix;
    float scale;
};

struct RuleDetail
{
    RuleDetail(float w, int iterations)
    : weight(w), iterations(iterations) { }
    
    std::vector<Statement> statements;
    float weight;
    int iterations;
};

struct RuleSet
{
    RuleSet(const std::string& n, const std::string& s, int d)
    : name(n), successor(s), max_depth(d)
    , successorIndex(-1), min_size(0.001f) { }
    
    RuleSet()
    : max_depth(1) { }
    
    ~RuleSet()
    {
        for (std::vector<RuleDetail*>::iterator i = rule.begin(); i != rule.end(); ++i)
            delete *i;
    }
    
    const RuleDetail* pick_rule(LSystem::Rand* random)
    {
        float sum = 0;
        for (std::vector<RuleDetail*>::const_iterator i = rule.begin(); i != rule.end(); ++i) {
            sum += (*i)->weight;
        }
        float n = random->rand(0, sum);

        //printf("sum: %f n: %f\n", sum, n);

        for (std::vector<RuleDetail*>::const_iterator i = rule.begin(); i != rule.end(); ++i) {
            if (n < (*i)->weight)
                return *i;
            n = n - (*i)->weight;
        }
        return rule.back();
    }
    
    std::vector<RuleDetail*> rule;
    std::string name;
    std::string successor;
    int successorIndex;
    int max_depth;
    float min_size;
};

class LSystemRules
{
public:
    ~LSystemRules()
    {
        for (std::vector<RuleSet*>::iterator i = rulesets.begin(); i != rulesets.end(); ++i)
            delete *i;
    }
    
    int find(const char* s)
    {
        int n = 0;
        for (std::vector<std::string>::iterator i = rulesetnames.begin(); i != rulesetnames.end(); ++i, ++n)
            if (!strcmp(s, (*i).c_str()))
                return n;
        
        return -1;
    }
    
    RuleSet* get(int i)
    {
        if (i > rulesets.size())
            return 0;
        return rulesets[i];
    }
    
    RuleSet* add(const char* s, RuleSet* r) 
    {
        /// @TODO
        //if (find(s) != -1)
        //   complain();
        rulesetnames.push_back(std::string(s));
        rulesets.push_back(r);
        return r;
    }
    
    std::vector<RuleSet*> rulesets;
    std::vector<std::string> rulesetnames;
};

LSystem::LSystem(std::ostream& console)
: _console(console)
, max_depth(1)
, rules(new LSystemRules())
, _random(new Rand())
{
}

LSystem::~LSystem()
{
    delete _random;
    delete rules;
}

const char* parseVec(const char* s, const char* e, glm::vec3& r)
{
    if (*s == 'x') {
        ++s;
        s = tsScanForNonWhiteSpace(s, e);
        r.x = atof(s);
        s = tsScanForNonWhiteSpace(s, e);
        s = tsScanForWhiteSpace(s, e);
    }
    else if (*s == 'y') {
        ++s;
        s = tsScanForNonWhiteSpace(s, e);
        r.y = atof(s);
        s = tsScanForNonWhiteSpace(s, e);
        s = tsScanForWhiteSpace(s, e);
    }
    else if (*s == 'z') {
        ++s;
        s = tsScanForNonWhiteSpace(s, e);
        r.z = atof(s);
        s = tsScanForNonWhiteSpace(s, e);
        s = tsScanForWhiteSpace(s, e);
    }
    else if (*s == 'a') {
        ++s;
        s = tsScanForNonWhiteSpace(s, e);
        r.x = atof(s);
        r.y = r.x;
        r.z = r.x;
        s = tsScanForNonWhiteSpace(s, e);
        s = tsScanForWhiteSpace(s, e);
    }
    else {
        ++s;
        s = tsScanForNonWhiteSpace(s, e);
        r.x = atof(s);
        s = tsScanForNonWhiteSpace(s, e);
        s = tsScanForWhiteSpace(s, e);
        r.y = atof(s);
        s = tsScanForNonWhiteSpace(s, e);
        s = tsScanForWhiteSpace(s, e);
        r.z = atof(s);
        s = tsScanForNonWhiteSpace(s, e);
        s = tsScanForWhiteSpace(s, e);
    }
    return s;
}

struct RuleExecutionContext
{
    RuleExecutionContext(const RuleSet* r, const RuleDetail* ruleDetail, const glm::mat4x4& m)
    : rule(r), ruleDetail(ruleDetail), matrix(m)
    , iterations(ruleDetail->iterations), currStatement(0) { }

    int iterations, currStatement;
    const RuleSet* rule;
    const RuleDetail* ruleDetail;
    glm::mat4x4 matrix;

private:
    RuleExecutionContext(const RuleExecutionContext& rhs)
    : rule(rhs.rule), ruleDetail(rhs.ruleDetail), matrix(rhs.matrix)
    , iterations(rhs.rule->max_depth), currStatement(0) { }

    RuleExecutionContext& operator= (const RuleExecutionContext& rhs) {
        rule = rhs.rule;
        ruleDetail = rhs.ruleDetail;
        matrix = rhs.matrix;
        return *this;
    }
};

#if 0
static void printMatrix(const char* name, const glm::mat4x4& v) {
    printf("%s\n", name);
    for (int y = 0; y < 4; ++y)
        printf("%2.2f %2.2f %2.2f %2.2f\n", v[y][0], v[y][1], v[y][2], v[y][3]);
    printf("--------------------------\n");
}
#endif


void LSystem::parseRules(char const*const rulesText) {
    char const*const end = rulesText + strlen(rulesText);
    char const* curr = rulesText;

    glm::vec3 referenceUnitVector(1,0,0);
    {
        glm::mat4x4 m = glm::eulerAngleYXZ(0.2f, 0.2f, 0.2f);
        referenceUnitVector = glm::vec3(m * glm::vec4(referenceUnitVector, 1.0f));
    }

    while (curr < end) {
        char const* token;
        uint32_t tokenLen;
        curr = tsGetToken(curr, end, ':', &token, &tokenLen);
        if (curr >= end) {
            printf("Malformed lsystem file");
            break;
        }
        ++curr; // skip ':'
        if (!strncmp(token, "system", tokenLen)) {
            curr = tsGetTokenAlphaNumeric(curr, end, &token, &tokenLen);
            _name.assign(token, tokenLen);
        }
        else if (!strncmp(token, "max_depth", tokenLen)) {
            curr = tsGetInt32(curr, end, &max_depth);
        }
        else if (!strncmp(token, "rule", tokenLen)) {
            string shape;

            curr = tsGetTokenAlphaNumeric(curr, end, &token, &tokenLen);
            string name;
            name.assign(token, tokenLen);
            int rule_max_depth = 1;
            RuleSet* newRule;
            int ruleIndex = rules->find(name.c_str());
            if (ruleIndex >= 0)
                newRule = rules->get(ruleIndex);
            else {
                newRule = new RuleSet(name, "", rule_max_depth);
                rules->add(name.c_str(), newRule);
            }
            newRule->rule.push_back(new RuleDetail(1, 1));
            RuleDetail* newRuleDetail = newRule->rule.back();

            glm::mat4x4 m(1);
            float scale = 1.0f;

            while (curr < end) {
                char const* rewind = curr;
                curr = tsGetTokenAlphaNumeric(curr, end, &token, &tokenLen);

                if (!strncmp(token, "rule", tokenLen)) {
                    curr = rewind;
                    break;
                }

                if (!strncmp(token, "max_depth", tokenLen)) {
                    ++curr; // skip :
                    curr = tsGetInt32(curr, end, &newRule->max_depth);
                }
                else if (!strncmp(token, "push", tokenLen)) {
                    newRuleDetail->statements.push_back(Statement(Statement::kPush, 1.0f, glm::mat4x4(1), ""));
                }
                else if (!strncmp(token, "pop", tokenLen)) {
                    newRuleDetail->statements.push_back(Statement(Statement::kPop, 1.0f, glm::mat4x4(1), ""));
                }
                else if (!strncmp(token, "weight", tokenLen)) {
                    ++curr; // skip :
                    curr = tsGetFloat(curr, end, &newRuleDetail->weight);
                }
                else if (!strncmp(token, "transform", tokenLen)) {
                    ++curr; // skip :
                    char const* lineEnd = tsScanForLastCharacterOnLine(curr, end);
                    while (curr < lineEnd) {
                        curr = tsSkipCommentsAndWhitespace(curr, end);
                        if (*curr == 'r') {
                            ++curr;
                            glm::vec3 rotate(0);
                            curr = parseVec(curr, lineEnd, rotate);
                            rotate *= 2.0f * float(M_PI) / 360.0f;
                            m *= glm::eulerAngleYXZ(rotate.y, rotate.x, rotate.z);
                        }
                        else if (*curr == 's') {
                            ++curr;
                            glm::vec3 scaleVec(1);
                            curr = parseVec(curr, lineEnd, scaleVec);
                            m = glm::scale(m, scaleVec);
                            glm::vec3 test = scaleVec * referenceUnitVector;
                            scale *= sqrtf(glm::dot(test, test));
                        }
                        else if (*curr == 't') {
                            ++curr;
                            glm::vec3 translate(0);
                            curr = parseVec(curr, lineEnd, translate);
                            m = glm::translate(m, translate);
                        }
                    }
                    newRuleDetail->statements.push_back(Statement(Statement::kTransform, scale, m, ""));
                    scale = 1;
                    m = glm::mat4x4(1);
                }
                else if (!strncmp(token, "iterations", tokenLen)) {
                    ++curr; // skip :
                    curr = tsGetInt32(curr, end, &newRuleDetail->iterations);
                }
                else if (!strncmp(token, "call", tokenLen)) {
                    ++curr; // skip :
                    std::string callRule;
                    curr = tsGetTokenAlphaNumeric(curr, end, &token, &tokenLen);
                    callRule.assign(token, tokenLen);
                    newRuleDetail->statements.push_back(Statement(Statement::kCall, 1.0f, glm::mat4x4(1), callRule));
                    //printMatrix("call", m);
                }
                else if (!strncmp(token, "next", tokenLen)) {
                    ++curr; // skip :
                    curr = tsGetTokenAlphaNumeric(curr, end, &token, &tokenLen);
                    newRule->successor.assign(token, tokenLen);
                }
                else if (!strncmp(token, "shape", tokenLen)) {
                    ++curr; // skip :
                    curr = tsGetTokenAlphaNumeric(curr, end, &token, &tokenLen);
                    shape.assign(token, tokenLen);
                    newRuleDetail->statements.push_back(Statement(Statement::kInstance, 1.0f, glm::mat4x4(1), shape));
                }
            }
        }
        else {
            _console << "Malformed Lsystem source text" << std::endl;
            return;
        }
    }

    //    the successors belong on the RULES not on the RULESETS

    // turn all successor names into successor indices.
    for (std::vector<RuleSet*>::iterator i = rules->rulesets.begin(); i != rules->rulesets.end(); ++i) {
        (*i)->successorIndex = rules->find((*i)->successor.c_str());
        for (std::vector<RuleDetail*>::iterator j = (*i)->rule.begin(); j != (*i)->rule.end(); ++j) {
            for (std::vector<Statement>::iterator k = (*j)->statements.begin(); k != (*j)->statements.end(); ++k) {
                (*k).argIndex = rules->find((*k).arg.c_str());
                if ((*k).argIndex ==-1) {
                    _console << "Couldn't find " <<(*k).arg.c_str() << std::endl;
                }
            }
        }
    }
}


#if 1
int LSystem::instance(int seed, bool showProgress)
{
    showProgress = true;
    if (showProgress)
        _console << "Evaluating Lindenmayer system\n";

    // get first rule
    int ruleIndex = rules->find("entry");
    if (ruleIndex < 0)
        return 0;

    RuleSet* ruleset = rules->get(ruleIndex);
    const RuleDetail* entry = ruleset->pick_rule(_random);

    std::vector<RuleExecutionContext*> stack;
    int maximum_depth_encountered = 0;
    _random->seed(seed);

    // push first rule
    glm::mat4x4 currMatrix(1);
    float currScale = 1.0f;

    stack.push_back(new RuleExecutionContext(ruleset, entry, currMatrix));

    std::vector<glm::mat4x4> matrixStack;
    std::vector<float> scaleStack;

    while (stack.begin() != stack.end()) {
        auto ruleExecContext = stack.back();
        const RuleDetail* ruleDetail = ruleExecContext->ruleDetail;
        bool looping = true;
        for (int i = ruleExecContext->currStatement; i < ruleDetail->statements.size() && looping; ++i) {
            auto statement = ruleDetail->statements[i];
            switch (statement.tag) {
                case Statement::kTransform:
                    currMatrix *= statement.matrix;
                    currScale *= statement.scale;
                    break;
                case Statement::kCall:
                    ruleIndex = statement.argIndex;
                    if (ruleIndex >= 0) {
                        ruleset = rules->get(ruleIndex);
                        stack.push_back(new RuleExecutionContext(ruleset, ruleset->pick_rule(_random), currMatrix));
#if VERBOSE
                        _console << "call:" << stack.back()->rule->name.c_str() << std::endl;
#endif
                    }
                    ruleExecContext->currStatement = i+1;     // the next statement to run
                    looping = false;
                    break;
                case Statement::kInstance:
#if VERBOSE
                    _console << "instance:xxx" << std::endl;
#endif
                    shapes.push_back(Shape(statement.arg, ruleExecContext->matrix, (int) stack.size()));
                    //printMatrix(ruleset->name.c_str(), currMatrix);
                    break;
                case Statement::kPush:
#if VERBOSE
                    _console << "push" << std::endl;
#endif
                    matrixStack.push_back(currMatrix);
                    scaleStack.push_back(currScale);
                    break;
                case Statement::kPop:
#if VERBOSE
                    _console << "pop" << std::endl;
#endif
                    if (matrixStack.size() == 0) {
                        if (showProgress)
                            _console << "Rule - pop not matched to push\n";
                    }
                    else {
                        currMatrix = matrixStack.back();
                        matrixStack.pop_back();
                        currScale = scaleStack.back();
                        scaleStack.pop_back();
                    }
                    break;
                default:
                    if (showProgress)
                        _console << "Malformed rule\n";
                    break;
            }
        }
        if (!looping)
            continue;

        --(*ruleExecContext).iterations;

        bool largeEnough = currScale > ruleset->min_size;
        bool termination = (ruleExecContext->iterations <= 0) || !largeEnough;

        if (termination) {
            int ruleIndex = -1;
            if (!ruleExecContext->rule->successor.empty()) {
                ruleIndex = ruleExecContext->rule->successorIndex;
            }
            stack.pop_back();

            if ((ruleIndex >= 0) && largeEnough) {
                ruleset = rules->get(ruleIndex);
                stack.push_back(new RuleExecutionContext(ruleset, ruleset->pick_rule(_random), ruleExecContext->matrix));
            }

            delete ruleExecContext;
        }
        else {
            // prepare for next iteration
            ruleExecContext->currStatement = 0;
        }
    }


    if (showProgress)
        _console << "\nGenerated " << shapes.size() << " shapes\n";

    return maximum_depth_encountered;
}
#else
int LSystem::instance(int seed, bool showProgress)
{  
    if (showProgress)
        _console << "Evaluating Lindenmayer system\n";
    
    int maximum_depth_encountered = 0;
    _random->seed(seed);
    
    int ruleIndex = rules->find("entry");
    if (ruleIndex < 0)
        return 0;
    RuleSet* ruleset = rules->get(ruleIndex);
    
    const RuleDetail* entry = ruleset->pick_rule(_random);

    std::vector<StackRule> stack;
    glm::mat4x4 identity(1);

    stack.push_back(StackRule(ruleset, entry, 0, identity));
    
    size_t progressCount = 0;
    while (stack.size() > 0) {
        
        if (showProgress && (shapes.size() > progressCount + 1000)) {
            _console << ".";
            progressCount = shapes.size();
        }
            
        StackRule sr = stack.back();

        int local_max_depth = sr.rule->max_depth > 0 ? sr.rule->max_depth : max_depth;
                
        if (stack.size() >= max_depth)
            continue;

        if (sr.iterations >= sr.rule->count) {
            stack.pop_back();
            if (!sr.rule->successor.empty()) {
                ruleIndex = sr.rule->successorIndex;
                if (ruleIndex >= 0) {
                    ruleset = rules->get(ruleIndex);
                    stack.push_back(StackRule(ruleset, ruleset->pick_rule(_random), 0, sr.matrix));
                }
            }
            continue;
        }

        // TODO: Is this test meaningless?
        if (sr.stack_depth >= local_max_depth) {
            stack.pop_back();
            if (!sr.rule->successor.empty()) {
                ruleIndex = sr.rule->successorIndex;
                if (ruleIndex >= 0) {
                    ruleset = rules->get(ruleIndex);
                    stack.push_back(StackRule(ruleset, ruleset->pick_rule(_random), 0, sr.matrix));
                }
            }
            continue;
        }

        ++sr.iterations;
        if (sr.iterations >= sr.rule->iterations) {
            stack.pop_back();
        }
 
        const RuleDetail* ruleDetail = sr.ruleDetail;
        for (std::vector<Statement>::const_iterator s = ruleDetail->statements.begin(); s != ruleDetail->statements.end(); ++s) {
            const glm::mat4x4& xform = (*s).matrix;
            for (int n = 0; n < (*s).count; ++n) {
                sr.matrix *= xform;
                if ((*s).tag == Statement::kCall) {
                    ruleIndex = (*s).argIndex ;
                    if (ruleIndex >= 0) {
                        ruleset = rules->get(ruleIndex);
                        stack.push_back(StackRule(ruleset, ruleset->pick_rule(_random), sr.stack_depth + 1, sr.matrix));
                    }
                }
                else if ((*s).tag == Statement::kInstance) {
                    int d = sr.stack_depth;
                    if (d > maximum_depth_encountered)
                        maximum_depth_encountered = d;
                    shapes.push_back(Shape((*s).arg, sr.matrix, d));
                }
                else {
                    if (showProgress)
                        _console << "Malformed rule\n";
                    break;
                }
            }
        }
    }

    if (showProgress)
        _console << "\nGenerated " << shapes.size() << " shapes\n";
    
    return maximum_depth_encountered;
}
#endif

#if 0
void LSystem::createRules(XmlTree* x)
{
    .    XmlTree rulesRoot = x->getChild("rules");
    .    max_depth = rulesRoot.getAttributeValue<int>("max_depth", 100);

    for( XmlTree::Iter child = rulesRoot.begin(); child != rulesRoot.end(); ++child ) {
        if (child->getTag() == "rule") {
            .            std::string name = child->getAttributeValue<std::string>( "name", "default" );
            .            std::string successor = child->getAttributeValue<std::string>( "next", "");
            .            int rule_max_depth = child->getAttributeValue<int>("max_depth", 0);
            .            int weight = child->getAttributeValue<int>("weight", 100);
            .            mConsole << name << "->" << successor << " " << rule_max_depth << " " << weight << std::endl;

            .            RuleSet* newRule;
            .           int ruleIndex = rules->find(name.c_str());
            .          if (ruleIndex >= 0)
                .             newRule = rules->get(ruleIndex);
            .        else {
                .           newRule = new RuleSet(name, successor, rule_max_depth);
                .          rules->add(name.c_str(), newRule);
                .     }

            .           newRule->rule.push_back(new RuleDetail(weight));
            .          RuleDetail* newRuleDetail = newRule->rule.back();

            for( XmlTree::Iter call = child->begin(); call != child->end(); ++call) {
                std::string tag = call->getTag();
                if (tag == "call" || tag == "instance") {
                    std::string rule = call->getAttributeValue<std::string>( "rule", "" );
                    std::string transforms = call->getAttributeValue<std::string>( "transforms", "" );
                    std::string shape = call->getAttributeValue<std::string>( "shape", "" );
                    int count = call->getAttributeValue<int>("count", 1);

                    Matrix44f m;
                    m.setToIdentity();

                    if (transforms.length() > 0) {
                        const char* s = transforms.c_str();
                        const char* e = s + transforms.length();
                        while (*s) {
                            if (*s == ' ')
                                ++s;
                            else if (*s == 'r') {
                                ++s;
                                Vec3f rotate(0,0,0);
                                s = parseVec(s, e, rotate);
                                rotate *= 2.0f * float(M_PI) / 360.0f;
                                m.rotate(rotate);
                            }
                            else if (*s == 's') {
                                ++s;
                                Vec3f scale(1,1,1);
                                s = parseVec(s, e, scale);
                                m.scale(scale);
                            }
                            else if (*s == 't') {
                                ++s;
                                Vec3f translate(0,0,0);
                                s = parseVec(s, e, translate);
                                m.translate(translate);
                            }
                        }
                    }

                    if (tag == "call")
                        newRuleDetail->statements.push_back(Statement(Statement::kCall, count, m, rule));
                    else if (tag == "instance")
                        newRuleDetail->statements.push_back(Statement(Statement::kInstance, count, m, shape));

                    mConsole << "\t" << tag << ": " << rule << " (" << count << ") " <<
                    " " << shape << std::endl;
                }
            }
        }
    }

    //    the successors belong on the RULES not on the RULESETS

    // turn all successor names into successor indices.
    for (std::vector<RuleSet*>::iterator i = rules->rulesets.begin(); i != rules->rulesets.end(); ++i) {
        (*i)->successorIndex = rules->find((*i)->successor.c_str());
        for (std::vector<RuleDetail*>::iterator j = (*i)->rule.begin(); j != (*i)->rule.end(); ++j) {
            for (std::vector<Statement>::iterator k = (*j)->statements.begin(); k != (*j)->statements.end(); ++k) {
                (*k).argIndex = rules->find((*k).arg.c_str());
            }
        }
    }
}
#endif

