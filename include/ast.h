#ifndef VANCTION_AST_H
#define VANCTION_AST_H

#include <string>
#include <vector>

// AST node base class
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

// Function parameter
struct FunctionParameter {
    std::string type; // Optional, defaults to "auto"
    std::string name;
    
    FunctionParameter(const std::string& name, const std::string& type = "auto")
        : type(type), name(name) {}
};

// Function declaration node
class FunctionDeclaration : public ASTNode {
public:
    std::string returnType;
    std::string name;
    std::vector<FunctionParameter> parameters;
    std::vector<ASTNode*> body;
    
    FunctionDeclaration(const std::string& returnType, const std::string& name)
        : returnType(returnType), name(name) {}
    
    ~FunctionDeclaration() {
        for (auto node : body) {
            delete node;
        }
    }
};

// Statement node base class
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// Comment node
class Comment : public Statement {
public:
    std::string text;
    
    Comment(const std::string& text)
        : text(text) {}
};

// Expression node base class
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

// Identifier expression
class Identifier : public Expression {
public:
    std::string name;
    
    Identifier(const std::string& name)
        : name(name) {}
};

// Integer literal expression
class IntegerLiteral : public Expression {
public:
    int value;
    
    IntegerLiteral(int value)
        : value(value) {}
};

// Float literal expression
class FloatLiteral : public Expression {
public:
    float value;
    
    FloatLiteral(float value)
        : value(value) {}
};

// Double literal expression
class DoubleLiteral : public Expression {
public:
    double value;
    
    DoubleLiteral(double value)
        : value(value) {}
};

// Char literal expression
class CharLiteral : public Expression {
public:
    char value;
    
    CharLiteral(char value)
        : value(value) {}
};

// Boolean literal expression
class BooleanLiteral : public Expression {
public:
    bool value;
    
    BooleanLiteral(bool value)
        : value(value) {}
};

// String literal expression
class StringLiteral : public Expression {
public:
    std::string value;
    
    StringLiteral(const std::string& value)
        : value(value) {}
};

// Function call expression
class FunctionCall : public Expression {
public:
    std::string objectName;
    std::string methodName;
    std::vector<Expression*> arguments;
    
    FunctionCall(const std::string& objectName, const std::string& methodName)
        : objectName(objectName), methodName(methodName) {}
    
    ~FunctionCall() {
        for (auto arg : arguments) {
            delete arg;
        }
    }
};

// Variable declaration statement
class VariableDeclaration : public Statement {
public:
    std::string type;
    std::string name;
    Expression* initializer;
    bool isAuto;
    bool isDefine;
    
    VariableDeclaration(const std::string& type, const std::string& name, Expression* initializer = nullptr, bool isAuto = false, bool isDefine = false)
        : type(type), name(name), initializer(initializer), isAuto(isAuto), isDefine(isDefine) {}
    
    ~VariableDeclaration() {
        if (initializer) {
            delete initializer;
        }
    }
};

// Binary expression (for string concatenation)
class BinaryExpression : public Expression {
public:
    Expression* left;
    std::string op;
    Expression* right;
    
    BinaryExpression(Expression* left, const std::string& op, Expression* right)
        : left(left), op(op), right(right) {}
    
    ~BinaryExpression() {
        delete left;
        delete right;
    }
};

// Assignment expression
class AssignmentExpression : public Expression {
public:
    Expression* left;
    Expression* right;
    
    AssignmentExpression(Expression* left, Expression* right)
        : left(left), right(right) {}
    
    ~AssignmentExpression() {
        delete left;
        delete right;
    }
};

// Expression statement
class ExpressionStatement : public Statement {
public:
    Expression* expression;
    
    ExpressionStatement(Expression* expression)
        : expression(expression) {}
    
    ~ExpressionStatement() {
        if (expression) {
            delete expression;
        }
    }
};

// Return statement
class ReturnStatement : public Statement {
public:
    Expression* expression;
    
    ReturnStatement(Expression* expression = nullptr)
        : expression(expression) {}
    
    ~ReturnStatement() {
        if (expression) {
            delete expression;
        }
    }
};

// If-else statement
class IfStatement : public Statement {
public:
    Expression* condition;
    std::vector<ASTNode*> ifBody;
    std::vector<IfStatement*> elseIfs;
    std::vector<ASTNode*> elseBody;
    
    IfStatement(Expression* condition, const std::vector<ASTNode*>& ifBody)
        : condition(condition), ifBody(ifBody) {}
    
    ~IfStatement() {
        delete condition;
        for (auto node : ifBody) {
            delete node;
        }
        for (auto elseIf : elseIfs) {
            delete elseIf;
        }
        for (auto node : elseBody) {
            delete node;
        }
    }
};

// For loop statement (traditional)
class ForLoopStatement : public Statement {
public:
    Statement* initialization;
    Expression* condition;
    Expression* increment;
    std::vector<ASTNode*> body;
    
    ForLoopStatement(Statement* initialization, Expression* condition, Expression* increment, const std::vector<ASTNode*>& body)
        : initialization(initialization), condition(condition), increment(increment), body(body) {}
    
