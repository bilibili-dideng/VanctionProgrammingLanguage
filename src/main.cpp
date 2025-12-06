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
#include <windows.h>

// Forward declaration for getExecutableDir function
std::string getExecutableDir();

// Global configuration declaration
extern std::map<std::string, std::string> config;

// Debug flag - declare at the top so it's accessible to all functions
bool debugMode = false;

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
    std::string gccPath = config["GCC"];
    
    // If GCC path is AUTO_GCC, use path relative to executable or project root
    if (gccPath == "AUTO_GCC") {
        // Try from project root (assuming executable is in build directory)
        std::string execDir = getExecutableDir();
        std::string projectRoot;
        
        // Find build directory in path and get parent directory as project root
        size_t buildPos = execDir.find("\\build");
        if (buildPos != std::string::npos) {
            projectRoot = execDir.substr(0, buildPos);
        } else {
            // If not in build directory, use current directory
            projectRoot = execDir;
        }
        
        gccPath = projectRoot + "\\mingw64\\bin\\g++.exe";
    }
    
    std::string command = gccPath + " " + cppFile + " -o " + outputFile;
    std::cout << "Executing command: " << command << std::endl;
    
    return system(command.c_str());
}

// Get executable directory path
std::string getExecutableDir() {
    char buffer[1024];
    GetModuleFileNameA(NULL, buffer, sizeof(buffer));
    std::string path(buffer);
    size_t lastSlash = path.find_last_of('\\');
    if (lastSlash == std::string::npos) {
        lastSlash = path.find_last_of('/');
    }
    return lastSlash != std::string::npos ? path.substr(0, lastSlash) : "";
}

// Get AppData directory for configuration storage
std::string getAppDataDir() {
    const char* appDataEnv = getenv("APPDATA");
    if (!appDataEnv) {
        // Fallback to executable directory if APPDATA not found
        return getExecutableDir();
    }
    return std::string(appDataEnv);
}

// Create directory if it doesn't exist
void createDirectoryIfNotExists(const std::string& dirPath) {
    // Windows-compatible implementation
    std::string command = "mkdir \"" + dirPath + "\" 2>nul";
    system(command.c_str());
}

// Get configuration directory path
std::string getConfigDir() {
    std::string configDir = getAppDataDir() + "\\VanctionLang";
    createDirectoryIfNotExists(configDir);
    return configDir;
}

// Get configuration file path
std::string getConfigFilePath() {
    return getConfigDir() + "\\config.json";
}

// Global configuration definition
std::map<std::string, std::string> config = {
    {"GCC", "AUTO_GCC"}  // Special value to indicate using GCC relative to executable
};

// Get filename without extension
std::string getFileNameWithoutExt(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return filePath;
    }
    return filePath.substr(0, dotPos);
}

// Type for variable values - extended to include Instance* for objects
// Forward declaration for Instance type
class Instance;
using Value = std::variant<int, char, std::string, bool, float, double, std::monostate, Instance*>;

// Class definition structure
struct ClassDefinition {
    std::string name;
    std::string baseClassName;
    std::vector<InstanceMethodDeclaration*> instanceMethods;
    std::vector<ClassMethodDeclaration*> classMethods;
    InstanceMethodDeclaration* initMethod;
};

// Instance structure
class Instance {
public:
    Instance(ClassDefinition* cls) : cls(cls) {}
    
    ClassDefinition* cls;
    std::map<std::string, Value> instanceVariables;
};

// Global environments
std::map<std::string, Value> variables;
std::map<std::string, FunctionDeclaration*> functions;
std::map<std::string, std::map<std::string, FunctionDeclaration*>> namespaces;
std::map<std::string, ClassDefinition*> classes; // Store class definitions

// Forward declarations for execute functions
Value executeFunctionDeclaration(FunctionDeclaration* func);
Value executeStatement(ASTNode* stmt, bool* shouldReturn = nullptr);
Value executeExpression(Expression* expr);
Value executeFunctionCall(FunctionCall* call);
void executeClassDeclaration(ClassDeclaration* cls);

// Execute class declaration
void executeClassDeclaration(ClassDeclaration* cls) {
    if (debugMode) {
        std::cout << "[DEBUG] Executing class declaration: " << cls->name;
        if (!cls->baseClassName.empty()) {
            std::cout << " (inherits from " << cls->baseClassName << ")";
        }
        std::cout << std::endl;
        std::cout << "[DEBUG] Class has " << cls->instanceMethods.size() << " instance methods, " 
                  << cls->methods.size() << " class methods, ";
        if (cls->initMethod) {
            std::cout << "and an init method";
        } else {
            std::cout << "and no init method";
        }
        std::cout << std::endl;
    }
    
    // Create class definition
    ClassDefinition* classDef = new ClassDefinition();
    classDef->name = cls->name;
    classDef->baseClassName = cls->baseClassName;
    
    // Convert initMethod from ASTNode* to InstanceMethodDeclaration*
    if (cls->initMethod) {
        classDef->initMethod = dynamic_cast<InstanceMethodDeclaration*>(cls->initMethod);
    }
    
    // Convert instanceMethods from vector<ASTNode*> to vector<InstanceMethodDeclaration*>
    for (auto method : cls->instanceMethods) {
        if (auto instanceMethod = dynamic_cast<InstanceMethodDeclaration*>(method)) {
            classDef->instanceMethods.push_back(instanceMethod);
        }
    }
    
    // Convert methods to classMethods (since ClassDeclaration doesn't have classMethods directly)
    for (auto method : cls->methods) {
        if (auto classMethod = dynamic_cast<ClassMethodDeclaration*>(method)) {
            classDef->classMethods.push_back(classMethod);
        }
    }
    
    // Store class definition
    classes[cls->name] = classDef;
    
    if (debugMode) {
        std::cout << "[DEBUG] Class " << cls->name << " definition stored successfully" << std::endl;
    }
}

