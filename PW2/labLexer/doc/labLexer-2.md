### 程序设计

对于relop.lex，我的设计是

先使用flex语法将下面代码直接贴到生成的c文件，因为需要定义一个全局变量other_number=0。

```
%{
#include "pl0.h"
#include <stdio.h>
int other_number=0;
%}
```

再定义下面两个函数供之后调用。

```
void end()//识别到结束符号
{
	if(other_number!=0)
	{
	 printf("(other,%d)",other_number);
	}
}
void print_other()//打印other相关信息
{
	if(other_number!=0)//不等于0才可以输出
	{
	printf("(other,%d)",other_number);
	other_number=0;
	}
}
```

然后对于relop的六种符号（用#表示），我们使用下列模板，同时下面也列出了处理其他字符即“.”和 END ('\n'|'\r\n')的相关操作。

```
"#"       {
            print_other();//说明other已经结束
            printf("(relop,#)");
           }
.	   	   {
            other_number+=1;//全局变量+1
           }
{END}	   {
            end();//调用结束函数
           }
```

对于labLexer-2.c我的设计比较简单粗暴，利用ifdef语句，直接将源代码粘贴进去来达到选择编译代码。

但是经过助教提醒，一般我们使用include不会include .c文件，所以我想的初步改进的方案是在lex.yy.c中找到相关函数，在labLexer-2.c中使用，接着在makefile中链接。

makefile设计直接参考example即可，但是由于上述描述，还需要加入链接选项。
