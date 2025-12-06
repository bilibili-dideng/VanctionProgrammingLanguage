#include "parser.h"
#include "code_generator.h"
#include "error.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <variant>
#include <any>
#include <stdexcept>

// Read file content
std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filePath << std::endl;
        exit(1);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    file.close();
    return content;
}

// Write file content
void writeFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot write to file " << filePath << std::endl;
        exit(1);
    }
    
    file << content;
    file.close();
}

// Call external compiler
int compileWithGCC(const std::string& cppFile, const std::string& outputFile) {
    std::string command = ".\\mingw64\\bin\\g++.exe " + cppFile + " -o " + outputFile;
    std::cout << "Executing command: " << command << std::endl;
    
    return system(command.c_str());
}

// Get filename without extension
std::string getFileNameWithoutExt(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return filePath;
    }
    return filePath.substr(0, dotPos);
}

// Type for variable values
using Value = std::variant<int, char, std::string, bool, float, double, std::monostate>;

// Global variable environment
std::map<std::string, Value> variables;

// Forward declarations for execute functions
void executeFunctionDeclaration(FunctionDeclaration* func);
void executeStatement(ASTNode* stmt);
Value executeExpression(Expression* expr);
Value executeFunctionCall(FunctionCall* call);

// Execute program
void executeProgram(Program* program) {
    // Execute all function declarations
    for (auto decl : program->declarations) {
        if (auto func = dynamic_cast<FunctionDeclaration*>(decl)) {
            executeFunctionDeclaration(func);
        }
    }
}

// Execute function declaration
void executeFunctionDeclaration(FunctionDeclaration* func) {
    // Only execute main function in interpret mode
    if (func->name == "main") {
        // Execute function body
        for (auto stmt : func->body) {
            executeStatement(stmt);
        }
    }
}

// Execute statement
void executeStatement(ASTNode* stmt) {
    if (auto comment = dynamic_cast<Comment*>(stmt)) {
        // Skip comments
        return;
    } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
        // Execute expression
        executeExpression(exprStmt->expression);
    } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(stmt)) {
        // Execute variable declaration
        if (varDecl->initializer) {
            // Execute initializer and store value
            Value value = executeExpression(varDecl->initializer);
            variables[varDecl->name] = value;
        } else {
            // Store default value (monostate for undefined)
            variables[varDecl->name] = std::monostate{};
        }
    }
}

