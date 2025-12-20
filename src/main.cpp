#include "parser.h"
#include "code_generator.h"
#include "error.h"
#include "module_manager.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <variant>
#include <any>
#include <stdexcept>
#include <functional>
#ifdef _WIN32
#include <windows.h>
#endif

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
#ifdef _WIN32
    char buffer[1024];
    GetModuleFileNameA(NULL, buffer, sizeof(buffer));
    std::string path(buffer);
    size_t lastSlash = path.find_last_of('\\');
    if (lastSlash == std::string::npos) {
        lastSlash = path.find_last_of('/');
    }
    return lastSlash != std::string::npos ? path.substr(0, lastSlash) : "";
#else
    // For non-Windows systems, use /proc/self/exe on Linux or other platform-specific methods
    // This is a simple fallback that should work on most Unix-like systems
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        std::string path(buffer);
        size_t lastSlash = path.find_last_of('/');
        return lastSlash != std::string::npos ? path.substr(0, lastSlash) : "";
    }
    // Fallback to current directory if readlink fails
    return ".";
#endif
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

// Forward declarations for data structures
class List;
class HashMap;

// Type for variable values - extended to include data structures and Instance* for objects
// Forward declaration for Instance type
class Instance;

// Define Value type
using Value = std::variant<int, char, std::string, bool, float, double, std::monostate, Instance*, ErrorObject*, List*, HashMap*, LambdaExpression*>;

// Class definition structure
struct ClassDefinition {
    std::string name;
    std::string baseClassName;
    std::vector<InstanceMethodDeclaration*> instanceMethods;
    std::vector<ClassMethodDeclaration*> classMethods;
    InstanceMethodDeclaration* initMethod;
};

// List data structure implementation
class List {
public:
    List() {}
    
    std::vector<Value> elements;
    
    // Add element to list
    void add(Value element) {
        elements.push_back(element);
    }
    
    // Get element by index (supports negative indices)
    Value get(int index) {
        if (index < 0) {
            index = elements.size() + index;
        }
        if (index < 0 || index >= elements.size()) {
                throw vanction_error::ListIndexError("List index out of range", 0, 0);
            }
        return elements[index];
    }
    
    // Set element by index (supports negative indices)
    void set(int index, Value element) {
        if (index < 0) {
            index = elements.size() + index;
        }
        if (index < 0 || index >= elements.size()) {
                throw vanction_error::ListIndexError("List index out of range", 0, 0);
            }
        elements[index] = element;
    }
    
    // Get list size
    int size() {
        return elements.size();
    }
};

// HashMap data structure implementation
class HashMap {
public:
    HashMap() {}
    
    std::map<std::string, Value> entries;
    
    // Get value by key with default support
    Value get(const std::string& key, Value defaultValue = std::monostate{}) {
        if (entries.find(key) != entries.end()) {
            return entries[key];
        }
        return defaultValue;
    }
    
    // Set value by key
    void set(const std::string& key, Value value) {
        entries[key] = value;
    }
    
    // Get all keys as List
    List* keys() {
        List* keyList = new List();
        for (auto& entry : entries) {
            keyList->add(std::string(entry.first));
        }
        return keyList;
    }
    
