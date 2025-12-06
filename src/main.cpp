#include "parser.h"
#include "code_generator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <variant>
#include <any>

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
using Value = std::variant<int, char, std::string, bool, std::monostate>;

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
        // Execute binary expression (string concatenation)
        auto leftVal = executeExpression(binaryExpr->left);
        auto rightVal = executeExpression(binaryExpr->right);
        
        // Only support string concatenation for now
        if (binaryExpr->op == "+") {
            // Convert both values to strings
            std::string leftStr, rightStr;
            
            if (std::holds_alternative<int>(leftVal)) {
                leftStr = std::to_string(std::get<int>(leftVal));
            } else if (std::holds_alternative<char>(leftVal)) {
                leftStr = std::string(1, std::get<char>(leftVal));
            } else if (std::holds_alternative<std::string>(leftVal)) {
                leftStr = std::get<std::string>(leftVal);
            } else if (std::holds_alternative<bool>(leftVal)) {
                leftStr = std::get<bool>(leftVal) ? "true" : "false";
            } else {
                leftStr = "undefined";
            }
            
            if (std::holds_alternative<int>(rightVal)) {
                rightStr = std::to_string(std::get<int>(rightVal));
            } else if (std::holds_alternative<char>(rightVal)) {
                rightStr = std::string(1, std::get<char>(rightVal));
            } else if (std::holds_alternative<std::string>(rightVal)) {
                rightStr = std::get<std::string>(rightVal);
            } else if (std::holds_alternative<bool>(rightVal)) {
                rightStr = std::get<bool>(rightVal) ? "true" : "false";
            } else {
                rightStr = "undefined";
            }
            
            // Concatenate strings
            return leftStr + rightStr;
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
    
    // Read file content
    std::string sourceCode = readFile(filePath);
    
    // Create lexer
    Lexer lexer(sourceCode);
    
    // Create parser
    Parser parser(lexer);
    
    if (mode == "-g") {
        // GCC编译模式: generate AST, then compile to executable
        std::cout << "Entering GCC compile mode..." << std::endl;
        
        // Generate AST
        auto program = parser.parseProgramAST();
        if (!program) {
            std::cerr << "Error: AST generation failed" << std::endl;
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
            std::cerr << "GCC compilation failed!" << std::endl;
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
            std::cerr << "Error: AST generation failed" << std::endl;
            return 1;
        }
        
        // Execute the program
        executeProgram(program);
        
        // Clean up AST
        delete program;
    }
    
    return 0;
}
