#include "module_manager.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <direct.h> // For getcwd on Windows
#include "error.h"

// Constructor
ModuleManager::ModuleManager() {
    // Add current directory as default search path
    addSearchPath(".");
    
    // Set current working directory
    char buffer[FILENAME_MAX];
    if (getcwd(buffer, FILENAME_MAX)) {
        currentDirectory = buffer;
    } else {
        currentDirectory = ".";
    }
    
    // Initialize current executing file directory to current directory
    currentExecutingFileDirectory = currentDirectory;
}

// Destructor
ModuleManager::~ModuleManager() {
    clearModules();
}

// Load a module by name
Module* ModuleManager::loadModule(const std::string& moduleName) {
    // Check if the module is already loaded
    Module* module = findModule(moduleName);
    if (module) {
        return module;
    }
    
    // Check for circular dependencies
    if (modulesLoading.find(moduleName) != modulesLoading.end()) {
        throw std::runtime_error("Circular dependency detected: " + moduleName);
    }
    
    // Mark the module as loading
    modulesLoading[moduleName] = true;
    
    try {
        // Find the module file path
        std::string filePath = findModuleFilePath(moduleName);
        if (filePath.empty()) {
            throw std::runtime_error("Module not found: " + moduleName);
        }
        
        // Parse the module file to generate AST
        Program* ast = parseModuleFile(filePath);
        if (!ast) {
            throw std::runtime_error("Failed to parse module: " + moduleName);
        }
        
        // Create module object
        module = new Module(moduleName, filePath, ast);
        
        // Add to loaded modules map
        modules[moduleName] = module;
        
        // Remove from loading map
        modulesLoading.erase(moduleName);
        
        return module;
    } catch (const std::exception& e) {
        // Remove from loading map
        modulesLoading.erase(moduleName);
        throw;
    }
}

// Find a module by name
Module* ModuleManager::findModule(const std::string& moduleName) {
    auto it = modules.find(moduleName);
    if (it != modules.end()) {
        return it->second;
    }
    return nullptr;
}

// Add a search path for modules
void ModuleManager::addSearchPath(const std::string& path) {
    searchPaths.push_back(path);
}

// Set the current working directory
void ModuleManager::setCurrentDirectory(const std::string& directory) {
    currentDirectory = directory;
}

// Set the directory of the currently executing .vn file
void ModuleManager::setCurrentExecutingFileDirectory(const std::string& directory) {
    currentExecutingFileDirectory = directory;
}

// Clear all loaded modules
void ModuleManager::clearModules() {
    for (auto& pair : modules) {
        delete pair.second;
    }
    modules.clear();
    modulesLoading.clear();
}

// Find the file path of a module
std::string ModuleManager::findModuleFilePath(const std::string& moduleName) {
    // First try in the current executing file directory
    std::string fullPath = currentExecutingFileDirectory + "/" + moduleName + ".vn";
    std::ifstream file(fullPath);
    if (file.good()) {
        file.close();
        return fullPath;
    }
    
    // Try in the current executing file directory with each search path
    for (const auto& path : searchPaths) {
        // Construct the full file path
        fullPath = currentExecutingFileDirectory + "/" + path + "/" + moduleName + ".vn";
        
        // Check if the file exists
        file.open(fullPath);
        if (file.good()) {
            file.close();
            return fullPath;
        }
    }
    
    // Try to find the module in each search path with current directory
    for (const auto& path : searchPaths) {
        // Construct the full file path
        fullPath = path + "/" + moduleName + ".vn";
        
        // Check if the file exists
        file.open(fullPath);
        if (file.good()) {
            file.close();
            return fullPath;
        }
        
        // Try with current directory if path is relative
        fullPath = currentDirectory + "/" + path + "/" + moduleName + ".vn";
        file.open(fullPath);
        if (file.good()) {
            file.close();
            return fullPath;
        }
    }
    
    // Try with current directory directly
    fullPath = currentDirectory + "/" + moduleName + ".vn";
    file.open(fullPath);
    if (file.good()) {
        file.close();
        return fullPath;
    }
    
    // Module not found
    return "";
}

// Read the content of a file
std::string ModuleManager::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
    
    file.close();
    return content;
}

// Parse a module file and generate AST
Program* ModuleManager::parseModuleFile(const std::string& filePath) {
    // Read the file content
    std::string content = readFile(filePath);
    
    // Create lexer and parser
    Lexer lexer(content);
    Parser parser(lexer);
    
    // Parse the program and generate AST
    Program* ast = parser.parseProgramAST();
    if (!ast) {
        throw std::runtime_error("Failed to parse program AST");
    }
    
    return ast;
}
