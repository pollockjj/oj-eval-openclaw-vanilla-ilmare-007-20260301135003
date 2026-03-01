/*
 * File: statement.cpp
 * -------------------
 * Implements Statement subclasses for the BASIC interpreter.
 */

#include "statement.hpp"
#include "program.hpp"
#include <iostream>

/* Base class */
Statement::Statement() = default;
Statement::~Statement() = default;

/* RemStatement */
RemStatement::RemStatement() = default;
RemStatement::~RemStatement() = default;
void RemStatement::execute(EvalState &state, Program &program) {
    // Do nothing
}

/* LetStatement */
LetStatement::LetStatement(Expression *exp) : exp(exp) {}
LetStatement::~LetStatement() { delete exp; }
void LetStatement::execute(EvalState &state, Program &program) {
    exp->eval(state);
}

/* PrintStatement */
PrintStatement::PrintStatement(Expression *exp) : exp(exp) {}
PrintStatement::~PrintStatement() { delete exp; }
void PrintStatement::execute(EvalState &state, Program &program) {
    std::cout << exp->eval(state) << std::endl;
}

/* InputStatement */
InputStatement::InputStatement(const std::string &varName) : varName(varName) {}
InputStatement::~InputStatement() = default;
void InputStatement::execute(EvalState &state, Program &program) {
    while (true) {
        std::cout << " ? ";
        std::string line;
        getline(std::cin, line);
        if (std::cin.fail()) error("INPUT: End of file");
        // Try to parse as integer
        // Must be a valid integer: optional leading minus, then digits
        bool valid = true;
        if (line.empty()) {
            valid = false;
        } else {
            size_t start = 0;
            if (line[0] == '-') start = 1;
            if (start >= line.size()) valid = false;
            else {
                for (size_t i = start; i < line.size(); i++) {
                    if (!isdigit(line[i])) {
                        valid = false;
                        break;
                    }
                }
            }
        }
        if (valid) {
            int value = stringToInteger(line);
            state.setValue(varName, value);
            return;
        } else {
            std::cout << "INVALID NUMBER" << std::endl;
        }
    }
}

/* EndStatement */
EndStatement::EndStatement() = default;
EndStatement::~EndStatement() = default;
void EndStatement::execute(EvalState &state, Program &program) {
    throw EndSignal();
}

/* GotoStatement */
GotoStatement::GotoStatement(int targetLine) : targetLine(targetLine) {}
GotoStatement::~GotoStatement() = default;
void GotoStatement::execute(EvalState &state, Program &program) {
    if (program.getSourceLine(targetLine).empty()) {
        error("LINE NUMBER ERROR");
    }
    throw GotoSignal(targetLine);
}

/* IfStatement */
IfStatement::IfStatement(Expression *lhs, const std::string &op, Expression *rhs, int targetLine)
    : lhs(lhs), rhs(rhs), op(op), targetLine(targetLine) {}
IfStatement::~IfStatement() {
    delete lhs;
    delete rhs;
}
void IfStatement::execute(EvalState &state, Program &program) {
    int left = lhs->eval(state);
    int right = rhs->eval(state);
    bool condition = false;
    if (op == "=") condition = (left == right);
    else if (op == "<") condition = (left < right);
    else if (op == ">") condition = (left > right);
    if (condition) {
        if (program.getSourceLine(targetLine).empty()) {
            error("LINE NUMBER ERROR");
        }
        throw GotoSignal(targetLine);
    }
}
