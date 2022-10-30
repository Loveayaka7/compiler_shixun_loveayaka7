#### 问题1-1

对于demoDriver.h，这个头文件主要定义了class demoDriver，定义内包含几个成员变量，同时声明了类成员函数，而类成员函数的实现位于demoDriver.cpp中。

对于main函数，实现了命令行的功能，当输入含`-h`时输出help内容，当输入含`-p`时，修改class driver属性来输出语法分析详情，输入含`-s`时，修改class属性来输出词法分析详情，输入含`-emit-ast`时将`print_ast`变为true，提示输出ast还原的代码，最后接受文件名。

对于文件名的处理由下列代码实现，可以看出代码将文件和标准输入转化为对词法分析器的输入流，从而实现对文件的词法分析。

```c++
lexer.set_debug(trace_scanning);//决定是否输出词法分析详情
// Try to open the file:
instream.open(file);
if(instream.good()) {
    lexer.switch_streams(&instream, 0);
}
else if(file == "-") {
    lexer.switch_streams(&std::cin, 0);
}
else {
    error("Cannot open file '" + file + "'.");
    exit(EXIT_FAILURE);
}
```

而词法与语法分析的核心函数及其注释如下，在main函数里使用，使用文件名作为输入，将整个文件流进行词法和语法分析。

```c++
SyntaxTree::Node* demoDriver::parse(const std::string &f)
{
    file = f;
    // lexer begin
    scan_begin();
    yy::demoParser parser(*this);
    parser.set_debug_level(trace_parsing);//决定是否输出语法分析详情
    // parser begin
    parser.parse();
    // lexer end
    scan_end();
    return this->root;
}
```

`scan_begin()`函数实现了词法分析的输入流修改，然后使用在`build/demoParser.h`中定义的class demoParser来实现语法分析。

对于词法分析，使用demo输出词法分析详情，我们可以得到：

![image-20221030020117938](C:\Users\IG JackeyLove\Desktop\image-20221030020117938.png)

查看词法分析文件demoScanner.ll，发现line为规则段的行号+1，在生成的demoScanner.cpp中可以找到有关数组，行号普遍高1。

在demoScanner.cpp的line828，有do_action段，在这个段里，首先用if语句判定输入的词法流token的类型然后输出词法分析详情，然后执行下面的代码，而这些代码均在demoParser.h中可以找到，可以发现这是对demoParser.yy中token段的定义，这样便可以将每一个词法分析的token转化为语法分析定义中的token段，从而将词法分析器与语法分析器联系起来。

```c++
//switch(yy_act)
case 1:
YY_RULE_SETUP
#line 96 "/mnt/cgshare/project/SysYF/expProject122-777/demo/grammar/demoScanner.ll"
{return yy::demoParser::make_INT(loc);}
	YY_BREAK
case 2:
YY_RULE_SETUP
#line 97 "/mnt/cgshare/project/SysYF/expProject122-777/demo/grammar/demoScanner.ll"
{return yy::demoParser::make_RETURN(loc);}
	YY_BREAK
```

对于语法分析，我们先执行输出语法分析详情：

![image-20221030030438562](C:\Users\IG JackeyLove\Desktop\image-20221030030438562.png)

去查看demoParser.cpp，line623定义的parse函数可以看得到处理的部分过程。由上图可以看出这是一个很清晰的LR分析，同时我们知道bison以LALR分析为主，所以语法分析的过程为：每次读入一个token，然后通过LALR分析表来转换各个状态进行LR分析，再构造AST语法树，接着按照语法树构造新的代码。语法树的root为begin节点，这在位于line851的switch语句的case 2可以发现。

至于main函数中打印ast还原的代码，则需要追溯到SyntaxTreePrinter.h，这个头文件里的class SyntaxTreePrinter 声明了所有的打印节点的visit函数，其实现位于SyntaxTreePrinter.cpp中，按照已经构造出的抽象语法树的结构来重新生成源代码，去掉了部分表达上的冗余，语法含义不变。

#### 问题1-2

由于SysYF语言有且仅有一个函数为main，所以直接在词法的规则定义中，改写Identifier和EOF

```c++
{Identifier}            {std::string s="main";
if(yytext==s)
{
	if(main_number_scanner==0)
	{
		main_number_scanner++;
		return yy::demoParser::make_IDENTIFIER(yytext, loc);
	}else
	{std::cout << "Too many main!" << '\n'; exit(1);}
}
return yy::demoParser::make_IDENTIFIER(yytext, loc);}

<<EOF>>                 {if(main_number_scanner==0)
    {std::cout << "no main!" << '\n'; exit(1);}
    return yy::demoParser::make_END(loc);}
```

在Declarations（声明）里加上一句话

```c++
int main_number_scanner=0;
```

在语法规则里，寻找FuncDef和Begin，改写规则

```c++
Begin: CompUnit END {
    if(main_number_parser==0)
    {std::cout << "no main!" << '\n'; exit(1);}
    $1->loc = @$;
    driver.root = $1;
    return 0;
  }
  ;

FuncDef: VOID IDENTIFIER LPARENTHESE RPARENTHESE Block{
    $$ = new SyntaxTree::FuncDef();
    $$->ret_type = SyntaxTree::Type::VOID;
    std::string s="main";
    if($2==s)
	{
		if(main_number_parser==0)
		{
			main_number_parser++;
		}else
		{std::cout << "Too many main!" << '\n'; exit(1);}
	}
    $$->name = $2;
    $$->body = SyntaxTree::Ptr<SyntaxTree::BlockStmt>($5);
    $$->loc = @$;
  }
  ;
```

然后在声明部分改写：

```
%code
{
#include "demoDriver.h"
#define yylex driver.lexer.yylex
int main_number_parser=0;
}
```

