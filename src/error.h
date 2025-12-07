#ifndef VANCTION_ERROR_H
#define VANCTION_ERROR_H

#include <string>
#include <vector>
#include <stdexcept>

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

// Error namespace containing all specific error classes
namespace vanction_error {
    // Base error class for all Vanction errors
    class VanctionError : public std::runtime_error {
    public:
        VanctionError(const std::string& type, const std::string& message, int line = 1, int column = 1)
            : std::runtime_error(type + ": " + message), type_(type), message_(message), line_(line), column_(column) {}
        
        virtual ~VanctionError() = default;
        
        const std::string& getType() const { return type_; }
        const std::string& getMessage() const { return message_; }
        int getLine() const { return line_; }
        int getColumn() const { return column_; }
        
    private:
        std::string type_;
        std::string message_;
        int line_;
        int column_;
    };
    
    // Specific error classes
    class CError : public VanctionError {
    public:
        explicit CError(const std::string& message, int line = 1, int column = 1)
            : VanctionError("CError", message, line, column) {}
    };
    
    class MethodError : public VanctionError {
    public:
        explicit MethodError(const std::string& message, int line = 1, int column = 1)
            : VanctionError("MethodError", message, line, column) {}
    };
    
    class CompilationError : public VanctionError {
    public:
        explicit CompilationError(const std::string& message, int line = 1, int column = 1)
            : VanctionError("CompilationError", message, line, column) {}
    };
    
    class DivideByZeroError : public VanctionError {
    public:
        explicit DivideByZeroError(const std::string& message, int line = 1, int column = 1)
            : VanctionError("DivideByZeroError", message, line, column) {}
    };
    
    class ValueError : public VanctionError {
    public:
        explicit ValueError(const std::string& message, int line = 1, int column = 1)
            : VanctionError("ValueError", message, line, column) {}
    };
    
    class TokenError : public VanctionError {
    public:
        explicit TokenError(const std::string& message, int line = 1, int column = 1)
            : VanctionError("TokenError", message, line, column) {}
    };
    
    class SyntaxError : public VanctionError {
    public:
        explicit SyntaxError(const std::string& message, int line = 1, int column = 1)
            : VanctionError("SyntaxError", message, line, column) {}
    };
    
    class MainFunctionError : public VanctionError {
    public:
        explicit MainFunctionError(const std::string& message, int line = 1, int column = 1)
            : VanctionError("MainFunctionError", message, line, column) {}
    };
    
    class UnknownError : public VanctionError {
    public:
        explicit UnknownError(const std::string& message, int line = 1, int column = 1)
            : VanctionError("UnknownError", message, line, column) {}
    };
}

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