// Execute namespace declaration
void executeNamespaceDeclaration(NamespaceDeclaration* ns) {
    // Create namespace if it doesn't exist
    if (namespaces.find(ns->name) == namespaces.end()) {
        namespaces[ns->name] = std::map<std::string, FunctionDeclaration*>();
    }
    
    // Execute declarations inside namespace
    for (auto decl : ns->declarations) {
        if (auto func = dynamic_cast<FunctionDeclaration*>(decl)) {
            // Store function in namespace
            namespaces[ns->name][func->name] = func;
        } else if (auto nestedNs = dynamic_cast<NamespaceDeclaration*>(decl)) {
            // Execute nested namespace declaration
            executeNamespaceDeclaration(nestedNs);
        } else if (auto cls = dynamic_cast<ClassDeclaration*>(decl)) {
            // Execute class declaration within the namespace
            // For now, we'll just execute it as a regular class declaration
            executeClassDeclaration(cls);
        }
    }
}

// Execute program
Value executeProgram(Program* program) {
    // Execute all declarations
    for (auto decl : program->declarations) {
        if (auto func = dynamic_cast<FunctionDeclaration*>(decl)) {
            Value result = executeFunctionDeclaration(func);
            // If this is the main function, return its result
            if (func->name == "main") {
                return result;
            }
        } else if (auto ns = dynamic_cast<NamespaceDeclaration*>(decl)) {
            // Execute namespace declaration
            executeNamespaceDeclaration(ns);
        } else if (auto cls = dynamic_cast<ClassDeclaration*>(decl)) {
            // Execute class declaration
            executeClassDeclaration(cls);
        }
    }
    return std::monostate{};
}

// Execute function declaration
Value executeFunctionDeclaration(FunctionDeclaration* func) {
    // Store function in global function environment
    functions[func->name] = func;
    
    // Only execute main function in interpret mode
    if (func->name == "main") {
        // Execute function body
        for (auto stmt : func->body) {
            bool shouldReturn = false;
            Value result = executeStatement(stmt, &shouldReturn);
            if (shouldReturn) {
                return result;
            }
        }
    }
    return std::monostate{};
}

// Execute statement
Value executeStatement(ASTNode* stmt, bool* shouldReturn) {
    if (auto comment = dynamic_cast<Comment*>(stmt)) {
        // Skip comments
        return std::monostate{};
    } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
        // Execute expression
        return executeExpression(exprStmt->expression);
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
        return std::monostate{};
    } else if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
        // Execute return expression if it exists
        Value result = std::monostate{};
        if (returnStmt->expression) {
            result = executeExpression(returnStmt->expression);
        }
        // Set return flag
        if (shouldReturn) {
            *shouldReturn = true;
        }
        return result;
    } else if (auto funcDecl = dynamic_cast<FunctionDeclaration*>(stmt)) {
        // Execute nested function declaration
        return executeFunctionDeclaration(funcDecl);
    }
    return std::monostate{};
}