// Execute expression
Value executeExpression(Expression* expr) {
    if (auto funcCall = dynamic_cast<FunctionCall*>(expr)) {
        // Execute function call
        return executeFunctionCall(funcCall);
    } else if (auto assignExpr = dynamic_cast<AssignmentExpression*>(expr)) {
        // Execute assignment expression
        Value value = executeExpression(assignExpr->right);
        variables[assignExpr->left->name] = value;
        return value;
    } else if (auto binaryExpr = dynamic_cast<BinaryExpression*>(expr)) {
        // Execute binary expression
        auto leftVal = executeExpression(binaryExpr->left);
        auto rightVal = executeExpression(binaryExpr->right);
        
        // Handle string operations
        if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
            std::string leftStr = std::get<std::string>(leftVal);
            std::string rightStr = std::get<std::string>(rightVal);
            
            if (binaryExpr->op == "+") {
                // String concatenation
                return leftStr + rightStr;
            } else if (binaryExpr->op == "*") {
                // String repetition - right operand must be a number
                // For simplicity, we'll skip this for now
                return leftStr;
            }
        }
        
        // Handle logical operations specially
        if (binaryExpr->op == "AND" || binaryExpr->op == "OR" || binaryExpr->op == "XOR") {
            auto getBool = [](Value val) -> bool {
                if (std::holds_alternative<bool>(val)) {
                    return std::get<bool>(val);
                } else if (std::holds_alternative<int>(val)) {
                    return std::get<int>(val) != 0;
                } else if (std::holds_alternative<float>(val)) {
                    return std::get<float>(val) != 0.0f;
                } else if (std::holds_alternative<double>(val)) {
                    return std::get<double>(val) != 0.0;
                } else {
                    return false;
                }
            };
            
            bool leftBool = getBool(leftVal);
            bool rightBool = getBool(rightVal);
            
            if (binaryExpr->op == "AND") {
                return leftBool && rightBool;
            } else if (binaryExpr->op == "OR") {
                return leftBool || rightBool;
            } else if (binaryExpr->op == "XOR") {
                return leftBool != rightBool;
            }
        }
        
        // Handle numeric operations
        auto getNumber = [](Value val) -> double {
            if (std::holds_alternative<int>(val)) {
                return static_cast<double>(std::get<int>(val));
            } else if (std::holds_alternative<bool>(val)) {
                return static_cast<double>(std::get<bool>(val));
            } else if (std::holds_alternative<float>(val)) {
                return static_cast<double>(std::get<float>(val));
            } else if (std::holds_alternative<double>(val)) {
                return std::get<double>(val);
            } else {
                throw std::runtime_error("Cannot convert to number");
            }
        };
        
        double leftNum = getNumber(leftVal);
        double rightNum = getNumber(rightVal);
        double result = 0.0;
        
        if (binaryExpr->op == "+") {
            result = leftNum + rightNum;
        } else if (binaryExpr->op == "-") {
            result = leftNum - rightNum;
        } else if (binaryExpr->op == "*") {
            result = leftNum * rightNum;
        } else if (binaryExpr->op == "/") {
            // Check for division by zero
            if (rightNum == 0.0) {
                throw std::runtime_error("Division by zero");
            }
            result = leftNum / rightNum;
        } else if (binaryExpr->op == "<<") {
            // Bit shift operations - cast to int
            result = static_cast<double>(static_cast<int>(leftNum) << static_cast<int>(rightNum));
        } else if (binaryExpr->op == ">>") {
            // Bit shift operations - cast to int
            result = static_cast<double>(static_cast<int>(leftNum) >> static_cast<int>(rightNum));
        }
        
        // Determine result type based on operands
        if (std::holds_alternative<int>(leftVal) && std::holds_alternative<int>(rightVal)) {
            // Both operands are integers - result is integer
            return static_cast<int>(result);
        } else if (std::holds_alternative<float>(leftVal) || std::holds_alternative<float>(rightVal)) {
            // At least one float operand - result is float
            return static_cast<float>(result);
        } else {
            // Default to double
            return result;
        }
    } else if (auto ident = dynamic_cast<Identifier*>(expr)) {
        // Get variable value
        if (variables.find(ident->name) != variables.end()) {
            return variables[ident->name];
        } else {
            std::cerr << "Error: Undefined variable '" << ident->name << "'" << std::endl;
            exit(1);
        }
    } else if (auto intLit = dynamic_cast<IntegerLiteral*>(expr)) {
        // Integer literal
        return intLit->value;
    } else if (auto floatLit = dynamic_cast<FloatLiteral*>(expr)) {
        // Float literal
        return floatLit->value;
    } else if (auto doubleLit = dynamic_cast<DoubleLiteral*>(expr)) {
        // Double literal
        return doubleLit->value;
    } else if (auto charLit = dynamic_cast<CharLiteral*>(expr)) {
        // Char literal
        return charLit->value;
    } else if (auto stringLit = dynamic_cast<StringLiteral*>(expr)) {
        // String literal
        return stringLit->value;
    } else if (auto boolLit = dynamic_cast<BooleanLiteral*>(expr)) {
        // Boolean literal
        return boolLit->value;
    }
    
    // Default return value
    return std::monostate{};
}

