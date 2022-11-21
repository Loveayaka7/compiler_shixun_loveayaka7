#include "SyntaxTreeChecker.h"
using namespace SyntaxTree;

//#define DEBUG

void SyntaxTreeChecker::visit(Assembly &node)
{
    make_new_table(); //建立全局符号表
    for (auto def : node.global_defs)
    {
        def->accept(*this);
    }
}

void SyntaxTreeChecker::visit(FuncDef &node)
{
    if (!Insert_Func(node))
    {
        err.error(node.loc, "Redefined Function");
        exit(int(ErrorType::FuncDuplicated));
    }
    is_func_block = true; //一个新的函数开始
    node.body->accept(*this);
}

void SyntaxTreeChecker::visit(BinaryExpr &node)
{
    node.lhs->accept(*this);
    bool lhs_int = this->Expr_int;
    struct Entry lhs_type = this->current_type;
    node.rhs->accept(*this);
    bool rhs_int = this->Expr_int;
    struct Entry rhs_type = this->current_type;

    if (node.op == SyntaxTree::BinOp::MODULO)
    {
        if (!lhs_int || !rhs_int)
        {
            err.error(node.loc, "Operands of modulo should be integers.");
            exit(int(ErrorType::Modulo));
        }
        this->current_type.type = SyntaxTree::Type::INT;//模运算结果为INT
    }
    else
    {
        //类型提升
        if (lhs_type.type == SyntaxTree::Type::FLOAT || rhs_type.type == SyntaxTree::Type::FLOAT)
            this->current_type.type = SyntaxTree::Type::FLOAT;
        else
            this->current_type.type = SyntaxTree::Type::INT;
    }
    this->Expr_int = lhs_int & rhs_int;
}

void SyntaxTreeChecker::visit(UnaryExpr &node)
{
    node.rhs->accept(*this);
}

void SyntaxTreeChecker::visit(LVal &node)
{
    for (auto exp : node.array_index)
    {
        exp->accept(*this);
    }
    struct Entry *type = Lookup(node.name);
    if (type == NULL)
    {
        err.error(node.loc, "Use Undefined Variable");
        exit(int(ErrorType::VarUnknown));
    }

    this->current_type.type = type->type;
    this->Expr_int = (type->type == SyntaxTree::Type::INT);
}

void SyntaxTreeChecker::visit(Literal &node)
{
    this->Expr_int = (node.literal_type == SyntaxTree::Type::INT);
    this->current_type.type = node.literal_type;
}

void SyntaxTreeChecker::visit(ReturnStmt &node)
{
    if (node.ret != nullptr)
        node.ret->accept(*this);
}

void SyntaxTreeChecker::visit(VarDef &node)
{
    if (node.is_inited) //S先处理初始化
    {
        node.initializers->accept(*this);
    }
    if (!Insert_Var(node))
    {
        err.error(node.loc, "Redefined Variable");
        exit(int(ErrorType::VarDuplicated));
    }
}

void SyntaxTreeChecker::visit(AssignStmt &node)
{
    node.target->accept(*this);
    bool target_int = this->Expr_int;
    node.value->accept(*this);
    bool value_int = this->Expr_int;

    this->Expr_int = target_int & value_int;
}

void SyntaxTreeChecker::visit(FuncCallStmt &node)
{

    struct Entry *type = Lookup(node.name);
    if (type == NULL)
    {
        err.error(node.loc, "Use Undefined Function");
        exit(int(ErrorType::FuncUnknown));
    }
    else
    {
        //参数长度不匹配
        if (type->func_params.size() != node.params.size())
        {
            err.error(node.loc, "Can not Match Function Parameters");
            exit(int(ErrorType::FuncParams));
        }
        else
        {
            int i = 0;
            //遍历实参与形参
            for (auto exp : node.params)
            {
                exp->accept(*this);
                if (this->current_type.type != type->func_params[i].type)
                {
                    err.error(node.loc, "Can not Match Function Parameters");
                    exit(int(ErrorType::FuncParams));
                }
                else
                    i++;
            }
        }
        this->current_type.type = type->type;
        this->Expr_int = (type->type == SyntaxTree::Type::INT);
    }
}

void SyntaxTreeChecker::visit(BlockStmt &node)
{
    //一个新的函数在插入时就已经建了新的符号表，所以不建立
    if (is_func_block)
        is_func_block = false;
    //对非函数定义的语句块添加新的符号表
    else
        make_new_table();

    for (auto stmt : node.body)
    {
        stmt->accept(*this);
    }
    //语句块作用域结束，删除之前添加的符号表
    delete_table();
}

void SyntaxTreeChecker::visit(EmptyStmt &node) {}

void SyntaxTreeChecker::visit(SyntaxTree::ExprStmt &node)//？
{
    node.exp->accept(*this);
}

void SyntaxTreeChecker::visit(SyntaxTree::FuncParam &node)
{
    for (auto exp : node.array_index)
    {
        exp->accept(*this);
    }
}

void SyntaxTreeChecker::visit(SyntaxTree::FuncFParamList &node)
{
    for (auto param : node.params)
    {
        param->accept(*this);
    }
}

void SyntaxTreeChecker::visit(SyntaxTree::BinaryCondExpr &node)
{
    node.lhs->accept(*this);
    node.rhs->accept(*this);
    this->Expr_int = false;
    this->current_type.type = SyntaxTree::Type::BOOL;
}

void SyntaxTreeChecker::visit(SyntaxTree::UnaryCondExpr &node)
{
    node.rhs->accept(*this);
    this->Expr_int = false;
    this->current_type.type = SyntaxTree::Type::BOOL;
}

void SyntaxTreeChecker::visit(SyntaxTree::IfStmt &node)
{
    node.cond_exp->accept(*this);
    node.if_statement->accept(*this);
    if (node.else_statement != NULL)
        node.else_statement->accept(*this);
}

void SyntaxTreeChecker::visit(SyntaxTree::WhileStmt &node)
{
    node.cond_exp->accept(*this);
    node.statement->accept(*this);
}

void SyntaxTreeChecker::visit(SyntaxTree::BreakStmt &node) {}

void SyntaxTreeChecker::visit(SyntaxTree::ContinueStmt &node) {}

void SyntaxTreeChecker::visit(SyntaxTree::InitVal &node)
{
    if (node.isExp)//变量
    {
        node.expr->accept(*this);
    }
    else
    {
        for (auto element : node.elementList)//数组
        {
            element->accept(*this);
        }
    }
}