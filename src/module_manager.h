#ifndef VANCTION_MODULE_MANAGER_H
#define VANCTION_MODULE_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "parser.h"
#include "lexer.h"
#include "../include/ast.h"

// Module class representing a loaded module
class Module {
public:
    std::string name;
    std::string filePath;
    Program* ast;
    
    Module(const std::string& name, const std::string& filePath, Program* ast)
        : name(name), filePath(filePath), ast(ast) {}
    
    ~Module() {
        delete ast;
    }
};

// Module manager class for handling module loading and dependencies
class ModuleManager {
public:
    // Constructor
    ModuleManager();
    
    // Destructor
    ~ModuleManager();
    
    // Load a module by name
    Module* loadModule(const std::string& moduleName);
    
    // Find a module by name
    Module* findModule(const std::string& moduleName);
    
    // Add a search path for modules
    void addSearchPath(const std::string& path);
    
    // Set the current working directory
    void setCurrentDirectory(const std::string& directory);
    
    // Set the directory of the currently executing .vn file
    void setCurrentExecutingFileDirectory(const std::string& directory);
    
    // Clear all loaded modules
    void clearModules();
    
private:
    // Search paths for modules
    std::vector<std::string> searchPaths;
    
    // Current working directory
    std::string currentDirectory;
    
    // Directory of the currently executing .vn file
    std::string currentExecutingFileDirectory;
    
    // Loaded modules map
    std::unordered_map<std::string, Module*> modules;
    
    // Modules being loaded (to detect circular dependencies)
    std::unordered_map<std::string, bool> modulesLoading;
    
    // Find the file path of a module
    std::string findModuleFilePath(const std::string& moduleName);
    
    // Read the content of a file
    std::string readFile(const std::string& filePath);
    
    // Parse a module file and generate AST
    Program* parseModuleFile(const std::string& filePath);
};

#endif // VANCTION_MODULE_MANAGER_H