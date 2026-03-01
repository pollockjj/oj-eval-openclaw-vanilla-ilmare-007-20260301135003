/*
 * File: Basic.cpp
 * ---------------
 * Main interpreter loop for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include <set>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "statement.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"

/* Function prototypes */
void processLine(std::string line, Program &program, EvalState &state);

static const std::set<std::string> keywords = {
    "REM", "LET", "PRINT", "INPUT", "END", "GOTO", "IF", "THEN",
    "RUN", "LIST", "CLEAR", "QUIT", "HELP"
};

bool isKeyword(const std::string &name) {
    return keywords.count(name) > 0;
}

/* Parse a statement from tokens (for storing in program or direct execution) */
Statement *parseStatement(const std::string &cmd, TokenScanner &scanner) {
    if (cmd == "REM") {
        return new RemStatement();
    }
    if (cmd == "LET") {
        // Parse: var = exp
        Expression *exp = parseExp(scanner);
        if (exp->getType() != COMPOUND || ((CompoundExp *)exp)->getOp() != "=") {
            delete exp;
            error("SYNTAX ERROR");
        }
        // Check variable name is not a keyword
        Expression *lhs = ((CompoundExp *)exp)->getLHS();
        if (lhs->getType() != IDENTIFIER) {
            delete exp;
            error("SYNTAX ERROR");
        }
        std::string varName = ((IdentifierExp *)lhs)->getName();
        if (isKeyword(varName)) {
            delete exp;
            error("SYNTAX ERROR");
        }
        return new LetStatement(exp);
    }
    if (cmd == "PRINT") {
        Expression *exp = parseExp(scanner);
        return new PrintStatement(exp);
    }
    if (cmd == "INPUT") {
        std::string varName = scanner.nextToken();
        if (scanner.getTokenType(varName) != WORD || isKeyword(varName)) {
            error("SYNTAX ERROR");
        }
        if (scanner.hasMoreTokens()) {
            error("SYNTAX ERROR");
        }
        return new InputStatement(varName);
    }
    if (cmd == "END") {
        if (scanner.hasMoreTokens()) {
            error("SYNTAX ERROR");
        }
        return new EndStatement();
    }
    if (cmd == "GOTO") {
        std::string lineStr = scanner.nextToken();
        if (scanner.getTokenType(lineStr) != NUMBER) {
            error("SYNTAX ERROR");
        }
        if (scanner.hasMoreTokens()) {
            error("SYNTAX ERROR");
        }
        int targetLine = stringToInteger(lineStr);
        return new GotoStatement(targetLine);
    }
    if (cmd == "IF") {
        // IF exp1 op exp2 THEN line
        // We need to parse: exp1 <op> exp2 THEN lineNumber
        // The comparison operators are =, <, >
        // Use readE with precedence 1 to avoid consuming '=' as assignment
        Expression *lhs = readE(scanner, 1);
        std::string op = scanner.nextToken();
        if (op != "=" && op != "<" && op != ">") {
            delete lhs;
            error("SYNTAX ERROR");
        }
        Expression *rhs = readE(scanner, 1);
        std::string thenToken = scanner.nextToken();
        if (thenToken != "THEN") {
            delete lhs;
            delete rhs;
            error("SYNTAX ERROR");
        }
        std::string lineStr = scanner.nextToken();
        if (scanner.getTokenType(lineStr) != NUMBER) {
            delete lhs;
            delete rhs;
            error("SYNTAX ERROR");
        }
        if (scanner.hasMoreTokens()) {
            delete lhs;
            delete rhs;
            error("SYNTAX ERROR");
        }
        int targetLine = stringToInteger(lineStr);
        return new IfStatement(lhs, op, rhs, targetLine);
    }
    error("SYNTAX ERROR");
    return nullptr; // unreachable
}

void runProgram(Program &program, EvalState &state) {
    int lineNumber = program.getFirstLineNumber();
    while (lineNumber != -1) {
        Statement *stmt = program.getParsedStatement(lineNumber);
        if (stmt == nullptr) {
            // This shouldn't happen if we parse properly, but skip
            lineNumber = program.getNextLineNumber(lineNumber);
            continue;
        }
        try {
            stmt->execute(state, program);
            lineNumber = program.getNextLineNumber(lineNumber);
        } catch (GotoSignal &g) {
            lineNumber = g.targetLine;
        } catch (EndSignal &) {
            return;
        }
    }
}

/* Main program */
int main() {
    EvalState state;
    Program program;
    while (true) {
        try {
            std::string input;
            getline(std::cin, input);
            if (std::cin.fail()) return 0;
            if (input.empty()) continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    std::string token = scanner.nextToken();
    if (token.empty()) return;

    TokenType type = scanner.getTokenType(token);

    // Check if this is a line number
    if (type == NUMBER) {
        int lineNumber = stringToInteger(token);
        // Check if there's anything after the line number
        if (!scanner.hasMoreTokens()) {
            // Delete the line
            program.removeSourceLine(lineNumber);
            return;
        }
        // Store the line and parse the statement
        std::string cmd = scanner.nextToken();
        try {
            Statement *stmt = parseStatement(cmd, scanner);
            program.addSourceLine(lineNumber, line);
            program.setParsedStatement(lineNumber, stmt);
        } catch (ErrorException &) {
            throw;
        }
        return;
    }

    // Direct commands
    if (token == "RUN") {
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
        runProgram(program, state);
        return;
    }
    if (token == "LIST") {
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
        int lineNumber = program.getFirstLineNumber();
        while (lineNumber != -1) {
            std::cout << program.getSourceLine(lineNumber) << std::endl;
            lineNumber = program.getNextLineNumber(lineNumber);
        }
        return;
    }
    if (token == "CLEAR") {
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
        program.clear();
        state.Clear();
        return;
    }
    if (token == "QUIT") {
        program.clear();
        exit(0);
    }
    if (token == "HELP") {
        std::cout << "Yet another basic interpreter" << std::endl;
        return;
    }

    // Direct execution of statements: only LET, PRINT, INPUT allowed
    if (token == "LET" || token == "PRINT" || token == "INPUT") {
        Statement *stmt = nullptr;
        try {
            stmt = parseStatement(token, scanner);
            stmt->execute(state, program);
        } catch (GotoSignal &) {
            delete stmt;
            error("SYNTAX ERROR");
        } catch (EndSignal &) {
            delete stmt;
            return;
        } catch (ErrorException &) {
            delete stmt;
            throw;
        }
        delete stmt;
        return;
    }

    // REM, END, GOTO, IF in direct mode are SYNTAX ERROR
    if (token == "REM" || token == "END" || token == "GOTO" || token == "IF") {
        error("SYNTAX ERROR");
    }

    // Anything else is a syntax error
    error("SYNTAX ERROR");
}