// Execute function call
Value executeFunctionCall(FunctionCall* call) {
    if (call->objectName == "System" && call->methodName == "print") {
        // Handle System.print
        for (auto arg : call->arguments) {
            Value value = executeExpression(arg);
            
            // Print based on value type
            if (std::holds_alternative<int>(value)) {
                std::cout << std::get<int>(value) << std::endl;
            } else if (std::holds_alternative<char>(value)) {
                std::cout << std::get<char>(value) << std::endl;
            } else if (std::holds_alternative<std::string>(value)) {
                std::cout << std::get<std::string>(value) << std::endl;
            } else if (std::holds_alternative<bool>(value)) {
                std::cout << (std::get<bool>(value) ? "true" : "false") << std::endl;
            } else if (std::holds_alternative<float>(value)) {
                std::cout << std::get<float>(value) << std::endl;
            } else if (std::holds_alternative<double>(value)) {
                std::cout << std::get<double>(value) << std::endl;
            } else {
                std::cout << "undefined" << std::endl;
            }
        }
    } else if (call->objectName == "System" && call->methodName == "input") {
        // Handle System.input
        if (!call->arguments.empty()) {
            // Print prompt
            Value promptValue = executeExpression(call->arguments[0]);
            if (std::holds_alternative<std::string>(promptValue)) {
                std::cout << std::get<std::string>(promptValue);
            }
        }
        
        // Get user input
        std::string input;
        std::getline(std::cin, input);
        
        // Return input as string
        return input;
    } else if (call->objectName == "type") {
        // Handle type conversion functions
        if (call->arguments.empty()) {
            return std::monostate{};
        }
        
        Value arg = executeExpression(call->arguments[0]);
        
        if (call->methodName == "int") {
            // Convert to int
            if (std::holds_alternative<int>(arg)) {
                return arg;
            } else if (std::holds_alternative<float>(arg)) {
                return static_cast<int>(std::get<float>(arg));
            } else if (std::holds_alternative<double>(arg)) {
                return static_cast<int>(std::get<double>(arg));
            } else if (std::holds_alternative<bool>(arg)) {
                return static_cast<int>(std::get<bool>(arg));
            } else if (std::holds_alternative<std::string>(arg)) {
                // Try to parse string as int
                try {
                    return std::stoi(std::get<std::string>(arg));
                } catch (...) {
                    throw std::runtime_error("Cannot convert string to int");
                }
            }
        } else if (call->methodName == "float") {
            // Convert to float
            if (std::holds_alternative<int>(arg)) {
                return static_cast<float>(std::get<int>(arg));
            } else if (std::holds_alternative<float>(arg)) {
                return arg;
            } else if (std::holds_alternative<double>(arg)) {
                return static_cast<float>(std::get<double>(arg));
            } else if (std::holds_alternative<bool>(arg)) {
                return static_cast<float>(std::get<bool>(arg));
            } else if (std::holds_alternative<std::string>(arg)) {
                // Try to parse string as float
                try {
                    return std::stof(std::get<std::string>(arg));
                } catch (...) {
                    throw std::runtime_error("Cannot convert string to float");
                }
            }
        } else if (call->methodName == "double") {
            // Convert to double
            if (std::holds_alternative<int>(arg)) {
                return static_cast<double>(std::get<int>(arg));
            } else if (std::holds_alternative<float>(arg)) {
                return static_cast<double>(std::get<float>(arg));
            } else if (std::holds_alternative<double>(arg)) {
                return arg;
            } else if (std::holds_alternative<bool>(arg)) {
                return static_cast<double>(std::get<bool>(arg));
            } else if (std::holds_alternative<std::string>(arg)) {
                // Try to parse string as double
                try {
                    return std::stod(std::get<std::string>(arg));
                } catch (...) {
                    throw std::runtime_error("Cannot convert string to double");
                }
            }
        } else if (call->methodName == "string") {
            // Convert to string
            if (std::holds_alternative<int>(arg)) {
                return std::to_string(std::get<int>(arg));
            } else if (std::holds_alternative<float>(arg)) {
                return std::to_string(std::get<float>(arg));
            } else if (std::holds_alternative<double>(arg)) {
                return std::to_string(std::get<double>(arg));
            } else if (std::holds_alternative<bool>(arg)) {
                return std::get<bool>(arg) ? "true" : "false";
            } else if (std::holds_alternative<char>(arg)) {
                return std::string(1, std::get<char>(arg));
            } else if (std::holds_alternative<std::string>(arg)) {
                return arg;
            }
        }
    }
    
    // Default return value
    return std::monostate{};
}

