#ifndef VANCTION_ERROR_H
#define VANCTION_ERROR_H

#include <string>
#include <vector>

// Error type enumeration
enum class ErrorType {
    CError,
    MethodError,
    CompilationError,
    DivideByZeroError,
    ValueError,
    TokenError,
    SyntaxError,
    MainFunctionError,
    UnknownError
};

// Error class to represent an error with all relevant information
class Error {
public:
    Error(ErrorType type, const std::string& message, const std::string& filePath, int line, int column);
    
    // Getters
    ErrorType getType() const;
    std::string getMessage() const;
    std::string getFilePath() const;
    int getLine() const;
    int getColumn() const;
    
    // Convert error type to string
    std::string getTypeString() const;
    
private:
    ErrorType type;
    std::string message;
    std::string filePath;
    int line;
    int column;
};

// Error reporter class to format and report errors
class ErrorReporter {
public:
    ErrorReporter(const std::string& sourceCode, const std::string& filePath);
    
    // Report an error with formatted output
    void report(const Error& error);
    
private:
    std::string sourceCode;
    std::string filePath;
    
    // Get lines of source code around the error
    std::vector<std::string> getErrorContext(int errorLine);
    
    // Generate the error indicator (^^^^^^^)
    std::string generateErrorIndicator(int lineLength, int column);
    
    // Convert error type to display string
    std::string errorTypeToString(ErrorType type);
    
    // Helper methods for colored output
    std::string blue(const std::string& text);
    std::string purple(const std::string& text);
    std::string red(const std::string& text);
    std::string reset();
};

#endif // VANCTION_ERROR_H
