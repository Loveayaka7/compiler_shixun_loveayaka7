#ifndef _SYSYF_SYNTAX_TREE_CHECKER_H_
#define _SYSYF_SYNTAX_TREE_CHECKER_H_

#include <cassert>
#include "ErrorReporter.h"
#include "SyntaxTree.h"
#include <map>
#include <vector>
#include <list>


enum class ErrorType {
    Accepted = 0,
    Modulo,
    VarUnknown,
    VarDuplicated,
    FuncUnknown,
    FuncDuplicated,
    FuncParams
};


class SyntaxTreeChecker : public SyntaxTree::Visitor {
   public:
    SyntaxTreeChecker(ErrorReporter& e) : err(e) {}
    virtual void visit(SyntaxTree::Assembly& node) override;
    virtual void visit(SyntaxTree::FuncDef& node) override;
    virtual void visit(SyntaxTree::BinaryExpr& node) override;
    virtual void visit(SyntaxTree::UnaryExpr& node) override;
    virtual void visit(SyntaxTree::LVal& node) override;
    virtual void visit(SyntaxTree::Literal& node) override;
    virtual void visit(SyntaxTree::ReturnStmt& node) override;
    virtual void visit(SyntaxTree::VarDef& node) override;
    virtual void visit(SyntaxTree::AssignStmt& node) override;
    virtual void visit(SyntaxTree::FuncCallStmt& node) override;
    virtual void visit(SyntaxTree::BlockStmt& node) override;
    virtual void visit(SyntaxTree::EmptyStmt& node) override;
    virtual void visit(SyntaxTree::ExprStmt& node) override;
    virtual void visit(SyntaxTree::FuncParam& node) override;
    virtual void visit(SyntaxTree::FuncFParamList& node) override;
    virtual void visit(SyntaxTree::BinaryCondExpr& node) override;
    virtual void visit(SyntaxTree::UnaryCondExpr& node) override;
    virtual void visit(SyntaxTree::IfStmt& node) override;
    virtual void visit(SyntaxTree::WhileStmt& node) override;
    virtual void visit(SyntaxTree::BreakStmt& node) override;
    virtual void visit(SyntaxTree::ContinueStmt& node) override;
    virtual void visit(SyntaxTree::InitVal& node) override;

   private:
    ErrorReporter& err;
    bool Expr_int;
    bool is_func_block; //是否为函数定义的block
    int Expr_value;         //表达式的值
    enum parser_type{array,func,var};//语法结构类型：数组、函数或变量
    //符号表的每一个表项
    struct Entry
    {
        enum parser_type parsertype;           //语法结构定义
        SyntaxTree::Type type;                 //若为变量/返回值，则定义类型
        int array_length;                      //若为数组则定义长度
        std::vector<int> arlen_value;          //数组的每一维长度
        std::vector<struct Entry> func_params; //若为函数则建立函数内的符号表
    };
    struct Entry current_type; //当前的type
    //list中每一项是一个作用域的符号表，作用域之间为嵌套关系
    std::list<std::map<std::string, struct Entry>> table;
    //在符号表中寻找名为name的元素
    struct Entry *Lookup(std::string name)
    {
        for (auto i = table.begin(); i != table.end(); i++)
        {
            auto iter = i->find(name);
            if (iter != i->end())
                return &(iter->second);
        }
        return NULL;
    }

    //将变量插入符号表
    bool Insert_Var(SyntaxTree::VarDef &node)
    {
        struct Entry temp;
        temp.type = node.btype;
        if (node.array_length.size() > 0)//数组长度不为0说明是数组
        {
            temp.parsertype = array;
            for (auto exp : node.array_length)
            {
                exp->accept(*this);
                temp.arlen_value.push_back(Expr_value);
            }
        }
        else
            temp.parsertype = var;
        //插入到当前作用域的符号表
        auto result = table.front().insert(std::pair<std::string, struct Entry>(node.name, temp));
        return result.second;
    }

    //将函数插入符号表
    bool Insert_Func(SyntaxTree::FuncDef &node)
    {
        //函数为一个作用域
        std::map<std::string, struct Entry> new_table;

        struct Entry temp;
        temp.parsertype=func;
        temp.type = node.ret_type;
        if (node.param_list != NULL)
        {
            for (auto param : (node.param_list)->params)//遍历参数
            {
                
                struct Entry p;
                p.type = param->param_type;
                
                if (param->array_index.size() > 0)//数组
                {
                    p.parsertype = array;
                    for (auto exp : param->array_index)
                    {
                        exp->accept(*this);
                        p.arlen_value.push_back(Expr_value);//加入表达式值
                    }
                }
                else
                    p.parsertype = var;
                //插入到函数参数列表和作用域符号表
                temp.func_params.push_back(p);
                auto result = new_table.insert(std::pair<std::string, struct Entry>(param->name, p));
                //函数形参重复定义
                if (!result.second)
                {
                    err.error(node.loc, "Redefined Variable");
                    exit(int(ErrorType::VarDuplicated));
                }
            }
        }
        //将函数符号表插入到函数所在作用域的符号表
        auto result = table.front().insert(std::pair<std::string, struct Entry>(node.name, temp));
        
        //插入成功（函数重定义）
        if (result.second)
        {
            table.push_front(new_table);
            return true;
        }
        else
            return false;
    }

    //在当前作用域内创建符号表
    void make_new_table()
    {
        std::map<std::string, struct Entry> new_table;
        table.push_front(new_table);
        return;
    }

    //在当前作用域内删除符号表
    void delete_table()
    {
        table.pop_front();
        return;
    }
};



#endif  // _SYSYF_SYNTAX_TREE_CHECKER_H_