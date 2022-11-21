[TOC]

# 实验报告

## task1

### 问题1-1

首先，查看函数`print_indent()`，该函数可以输出长度为`indent`的空格，主要用途是实现缩进，可以理解indent是打印语句是的缩进，以4个空格即一个tab为单位。

再查看各个函数的实现，有：

1.`Assembly`中包含了所有的全局定义`global_defs`，对结点中每一个定义进行访问，而且由于是全局定义，缩进为0。

2.`FuncDef`中输出结构为`type func_name(param_list){func_body}`，对每个函数的结点中的返回值类型、参数列表、函数名、函数体进行访问，其中参数列表为`FuncParamlist`，函数体为`BlockStmt`，二个子类需要单独进行visit操作。由于为全局定义，所以缩进为0；

3.`BlockStmt`先打印缩进，然后输出`{each stmt}`，里面的语句缩进加了一个tab，最后将加的缩进减回去再输出后面的”}“，对于每一个语句，都是一个子类`Stmt`，所以去visit对应的子类即可。

4.`VarDef`语句先需要打印缩进，然后输出格式为`(const) type val_name ([n]) = initializers;`，结点元素包含是否为const、类型、名称以及数组的长度、初始化信息，而且如果检测到需要初始化，还需访问`InitVal`子类来给变量赋值。

5.`InitVal`判断是否为表达式，如果是的话直接访问`Expr`子类，否则说明该语句是数组初始化，读取到数组长度后去访问每一个维度的值即可。

6.`LVal`左值表达式包含了变量名与数组索引值，索引值需要去访问`Expr`子类得到打印结果。

7.`Literal`只包含了INT和FLOAT，没有支持字符串类型，按照表达式类型直接输出对应的值。

8.`FuncCallStmt`输出格式为`func_name(param_list)`，其中参数列表的结果是访问自己结点存储的params，然后逐个访问子类`Expr`，中间用”，“隔开。

9.`FuncParam`中，由于结点元素中`array_index`在不是数组时为空指针，所以可以直接去用迭代器去访问`array_index`，如果不是数组便可以直接跳过。`FuncFParamList`为函数参数列表，在`FuncDef`中使用，由`FuncParam`组成，中间用”，“隔开。

根据上述，可以发现不同的访问者访问结点的结构均不同，较长的结构也会visit较小的子结构，结构组成比较严谨，以比较美观的格式化输出了和输入相同语义的代码，可读性也比较高。

## task2

### 思路

按照实验引导，先处理不会出现变量和函数未定义或重复定义的情况，且所有表达式均不含没有返回值的函数。

对每个变量和函数，插入对应的作用域的符号表，然后在判断的时候，给予变量和函数返回值以及其他表达式处理的时候也去修改对应的`Expr_int`，然后使用原语句判断，这样便可以实现处理函数体内部作用域中出现的情况以及变量出现的情况。

然后对于重复定义，查看部分测试样例，我们可以得知，需要处理作用域内变量重复，所需变量未定义、函数定义中形参重复等，实现方法也是查询符号表，如果已存在说明重复，未存在说明未定义。

具体实现细节如下。

#### SyntaxTreeChecker.h

1.先定义符号表，表项为

```c++
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
```

每一个表项中包含了之后所需的信息。

2.定义所需类成员函数：

对一个`node.name`，需要寻找在符号表table内是否存在以及存在时的地址。

```c++
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
```

对一个变量，我们需要将其插入符号表，变量可以为数组，返回是否插入成功，插入失败说明符号表已经有了该变量，我们在之后可以处理变量重复定义。

```c++
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
```

对于一个函数，我们也须将其插入符号表，插入过程中对函数的参数列表进行检查，对于每一个形式参数，都将其加入函数作用域所在的符号表，加入过程中如果重复则会出现`result.second`为`false`，这样我们可以发现类似`test2/04.sy`中的形参重复定义。

```c++
void Insert_Func(SyntaxTree::FuncDef &node)
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
    table.push_front(new_table);
}
```

由于函数中用到了`ErrorType`定义，所以需要将其转移到`class SyntaxTreeChecker`定义的前面。

每一个作用域需要创建符号表，作用域结束后也需要删除符号表。

```c++
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
```

#### SyntaxTreeChecker.cpp

1.对于`Assembly`，需要建立全局符号表

