/*
 * File: statement.h
 * -----------------
 * This file defines the Statement abstract type and its subclasses.
 */

#ifndef _statement_h
#define _statement_h

#include <string>
#include <sstream>
#include "evalstate.hpp"
#include "exp.hpp"
#include "Utils/tokenScanner.hpp"
#include "parser.hpp"
#include "Utils/error.hpp"
#include "Utils/strlib.hpp"

class Program;

/*
 * Class: Statement
 * ----------------
 * Abstract base class for BASIC statements.
 */
class Statement {

public:
    Statement();
    virtual ~Statement();
    virtual void execute(EvalState &state, Program &program) = 0;
};

// Special signal values for control flow
// We use a simple int field in EvalState-like approach, but simpler:
// We'll use a struct to signal control flow from execute.

// Instead we'll use exceptions for control flow signaling.

struct GotoSignal {
    int targetLine;
    GotoSignal(int line) : targetLine(line) {}
};

struct EndSignal {};

/*
 * RemStatement: REM comment - does nothing
 */
class RemStatement : public Statement {
public:
    RemStatement();
    ~RemStatement() override;
    void execute(EvalState &state, Program &program) override;
};

/*
 * LetStatement: LET var = exp
 */
class LetStatement : public Statement {
public:
    LetStatement(Expression *exp);
    ~LetStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *exp;
};

/*
 * PrintStatement: PRINT exp
 */
class PrintStatement : public Statement {
public:
    PrintStatement(Expression *exp);
    ~PrintStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *exp;
};

/*
 * InputStatement: INPUT var
 */
class InputStatement : public Statement {
public:
    InputStatement(const std::string &varName);
    ~InputStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    std::string varName;
};

/*
 * EndStatement: END - terminates program
 */
class EndStatement : public Statement {
public:
    EndStatement();
    ~EndStatement() override;
    void execute(EvalState &state, Program &program) override;
};

/*
 * GotoStatement: GOTO lineNumber
 */
class GotoStatement : public Statement {
public:
    GotoStatement(int targetLine);
    ~GotoStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    int targetLine;
};

/*
 * IfStatement: IF exp1 op exp2 THEN lineNumber
 */
class IfStatement : public Statement {
public:
    IfStatement(Expression *lhs, const std::string &op, Expression *rhs, int targetLine);
    ~IfStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *lhs;
    Expression *rhs;
    std::string op;
    int targetLine;
};

#endif