int main(int argc, char* argv[]) {
    // Command line arguments handling
    std::string filePath;
    std::string outputFile;
    std::string mode;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-i" || arg == "-g") {
            mode = arg;
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            } else {
                std::cerr << "Error: -o option requires an output filename" << std::endl;
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: vanction <RunMod> [options] <file.vn>" << std::endl;
            std::cout << "<RunMod> must be either -i or -g" << std::endl;
            std::cout << "Options: " << std::endl;
            std::cout << "  -i         Interpret the Vanction program" << std::endl;
            std::cout << "  -g         Compile to executable file (using GCC)" << std::endl;
            std::cout << "  -o <file>  Specify output filename for compilation" << std::endl;
            std::cout << "  -h, --help Show this help message" << std::endl;
            return 0;
        } else if (arg.substr(0, 1) != "-") {
            filePath = arg;
        } else {
            std::cerr << "Error: Unknown option " << arg << std::endl;
            std::cerr << "Usage: vanction <RunMod> [options] <file.vn>" << std::endl;
            std::cerr << "<RunMod> must be either -i or -g" << std::endl;
            std::cerr << "Options: " << std::endl;
            std::cerr << "  -i         Interpret the Vanction program" << std::endl;
            std::cerr << "  -g         Compile to executable file (using GCC)" << std::endl;
            std::cerr << "  -o <file>  Specify output filename for compilation" << std::endl;
            std::cerr << "  -h, --help Show this help message" << std::endl;
            return 1;
        }
    }
    
    // Check if mode is specified
    if (mode.empty()) {
        std::cerr << "Error: Mode must be specified (-i or -g)" << std::endl;
        std::cerr << "Usage: vanction <RunMod> [options] <file.vn>" << std::endl;
        std::cerr << "<RunMod> must be either -i or -g" << std::endl;
        std::cerr << "Options: " << std::endl;
        std::cerr << "  -i         Interpret the Vanction program" << std::endl;
        std::cerr << "  -g         Compile to executable file (using GCC)" << std::endl;
        std::cerr << "  -o <file>  Specify output filename for compilation" << std::endl;
        std::cerr << "  -h, --help Show this help message" << std::endl;
        return 1;
    }
    
    // Check if file is specified
    if (filePath.empty()) {
        std::cerr << "Error: Input file must be specified" << std::endl;
        std::cerr << "Usage: vanction <RunMod> [options] <file.vn>" << std::endl;
        std::cerr << "<RunMod> must be either -i or -g" << std::endl;
        std::cerr << "Options: " << std::endl;
        std::cerr << "  -i         Interpret the Vanction program" << std::endl;
        std::cerr << "  -g         Compile to executable file (using GCC)" << std::endl;
        std::cerr << "  -o <file>  Specify output filename for compilation" << std::endl;
        std::cerr << "  -h, --help Show this help message" << std::endl;
        return 1;
    }
    
    // Check file extension
    if (filePath.length() < 3 || filePath.substr(filePath.length() - 3) != ".vn") {
        std::cerr << "Error: File must end with .vn" << std::endl;
        return 1;
    }
    
    try {
        // Read file content
        std::string sourceCode = readFile(filePath);
        
        // Create error reporter
        ErrorReporter errorReporter(sourceCode, filePath);
        
        // Create lexer
        Lexer lexer(sourceCode);
        
        // Create parser
        Parser parser(lexer);
        
        if (mode == "-g") {
            // GCC compile mode: generate AST, then compile to executable
            std::cout << "Entering GCC compile mode..." << std::endl;
            
            // Generate AST
            auto program = parser.parseProgramAST();
            if (!program) {
                throw std::runtime_error("AST generation failed");
            }
            
            // Check if main function exists
            bool hasMainFunction = false;
            for (auto decl : program->declarations) {
                if (auto func = dynamic_cast<FunctionDeclaration*>(decl)) {
                    if (func->name == "main") {
                        hasMainFunction = true;
                        break;
                    }
                }
            }
            
            if (!hasMainFunction) {
                Error error(ErrorType::MainFunctionError, "Program must have a main function", filePath, 1, 1);
                errorReporter.report(error);
                delete program;
                return 1;
            }
            
            // Generate C++ code
            CodeGenerator codeGen;
            std::string cppCode = codeGen.generate(program);
            
            // Save generated C++ code
            std::string cppFile = getFileNameWithoutExt(filePath) + ".cpp";
            writeFile(cppFile, cppCode);
            std::cout << "Generated C++ code: " << cppFile << std::endl;
            
            // Set default output filename
            if (outputFile.empty()) {
                outputFile = getFileNameWithoutExt(filePath) + ".exe";
            }
            
            // Call external compiler
            std::cout << "Compiling to executable: " << outputFile << std::endl;
            int result = compileWithGCC(cppFile, outputFile);
            
            if (result == 0) {
                std::cout << "GCC compilation successful!" << std::endl;
            } else {
                Error error(ErrorType::CompilationError, "GCC compilation failed", filePath, 1, 1);
                errorReporter.report(error);
                std::remove(cppFile.c_str());
                delete program;
                return 1;
            }
            
            // Clean up temporary file
            std::remove(cppFile.c_str());
            std::cout << "Cleaned up temporary files" << std::endl;
            
            // Clean up AST
            delete program;
        } else if (mode == "-i") {
            // Interpret mode: generate AST and execute
            
            // Generate AST
            auto program = parser.parseProgramAST();
            if (!program) {
                throw std::runtime_error("AST generation failed");
            }
            
            // Check if main function exists
            bool hasMainFunction = false;
            for (auto decl : program->declarations) {
                if (auto func = dynamic_cast<FunctionDeclaration*>(decl)) {
                    if (func->name == "main") {
                        hasMainFunction = true;
                        break;
                    }
                }
            }
            
            if (!hasMainFunction) {
                Error error(ErrorType::MainFunctionError, "Program must have a main function", filePath, 1, 1);
                errorReporter.report(error);
                delete program;
                return 1;
            }
            
            // Execute the program
            executeProgram(program);
            
            // Clean up AST
            delete program;
        }
    } catch (const std::runtime_error& e) {
        // Read file content again for error reporting
        std::string sourceCode = readFile(filePath);
        ErrorReporter errorReporter(sourceCode, filePath);
        
        // Determine error type based on error message
        std::string errorMsg = e.what();
        ErrorType errorType = ErrorType::CError;
        
        if (errorMsg.find("Syntax error") != std::string::npos) {
            errorType = ErrorType::SyntaxError;
        } else if (errorMsg.find("Unknown character") != std::string::npos) {
            errorType = ErrorType::TokenError;
        } else if (errorMsg.find("Division by zero") != std::string::npos) {
            errorType = ErrorType::DivideByZeroError;
        } else if (errorMsg.find("Cannot convert") != std::string::npos) {
            errorType = ErrorType::ValueError;
        } else if (errorMsg.find("Main function") != std::string::npos) {
            errorType = ErrorType::MainFunctionError;
        }
        
        // Create and report error
        Error error(errorType, errorMsg, filePath, 1, 1);
        errorReporter.report(error);
        return 1;
    } catch (const std::exception& e) {
        // Handle other exceptions as CError
        std::string sourceCode = readFile(filePath);
        ErrorReporter errorReporter(sourceCode, filePath);
        Error error(ErrorType::CError, e.what(), filePath, 1, 1);
        errorReporter.report(error);
        return 1;
    }
    
    return 0;
}
