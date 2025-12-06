#ifndef VANCTION_AST_H
#define VANCTION_AST_H

#include <string>
#include <vector>

// AST node base class
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

// Function declaration node
class FunctionDeclaration : public ASTNode {
public:
    std::string name;
    std::vector<ASTNode*> body;
    
    FunctionDeclaration(const std::string& name)
        : name(name) {}
    
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
    Identifier* left;
    Expression* right;
    
    AssignmentExpression(Identifier* left, Expression* right)
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
        delete expression;
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
