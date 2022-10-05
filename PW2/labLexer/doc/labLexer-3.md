### 程序设计

对于relop.g4，只定义词法，将relop六个符号与OTHER，END类似任务二一样定义，然后记好顺序。

对于labLexer-3.cpp，参考example，通过查阅官方文档，可以得知处理token时，使用getType()可以识别token的类型。

```c++
ANTLRInputStream input(m);
relop lexer(&input);
CommonTokenStream tokens(&lexer);

tokens.fill();
int other_number=0,*p,c;
p=&other_number;
for (auto token : tokens.getTokens()) {
    c=token->getType();//c的值对应relop.g4中的顺序
    switch(c)
    {
        //对每一个令牌执行对应操作，操作与任务二一样
        //case -1指EOF
    }
```

同时我们也需要打印other的函数，通过指针修改other_number的值。

```c++
void print_other(int* p)
{
    if(*p!=0)
    {
        cout<<"(other,"<<*p<<")";
        *p=0;
    }
}
```

对于CmakeLists-3.txt，参照example，将文件名修改即可。