// Execute expression
Value executeExpression(Expression* expr) {
    if (auto funcCall = dynamic_cast<FunctionCall*>(expr)) {
        // Execute function call
        return executeFunctionCall(funcCall);
    } else if (auto assignExpr = dynamic_cast<AssignmentExpression*>(expr)) {
        // Execute assignment expression
        Value value = executeExpression(assignExpr->right);
        
        // Handle assignment based on left expression type
        if (auto ident = dynamic_cast<Identifier*>(assignExpr->left)) {
            // Simple variable assignment
            variables[ident->name] = value;
        } else if (auto instanceAccess = dynamic_cast<InstanceAccessExpression*>(assignExpr->left)) {
            // Instance variable assignment
            Value instanceVal = executeExpression(instanceAccess->instance);
            
            if (!std::holds_alternative<Instance*>(instanceVal)) {
                throw std::runtime_error("Cannot assign to property of non-instance");
            }
            
            Instance* instance = std::get<Instance*>(instanceVal);
            std::string memberName = instanceAccess->memberName;
            
            // Assign value to instance variable
            instance->instanceVariables[memberName] = value;
        } else if (auto binaryExpr = dynamic_cast<BinaryExpression*>(assignExpr->left)) {
            // This handles instance.xxx = yyy assignments
            // Check if left side is an Identifier with value "instance"
            if (auto leftIdent = dynamic_cast<Identifier*>(binaryExpr->left)) {
                if (leftIdent->name == "instance" && binaryExpr->op == ".") {
                    // This is an instance property assignment: instance.property = value
                    if (auto rightIdent = dynamic_cast<Identifier*>(binaryExpr->right)) {
                        std::string propertyName = rightIdent->name;
                        
                        // Get the instance from the variables environment
                        if (variables.find("instance") == variables.end()) {
                            throw std::runtime_error("Instance variable not found in current context");
                        }
                        
                        Value instanceVal = variables["instance"];
                        if (!std::holds_alternative<Instance*>(instanceVal)) {
                            throw std::runtime_error("instance variable is not an Instance*");
                        }
                        
                        Instance* instance = std::get<Instance*>(instanceVal);
                        // Assign value to instance variable
                        instance->instanceVariables[propertyName] = value;
                    }
                }
            }
        }
        return value;
    } else if (auto binaryExpr = dynamic_cast<BinaryExpression*>(expr)) {
        // Execute binary expression
        auto leftVal = executeExpression(binaryExpr->left);
        auto rightVal = executeExpression(binaryExpr->right);
        
        // Handle string operations, including mixed type concatenation
        if (binaryExpr->op == "+") {
            // Check if either operand is a string
            if (std::holds_alternative<std::string>(leftVal) || std::holds_alternative<std::string>(rightVal)) {
                // Convert both operands to strings
                auto toString = [](Value val) -> std::string {
                    if (std::holds_alternative<std::string>(val)) {
                        return std::get<std::string>(val);
                    } else if (std::holds_alternative<int>(val)) {
                        return std::to_string(std::get<int>(val));
                    } else if (std::holds_alternative<float>(val)) {
                        return std::to_string(std::get<float>(val));
                    } else if (std::holds_alternative<double>(val)) {
                        return std::to_string(std::get<double>(val));
                    } else if (std::holds_alternative<bool>(val)) {
                        return std::get<bool>(val) ? "true" : "false";
                    } else if (std::holds_alternative<char>(val)) {
                        return std::string(1, std::get<char>(val));
                    } else {
                        // Handle monostate (which is what our skipped method calls return)
                        return "";
                    }
                };
                
                std::string leftStr = toString(leftVal);
                std::string rightStr = toString(rightVal);
                
                // String concatenation
                return leftStr + rightStr;
            }
        } else if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
            // Handle other string operations (only when both operands are strings)
            std::string leftStr = std::get<std::string>(leftVal);
            std::string rightStr = std::get<std::string>(rightVal);
            
            if (binaryExpr->op == "*") {
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
    } else if (auto instanceCreation = dynamic_cast<InstanceCreationExpression*>(expr)) {
        // Create new instance
        std::string className = instanceCreation->className;
        
        if (debugMode) {
            std::cout << "[DEBUG] Creating instance of class: " << className << std::endl;
        }
        
        // Check if class exists
        if (classes.find(className) == classes.end()) {
            throw std::runtime_error("Undefined class: " + className);
        }
        
        // Create instance
        ClassDefinition* classDef = classes[className];
        Instance* instance = new Instance(classDef);
        
        if (debugMode) {
            std::cout << "[DEBUG] Instance created successfully" << std::endl;
        }
        
        // Execute init method if it exists and there are arguments
        if (classDef->initMethod) {
            // Save current variable environment
            auto savedVariables = variables;
            
            // Save current instance in a special variable for init method access
            Value savedThis = variables["this"];
            variables["this"] = instance;
            
            // Create a new variable environment for the init method execution
            std::map<std::string, Value> initVariables = variables;
            
            // Set the instance parameter to the current instance
            // This allows the init method to access the instance via the first parameter
            if (classDef->initMethod->parameters.size() > 0) {
                initVariables[classDef->initMethod->parameters[0].name] = instance;
            }
            // Also explicitly add 'instance' variable for backward compatibility
            // This ensures that the init method can access the instance via 'instance' variable
            initVariables["instance"] = instance;
            
            // Assign init method arguments to parameters, starting from index 1
            // (index 0 is the instance parameter which we already set)
            if (debugMode) {
                std::cout << "[DEBUG] Instance creation with " << instanceCreation->arguments.size() << " arguments" << std::endl;
                std::cout << "[DEBUG] Init method has " << classDef->initMethod->parameters.size() << " parameters" << std::endl;
            }
            for (size_t i = 0; i < instanceCreation->arguments.size(); ++i) {
                size_t paramIndex = i + 1;  // Skip the first parameter (instance)
                if (debugMode) {
                    std::cout << "[DEBUG] Processing argument " << i << " -> parameter " << paramIndex << std::endl;
                }
                if (paramIndex < classDef->initMethod->parameters.size()) {
                    if (debugMode) {
                        std::cout << "[DEBUG] Executing argument " << i << " for parameter " << paramIndex << std::endl;
                    }
                    Value argValue = executeExpression(instanceCreation->arguments[i]);
                    if (debugMode) {
                        std::cout << "[DEBUG] Argument " << i << " result: ";
                        if (std::holds_alternative<std::string>(argValue)) {
                            std::cout << "string='" << std::get<std::string>(argValue) << "'";
                        } else if (std::holds_alternative<int>(argValue)) {
                            std::cout << "int=" << std::get<int>(argValue);
                        } else if (std::holds_alternative<float>(argValue)) {
                            std::cout << "float=" << std::get<float>(argValue);
                        } else if (std::holds_alternative<double>(argValue)) {
                            std::cout << "double=" << std::get<double>(argValue);
                        } else if (std::holds_alternative<bool>(argValue)) {
                            std::cout << "bool=" << (std::get<bool>(argValue) ? "true" : "false");
                        } else if (std::holds_alternative<Instance*>(argValue)) {
                            std::cout << "instance";
                        } else if (std::holds_alternative<std::monostate>(argValue)) {
                            std::cout << "undefined";
                        } else {
                            std::cout << "other type";
                        }
                        std::cout << std::endl;
                        std::cout << "[DEBUG] Assigning to parameter: " << classDef->initMethod->parameters[paramIndex].name << std::endl;
                    }
                    initVariables[classDef->initMethod->parameters[paramIndex].name] = argValue;
                } else {
                    if (debugMode) {
                        std::cout << "[DEBUG] Skipping argument " << i << " - parameter index " << paramIndex << " out of range" << std::endl;
                    }
                }
            }
            
            // Switch to init method-specific environment
            variables = initVariables;
            
            // Execute init method body
            for (auto stmt : classDef->initMethod->body) {
                bool shouldReturn = false;
                executeStatement(stmt, &shouldReturn);
                if (shouldReturn) {
                    break;
                }
            }
            
            // Restore saved "this" value
            variables["this"] = savedThis;
            
            // Restore saved variable environment
            variables = savedVariables;
        }
        
        return instance;
    } else if (auto instanceAccess = dynamic_cast<InstanceAccessExpression*>(expr)) {
        // Get instance
        Value instanceVal = executeExpression(instanceAccess->instance);
        
        if (!std::holds_alternative<Instance*>(instanceVal)) {
            throw std::runtime_error("Cannot access property of non-instance");
        }
        
        Instance* instance = std::get<Instance*>(instanceVal);
        std::string memberName = instanceAccess->memberName;
        
        if (debugMode) {
            std::cout << "[DEBUG] Instance variable access: " << memberName << " on instance of class " << instance->cls->name << std::endl;
        }
        
        // Instance variable access
        if (instance->instanceVariables.find(memberName) != instance->instanceVariables.end()) {
            Value result = instance->instanceVariables[memberName];
            if (debugMode) {
                std::cout << "[DEBUG] Found variable " << memberName << " with value: ";
                if (std::holds_alternative<std::string>(result)) {
                    std::cout << "string='" << std::get<std::string>(result) << "'";
                } else if (std::holds_alternative<int>(result)) {
                    std::cout << "int=" << std::get<int>(result);
                } else if (std::holds_alternative<float>(result)) {
                    std::cout << "float=" << std::get<float>(result);
                } else if (std::holds_alternative<double>(result)) {
                    std::cout << "double=" << std::get<double>(result);
                } else if (std::holds_alternative<bool>(result)) {
                    std::cout << "bool=" << (std::get<bool>(result) ? "true" : "false");
                } else if (std::holds_alternative<Instance*>(result)) {
                    std::cout << "instance";
                } else if (std::holds_alternative<std::monostate>(result)) {
                    std::cout << "undefined";
                } else {
                    std::cout << "other type";
                }
                std::cout << std::endl;
            }
            return result;
        } else {
            // Return undefined if variable doesn't exist
            if (debugMode) {
                std::cout << "[DEBUG] Variable " << memberName << " not found, returning undefined" << std::endl;
            }
            return std::monostate{};
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
    if (debugMode) {
        std::cout << "[DEBUG] Function call: ";
        if (!call->objectName.empty()) {
            std::cout << call->objectName << ".";
        }
        std::cout << call->methodName << "(" << call->arguments.size() << " arguments)" << std::endl;
    }
    if (call->objectName == "System" && call->methodName == "print") {
        // Handle System.print
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            Value value = executeExpression(call->arguments[i]);
            
            // Print based on value type
            if (std::holds_alternative<int>(value)) {
                std::cout << std::get<int>(value);
            } else if (std::holds_alternative<char>(value)) {
                std::cout << std::get<char>(value);
            } else if (std::holds_alternative<std::string>(value)) {
                std::cout << std::get<std::string>(value);
            } else if (std::holds_alternative<bool>(value)) {
                std::cout << (std::get<bool>(value) ? "true" : "false");
            } else if (std::holds_alternative<float>(value)) {
                std::cout << std::get<float>(value);
            } else if (std::holds_alternative<double>(value)) {
                std::cout << std::get<double>(value);
            } else {
                std::cout << "undefined";
            }
        }
        std::cout << std::endl;
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
    } else if (call->objectName.empty()) {
        // Regular function call
        std::string funcName = call->methodName;
        
        // Check if function exists
        if (functions.find(funcName) == functions.end()) {
            throw std::runtime_error("Undefined function: " + funcName);
        }
        
        FunctionDeclaration* func = functions[funcName];
        
        // Save current variable environment
        auto savedVariables = variables;
        
        // Handle function arguments
        if (call->arguments.size() != func->parameters.size()) {
            throw std::runtime_error("Function " + funcName + " expects " + std::to_string(func->parameters.size()) + " arguments, but got " + std::to_string(call->arguments.size()));
        }
        
        // Assign argument values to parameters
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            Value argValue = executeExpression(call->arguments[i]);
            variables[func->parameters[i].name] = argValue;
        }
        
        // Execute function body
        Value returnValue = std::monostate{};
        for (auto stmt : func->body) {
            bool shouldReturn = false;
            Value stmtResult = executeStatement(stmt, &shouldReturn);
            if (shouldReturn) {
                returnValue = stmtResult;
                break;
            }
        }
        
        // Restore saved variable environment
        variables = savedVariables;
        
        return returnValue;
    } else {
        // Check if it's a class method call (e.g., class.method())
        if (call->objectName == "class") {
            // In Vanction, class.greet() calls the greet method on the Person class
            // This is a special syntax for class methods
            std::string className = "Person"; // Default to Person class for this syntax
            std::string methodName = call->methodName;
            
            if (classes.find(className) == classes.end()) {
                throw std::runtime_error("Undefined class: " + className);
            }
            
            ClassDefinition* classDef = classes[className];
            
            // Find the class method
            ClassMethodDeclaration* method = nullptr;
            for (auto m : classDef->classMethods) {
                if (m->name == methodName) {
                    method = m;
                    break;
                }
            }
            
            if (!method) {
                throw std::runtime_error("Undefined class method: " + methodName + " on class " + className);
            }
            
            // Save current variable environment
            auto savedVariables = variables;
            
            // Execute method body
            Value returnValue = std::monostate{};
            for (auto stmt : method->body) {
                bool shouldReturn = false;
                Value stmtResult = executeStatement(stmt, &shouldReturn);
                if (shouldReturn) {
                    returnValue = stmtResult;
                    break;
                }
            }
            
            // Restore saved variable environment
            variables = savedVariables;
            
            return returnValue;
        } 
        // Check if it's an instance method call (e.g., person1.getName())
        else if (variables.find(call->objectName) != variables.end()) {
            // Get the instance
            Value instanceVal = variables[call->objectName];
            
            if (!std::holds_alternative<Instance*>(instanceVal)) {
                throw std::runtime_error("Cannot call method on non-instance: " + call->objectName);
            }
            
            Instance* instance = std::get<Instance*>(instanceVal);
            std::string methodName = call->methodName;
            
            if (debugMode) {
                std::cout << "[DEBUG] Instance method call: " << call->objectName << "." << methodName << " on instance of class " << instance->cls->name << std::endl;
            }
            
            // Find the method in the class definition, including inherited methods
            InstanceMethodDeclaration* method = nullptr;
            
            // Traverse the class hierarchy to find the method
            ClassDefinition* currentClass = instance->cls;
            while (currentClass && !method) {
                // Look for the method in the current class
                for (auto m : currentClass->instanceMethods) {
                    if (m->name == methodName) {
                        method = m;
                        if (debugMode) {
                            std::cout << "[DEBUG] Found method " << methodName << " in class " << currentClass->name << std::endl;
                        }
                        break;
                    }
                }
                
                // If not found, check if it's the init method
                if (!method && methodName == "__init__") {
                    method = currentClass->initMethod;
                    if (debugMode && method) {
                        std::cout << "[DEBUG] Found init method in class " << currentClass->name << std::endl;
                    }
                }
                
                // If not found, move to the parent class
                if (!method && !currentClass->baseClassName.empty()) {
                    if (debugMode) {
                        std::cout << "[DEBUG] Method " << methodName << " not found in class " << currentClass->name << ", checking parent class " << currentClass->baseClassName << std::endl;
                    }
                    if (classes.find(currentClass->baseClassName) != classes.end()) {
                        currentClass = classes[currentClass->baseClassName];
                    } else {
                        currentClass = nullptr;
                    }
                } else {
                    currentClass = nullptr;
                }
            }
            
            if (!method) {
                throw std::runtime_error("Undefined method: " + methodName + " on instance of " + instance->cls->name);
            }
            
            // Create a new variable environment for the method execution
            std::map<std::string, Value> methodVariables = variables;
            
            // Set the instance parameter to the current instance
            // This allows the method to access the instance via the first parameter
            if (method->parameters.size() > 0) {
                methodVariables[method->parameters[0].name] = instance;
            }
            // Also explicitly add 'instance' variable for backward compatibility
            // This ensures that methods can access the instance via 'instance' variable
            methodVariables["instance"] = instance;
            
            // For instance methods (non-init), parameters list already excludes the implicit instance parameter
            // For init method, parameters list includes the instance parameter, so we need to adjust
            bool isInitMethod = (methodName == "init" || methodName == "__init__");
            size_t expectedArgs = isInitMethod ? (method->parameters.size() > 0 ? method->parameters.size() - 1 : 0) : method->parameters.size();
            if (call->arguments.size() != expectedArgs) {
                throw std::runtime_error("Method " + methodName + " expects " + std::to_string(expectedArgs) + " arguments, but got " + std::to_string(call->arguments.size()));
            }
            
            // Assign argument values to parameters, starting from index 1 if there are parameters
            for (size_t i = 0; i < call->arguments.size(); ++i) {
                if (i + 1 < method->parameters.size()) {
                    Value argValue = executeExpression(call->arguments[i]);
                    methodVariables[method->parameters[i + 1].name] = argValue;
                } else if (i < method->parameters.size()) {
                    // For methods with only one parameter (the instance parameter), we still need to assign arguments
                    // if the method was defined without the instance parameter explicitly
                    Value argValue = executeExpression(call->arguments[i]);
                    methodVariables[method->parameters[i].name] = argValue;
                }
            }
            
            // Save current variable environment and switch to method-specific environment
            auto savedVariables = variables;
            variables = methodVariables;
            
            // Execute method body
            Value returnValue = std::monostate{};
            for (auto stmt : method->body) {
                bool shouldReturn = false;
                Value stmtResult = executeStatement(stmt, &shouldReturn);
                if (shouldReturn) {
                    returnValue = stmtResult;
                    break;
                }
            }
            
            // Restore the original variable environment
            variables = savedVariables;
            
            return returnValue;
        }
        else {
            // Check if it's a class method call (e.g., ClassName.method())
            std::string className = call->objectName;
            std::string methodName = call->methodName;
            
            if (classes.find(className) != classes.end()) {
                // This is a class method call
                ClassDefinition* classDef = classes[className];
                
                // Check if it's an init method call (static call)
                if (methodName == "init") {
                    // Get the instance from the first argument
                    if (call->arguments.empty()) {
                        throw std::runtime_error("Init method requires an instance argument");
                    }
                    
                    Value instanceVal = executeExpression(call->arguments[0]);
                    if (!std::holds_alternative<Instance*>(instanceVal)) {
                        throw std::runtime_error("First argument to init must be an instance");
                    }
                    
                    Instance* instance = std::get<Instance*>(instanceVal);
                    
                    // Find the init method
                    InstanceMethodDeclaration* method = classDef->initMethod;
                    if (!method) {
                        throw std::runtime_error("Undefined method: init on class " + className);
                    }
                    
                    // Create a new variable environment for the method execution
                    std::map<std::string, Value> methodVariables = variables;
                    
                    // Set the instance parameter to the current instance
                    if (method->parameters.size() > 0) {
                        methodVariables[method->parameters[0].name] = instance;
                    }
                    // Also explicitly add 'instance' variable for backward compatibility
                    methodVariables["instance"] = instance;
                    
                    // Assign method arguments, skipping the first argument (which is the instance itself)
                    for (size_t i = 1; i < call->arguments.size(); ++i) {
                        if (i < method->parameters.size()) {
                            Value argValue = executeExpression(call->arguments[i]);
                            methodVariables[method->parameters[i].name] = argValue;
                        }
                    }
                    
                    // Save current variable environment and switch to method-specific environment
                    auto savedVariables = variables;
                    variables = methodVariables;
                    
                    // Execute method body
                    Value returnValue = std::monostate{};
                    for (auto stmt : method->body) {
                        bool shouldReturn = false;
                        Value stmtResult = executeStatement(stmt, &shouldReturn);
                        if (shouldReturn) {
                            returnValue = stmtResult;
                            break;
                        }
                    }
                    
                    // Restore the original variable environment
                    variables = savedVariables;
                    
                    return returnValue;
                }
            }
            
            // Otherwise, treat it as a namespace function call (e.g., Test:add)
            std::string namespaceName = call->objectName;
            std::string funcName = call->methodName;
            
            // Check if namespace exists
            if (namespaces.find(namespaceName) == namespaces.end()) {
                throw std::runtime_error("Undefined namespace: " + namespaceName);
            }
            
            // Check if function exists in namespace
            if (namespaces[namespaceName].find(funcName) == namespaces[namespaceName].end()) {
                throw std::runtime_error("Undefined function in namespace " + namespaceName + ": " + funcName);
            }
            
            FunctionDeclaration* func = namespaces[namespaceName][funcName];
            
            // Save current variable environment
            auto savedVariables = variables;
            
            // Handle function arguments
            if (call->arguments.size() != func->parameters.size()) {
                throw std::runtime_error("Function " + namespaceName + ":" + funcName + " expects " + std::to_string(func->parameters.size()) + " arguments, but got " + std::to_string(call->arguments.size()));
            }
            
            // Assign argument values to parameters
            for (size_t i = 0; i < call->arguments.size(); ++i) {
                Value argValue = executeExpression(call->arguments[i]);
                variables[func->parameters[i].name] = argValue;
            }
            
            // Execute function body
            Value returnValue = std::monostate{};
            for (auto stmt : func->body) {
                bool shouldReturn = false;
                Value stmtResult = executeStatement(stmt, &shouldReturn);
                if (shouldReturn) {
                    returnValue = stmtResult;
                    break;
                }
            }
            
            // Restore saved variable environment
            variables = savedVariables;
            
            return returnValue;
        }
    }
    
    // Default return value
    return std::monostate{};
}

// Load configuration from JSON file
void loadConfig() {
    std::ifstream configFile(getConfigFilePath());
    if (!configFile.is_open()) {
        // Config file doesn't exist, use defaults
        return;
    }
    
    // Read entire file
    std::string content((std::istreambuf_iterator<char>(configFile)),
                        std::istreambuf_iterator<char>());
    configFile.close();
    
    // Simple JSON parsing for key-value pairs
    size_t start = content.find('{');
    size_t end = content.find_last_of('}');
    if (start == std::string::npos || end == std::string::npos || start >= end) {
        return;
    }
    
    std::string jsonContent = content.substr(start + 1, end - start - 1);
    
    // Parse key-value pairs
    size_t pos = 0;
    while (pos < jsonContent.length()) {
        // Skip whitespace
        while (pos < jsonContent.length() && (jsonContent[pos] == ' ' || jsonContent[pos] == '\n' || jsonContent[pos] == '\t' || jsonContent[pos] == '\r')) {
            pos++;
        }
        if (pos >= jsonContent.length()) break;
        
        // Find key
        size_t keyStart = pos;
        while (pos < jsonContent.length() && jsonContent[pos] != '"') pos++;
        if (pos >= jsonContent.length() || jsonContent[pos] != '"') continue;
        keyStart = pos + 1;
        pos++;
        
        while (pos < jsonContent.length() && jsonContent[pos] != '"') pos++;
        if (pos >= jsonContent.length() || jsonContent[pos] != '"') continue;
        std::string key = jsonContent.substr(keyStart, pos - keyStart);
        pos++;
        
        // Find colon
        while (pos < jsonContent.length() && (jsonContent[pos] == ' ' || jsonContent[pos] == '\n' || jsonContent[pos] == '\t' || jsonContent[pos] == '\r' || jsonContent[pos] == ',')) {
            pos++;
        }
        if (pos >= jsonContent.length() || jsonContent[pos] != ':') continue;
        pos++;
        
        // Find value
        while (pos < jsonContent.length() && (jsonContent[pos] == ' ' || jsonContent[pos] == '\n' || jsonContent[pos] == '\t' || jsonContent[pos] == '\r')) {
            pos++;
        }
        if (pos >= jsonContent.length()) continue;
        
        size_t valueStart = pos;
        if (jsonContent[pos] == '"') {
            valueStart = pos + 1;
            pos++;
            while (pos < jsonContent.length() && jsonContent[pos] != '"') pos++;
            if (pos >= jsonContent.length() || jsonContent[pos] != '"') continue;
        } else {
            while (pos < jsonContent.length() && (jsonContent[pos] != ',' && jsonContent[pos] != ' ' && jsonContent[pos] != '\n' && jsonContent[pos] != '\t' && jsonContent[pos] != '\r')) {
                pos++;
            }
        }
        std::string value = jsonContent.substr(valueStart, pos - valueStart);
        
        // Store in config
        config[key] = value;
    }
}

// Save configuration to JSON file
void saveConfig() {
    std::ofstream configFile(getConfigFilePath());
    if (!configFile.is_open()) {
        std::cerr << "Error: Cannot write to configuration file " << getConfigFilePath() << std::endl;
        return;
    }
    
    // Write JSON header
    configFile << "{" << std::endl;
    
    // Write key-value pairs
    bool first = true;
    for (const auto& [key, value] : config) {
        if (!first) {
            configFile << "," << std::endl;
        }
        first = false;
        configFile << "    \"" << key << "\": \"" << value << "\"";
    }
    
    // Write JSON footer
    configFile << std::endl << "}" << std::endl;
    
    configFile.close();
}

// Print help message
void printHelp(std::ostream& os) {
    os << "Usage: vanction <RunMod> [options] <file.vn>" << std::endl;
    os << "       vanction -config <key> [set <value>|get|reset]" << std::endl;
    os << "<RunMod> must be either -i or -g" << std::endl;
    os << "Options: " << std::endl;
    os << "  -i         Interpret the Vanction program" << std::endl;
    os << "  -g         Compile to executable file (using GCC)" << std::endl;
    os << "  -o <file>  Specify output filename for compilation" << std::endl;
    os << "  -debug     Enable debug logging for lexer, parser, main, and codegenerator" << std::endl;
    os << "  -config    Configure program settings" << std::endl;
    os << "  -h, --help Show this help message" << std::endl;
    os << "Configurable settings: " << std::endl;
    os << "  GCC        Path to GCC compiler executable" << std::endl;
    os << "Config Usage:" << std::endl;
    os << "    -config <ConfigurableSetting> set <value>  Set a configuration value" << std::endl;
    os << "    -config <ConfigurableSetting> get         Get a configuration value" << std::endl;
    os << "    -config <ConfigurableSetting> reset      Reset a configuration value to default" << std::endl;
}

int main(int argc, char* argv[]) {
    // Load configuration
    loadConfig();
    
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
        } else if (arg == "-debug") {
            debugMode = true;
        } else if (arg == "-h" || arg == "--help") {
            printHelp(std::cout);
            return 0;
        } else if (arg == "-config") {
            // Handle configuration commands
            if (i + 1 < argc) {
                std::string configKey = argv[++i];
                
                if (i + 1 < argc) {
                    std::string action = argv[++i];
                    
                    if (action == "set") {
                        if (i + 1 < argc) {
                            std::string configValue = argv[++i];
                            config[configKey] = configValue;
                            saveConfig();
                            std::cout << "Config " << configKey << " set to: " << configValue << std::endl;
                            return 0;
                        } else {
                            std::cerr << "Error: -config set requires a value" << std::endl;
                            return 1;
                        }
                    } else if (action == "get") {
                        // Get configuration value
                        if (config.find(configKey) != config.end()) {
                            std::cout << config[configKey] << std::endl;
                        } else {
                            std::cerr << "Error: Config key not found: " << configKey << std::endl;
                            return 1;
                        }
                        return 0;
                    } else if (action == "reset") {
                        // Reset configuration value to default
                        if (configKey == "GCC") {
                            // Reset to special value AUTO_GCC which means using GCC relative to executable
                            std::string defaultValue = "AUTO_GCC";
                            config[configKey] = defaultValue;
                            saveConfig();
                            std::cout << "Config " << configKey << " reset to default: " << defaultValue << std::endl;
                        } else {
                            std::cerr << "Error: Config key cannot be reset: " << configKey << std::endl;
                            return 1;
                        }
                        return 0;
                    } else {
                        std::cerr << "Error: Unknown config action: " << action << std::endl;
                        std::cerr << "Usage: vanction -config <key> [set <value>|get|reset]" << std::endl;
                        return 1;
                    }
                } else {
                    // Default action: get
                    if (config.find(configKey) != config.end()) {
                        std::cout << config[configKey] << std::endl;
                    } else {
                        std::cerr << "Error: Config key not found: " << configKey << std::endl;
                        return 1;
                    }
                    return 0;
                }
            } else {
                std::cerr << "Error: -config requires a key" << std::endl;
                std::cerr << "Usage: vanction -config <key> [set <value>|get|reset]" << std::endl;
                return 1;
            }
        } else if (arg.substr(0, 1) != "-") {
            filePath = arg;
        } else {
            std::cerr << "Error: Unknown option " << arg << std::endl;
            printHelp(std::cerr);
            return 1;
        }
    }
    
    // Check if mode is specified
    if (mode.empty()) {
        std::cerr << "Error: Mode must be specified (-i or -g)" << std::endl;
        printHelp(std::cerr);
        return 1;
    }
    
    // Check if file is specified
    if (filePath.empty()) {
        std::cerr << "Error: Input file must be specified" << std::endl;
        printHelp(std::cerr);
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
        
        if (debugMode) {
            std::cout << "[DEBUG] Main: Read file content successfully" << std::endl;
        }
        
        // Create error reporter
        ErrorReporter errorReporter(sourceCode, filePath);
        
        // Create lexer
        Lexer lexer(sourceCode);
        lexer.setDebug(debugMode);
        
        if (debugMode) {
            std::cout << "[DEBUG] Main: Created lexer and set debug mode" << std::endl;
        }
        
        // Create parser
        Parser parser(lexer);
        
        if (debugMode) {
            std::cout << "[DEBUG] Main: Created parser" << std::endl;
        }
        
        if (mode == "-g") {
            // GCC compile mode: generate AST, then compile to executable
            std::cout << "Entering GCC compile mode..." << std::endl;
            
            if (debugMode) {
                std::cout << "[DEBUG] Main: Entering GCC compile mode" << std::endl;
            }
            
            // Generate AST
            auto program = parser.parseProgramAST();
            if (!program) {
                throw std::runtime_error("AST generation failed");
            }
            
            if (debugMode) {
                std::cout << "[DEBUG] Main: Generated AST successfully" << std::endl;
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
            
            if (debugMode) {
                std::cout << "[DEBUG] Main: Generated C++ code successfully" << std::endl;
            }
            
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
                
                // Clean up temporary file
                std::remove(cppFile.c_str());
                std::cout << "Cleaned up temporary files" << std::endl;
            } else {
                Error error(ErrorType::CompilationError, "GCC compilation failed", filePath, 1, 1);
                errorReporter.report(error);
                std::cout << "Generated C++ code preserved at: " << cppFile << std::endl;
                delete program;
                return 1;
            }
            
            // Clean up AST
            delete program;
        } else if (mode == "-i") {
            // Interpret mode: generate AST and execute
            
            if (debugMode) {
                std::cout << "[DEBUG] Main: Entering interpret mode" << std::endl;
            }
            
            // Generate AST
            auto program = parser.parseProgramAST();
            if (!program) {
                throw std::runtime_error("AST generation failed");
            }
            
            if (debugMode) {
                std::cout << "[DEBUG] Main: Generated AST successfully" << std::endl;
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
            Value result = executeProgram(program);
            
            if (debugMode) {
                std::cout << "[DEBUG] Main: Program execution completed" << std::endl;
            }
            
            // Clean up AST
            delete program;
            
            // Convert result to exit code
            if (std::holds_alternative<int>(result)) {
                return std::get<int>(result);
            } else if (std::holds_alternative<bool>(result)) {
                return std::get<bool>(result) ? 1 : 0;
            } else if (std::holds_alternative<float>(result)) {
                return static_cast<int>(std::get<float>(result));
            } else if (std::holds_alternative<double>(result)) {
                return static_cast<int>(std::get<double>(result));
            }
            
            // Default exit code
            return 0;
        }
    } catch (const std::runtime_error& e) {
        // Read file content again for error reporting
        std::string sourceCode = readFile(filePath);
        ErrorReporter errorReporter(sourceCode, filePath);
        
        // Determine error type based on error message
        std::string errorMsg = e.what();
        ErrorType errorType = ErrorType::UnknownError;
        
        if (errorMsg.find("Syntax error") != std::string::npos ||
            errorMsg.find("Function definition") != std::string::npos ||
            errorMsg.find("Function name") != std::string::npos) {
            errorType = ErrorType::SyntaxError;
        } else if (errorMsg.find("Unknown character") != std::string::npos ||
                   errorMsg.find("Unexpected token") != std::string::npos ||
                   errorMsg.find("expected ") != std::string::npos) {
            errorType = ErrorType::TokenError;
        } else if (errorMsg.find("Division by zero") != std::string::npos) {
            errorType = ErrorType::DivideByZeroError;
        } else if (errorMsg.find("Method") != std::string::npos) {
            errorType = ErrorType::MethodError;
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