```c++
void SyntaxTreeChecker::visit(Assembly &node)
{
    make_new_table(); //建立全局符号表
    for (auto def : node.global_defs)
    {
        def->accept(*this);
    }
}
```

2.对于`FuncDef`，我们要将函数插入符号表，但是由于函数定义自带了BlockStmt，所以我们要将其与之后的`BlockStmt`分别，在之前的头文件中定义bool值`is_func_block`，如果是函数定义的则将其置为true，同时处理`BlockStmt`时，如果识别出前者为true，我们将其置为false，然后无须建立新符号表，但是前者为false时需要建立新符号表，最后在语句块结束后还需删除之前的符号表。

```c++
void SyntaxTreeChecker::visit(FuncDef &node)
{
    Insert_Func(node);
    is_func_block = true; 
    node.body->accept(*this);
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
```

3.对于之前的二元表达式，我们沿用之前的方法，同时修改左右两个表达式在符号表的表项。由于变量计算过程中也会有类型提升这种情况，因此我们还需要实现，如果二者有一个为FLOAT类型，则将表达式类型改为FLOAT，否则说明为INT。

```c++
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
```

4.对于左值表达式`LVal`，我们使用`Lookup()`搜索该名称是否在符号表内，如果不存在则报错变量未定义，如果在符号表内则读取类型来写入`current_type`和`Expr_int`，对于`Literal`，我们只须更新`current_type`和`Expr_int`。

```c++
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
```

5.对于变量定义`VarDef`，我们需要先将其初始化，然后再去插入符号表，插入过程中如果发现符号表已有变量名称，便返回变量重复定义错误。而变量初始化中，我们需要识别出是否是数组，如果是数组的话我们需要将其每一个值都初始化。

```c++
void SyntaxTreeChecker::visit(VarDef &node)
{
    if (node.is_inited) //先处理初始化
    {
        node.initializers->accept(*this);
    }
    if (!Insert_Var(node))
    {
        err.error(node.loc, "Redefined Variable");
        exit(int(ErrorType::VarDuplicated));
    }
}
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
```

6.对于赋值语句`AssighStmt`，访问结点，获取左右两指针是否为int，然后将整个表达式的类型标记为两个值的按位与，当二者为int时才为int，因为赋值语句可以传递同时还会出现类型提升。

```c++
void SyntaxTreeChecker::visit(AssignStmt &node)
{
    node.target->accept(*this);
    bool target_int = this->Expr_int;
    node.value->accept(*this);
    bool value_int = this->Expr_int;

    this->Expr_int = target_int & value_int;
}
```

7.函数调用`FuncCallStmt`的操作较为复杂，主要还须检查调用格式是否正确。使用`Lookup()`搜索符号表内函数信息，将其与函数调用语句的信息进行对比，查看是否有错误，具体为实参与形参的类型是否匹配符号表。最后写入函数调用的整体表达式属性也就是返回值属性`current_type`和`Expr_int`。

```c++
void SyntaxTreeChecker::visit(FuncCallStmt &node)
{

    struct Entry *type = Lookup(node.name);
    //参数类型是否匹配
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
    this->current_type.type = type->type;
    this->Expr_int = (type->type == SyntaxTree::Type::INT);
}
```

8.对于其他元素，本次不需要太多关注，参考printer中相关的用法来复刻即可。

## task3

### 思路

对于函数参数调用时，检查实际参数的个数和类型都与函数声明中的形式参数是否完全匹配这一情况，首先我们需要修改`FuncCallStmt`相关函数，来判断参数长度是否匹配，由于之前已经实现参数类型的检查，所以直接将二者结合。

对于检查函数是否未定义以及是否重复定义，检查重复定义可以在`Insert_Func`函数中加入判断，如果在将函数插入table的时候出现重复，则返回false，然后在`FuncDef`中，判定如果`Insert_Func`返回false时报错；检查未定义可以在函数调用`FuncCallStmt`时使用`Lookup()`之后判断，如果返回空指针则证明未定义。

类型提升在task2中已经实现，这里不再叙述。

具体实现如下：

#### SyntaxTreeChecker.h

1.新的`Insert_Func()`中，在最后加入了判断是否插入成功：

```c++
//将函数返回值改为bool
//...省略前面语句，将38行改为下面语句
if (result.second)
{
    table.push_front(new_table);
    return true;
}
else
   return false;
```

