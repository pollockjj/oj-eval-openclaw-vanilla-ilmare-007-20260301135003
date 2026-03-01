/*
 * File: program.cpp
 * -----------------
 * This file implements the Program class.
 */

#include "program.hpp"

Program::Program() = default;

Program::~Program() {
    clear();
}

void Program::clear() {
    for (auto &p : parsedStatements) {
        delete p.second;
    }
    sourceLines.clear();
    parsedStatements.clear();
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    sourceLines[lineNumber] = line;
    // If there was a parsed statement, delete it since source changed
    auto it = parsedStatements.find(lineNumber);
    if (it != parsedStatements.end()) {
        delete it->second;
        parsedStatements.erase(it);
    }
}

void Program::removeSourceLine(int lineNumber) {
    sourceLines.erase(lineNumber);
    auto it = parsedStatements.find(lineNumber);
    if (it != parsedStatements.end()) {
        delete it->second;
        parsedStatements.erase(it);
    }
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = sourceLines.find(lineNumber);
    if (it == sourceLines.end()) return "";
    return it->second;
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    auto it = parsedStatements.find(lineNumber);
    if (it != parsedStatements.end()) {
        delete it->second;
    }
    parsedStatements[lineNumber] = stmt;
}

Statement *Program::getParsedStatement(int lineNumber) {
    auto it = parsedStatements.find(lineNumber);
    if (it == parsedStatements.end()) return nullptr;
    return it->second;
}

int Program::getFirstLineNumber() {
    if (sourceLines.empty()) return -1;
    return sourceLines.begin()->first;
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = sourceLines.upper_bound(lineNumber);
    if (it == sourceLines.end()) return -1;
    return it->first;
}
