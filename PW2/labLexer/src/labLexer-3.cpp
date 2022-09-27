#include <iostream>

#include "antlr4-runtime.h"
#include "relop.h"
using namespace antlr4;
using namespace std;
void print_other(int* p)
{
    if(*p!=0)
    {
        cout<<"(other,"<<*p<<")";
        *p=0;
    }
}
int main(int argc, const char* argv[])
{
    char m[100];
    std::cin.getline(m,100,'\n');
    ANTLRInputStream input(m);
    relop lexer(&input);
    CommonTokenStream tokens(&lexer);

    tokens.fill();
    int other_number=0,*p,c;
    p=&other_number;
    for (auto token : tokens.getTokens()) {
        c=token->getType();
        //cout<<c;
        switch(c)
        {
        case 1:
            print_other(p);
            cout<<"(relop,<)";
            break;
        case 2:
            print_other(p);
            cout<<"(relop,<=)";
            break;
        case 3:
            print_other(p);
            cout<<"(relop,=)";
            break;
        case 4:
            print_other(p);
            cout<<"(relop,<>)";
            break;
        case 5:
            print_other(p);
            cout<<"(relop,>)";
            break;
        case 6:
            print_other(p);
            cout<<"(relop,>=)";
            break;
        case 7:
            *p+=1;
            break;
        case 8:
            print_other(p);
            break;
        case -1://EOF=-1
            print_other(p);
            break;
        default:
            break;
        }
    }

    return 0;
}