#### SyntaxTreeChecker.cpp

1.新的`FuncDef`中，加入判定是否插入，来确认有没有出现重定义。

```c++
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
```

2.新的`FuncCallStmt`中，加入判定参数长度与是否定义的判定，只有定义了才去判断参数长度是否匹配，长度相同才去判断每一个参数类型是否正确。

```c++
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
```

### 问题3-1

实验中遇到的困难：

1.我遇到的第一个困难是，如何定义符号表项，因为符号表项种类比较多，在看完7.2.2节之后，我思考将表项元素的类型写成一个enum，对不同的类型做不同处理，表项内关于其他类型的信息可以直接不初始化，因为二者几乎没有交集；但是由于AST语法树定义中，数组与变量比较像，所以经过阅读语法树定义源码，我发现可以将array_length长度作为识别变量与数组的方法，因为数组长度一定是大于0的，所以判断为长度大于0就是数组，否则是变量。

2.我遇到的第二个困难是函数定义自带的block与普通block发生冲突，生成了两个符号表，最开始想着两个表都存在的话多了一个选择，但是之后发现实现其他功能的时候无须使用两个表，反而在访问的时候会出现问题。经过测试，我加入了变量`is_func_block`来判定是否是函数定义使用的block，如果是的话，使用函数的符号表代替这次block的符号表来使用，同时将删除表操作放在了block处理的结尾，连同函数的符号表删除操作一起处理。

实验难点与考察倾向：

我认为这次实验的难点首先在访问者模式上，因为我之前没有接触过多少c++的知识，所以要去同时理解面向对象编程与访问者模式，包括虚函数、类继承等的知识确实有点困难。

其次难度在于对符号表操作编写总体的把握，这需要我们结合之前学习的作用域的知识来解决这里的问题，而且很多操作一环扣一环，比较难以调试错误。

考察倾向我认为主要是如何将c++访问者模式、AST语法树实现与学过的知识结合起来去解决其他的问题，这其中包含了类型检查、作用域以及一些对特殊错误的把握来调整一些细节。

### 问题3-2

处理变量声明时应该先处理初始化，因为如果初始化出现问题的话，不能将其加入符号表，比如出现了`int a=a+1;`这种情况，我们如果先将其加入符号表，则这时候赋值语句会查询到符号表内存在a的name，然后用右边的a给左边的a赋值，这时我们不知道值是什么，出现了奇怪的错误；但是如果我们先初始化而不加入符号表，初始化时赋值语句查询时发现右边的a不在符号表内，返回错误：a未定义。因此需要先初始化。

处理函数定义时，需要先将函数名加入符号表再去处理函数体，因为函数存在递归调用，但是在处理函数体递归调用时如果没有在之前将函数加入符号表，函数调用语句`FuncCallStmt`中会出现函数未定义错误，从而使得编译器不支持递归调用操作，所以先将函数名加入符号表之后，在作用域内，`FuncCallStmt`才能正常工作。因此需要先加入符号表。

### 问题3-3

由于当我们修改`Expr`中属性之后，还须修改`Expr`结构以上的所有类型值，而`Expr`几乎在所有结点类中出现，所以我们需要在许多结点的操作中均去维护这一属性，破坏了封装性，大大提高了维护与调试的难度。

而将每一个错误都使用一个类来处理，首先增加了代码的复杂度，难以维护，其次由于增加了许多的重复操作，导致检查器性能降低。在实现语义检查的过程中，有许多的检查操作环环相扣，如果要将其分开，势必造成操作的重复，这些重复完全是没有必要的，比如`FuncCallStmt`中，先去符号表中查找函数参数相关信息，来判定函数是否未定义，然后如果定义了才去判定函数参数是否正确，但是如果分开，当我们单独判定函数参数是否正确时，我们还需去符号表中查找函数参数相关信息，查找操作会造成重复，完全没有必要。

### 问题 3-4

我的思路是，将所有错误发现之后，先不要退出，在class中定义一个list来存储每一条error信息，在所有代码检查完之后，再去按照优先级来输出错误信息。或者不需要list存储，在每次发现错误之后，判定原错误和新错误的优先级，留下优先级高的那个错误，在检查结束之后输出。（类似于找最大值）

