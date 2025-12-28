#include "module_manager.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <direct.h> // For getcwd on Windows
#include <string>
#ifdef _WIN32
#include <windows.h> // For GetModuleFileNameA
#endif
#include "error.h"

// Constructor
ModuleManager::ModuleManager() {
    // Set current working directory
    char buffer[FILENAME_MAX];
    if (getcwd(buffer, FILENAME_MAX)) {
        currentDirectory = buffer;
    } else {
        currentDirectory = ".";
    }
    
    // Get executable directory
    std::string execDir;
#ifdef _WIN32
    char exeBuffer[FILENAME_MAX];
    GetModuleFileNameA(NULL, exeBuffer, FILENAME_MAX);
    std::string exePath(exeBuffer);
    execDir = exePath.substr(0, exePath.find_last_of('\\'));
#else
    char exeBuffer[FILENAME_MAX];
    ssize_t len = readlink("/proc/self/exe", exeBuffer, FILENAME_MAX - 1);
    if (len != -1) {
        exeBuffer[len] = '\0';
        std::string exePath(exeBuffer);
        execDir = exePath.substr(0, exePath.find_last_of('/'));
    } else {
        execDir = ".";
    }
#endif
    
    // Initialize current executing file directory to current directory
    currentExecutingFileDirectory = currentDirectory;
    
    // Add search paths in order of precedence
    // 1. Current directory (project directory)
    addSearchPath(".");
    // 2. Current directory's stdlib
    addSearchPath("stdlib");
    // 3. Executable directory's stdlib (for shared standard library)
    addSearchPath(execDir + "/stdlib");
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
    // Handle nested modules (with dots)
    std::string fullPath;
    std::ifstream file;
    
    // Convert module name to file path
    std::string modulePath = moduleName;
    size_t dotPos;
    while ((dotPos = modulePath.find('.')) != std::string::npos) {
        modulePath.replace(dotPos, 1, "/");
    }
    
    // First try as direct file (with .vn extension)
    // Try in the current executing file directory
    fullPath = currentExecutingFileDirectory + "/" + modulePath + ".vn";
    file.open(fullPath);
    if (file.is_open()) {
        file.close();
        return fullPath;
    }
    
    // Try in the current executing file directory with each search path
    for (const auto& path : searchPaths) {
        // Check if path is absolute (starts with drive letter or slash)
        bool isAbsolute = false;
        if (!path.empty()) {
            isAbsolute = (path[0] == '/' || (path.size() > 1 && path[1] == ':'));
        }
        
        // Construct full path based on whether path is absolute
        std::string combinedPath;
        if (isAbsolute) {
            combinedPath = path;
        } else {
            combinedPath = currentExecutingFileDirectory + "/" + path;
        }
        
        fullPath = combinedPath + "/" + modulePath + ".vn";
        file.open(fullPath);
        if (file.is_open()) {
            file.close();
            return fullPath;
        }
    }
    
    // Try with current directory directly
    fullPath = currentDirectory + "/" + modulePath + ".vn";
    file.open(fullPath);
    if (file.is_open()) {
        file.close();
        return fullPath;
    }
    
    // Try as directory with _package_.vn
    // Try in the current executing file directory
    fullPath = currentExecutingFileDirectory + "/" + modulePath + "/_package_.vn";
    file.open(fullPath);
    if (file.is_open()) {
        file.close();
        return fullPath;
    }
    
    // Try in the current executing file directory with each search path
    for (const auto& path : searchPaths) {
        // Check if path is absolute (starts with drive letter or slash)
        bool isAbsolute = false;
        if (!path.empty()) {
            isAbsolute = (path[0] == '/' || (path.size() > 1 && path[1] == ':'));
        }
        
        // Construct full path based on whether path is absolute
        std::string combinedPath;
        if (isAbsolute) {
            combinedPath = path;
        } else {
            combinedPath = currentExecutingFileDirectory + "/" + path;
        }
        
        fullPath = combinedPath + "/" + modulePath + "/_package_.vn";
        file.open(fullPath);
        if (file.is_open()) {
            file.close();
            return fullPath;
        }
    }
    
    // Try with current directory directly
    fullPath = currentDirectory + "/" + modulePath + "/_package_.vn";
    file.open(fullPath);
    if (file.is_open()) {
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
        throw std::runtime_error("Failed to parse program AST in file: " + filePath);
    }
    
    return ast;
}