    ~ForLoopStatement() {
        delete initialization;
        delete condition;
        delete increment;
        for (auto node : body) {
            delete node;
        }
    }
};

// For in loop statement (enhanced)
class ForInLoopStatement : public Statement {
public:
    std::string variableName;
    Expression* collection;
    std::vector<ASTNode*> body;
    
    ForInLoopStatement(const std::string& variableName, Expression* collection, const std::vector<ASTNode*>& body)
        : variableName(variableName), collection(collection), body(body) {}
    
    ~ForInLoopStatement() {
        delete collection;
        for (auto node : body) {
            delete node;
        }
    }
};

// While loop statement
class WhileLoopStatement : public Statement {
public:
    Expression* condition;
    std::vector<ASTNode*> body;
    
    WhileLoopStatement(Expression* condition, const std::vector<ASTNode*>& body)
        : condition(condition), body(body) {}
    
    ~WhileLoopStatement() {
        delete condition;
        for (auto node : body) {
            delete node;
        }
    }
};

// Do-while loop statement
class DoWhileLoopStatement : public Statement {
public:
    std::vector<ASTNode*> body;
    Expression* condition;
    
    DoWhileLoopStatement(const std::vector<ASTNode*>& body, Expression* condition)
        : body(body), condition(condition) {}
    
    ~DoWhileLoopStatement() {
        for (auto node : body) {
            delete node;
        }
        delete condition;
    }
};

// Case statement for switch
class CaseStatement : public Statement {
public:
    Expression* value;
    std::vector<ASTNode*> body;
    
    CaseStatement(Expression* value, const std::vector<ASTNode*>& body)
        : value(value), body(body) {}
    
    ~CaseStatement() {
        delete value;
        for (auto node : body) {
            delete node;
        }
    }
};

// Switch statement
class SwitchStatement : public Statement {
public:
    Expression* expression;
    std::vector<CaseStatement*> cases;
    
    SwitchStatement(Expression* expression, const std::vector<CaseStatement*>& cases)
        : expression(expression), cases(cases) {}
    
    ~SwitchStatement() {
        delete expression;
        for (auto caseStmt : cases) {
            delete caseStmt;
        }
    }
};

// Namespace declaration node
class NamespaceDeclaration : public ASTNode {
public:
    std::string name;
    std::vector<ASTNode*> declarations;
    
    NamespaceDeclaration(const std::string& name)
        : name(name) {}
    
    ~NamespaceDeclaration() {
        for (auto node : declarations) {
            delete node;
        }
    }
};

// Namespace access expression
class NamespaceAccess : public Expression {
public:
    std::string namespaceName;
    std::string memberName;
    
    NamespaceAccess(const std::string& namespaceName, const std::string& memberName)
        : namespaceName(namespaceName), memberName(memberName) {}
};

// Class declaration node
class ClassDeclaration : public ASTNode {
public:
    std::string name;
    std::string baseClassName;
    std::vector<ASTNode*> methods;
    std::vector<ASTNode*> instanceMethods;
    ASTNode* initMethod;
    
    ClassDeclaration(const std::string& name, const std::string& baseClassName = "")
        : name(name), baseClassName(baseClassName), initMethod(nullptr) {}
    
    ~ClassDeclaration() {
        for (auto method : methods) {
            delete method;
        }
        for (auto method : instanceMethods) {
            delete method;
        }
        if (initMethod) {
            delete initMethod;
        }
    }
};

// Class method declaration node
class ClassMethodDeclaration : public FunctionDeclaration {
public:
    std::string className;
    
    ClassMethodDeclaration(const std::string& className, const std::string& name, const std::string& returnType = "void")
        : FunctionDeclaration(returnType, name), className(className) {}
};

// Instance method declaration node
class InstanceMethodDeclaration : public FunctionDeclaration {
public:
    std::string className;
    
    InstanceMethodDeclaration(const std::string& className, const std::string& name, const std::string& returnType = "void")
        : FunctionDeclaration(returnType, name), className(className) {}
};

// Instance creation expression
class InstanceCreationExpression : public Expression {
public:
    std::string namespaceName;
    std::string className;
    std::vector<Expression*> arguments;
    
    InstanceCreationExpression(const std::string& className, const std::string& namespaceName = "")
        : className(className), namespaceName(namespaceName) {}
    
    ~InstanceCreationExpression() {
        for (auto arg : arguments) {
            delete arg;
        }
    }
};

// Instance access expression
class InstanceAccessExpression : public Expression {
public:
    Expression* instance;
    std::string memberName;
    
    InstanceAccessExpression(Expression* instance, const std::string& memberName)
        : instance(instance), memberName(memberName) {}
    
    ~InstanceAccessExpression() {
        delete instance;
    }
};

// Program node
class Program : public ASTNode {
public:
    std::vector<ASTNode*> declarations;
    
    ~Program() {
        for (auto node : declarations) {
            delete node;
        }
    }
};

#endif // VANCTION_AST_H