    // Get all values as List
    List* values() {
        List* valueList = new List();
        for (auto& entry : entries) {
            valueList->add(entry.second);
        }
        return valueList;
    }
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
std::map<std::string, Value> constants; // Store constants (immut variables)
std::map<std::string, std::string> variableTypes; // Store variable types
std::map<std::string, FunctionDeclaration*> functions;
std::map<std::string, std::map<std::string, FunctionDeclaration*>> namespaces;
std::map<std::string, ClassDefinition*> classes; // Store class definitions

// Initialize global constants
void initializeConstants() {
    // Boolean constants
    constants["true"] = true;
    constants["false"] = false;
    variableTypes["true"] = "bool";
    variableTypes["false"] = "bool";
}


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

// Forward declaration for executeProgram function
Value executeProgram(Program* program, const std::string& namespaceName = "");

// Global module manager to avoid dangling pointer issues
ModuleManager* globalModuleManager = nullptr;

// Execute import statement
void executeImportStatement(ImportStatement* importStmt) {
    std::string moduleName = importStmt->moduleName;
    
    // Initialize global module manager if it doesn't exist
    if (!globalModuleManager) {
        globalModuleManager = new ModuleManager();
    }
    
    // Load the module
    Module* module = globalModuleManager->loadModule(moduleName);
    if (!module) {
        throw vanction_error::MethodError("Cannot load module: " + moduleName);
    }
    
    // Execute the imported module with the specified alias as namespace
    executeProgram(module->ast, importStmt->alias);
}

// Execute program
Value executeProgram(Program* program, const std::string& namespaceName) {
    // Execute all declarations
    for (auto decl : program->declarations) {
        if (auto func = dynamic_cast<FunctionDeclaration*>(decl)) {
            if (!namespaceName.empty()) {
                // If we're in a namespace, add the function to the namespace
                if (namespaces.find(namespaceName) == namespaces.end()) {
                    namespaces[namespaceName] = std::map<std::string, FunctionDeclaration*>();
                }
                namespaces[namespaceName][func->name] = func;
            } else {
                Value result = executeFunctionDeclaration(func);
                // If this is the main function, return its result
                if (func->name == "main") {
                    return result;
                }
            }
        } else if (auto ns = dynamic_cast<NamespaceDeclaration*>(decl)) {
            // Execute namespace declaration
            executeNamespaceDeclaration(ns);
        } else if (auto cls = dynamic_cast<ClassDeclaration*>(decl)) {
            // Execute class declaration
            executeClassDeclaration(cls);
        } else if (auto importStmt = dynamic_cast<ImportStatement*>(decl)) {
            // Execute import statement
            executeImportStatement(importStmt);
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

// Execute if statement
Value executeIfStatement(IfStatement* stmt, bool* shouldReturn) {
    // Evaluate condition
    Value conditionValue = executeExpression(stmt->condition);
    
    // Convert condition to boolean
    bool condition = false;
    if (std::holds_alternative<bool>(conditionValue)) {
        condition = std::get<bool>(conditionValue);
    } else if (std::holds_alternative<int>(conditionValue)) {
        condition = (std::get<int>(conditionValue) != 0);
    } else if (std::holds_alternative<float>(conditionValue)) {
        condition = (std::get<float>(conditionValue) != 0.0f);
    } else if (std::holds_alternative<double>(conditionValue)) {
        condition = (std::get<double>(conditionValue) != 0.0);
    } else if (std::holds_alternative<std::string>(conditionValue)) {
        condition = !std::get<std::string>(conditionValue).empty();
    }
    
    // Execute if body if condition is true
    if (condition) {
        for (auto bodyStmt : stmt->ifBody) {
            Value result = executeStatement(bodyStmt, shouldReturn);
            if (shouldReturn && *shouldReturn) {
                return result;
            }
        }
    } else {
        // Check else-if clauses
        for (auto elseIf : stmt->elseIfs) {
            Value result = executeIfStatement(elseIf, shouldReturn);
            if (shouldReturn && *shouldReturn) {
                return result;
            }
        }
        
        // Execute else body if no else-if matched
        for (auto bodyStmt : stmt->elseBody) {
            Value result = executeStatement(bodyStmt, shouldReturn);
            if (shouldReturn && *shouldReturn) {
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
            
            // Determine variable type
            std::string varType;
            if (std::holds_alternative<int>(value)) {
                varType = "int";
            } else if (std::holds_alternative<char>(value)) {
                varType = "char";
            } else if (std::holds_alternative<std::string>(value)) {
                varType = "string";
            } else if (std::holds_alternative<bool>(value)) {
                varType = "bool";
            } else if (std::holds_alternative<float>(value)) {
                varType = "float";
            } else if (std::holds_alternative<double>(value)) {
                varType = "double";
            } else if (std::holds_alternative<Instance*>(value)) {
                varType = "instance";
            } else {
                varType = "unknown";
            }
            
            // Store variable type
            variableTypes[varDecl->name] = varType;
            
            if (varDecl->isImmut) {
                // Store in constants map for immut variables
                constants[varDecl->name] = value;
            } else {
                // Store in variables map for regular variables
                variables[varDecl->name] = value;
            }
        } else {
            // Store default value (monostate for undefined)
            variables[varDecl->name] = std::monostate{};
            variableTypes[varDecl->name] = "unknown";
        }
        return std::monostate{};
    } else if (auto ifStmt = dynamic_cast<IfStatement*>(stmt)) {
        // Execute if statement
        return executeIfStatement(ifStmt, shouldReturn);
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
    } else if (auto tryHappenStmt = dynamic_cast<TryHappenStatement*>(stmt)) {
        // Execute try-happen statement
        try {
            // Execute try body
            for (auto tryStmt : tryHappenStmt->tryBody) {
                bool tryShouldReturn = false;
                Value tryResult = executeStatement(tryStmt, &tryShouldReturn);
                if (tryShouldReturn) {
                    return tryResult;
                }
            }
        } catch (const vanction_error::VanctionError& e) {
            // Check if error type matches
            if (tryHappenStmt->errorType == e.getType() || tryHappenStmt->errorType == "Error") {
                // Create error object
                auto errorObj = new ErrorObject(e.what(), e.getType(), e.getMessage());
                
                // Store error object in variable
                variables[tryHappenStmt->errorVariableName] = errorObj;
                
                // Execute happen body
                for (auto happenStmt : tryHappenStmt->happenBody) {
                    bool happenShouldReturn = false;
                    Value happenResult = executeStatement(happenStmt, &happenShouldReturn);
                    if (happenShouldReturn) {
                        return happenResult;
                    }
                }
            } else {
                // Re-throw if error type doesn't match
                throw;
            }
        } catch (const std::exception& e) {
            // Handle other exceptions
            if (tryHappenStmt->errorType == "CError" || tryHappenStmt->errorType == "Error") {
                // Create error object
                std::string errorMsg = e.what();
                std::string errorType = "CError";
                
                // Create error object
                auto errorObj = new ErrorObject(errorMsg, errorType, errorMsg);
                
                // Store error object in variable
                variables[tryHappenStmt->errorVariableName] = errorObj;
                
                // Execute happen body
                for (auto happenStmt : tryHappenStmt->happenBody) {
                    bool happenShouldReturn = false;
                    Value happenResult = executeStatement(happenStmt, &happenShouldReturn);
                    if (happenShouldReturn) {
                        return happenResult;
                    }
                }
            } else {
                // Re-throw if error type doesn't match
                throw;
            }
        }
        return std::monostate{};
    } else if (auto forLoopStmt = dynamic_cast<ForLoopStatement*>(stmt)) {
        // Execute traditional for loop
        // Execute initialization
        executeStatement(forLoopStmt->initialization);
        
        // Execute loop while condition is true
        while (true) {
            // Evaluate condition
            Value conditionValue = executeExpression(forLoopStmt->condition);
            bool condition = false;
            if (std::holds_alternative<bool>(conditionValue)) {
                condition = std::get<bool>(conditionValue);
            } else if (std::holds_alternative<int>(conditionValue)) {
                condition = (std::get<int>(conditionValue) != 0);
            } else if (std::holds_alternative<float>(conditionValue)) {
                condition = (std::get<float>(conditionValue) != 0.0f);
            } else if (std::holds_alternative<double>(conditionValue)) {
                condition = (std::get<double>(conditionValue) != 0.0);
            }
            
            if (!condition) {
                break;
            }
            
            // Execute loop body
            for (auto bodyStmt : forLoopStmt->body) {
                bool bodyShouldReturn = false;
                Value bodyResult = executeStatement(bodyStmt, &bodyShouldReturn);
                if (bodyShouldReturn) {
                    return bodyResult;
                }
            }
            
            // Execute increment
            executeExpression(forLoopStmt->increment);
        }
        return std::monostate{};
    } else if (auto forInStmt = dynamic_cast<ForInLoopStatement*>(stmt)) {
        // Execute for-in loop
        
        // First, execute the collection expression to get the actual collection object
        Value collectionValue = executeExpression(forInStmt->collection);
        
        // Handle List* object (from variable or expression)
        if (std::holds_alternative<List*>(collectionValue)) {
            List* list = std::get<List*>(collectionValue);
            // Iterate over list elements
            for (size_t i = 0; i < list->elements.size(); ++i) {
                // Get element value
                Value elementValue = list->elements[i];
                
                // Store current element in loop variable
                variables[forInStmt->keyVariableName] = elementValue;
                
                // Execute loop body
                for (auto bodyStmt : forInStmt->body) {
                    bool bodyShouldReturn = false;
                    Value bodyResult = executeStatement(bodyStmt, &bodyShouldReturn);
                    if (bodyShouldReturn) {
                        return bodyResult;
                    }
                }
            }
        }
        // Handle HashMap* object (from variable or expression)
        else if (std::holds_alternative<HashMap*>(collectionValue)) {
            HashMap* hashMap = std::get<HashMap*>(collectionValue);
            // Iterate over hash map entries
            for (auto& entry : hashMap->entries) {
                // Get key and value
                std::string key = entry.first;
                Value value = entry.second;
                
                // Store current key and value in loop variables
                variables[forInStmt->keyVariableName] = key;
                variables[forInStmt->valueVariableName] = value;
                
                // Execute loop body
                for (auto bodyStmt : forInStmt->body) {
                    bool bodyShouldReturn = false;
                    Value bodyResult = executeStatement(bodyStmt, &bodyShouldReturn);
                    if (bodyShouldReturn) {
                        return bodyResult;
                    }
                }
            }
        }
        // Handle ListLiteral
        else if (auto listLit = dynamic_cast<ListLiteral*>(forInStmt->collection)) {
            // Iterate over list elements
            for (auto elementExpr : listLit->elements) {
                // Execute element expression and get value
                Value elementValue = executeExpression(elementExpr);
                
                // Store current element in loop variable
                variables[forInStmt->keyVariableName] = elementValue;
                
                // Execute loop body
                for (auto bodyStmt : forInStmt->body) {
                    bool bodyShouldReturn = false;
                    Value bodyResult = executeStatement(bodyStmt, &bodyShouldReturn);
                    if (bodyShouldReturn) {
                        return bodyResult;
                    }
                }
            }
        }
        // Handle HashMapLiteral
        else if (auto hashMapLit = dynamic_cast<HashMapLiteral*>(forInStmt->collection)) {
            // Iterate over hash map entries
            for (auto entry : hashMapLit->entries) {
                // Execute key and value expressions
                Value keyValue = executeExpression(entry->key);
                Value valueValue = executeExpression(entry->value);
                
                // Store current key and value in loop variables
                variables[forInStmt->keyVariableName] = keyValue;
                variables[forInStmt->valueVariableName] = valueValue;
                
                // Execute loop body
                for (auto bodyStmt : forInStmt->body) {
                    bool bodyShouldReturn = false;
                    Value bodyResult = executeStatement(bodyStmt, &bodyShouldReturn);
                    if (bodyShouldReturn) {
                        return bodyResult;
                    }
                }
            }
        }
        // Handle RangeExpression
        else if (auto rangeExpr = dynamic_cast<RangeExpression*>(forInStmt->collection)) {
            // Execute range start, end, and step expressions
            Value startValue = executeExpression(rangeExpr->start);
            Value endValue = executeExpression(rangeExpr->end);
            Value stepValue = (rangeExpr->step) ? executeExpression(rangeExpr->step) : Value{1};
            
            // Convert to integers
            int start = (std::holds_alternative<int>(startValue)) ? std::get<int>(startValue) : 0;
            int end = (std::holds_alternative<int>(endValue)) ? std::get<int>(endValue) : 0;
            int step = (std::holds_alternative<int>(stepValue)) ? std::get<int>(stepValue) : 1;
            
            // Iterate over range
            for (int i = start; i < end; i += step) {
                // Store current index in loop variable
                variables[forInStmt->keyVariableName] = i;
                
                // Execute loop body
                for (auto bodyStmt : forInStmt->body) {
                    bool bodyShouldReturn = false;
                    Value bodyResult = executeStatement(bodyStmt, &bodyShouldReturn);
                    if (bodyShouldReturn) {
                        return bodyResult;
                    }
                }
            }
        }
        // Handle function call that returns range (e.g., range(10))
        else if (auto funcCall = dynamic_cast<FunctionCall*>(forInStmt->collection)) {
            if (funcCall->methodName == "range" && funcCall->objectName.empty()) {
                // Parse range function arguments
                int start = 0;
                int end = 0;
                int step = 1;
                
                if (funcCall->arguments.size() == 1) {
                    // range(end)
                    Value endValue = executeExpression(funcCall->arguments[0]);
                    end = (std::holds_alternative<int>(endValue)) ? std::get<int>(endValue) : 0;
                } else if (funcCall->arguments.size() >= 2) {
                    // range(start, end, step?)
                    Value startValue = executeExpression(funcCall->arguments[0]);
                    Value endValue = executeExpression(funcCall->arguments[1]);
                    start = (std::holds_alternative<int>(startValue)) ? std::get<int>(startValue) : 0;
                    end = (std::holds_alternative<int>(endValue)) ? std::get<int>(endValue) : 0;
                    
                    if (funcCall->arguments.size() >= 3) {
                        Value stepValue = executeExpression(funcCall->arguments[2]);
                        step = (std::holds_alternative<int>(stepValue)) ? std::get<int>(stepValue) : 1;
                    }
                }
                
                // Iterate over range
                for (int i = start; i < end; i += step) {
                    // Store current index in loop variable
                    variables[forInStmt->keyVariableName] = i;
                    
                    // Execute loop body
                    for (auto bodyStmt : forInStmt->body) {
                        bool bodyShouldReturn = false;
                        Value bodyResult = executeStatement(bodyStmt, &bodyShouldReturn);
                        if (bodyShouldReturn) {
                            return bodyResult;
                        }
                    }
                }
            }
        }
        return std::monostate{};
    } else if (auto whileStmt = dynamic_cast<WhileLoopStatement*>(stmt)) {
        // Execute while loop
        while (true) {
            // Evaluate condition
            Value conditionValue = executeExpression(whileStmt->condition);
            bool condition = false;
            if (std::holds_alternative<bool>(conditionValue)) {
                condition = std::get<bool>(conditionValue);
            } else if (std::holds_alternative<int>(conditionValue)) {
                condition = (std::get<int>(conditionValue) != 0);
            } else if (std::holds_alternative<float>(conditionValue)) {
                condition = (std::get<float>(conditionValue) != 0.0f);
            } else if (std::holds_alternative<double>(conditionValue)) {
                condition = (std::get<double>(conditionValue) != 0.0);
            } else if (std::holds_alternative<std::string>(conditionValue)) {
                condition = !std::get<std::string>(conditionValue).empty();
            }
            
            if (!condition) {
                break;
            }
            
            // Execute loop body
            for (auto bodyStmt : whileStmt->body) {
                bool bodyShouldReturn = false;
                Value bodyResult = executeStatement(bodyStmt, &bodyShouldReturn);
                if (bodyShouldReturn) {
                    return bodyResult;
                }
            }
        }
        return std::monostate{};
    } else if (auto doWhileStmt = dynamic_cast<DoWhileLoopStatement*>(stmt)) {
        // Execute do-while loop
        do {
            // Execute loop body
            for (auto bodyStmt : doWhileStmt->body) {
                bool bodyShouldReturn = false;
                Value bodyResult = executeStatement(bodyStmt, &bodyShouldReturn);
                if (bodyShouldReturn) {
                    return bodyResult;
                }
            }
            
            // Evaluate condition
            Value conditionValue = executeExpression(doWhileStmt->condition);
            bool condition = false;
            if (std::holds_alternative<bool>(conditionValue)) {
                condition = std::get<bool>(conditionValue);
            } else if (std::holds_alternative<int>(conditionValue)) {
                condition = (std::get<int>(conditionValue) != 0);
            } else if (std::holds_alternative<float>(conditionValue)) {
                condition = (std::get<float>(conditionValue) != 0.0f);
            } else if (std::holds_alternative<double>(conditionValue)) {
                condition = (std::get<double>(conditionValue) != 0.0);
            } else if (std::holds_alternative<std::string>(conditionValue)) {
                condition = !std::get<std::string>(conditionValue).empty();
            }
            
            if (!condition) {
                break;
            }
        } while (true);
        return std::monostate{};
    } else if (auto switchStmt = dynamic_cast<SwitchStatement*>(stmt)) {
        // Execute switch statement
        // First, execute the switch expression
        Value switchValue = executeExpression(switchStmt->expression);
        
        // Iterate through all case statements
        for (auto caseStmt : switchStmt->cases) {
            // Execute case expression
            Value caseValue = executeExpression(caseStmt->value);
            
            // Compare case value with switch value
            bool match = false;
            
            // Handle different types of comparisons
            if (std::holds_alternative<int>(switchValue) && std::holds_alternative<int>(caseValue)) {
                match = (std::get<int>(switchValue) == std::get<int>(caseValue));
            } else if (std::holds_alternative<std::string>(switchValue) && std::holds_alternative<std::string>(caseValue)) {
                match = (std::get<std::string>(switchValue) == std::get<std::string>(caseValue));
            } else if (std::holds_alternative<bool>(switchValue) && std::holds_alternative<bool>(caseValue)) {
                match = (std::get<bool>(switchValue) == std::get<bool>(caseValue));
            } else if (std::holds_alternative<float>(switchValue) && std::holds_alternative<float>(caseValue)) {
                match = (std::get<float>(switchValue) == std::get<float>(caseValue));
            } else if (std::holds_alternative<double>(switchValue) && std::holds_alternative<double>(caseValue)) {
                match = (std::get<double>(switchValue) == std::get<double>(caseValue));
            }
            
            // If case matches, execute the case body
            if (match) {
                for (auto bodyStmt : caseStmt->body) {
                    bool bodyShouldReturn = false;
                    Value bodyResult = executeStatement(bodyStmt, &bodyShouldReturn);
                    if (bodyShouldReturn) {
                        return bodyResult;
                    }
                }
                // Fallthrough behavior - continue to next case
            }
        }
        return std::monostate{};
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
    } else if (auto funcCallExpr = dynamic_cast<FunctionCallExpression*>(expr)) {
        // Execute function call expression (for lambdas, etc.)
        // First execute the callee to get the function
        Value calleeValue = executeExpression(funcCallExpr->callee);
        
        // Check if it's a lambda expression
        if (LambdaExpression** lambdaPtr = std::get_if<LambdaExpression*>(&calleeValue)) {
            LambdaExpression* lambda = *lambdaPtr;
            // Bind arguments to parameters
            if (lambda->parameters.size() != funcCallExpr->arguments.size()) {
                throw vanction_error::MethodError("Argument count mismatch for lambda call");
            }
            
            // Execute arguments
            std::vector<Value> argValues;
            for (auto arg : funcCallExpr->arguments) {
                argValues.push_back(executeExpression(arg));
            }
            
            // Create a copy of the current variables to save the state
            std::map<std::string, Value> originalVariables = variables;
            std::map<std::string, std::string> originalVariableTypes = variableTypes;
            
            // Bind parameters to argument values in the current environment
            // This is important for nested lambdas to access outer lambda parameters
            std::map<std::string, Value> tempVariables = variables;
            std::map<std::string, std::string> tempVariableTypes = variableTypes;
            
            for (size_t i = 0; i < lambda->parameters.size(); i++) {
                const std::string& paramName = lambda->parameters[i].name;
                tempVariables[paramName] = argValues[i];
                tempVariableTypes[paramName] = "auto";
            }
            
            // If lambda has a closure environment, merge it into the temp environment
            // This ensures we have all variables from outer scopes
            if (lambda->closureEnv) {
                auto closureEnv = static_cast<std::pair<std::map<std::string, Value>, std::map<std::string, std::string>>*>(lambda->closureEnv);
                for (const auto& pair : closureEnv->first) {
                    if (tempVariables.find(pair.first) == tempVariables.end()) {
                        tempVariables[pair.first] = pair.second;
                    }
                }
                for (const auto& pair : closureEnv->second) {
                    if (tempVariableTypes.find(pair.first) == tempVariableTypes.end()) {
                        tempVariableTypes[pair.first] = pair.second;
                    }
                }
            }
            
            // Set the temp environment as current for execution
            variables = tempVariables;
            variableTypes = tempVariableTypes;
            
            // Execute the lambda body
            Value result = executeExpression(lambda->body);
            
            // Restore original variables
            variables = originalVariables;
            variableTypes = originalVariableTypes;
            
            return result;
        } else {
            throw vanction_error::MethodError("Attempt to call a non-function value");
        }
    } else if (auto assignExpr = dynamic_cast<AssignmentExpression*>(expr)) {
        // Execute assignment expression
        Value value = executeExpression(assignExpr->right);
        
        // Handle assignment based on left expression type
        if (auto ident = dynamic_cast<Identifier*>(assignExpr->left)) {
            // Simple variable assignment
            std::string varName = ident->name;
            
            // Check if variable is a constant (immut var)
            if (constants.find(varName) != constants.end()) {
                throw vanction_error::ImmutError("Cannot assign to constant '" + varName + "'");
            }
            
            // Check if variable exists
            if (variables.find(varName) == variables.end()) {
                throw vanction_error::MethodError("Variable '" + varName + "' not declared");
            }
            
            // Check type compatibility
            if (variableTypes.find(varName) != variableTypes.end()) {
                std::string existingType = variableTypes[varName];
                std::string newValueType;
                
                if (std::holds_alternative<int>(value)) {
                    newValueType = "int";
                } else if (std::holds_alternative<char>(value)) {
                    newValueType = "char";
                } else if (std::holds_alternative<std::string>(value)) {
                    newValueType = "string";
                } else if (std::holds_alternative<bool>(value)) {
                    newValueType = "bool";
                } else if (std::holds_alternative<float>(value)) {
                    newValueType = "float";
                } else if (std::holds_alternative<double>(value)) {
                    newValueType = "double";
                } else if (std::holds_alternative<List*>(value)) {
                    newValueType = "list";
                } else if (std::holds_alternative<HashMap*>(value)) {
                    newValueType = "hashmap";
                } else if (std::holds_alternative<Instance*>(value)) {
                    newValueType = "instance";
                } else {
                    newValueType = "unknown";
                }
                
                // Check if types are compatible
                if (existingType != "unknown" && newValueType != "unknown" && existingType != newValueType) {
                    throw vanction_error::MethodError("Type mismatch: cannot assign '" + newValueType + "' to variable of type '" + existingType + "'");
                }
            }
            
            // Update variable value
            variables[varName] = value;
        } else if (auto instanceAccess = dynamic_cast<InstanceAccessExpression*>(assignExpr->left)) {
            // Instance variable assignment
            Value instanceVal = executeExpression(instanceAccess->instance);
            
            if (!std::holds_alternative<Instance*>(instanceVal)) {
                throw vanction_error::MethodError("Cannot assign to property of non-instance");
            }
            
            Instance* instance = std::get<Instance*>(instanceVal);
            std::string memberName = instanceAccess->memberName;
            
            // Assign value to instance variable
            instance->instanceVariables[memberName] = value;
        } else if (auto binaryExpr = dynamic_cast<BinaryExpression*>(assignExpr->left)) {
            // Handle index assignment: obj[index] = value
            if (binaryExpr->op == "[") {
                // Execute the left side (object being indexed)
                Value leftObj = executeExpression(binaryExpr->left);
                // Execute the index expression
                Value indexExpr = executeExpression(binaryExpr->right);
                
                // Handle List index assignment
                if (std::holds_alternative<List*>(leftObj)) {
                    List* list = std::get<List*>(leftObj);
                    
                    // Convert index to integer
                    int index;
                    if (std::holds_alternative<int>(indexExpr)) {
                        index = std::get<int>(indexExpr);
                    } else {
                        throw vanction_error::TypeError("List index must be an integer");
                    }
                    
                    // Handle negative indices
                    if (index < 0) {
                        index = list->elements.size() + index;
                    }
                    
                    // Check bounds
                    if (index < 0 || index >= list->elements.size()) {
                        throw vanction_error::RangeError("List index out of range", 0, 0);
                    }
                    
                    // Assign value to list index
                    list->set(index, value);
                }
                // Handle HashMap index assignment
                else if (std::holds_alternative<HashMap*>(leftObj)) {
                    HashMap* map = std::get<HashMap*>(leftObj);
                    
                    // Convert key to string
                    std::string key;
                    if (std::holds_alternative<std::string>(indexExpr)) {
                        key = std::get<std::string>(indexExpr);
                    } else {
                        // Convert other types to string
                        auto toString = [](Value val) -> std::string {
                            if (std::holds_alternative<int>(val)) {
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
                                throw vanction_error::TypeError("HashMap key must be a string or convertible to string");
                            }
                        };
                        key = toString(indexExpr);
                    }
                    
                    // Assign value to HashMap key
                    map->set(key, value);
                }
                // Handle string index assignment (immutable strings)
                else if (std::holds_alternative<std::string>(leftObj)) {
                    throw vanction_error::TypeError("Strings are immutable, cannot assign to index");
                }
                else {
                    throw vanction_error::TypeError("Index assignment not supported for this type");
                }
            }
            // This handles instance.xxx = yyy assignments
            // Check if left side is an Identifier with value "instance"
            else if (binaryExpr->op == ".") {
                if (auto leftIdent = dynamic_cast<Identifier*>(binaryExpr->left)) {
                    if (leftIdent->name == "instance") {
                        // This is an instance property assignment: instance.property = value
                        if (auto rightIdent = dynamic_cast<Identifier*>(binaryExpr->right)) {
                            std::string propertyName = rightIdent->name;
                            
                            // Get the instance from the variables environment
                            if (variables.find("instance") == variables.end()) {
                                throw vanction_error::MethodError("Instance variable not found in current context");
                            }
                            
                            Value instanceVal = variables["instance"];
                            if (!std::holds_alternative<Instance*>(instanceVal)) {
                                throw vanction_error::MethodError("instance variable is not an Instance*");
                            }
                            
                            Instance* instance = std::get<Instance*>(instanceVal);
                            // Assign value to instance variable
                            instance->instanceVariables[propertyName] = value;
                        }
                    }
                }
            }
        }
        return value;
    } else if (auto binaryExpr = dynamic_cast<BinaryExpression*>(expr)) {
        // Execute binary expression
        auto leftVal = executeExpression(binaryExpr->left);
        auto rightVal = executeExpression(binaryExpr->right);
        
        // Handle array indexing: obj[expr]
        if (binaryExpr->op == "[") {
            // Handle string indexing
            if (std::holds_alternative<std::string>(leftVal)) {
                std::string str = std::get<std::string>(leftVal);
                
                // Convert index to integer
                int index;
                if (std::holds_alternative<int>(rightVal)) {
                    index = std::get<int>(rightVal);
                } else {
                    throw vanction_error::TypeError("String index must be an integer");
                }
                
                // Handle negative indices
                if (index < 0) {
                    index = str.length() + index;
                }
                
                // Check bounds
                if (index < 0 || index >= str.length()) {
                    throw vanction_error::RangeError("String index out of range", 0, 0);
                }
                
                return str[index];
            }
            // Handle List indexing
            else if (std::holds_alternative<List*>(leftVal)) {
                List* list = std::get<List*>(leftVal);
                
                // Convert index to integer
                int index;
                if (std::holds_alternative<int>(rightVal)) {
                    index = std::get<int>(rightVal);
                } else {
                    throw vanction_error::TypeError("List index must be an integer");
                }
                
                return list->get(index);
            }
            // Handle HashMap indexing
            else if (std::holds_alternative<HashMap*>(leftVal)) {
                HashMap* map = std::get<HashMap*>(leftVal);
                
                // Convert key to string
                std::string key;
                if (std::holds_alternative<std::string>(rightVal)) {
                    key = std::get<std::string>(rightVal);
                } else {
                    // Convert other types to string
                    auto toString = [](Value val) -> std::string {
                        if (std::holds_alternative<int>(val)) {
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
                            throw vanction_error::TypeError("HashMap key must be a string or convertible to string");
                        }
                    };
                    key = toString(rightVal);
                }
                
                return map->get(key);
            }
            
            throw vanction_error::TypeError("Indexing not supported for this type");
        }
        
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
                    } else if (std::holds_alternative<List*>(val)) {
                        List* list = std::get<List*>(val);
                        std::string result = "[";
                        for (size_t i = 0; i < list->elements.size(); ++i) {
                            // Recursively convert each element to string
                            Value elem = list->elements[i];
                            std::string elemStr;
                            if (std::holds_alternative<std::string>(elem)) {
                                elemStr = std::get<std::string>(elem);
                            } else if (std::holds_alternative<int>(elem)) {
                                elemStr = std::to_string(std::get<int>(elem));
                            } else if (std::holds_alternative<float>(elem)) {
                                elemStr = std::to_string(std::get<float>(elem));
                            } else if (std::holds_alternative<double>(elem)) {
                                elemStr = std::to_string(std::get<double>(elem));
                            } else if (std::holds_alternative<bool>(elem)) {
                                elemStr = std::get<bool>(elem) ? "true" : "false";
                            } else if (std::holds_alternative<char>(elem)) {
                                elemStr = std::string(1, std::get<char>(elem));
                            } else {
                                elemStr = "unknown";
                            }
                            result += elemStr;
                            if (i < list->elements.size() - 1) {
                                result += ", ";
                            }
                        }
                        result += "]";
                        return result;
                    } else if (std::holds_alternative<HashMap*>(val)) {
                        return "{...}";
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
        if (binaryExpr->op == "&" || binaryExpr->op == "|" || binaryExpr->op == "^") {
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
            
            if (binaryExpr->op == "&") {
                return leftBool && rightBool;
            } else if (binaryExpr->op == "|") {
                return leftBool || rightBool;
            } else if (binaryExpr->op == "^") {
                return leftBool != rightBool;
            }
        }
        
        // Handle comparison operators
        if (binaryExpr->op == "==" || binaryExpr->op == "!=") {
            // Handle string comparisons
            if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
                std::string leftStr = std::get<std::string>(leftVal);
                std::string rightStr = std::get<std::string>(rightVal);
                
                if (binaryExpr->op == "==") {
                    return (leftStr == rightStr);
                } else {
                    return (leftStr != rightStr);
                }
            }
            
            // Handle numeric comparisons
            auto getNumber = [](Value val) -> double {
                if (std::holds_alternative<int>(val)) {
                    return static_cast<double>(std::get<int>(val));
                } else if (std::holds_alternative<bool>(val)) {
                    return static_cast<double>(std::get<bool>(val));
                } else if (std::holds_alternative<float>(val)) {
                    return static_cast<double>(std::get<float>(val));
                } else if (std::holds_alternative<double>(val)) {
                    return std::get<double>(val);
                } else if (std::holds_alternative<char>(val)) {
                    return static_cast<double>(std::get<char>(val));
                } else {
                    throw vanction_error::ValueError("Cannot convert to number");
                }
            };
            
            double leftNum = getNumber(leftVal);
            double rightNum = getNumber(rightVal);
            
            if (binaryExpr->op == "==") {
                return (leftNum == rightNum);
            } else {
                return (leftNum != rightNum);
            }
        } else if (binaryExpr->op == "<" || binaryExpr->op == "<=" || binaryExpr->op == ">" || binaryExpr->op == ">=") {
            // Handle all comparison operators
            auto getNumber = [](Value val) -> double {
                if (std::holds_alternative<int>(val)) {
                    return static_cast<double>(std::get<int>(val));
                } else if (std::holds_alternative<bool>(val)) {
                    return static_cast<double>(std::get<bool>(val));
                } else if (std::holds_alternative<float>(val)) {
                    return static_cast<double>(std::get<float>(val));
                } else if (std::holds_alternative<double>(val)) {
                    return std::get<double>(val);
                } else if (std::holds_alternative<char>(val)) {
                    return static_cast<double>(std::get<char>(val));
                } else {
                    throw std::runtime_error("Cannot convert to number");
                }
            };
            
            double leftNum = getNumber(leftVal);
            double rightNum = getNumber(rightVal);
            
            if (binaryExpr->op == "<") {
                return (leftNum < rightNum);
            } else if (binaryExpr->op == "<=") {
                return (leftNum <= rightNum);
            } else if (binaryExpr->op == ">") {
                return (leftNum > rightNum);
            } else {
                return (leftNum >= rightNum);
            }
        } else {
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
                } else if (std::holds_alternative<char>(val)) {
                    return static_cast<double>(std::get<char>(val));
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
                    throw vanction_error::DivideByZeroError("Division by zero", binaryExpr->getLine(), binaryExpr->getColumn());
                }
                result = leftNum / rightNum;
            } else if (binaryExpr->op == "<<") {
                // Bit shift operations - cast to int
                result = static_cast<double>(static_cast<int>(leftNum) << static_cast<int>(rightNum));
            } else if (binaryExpr->op == ">>") {
                // Bit shift operations - cast to int
                result = static_cast<double>(static_cast<int>(leftNum) >> static_cast<int>(rightNum));
            } else if (binaryExpr->op == "%") {
                // Modulo operation - cast to int
                result = static_cast<double>(static_cast<int>(leftNum) % static_cast<int>(rightNum));
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
        }
    } else if (auto instanceCreation = dynamic_cast<InstanceCreationExpression*>(expr)) {
        // Create new instance
        std::string className = instanceCreation->className;
        
        if (debugMode) {
            std::cout << "[DEBUG] Creating instance of class: " << className;
            if (!instanceCreation->namespaceName.empty()) {
                std::cout << " (namespace: " << instanceCreation->namespaceName << ")";
            }
            std::cout << std::endl;
        }
        
        // Check if class exists
        if (classes.find(className) == classes.end()) {
            throw vanction_error::MethodError("Undefined class: " + className);
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
        
        // Check if it's an Instance*
        if (std::holds_alternative<Instance*>(instanceVal)) {
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
                    } else if (std::holds_alternative<List*>(result)) {
                        std::cout << "list";
                    } else if (std::holds_alternative<HashMap*>(result)) {
                        std::cout << "hashmap";
                    } else if (std::holds_alternative<std::monostate>(result)) {
                        std::cout << "undefined";
                    } else if (std::holds_alternative<ErrorObject*>(result)) {
                        std::cout << "errorobject";
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
        } 
        // Check if it's an ErrorObject*
        else if (std::holds_alternative<ErrorObject*>(instanceVal)) {
            ErrorObject* errorObj = std::get<ErrorObject*>(instanceVal);
            std::string memberName = instanceAccess->memberName;
            
            // Access ErrorObject properties
            if (memberName == "text") {
                return errorObj->text;
            } else if (memberName == "type") {
                return errorObj->type;
            } else if (memberName == "info") {
                return errorObj->info;
            } else {
                // Return undefined if property doesn't exist
                return std::monostate{};
            }
        } 
        // Not an instance or error object
        else {
            throw vanction_error::MethodError("Cannot access property of non-instance");
        }
    } else if (auto ident = dynamic_cast<Identifier*>(expr)) {
        // Get variable value
        // First check constants map
        if (constants.find(ident->name) != constants.end()) {
            return constants[ident->name];
        }
        // Then check variables map
        else if (variables.find(ident->name) != variables.end()) {
            return variables[ident->name];
        } else {
            throw vanction_error::VariableError("Undefined variable '" + ident->name + "'", ident->getLine(), ident->getColumn());
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
        if (stringLit->type == "format") {
            // Process formatted string
            std::string formatted = stringLit->value;
            size_t pos = 0;
            while ((pos = formatted.find('{', pos)) != std::string::npos) {
                // Check if it's already escaped
                if (pos > 0 && formatted[pos - 1] == '\\') {
                    // Skip escaped brace
                    pos += 2;
                    continue;
                }
                
                // Find closing brace
                size_t endPos = formatted.find('}', pos + 1);
                if (endPos == std::string::npos) {
                    // No closing brace, just continue
                    pos += 1;
                    continue;
                }
                
                // Extract variable name
                std::string varName = formatted.substr(pos + 1, endPos - pos - 1);
                
                // Get variable value
                std::string varValue;
                if (variables.find(varName) != variables.end()) {
                    Value val = variables[varName];
                    if (std::holds_alternative<std::string>(val)) {
                        varValue = std::get<std::string>(val);
                    } else if (std::holds_alternative<int>(val)) {
                        varValue = std::to_string(std::get<int>(val));
                    } else if (std::holds_alternative<float>(val)) {
                        varValue = std::to_string(std::get<float>(val));
                    } else if (std::holds_alternative<double>(val)) {
                        varValue = std::to_string(std::get<double>(val));
                    } else if (std::holds_alternative<bool>(val)) {
                        varValue = std::get<bool>(val) ? "true" : "false";
                    } else {
                        varValue = "undefined";
                    }
                } else {
                    varValue = "undefined";
                }
                
                // Replace {var} with its value
                formatted.replace(pos, endPos - pos + 1, varValue);
                
                // Move past the replaced text
                pos += varValue.length();
            }
            return formatted;
        } else {
            // Normal or raw string
            return stringLit->value;
        }
    } else if (auto boolLit = dynamic_cast<BooleanLiteral*>(expr)) {
        // Boolean literal
        return boolLit->value;
    } else if (auto errorObj = dynamic_cast<ErrorObject*>(expr)) {
        // Error object - return itself as a value
        return errorObj;
    } else if (auto listLit = dynamic_cast<ListLiteral*>(expr)) {
        // List literal - create a List object and populate it with elements
        List* list = new List();
        for (auto elemExpr : listLit->elements) {
            Value elemValue = executeExpression(elemExpr);
            list->add(elemValue);
        }
        return list;
    } else if (auto hashMapLit = dynamic_cast<HashMapLiteral*>(expr)) {
        // HashMap literal - create a HashMap object and populate it with entries
        HashMap* map = new HashMap();
        for (auto entry : hashMapLit->entries) {
            Value keyValue = executeExpression(entry->key);
            Value valueValue = executeExpression(entry->value);
            
            // Convert key to string
            std::string key;
            if (std::holds_alternative<std::string>(keyValue)) {
                key = std::get<std::string>(keyValue);
            } else {
                // Convert other types to string
                auto toString = [](Value val) -> std::string {
                    if (std::holds_alternative<int>(val)) {
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
                        return "";
                    }
                };
                key = toString(keyValue);
            }
            
            map->set(key, valueValue);
        }
        return map;
    } else if (auto lambdaExpr = dynamic_cast<LambdaExpression*>(expr)) {
        // Create a copy of the current variable environment for closure
        // Store it as a pair of maps: (variables, variableTypes)
        auto closureEnv = new std::pair<std::map<std::string, Value>, std::map<std::string, std::string>>();
        
        // Get the current environment, including any variables from outer scopes
        // This is important for nested lambdas to access variables from their outer lambdas
        closureEnv->first = variables;
        closureEnv->second = variableTypes;
        
        // Set the closure environment for the lambda
        lambdaExpr->closureEnv = closureEnv;
        
        // Return the lambda expression directly as a value
        return lambdaExpr;
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
    
    // Check if this is a lambda function call (variable name followed by parentheses)
    if (call->objectName.empty()) {
        // Check if the method name corresponds to a variable that's a lambda function
        // First check constants map
        if (constants.find(call->methodName) != constants.end()) {
            Value funcVal = constants[call->methodName];
            if (std::holds_alternative<LambdaExpression*>(funcVal)) {
                LambdaExpression* lambdaExpr = std::get<LambdaExpression*>(funcVal);
                // Execute lambda function with arguments
                std::vector<Value> args;
                for (auto argExpr : call->arguments) {
                    args.push_back(executeExpression(argExpr));
                }
                
                // Save current variable environment
                auto savedVariables = variables;
                auto savedVariableTypes = variableTypes;
                
                // Create a new variable environment for the lambda execution
                std::map<std::string, Value> lambdaVariables = variables;
                std::map<std::string, std::string> lambdaVariableTypes = variableTypes;
                
                // If lambda has a closure environment, use it as the base environment
                // This ensures nested lambdas can access variables from outer scopes
                if (lambdaExpr->closureEnv) {
                    auto closureEnv = static_cast<std::pair<std::map<std::string, Value>, std::map<std::string, std::string>>*>(lambdaExpr->closureEnv);
                    // Use closure environment as base
                    lambdaVariables = closureEnv->first;
                    lambdaVariableTypes = closureEnv->second;
                }
                
                // Assign arguments to parameters in the lambda's environment
                for (size_t i = 0; i < lambdaExpr->parameters.size() && i < args.size(); i++) {
                    const std::string& paramName = lambdaExpr->parameters[i].name;
                    lambdaVariables[paramName] = args[i];
                    lambdaVariableTypes[paramName] = "auto";
                }
                
                // Switch to lambda-specific environment
                variables = lambdaVariables;
                variableTypes = lambdaVariableTypes;
                
                // Execute the lambda body
                Value result = executeExpression(lambdaExpr->body);
                
                // Restore original variables
                variables = savedVariables;
                variableTypes = savedVariableTypes;
                
                return result;
            }
        }
        // Then check variables map
        else if (variables.find(call->methodName) != variables.end()) {
            Value funcVal = variables[call->methodName];
            if (std::holds_alternative<LambdaExpression*>(funcVal)) {
                LambdaExpression* lambdaExpr = std::get<LambdaExpression*>(funcVal);
                // Execute lambda function with arguments
                std::vector<Value> args;
                for (auto argExpr : call->arguments) {
                    args.push_back(executeExpression(argExpr));
                }
                
                // Save current variable environment
                auto savedVariables = variables;
                auto savedVariableTypes = variableTypes;
                
                // Create a new variable environment for the lambda execution
                std::map<std::string, Value> lambdaVariables = variables;
                std::map<std::string, std::string> lambdaVariableTypes = variableTypes;
                
                // If lambda has a closure environment, use it as the base environment
                // This ensures nested lambdas can access variables from outer scopes
                if (lambdaExpr->closureEnv) {
                    auto closureEnv = static_cast<std::pair<std::map<std::string, Value>, std::map<std::string, std::string>>*>(lambdaExpr->closureEnv);
                    // Use closure environment as base
                    lambdaVariables = closureEnv->first;
                    lambdaVariableTypes = closureEnv->second;
                }
                
                // Assign arguments to parameters in the lambda's environment
                for (size_t i = 0; i < lambdaExpr->parameters.size() && i < args.size(); i++) {
                    const std::string& paramName = lambdaExpr->parameters[i].name;
                    lambdaVariables[paramName] = args[i];
                    lambdaVariableTypes[paramName] = "auto";
                }
                
                // Switch to lambda-specific environment
                variables = lambdaVariables;
                variableTypes = lambdaVariableTypes;
                
                // Execute the lambda body
                Value result = executeExpression(lambdaExpr->body);
                
                // Restore original variables
                variables = savedVariables;
                variableTypes = savedVariableTypes;
                
                return result;
            }
        }
    }
    
    if ((call->methodName == "print" && call->objectName.empty()) || (call->objectName == "System" && call->methodName == "print")) {
        // Handle both print() and System.print()
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
            } else if (std::holds_alternative<List*>(value)) {
                List* list = std::get<List*>(value);
                std::cout << "[";
                for (size_t i = 0; i < list->elements.size(); ++i) {
                    Value elem = list->elements[i];
                    if (std::holds_alternative<int>(elem)) {
                        std::cout << std::get<int>(elem);
                    } else if (std::holds_alternative<char>(elem)) {
                        std::cout << "'" << std::get<char>(elem) << "'";
                    } else if (std::holds_alternative<std::string>(elem)) {
                        std::cout << '"' << std::get<std::string>(elem) << '"';
                    } else if (std::holds_alternative<bool>(elem)) {
                        std::cout << (std::get<bool>(elem) ? "true" : "false");
                    } else if (std::holds_alternative<float>(elem)) {
                        std::cout << std::get<float>(elem);
                    } else if (std::holds_alternative<double>(elem)) {
                        std::cout << std::get<double>(elem);
                    } else if (std::holds_alternative<List*>(elem)) {
                        std::cout << "<list>";
                    } else if (std::holds_alternative<HashMap*>(elem)) {
                        std::cout << "<hashmap>";
                    } else {
                        std::cout << "undefined";
                    }
                    if (i < list->elements.size() - 1) {
                        std::cout << ", ";
                    }
                }
                std::cout << "]";
            } else if (std::holds_alternative<HashMap*>(value)) {
                std::cout << "{";
                HashMap* map = std::get<HashMap*>(value);
                size_t count = 0;
                for (auto& entry : map->entries) {
                    std::cout << entry.first << ": ";
                    Value val = entry.second;
                    if (std::holds_alternative<int>(val)) {
                        std::cout << std::get<int>(val);
                    } else if (std::holds_alternative<char>(val)) {
                        std::cout << "'" << std::get<char>(val) << "'";
                    } else if (std::holds_alternative<std::string>(val)) {
                        std::cout << '"' << std::get<std::string>(val) << '"';
                    } else if (std::holds_alternative<bool>(val)) {
                        std::cout << (std::get<bool>(val) ? "true" : "false");
                    } else if (std::holds_alternative<float>(val)) {
                        std::cout << std::get<float>(val);
                    } else if (std::holds_alternative<double>(val)) {
                        std::cout << std::get<double>(val);
                    } else if (std::holds_alternative<List*>(val)) {
                        std::cout << "<list>";
                    } else if (std::holds_alternative<HashMap*>(val)) {
                        std::cout << "<hashmap>";
                    } else {
                        std::cout << "undefined";
                    }
                    if (++count < map->entries.size()) {
                        std::cout << ", ";
                    }
                }
                std::cout << "}";
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
        
        // Trim whitespace from input
        size_t start = input.find_first_not_of(" \t\n\r");
        size_t end = input.find_last_not_of(" \t\n\r");
        if (start != std::string::npos && end != std::string::npos) {
            input = input.substr(start, end - start + 1);
        } else {
            input = "";
        }
        
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
                    throw vanction_error::ValueError("Cannot convert string to int");
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
                    throw vanction_error::ValueError("Cannot convert string to float");
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
                    throw vanction_error::ValueError("Cannot convert string to double");
                }
            }
        } else if (call->methodName == "char") {
            // Convert to char
            if (std::holds_alternative<char>(arg)) {
                return arg;
            } else if (std::holds_alternative<int>(arg)) {
                return static_cast<char>(std::get<int>(arg));
            } else if (std::holds_alternative<float>(arg)) {
                return static_cast<char>(std::get<float>(arg));
            } else if (std::holds_alternative<double>(arg)) {
                return static_cast<char>(std::get<double>(arg));
            } else if (std::holds_alternative<std::string>(arg)) {
                // Get first character of string
                const std::string& str = std::get<std::string>(arg);
                if (!str.empty()) {
                    return str[0];
                } else {
                    return '\0'; // Return null character for empty string
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
            throw vanction_error::MethodError("Undefined function: " + funcName);
        }
        
        FunctionDeclaration* func = functions[funcName];
        
        // Save current variable environment
        auto savedVariables = variables;
        
        // Handle function arguments
        if (call->arguments.size() != func->parameters.size()) {
            throw vanction_error::MethodError("Function " + funcName + " expects " + std::to_string(func->parameters.size()) + " arguments, but got " + std::to_string(call->arguments.size()));
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
        // Check if it's a class method call (e.g., Person.init() or class.method())
        if (call->objectName == "class" || classes.find(call->objectName) != classes.end()) {
            // Handle both class.method() and Person.method() syntax
            std::string className;
            if (call->objectName == "class") {
                // In Vanction, class.greet() calls the greet method on the Person class
                // This is a special syntax for class methods
                className = "Person"; // Default to Person class for this syntax
            } else {
                // Handle Person.method() syntax
                className = call->objectName;
            }
            
            std::string methodName = call->methodName;
            
            if (classes.find(className) == classes.end()) {
                throw vanction_error::MethodError("Undefined class: " + className);
            }
            
            ClassDefinition* classDef = classes[className];
            
            // Special handling for init method
            if (methodName == "init") {
                // This is an init method call like Person.init(instance, name, age)
                // Check if we have at least one argument (the instance)
                if (call->arguments.empty()) {
                    throw vanction_error::MethodError("Init method expects at least one argument (instance)");
                }
                
                // Get the instance argument
                Value instanceArg = executeExpression(call->arguments[0]);
                if (!std::holds_alternative<Instance*>(instanceArg)) {
                    throw vanction_error::MethodError("First argument to init must be an instance");
                }
                
                Instance* instance = std::get<Instance*>(instanceArg);
                
                // Save current variable environment
                auto savedVariables = variables;
                
                // Create a new variable environment for the init method execution
                std::map<std::string, Value> initVariables = variables;
                
                // Set the instance parameter to the current instance
                if (classDef->initMethod->parameters.size() > 0) {
                    initVariables[classDef->initMethod->parameters[0].name] = instance;
                }
                // Also explicitly add 'instance' variable for backward compatibility
                initVariables["instance"] = instance;
                
                // Assign init method arguments to parameters, starting from index 1
                for (size_t i = 1; i < call->arguments.size(); ++i) {
                    size_t paramIndex = i;
                    if (paramIndex < classDef->initMethod->parameters.size()) {
                        Value argValue = executeExpression(call->arguments[i]);
                        initVariables[classDef->initMethod->parameters[paramIndex].name] = argValue;
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
                
                // Restore saved variable environment
                variables = savedVariables;
                
                return std::monostate{};
            }
            
            // Find the class method
            ClassMethodDeclaration* method = nullptr;
            for (auto m : classDef->classMethods) {
                if (m->name == methodName) {
                    method = m;
                    break;
                }
            }
            
            if (!method) {
                throw vanction_error::MethodError("Undefined class method: " + methodName + " on class " + className);
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
            // Get the value
            Value value = variables[call->objectName];
            
            // Check if it's a List*
            if (std::holds_alternative<List*>(value)) {
                List* list = std::get<List*>(value);
                std::string methodName = call->methodName;
                
                // Handle List methods
                if (methodName == "add") {
                    // Add element to list
                    if (call->arguments.size() == 1) {
                        Value arg = executeExpression(call->arguments[0]);
                        list->add(arg);
                        return std::monostate{};
                    } else {
                        throw vanction_error::MethodError("List.add() expects exactly 1 argument");
                    }
                } else if (methodName == "get") {
                    // Get element from list
                    if (call->arguments.size() == 1) {
                        Value indexArg = executeExpression(call->arguments[0]);
                        if (std::holds_alternative<int>(indexArg)) {
                            int index = std::get<int>(indexArg);
                            return list->get(index);
                        } else {
                            throw vanction_error::TypeError("List.get() expects integer index");
                        }
                    } else {
                        throw vanction_error::MethodError("List.get() expects exactly 1 argument");
                    }
                } else {
                    throw vanction_error::MethodError("Undefined method: " + methodName + " on List");
                }
            }
            // Check if it's a HashMap*
            else if (std::holds_alternative<HashMap*>(value)) {
                HashMap* map = std::get<HashMap*>(value);
                std::string methodName = call->methodName;
                
                // Handle HashMap methods
                if (methodName == "get") {
                    // Get value from HashMap
                    if (call->arguments.size() == 1 || call->arguments.size() == 2) {
                        Value keyArg = executeExpression(call->arguments[0]);
                        if (std::holds_alternative<std::string>(keyArg)) {
                            std::string key = std::get<std::string>(keyArg);
                            
                            if (call->arguments.size() == 2) {
                                // With default value
                                Value defaultValue = executeExpression(call->arguments[1]);
                                return map->get(key, defaultValue);
                            } else {
                                // Without default value
                                return map->get(key);
                            }
                        } else {
                            throw vanction_error::TypeError("HashMap.get() expects string key");
                        }
                    } else {
                        throw vanction_error::MethodError("HashMap.get() expects 1 or 2 arguments");
                    }
                } else if (methodName == "keys" || methodName == "key") {
                    // Get all keys as List* (support both singular and plural)
                    return map->keys();
                } else if (methodName == "values" || methodName == "value") {
                    // Get all values as List* (support both singular and plural)
                    return map->values();
                } else {
                    throw vanction_error::MethodError("Undefined method: " + methodName + " on HashMap");
                }
            }
            // Check if it's a string
            else if (std::holds_alternative<std::string>(value)) {
                std::string strVal = std::get<std::string>(value);
                std::string methodName = call->methodName;
                
                // Handle string methods
                if (methodName == "replace") {
                    // Replace substring
                    if (call->arguments.size() == 2) {
                        Value oldArg = executeExpression(call->arguments[0]);
                        Value newArg = executeExpression(call->arguments[1]);
                        
                        if (std::holds_alternative<std::string>(oldArg) && std::holds_alternative<std::string>(newArg)) {
                            std::string oldStr = std::get<std::string>(oldArg);
                            std::string newStr = std::get<std::string>(newArg);
                            
                            // Simple string replacement
                            std::string result = strVal;
                            size_t pos = 0;
                            while ((pos = result.find(oldStr, pos)) != std::string::npos) {
                                result.replace(pos, oldStr.length(), newStr);
                                pos += newStr.length();
                            }
                            return result;
                        } else {
                            throw vanction_error::TypeError("String.replace() expects string arguments");
                        }
                    } else {
                        throw vanction_error::MethodError("String.replace() expects exactly 2 arguments");
                    }
                } else if (methodName == "excision") {
                    // Split string by delimiter (excision is like split)
                    if (call->arguments.size() == 1) {
                        Value delimArg = executeExpression(call->arguments[0]);
                        
                        if (std::holds_alternative<std::string>(delimArg)) {
                            std::string delim = std::get<std::string>(delimArg);
                            
                            List* result = new List();
                            size_t start = 0;
                            size_t end = strVal.find(delim);
                            
                            while (end != std::string::npos) {
                                result->add(strVal.substr(start, end - start));
                                start = end + delim.length();
                                end = strVal.find(delim, start);
                            }
                            
                            // Add the last part
                            result->add(strVal.substr(start));
                            
                            return result;
                        } else {
                            throw vanction_error::TypeError("String.excision() expects string delimiter");
                        }
                    } else {
                        throw vanction_error::MethodError("String.excision() expects exactly 1 argument");
                    }
                } else {
                    throw vanction_error::MethodError("Undefined method: " + methodName + " on String");
                }
            }
            // Check if it's an Instance*
            else if (!std::holds_alternative<Instance*>(value)) {
                throw vanction_error::MethodError("Cannot call method on non-instance: " + call->objectName);
            }
            // Continue with instance method call handling
            Value instanceVal = value;
            
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
                throw vanction_error::MethodError("Undefined method: " + methodName + " on instance of " + instance->cls->name);
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
                throw vanction_error::MethodError("Method " + methodName + " expects " + std::to_string(expectedArgs) + " arguments, but got " + std::to_string(call->arguments.size()));
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
                        throw vanction_error::MethodError("Init method requires an instance argument");
                    }
                    
                    Value instanceVal = executeExpression(call->arguments[0]);
                    if (!std::holds_alternative<Instance*>(instanceVal)) {
                        throw vanction_error::MethodError("First argument to init must be an instance");
                    }
                    
                    Instance* instance = std::get<Instance*>(instanceVal);
                    
                    // Find the init method
                    InstanceMethodDeclaration* method = classDef->initMethod;
                    if (!method) {
                        throw vanction_error::MethodError("Undefined method: init on class " + className);
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
                throw vanction_error::MethodError("Undefined namespace: " + namespaceName);
            }
            
            // Check if function exists in namespace
            if (namespaces[namespaceName].find(funcName) == namespaces[namespaceName].end()) {
                throw vanction_error::MethodError("Undefined function in namespace " + namespaceName + ": " + funcName);
            }
            
            FunctionDeclaration* func = namespaces[namespaceName][funcName];
            
            // Save current variable environment
            auto savedVariables = variables;
            
            // Handle function arguments
            if (call->arguments.size() != func->parameters.size()) {
                throw vanction_error::MethodError("Function " + namespaceName + ":" + funcName + " expects " + std::to_string(func->parameters.size()) + " arguments, but got " + std::to_string(call->arguments.size()));
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
                throw vanction_error::SyntaxError("AST generation failed");
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
                
                // Always preserve generated C++ file for debugging
                std::cout << "Generated C++ code preserved at: " << cppFile << std::endl;
                // std::remove(cppFile.c_str());
                // std::cout << "Cleaned up temporary files" << std::endl;
            } else {
                Error error(ErrorType::CompilationError, "GCC compilation failed", filePath, 1, 1);
                errorReporter.report(error);
                std::cout << "Generated C++ code preserved at: " << cppFile << std::endl;
                // Clean up temporary file
                std::remove(cppFile.c_str());
                std::cout << "Cleaned up temporary files" << std::endl;
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
                throw vanction_error::SyntaxError("AST generation failed");
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
            
            // Extract the directory of the currently executing file
            std::string fileDirectory;
            size_t lastSlash = filePath.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                fileDirectory = filePath.substr(0, lastSlash);
            } else {
                fileDirectory = ".";
            }
            
            // Initialize global module manager before executing program
            if (!globalModuleManager) {
                globalModuleManager = new ModuleManager();
            }
            
            // Set the directory of the currently executing file
            globalModuleManager->setCurrentExecutingFileDirectory(fileDirectory);
            
            // Initialize global constants
            initializeConstants();
            
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
    } catch (const vanction_error::VanctionError& e) {
        // Read file content again for error reporting
        std::string sourceCode = readFile(filePath);
        ErrorReporter errorReporter(sourceCode, filePath);
        
        // Determine error type from the exception object
        ErrorType errorType = ErrorType::UnknownError;
        
        if (e.getType() == "CError") {
            errorType = ErrorType::CError;
        } else if (e.getType() == "MethodError") {
            errorType = ErrorType::MethodError;
        } else if (e.getType() == "CompilationError") {
            errorType = ErrorType::CompilationError;
        } else if (e.getType() == "DivideByZeroError") {
            errorType = ErrorType::DivideByZeroError;
        } else if (e.getType() == "ValueError") {
            errorType = ErrorType::ValueError;
        } else if (e.getType() == "TokenError") {
            errorType = ErrorType::TokenError;
        } else if (e.getType() == "SyntaxError") {
            errorType = ErrorType::SyntaxError;
        } else if (e.getType() == "MainFunctionError") {
            errorType = ErrorType::MainFunctionError;
        } else if (e.getType() == "ImmutError") {
            errorType = ErrorType::ImmutError;
        }
        
        // Create and report error
        Error error(errorType, e.getMessage(), filePath, e.getLine(), e.getColumn());
        errorReporter.report(error);
        return 1;
    } catch (const std::runtime_error& e) {
        // Read file content again for error reporting
        std::string sourceCode = readFile(filePath);
        ErrorReporter errorReporter(sourceCode, filePath);
        
        // Determine error type based on error message (fallback for old-style errors)
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