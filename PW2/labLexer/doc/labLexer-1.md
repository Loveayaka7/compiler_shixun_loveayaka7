### 程序设计

程序使用gets函数接收标准输入流，然后先计算流输入的总长度input_size，不计算换行符长度。

接着程序使用下列代码框架遍历输入流input

```c
while(i<inputsize)
{
	c=input[i];
	//if...else if ...判定
}
```

由于我们已经计算得到input_size，所以当i=input_size-1的时候，我们便可以把该字符当作最后一个字符来做特殊处理，然后break。

判定过程中，如果为其他字符，我们将other_number++。

如果为“=”，”<“，“>”,说明other结束了，先输出（other，other_number），然后将other_number置0，再输出relop相关信息，相关信息处理如下：

如果为“=”，便输出（relop，=）

如果为“<”，我们需要判断后面一个字符是什么：如果是input的最后一个字符，输出（relop，“<”），如果是“=”，“>”，则输出对应relop，如果都不是则判断为“<”，同时other_number++。

对于“>”，对比上面只是少了relop“<>”，其他做相同操作处理即可。

在i=input_size-1然后break之后，考虑到之后会有other_number没有输出，则输出（other，other_number）。

### 遇到的问题与分析

对于标准输入”= if（a>b）a=b；“，我们可以发现程序先输出了（other，0），不符合规定输出格式。

解决方案比较简单，在所有输出（other，other_number）前面加上判定，如果other_number不为0再输出。



