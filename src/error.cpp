#include "error.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

// Error class implementation
Error::Error(ErrorType type, const std::string& message, const std::string& filePath, int line, int column)
    : type(type), message(message), filePath(filePath), line(line), column(column) {}

ErrorType Error::getType() const {
    return type;
}

std::string Error::getMessage() const {
    return message;
}

std::string Error::getFilePath() const {
    return filePath;
}

int Error::getLine() const {
    return line;
}

int Error::getColumn() const {
    return column;
}

std::string Error::getTypeString() const {
    switch (type) {
        case ErrorType::CError: return "CError";
        case ErrorType::MethodError: return "MethodError";
        case ErrorType::CompilationError: return "CompilationError";
        case ErrorType::DivideByZeroError: return "DivideByZeroError";
        case ErrorType::ValueError: return "ValueError";
        case ErrorType::TokenError: return "TokenError";
        case ErrorType::SyntaxError: return "SyntaxError";
        case ErrorType::MainFunctionError: return "MainFunctionError";
        case ErrorType::UnknownError: return "UnknownError";
        default: return "UnknownError";
    }
}

// Helper function to get absolute path
std::string getAbsolutePath(const std::string& path) {
    char absolutePath[MAX_PATH];
    DWORD result = GetFullPathNameA(path.c_str(), MAX_PATH, absolutePath, NULL);
    if (result == 0) {
        // Failed to get absolute path, return original path
        return path;
    }
    return std::string(absolutePath);
}

// ErrorReporter class implementation
ErrorReporter::ErrorReporter(const std::string& sourceCode, const std::string& filePath)
    : sourceCode(sourceCode), filePath(getAbsolutePath(filePath)) {}

void ErrorReporter::report(const Error& error) {
    // Get error context
    std::vector<std::string> context = getErrorContext(error.getLine());
    
    // Print error header in blue
    std::cout << blue("error occurred to:") << std::endl;
    
    // Print file path with line and column in purple
    std::cout << "    " << purple(filePath + ":" + std::to_string(error.getLine()) + ":" + std::to_string(error.getColumn())) << std::endl;
    
    // Print error context in purple
    for (size_t i = 0; i < context.size(); ++i) {
        int actualLine = error.getLine() - 2 + i;
        std::cout << "    " << purple(context[i]) << std::endl;
        
        // Print error indicator (^^^^^^^) in red if this is the error line
        if (i == 1) { // Middle line is the error line
            int lineLength = context[i].length();
            std::string indicator = generateErrorIndicator(lineLength, error.getColumn());
            std::cout << "    " << red(indicator) << std::endl;
        }
    }
    
    // Print error type and message in purple
    std::cout << purple(error.getTypeString()) << ": " << blue(error.getMessage()) << reset() << std::endl;
    std::cout << std::endl;
}

std::vector<std::string> ErrorReporter::getErrorContext(int errorLine) {
    std::vector<std::string> lines;
    std::istringstream stream(sourceCode);
    std::string line;
    
    // Read all lines from source code
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    
    std::vector<std::string> context;
    
    // Adjust errorLine to be zero-based
    int zeroBasedErrorLine = errorLine - 1;
    
    // Calculate start and end line indices for context (three lines total)
    int startLine = std::max(0, zeroBasedErrorLine - 1);
    int endLine = std::min(static_cast<int>(lines.size()), zeroBasedErrorLine + 2);
    
    // Add context lines
    for (int i = startLine; i < endLine; ++i) {
        context.push_back(lines[i]);
    }
    
    // If we don't have enough lines, pad with empty strings to ensure we always show three lines
    while (context.size() < 3) {
        context.insert(context.begin(), "");
    }
    
    return context;
}

std::string ErrorReporter::generateErrorIndicator(int lineLength, int column) {
    std::string indicator;
    
    // Generate spaces up to the error column
    for (int i = 0; i < column - 1; ++i) {
        indicator += " ";
    }
    
    // Generate caret (^) characters to indicate the error
    // We'll use 7 carets by default, but adjust if it goes beyond the line
    int caretCount = 7;
    for (int i = 0; i < caretCount && (column - 1 + i) < lineLength; ++i) {
        indicator += "^";
    }
    
    return indicator;
}

std::string ErrorReporter::errorTypeToString(ErrorType type) {
    switch (type) {
        case ErrorType::CError: return "C++ Error";
        case ErrorType::CompilationError: return "Compilation Error";
        case ErrorType::DivideByZeroError: return "Divide by Zero Error";
        case ErrorType::ValueError: return "Value Error";
        case ErrorType::TokenError: return "Token Error";
        case ErrorType::SyntaxError: return "Syntax Error";
        case ErrorType::MainFunctionError: return "Main Function Error";
        default: return "Unknown Error";
    }
}

// Colored output helper methods
std::string ErrorReporter::blue(const std::string& text) {
    return "\033[34m" + text + "\033[0m";
}

std::string ErrorReporter::purple(const std::string& text) {
    return "\033[35m" + text + "\033[0m";
}

std::string ErrorReporter::red(const std::string& text) {
    return "\033[31m" + text + "\033[0m";
}

std::string ErrorReporter::reset() {
    return "\033[0m";
}
