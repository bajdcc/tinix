#include <tinix.h>

#define MaxSize 99

void translate(char str[],char exp[])
{
    struct
    {
        char data[MaxSize];
        int top;    
    }op;    
    char ch;                     
    int i = 0,t = 0;
    op.top = -1;
    ch = str[i];    
    i++;
    while(ch != '\0')   
    {
        switch(ch)
        {
        case '(': 
            op.top++;op.data[op.top]=ch;
            break;
        case ')':               
            while(op.data[op.top] != '(')    
            { 
                exp[t]=op.data[op.top];
                op.top--;
                t++;
            }
            op.top--;
            break;
        case '+':
        case '-':
            while(op.top != -1&&op.data[op.top] != '(')
            {
                exp[t] = op.data[op.top];
                op.top--;
                t++;
            }
            op.top++; 
            op.data[op.top] = ch;
            break;
        case '*':
        case '/':
            while(op.top == '/'||op.top == '*')
            {
                exp[t] = op.data[op.top];
                op.top--;
                t++;
            }
            op.top++;
            op.data[op.top] = ch;
            break;
        case ' ':
             break;
        default:
            while(ch >= '0'&&ch <= '9')
            {
                exp[t] = ch;t++;
                ch = str[i];i++;
            }
            i--;
            exp[t] = '|';
            t++;
        }
        ch = str[i];
        i++;
    }
    while(op.top != -1)
    {
        exp[t] = op.data[op.top];
        t++;
        op.top--;
    }
    exp[t] = '\0';
}
float cal_value(char exp[])
{
    struct
    {
        float data[MaxSize];
        int top;
    }st;
    float d;
    char ch;
    int t = 0;
    st.top = -1;
    ch = exp[t];
    t++;
    while(ch != '\0')
    {
        switch(ch) 
        {
        case '+':
            st.data[st.top-1] = st.data[st.top-1]+st.data[st.top];
            st.top--;
            break;
        case '-':
            st.data[st.top-1] = st.data[st.top-1]-st.data[st.top];
            st.top--;
            break;
        case '*':
            st.data[st.top-1] = st.data[st.top-1]*st.data[st.top];
            st.top--;
            break;
        case '/':
            if(st.data[st.top] != 0)
                st.data[st.top-1]=st.data[st.top-1]/st.data[st.top];
            else
            {
                printf("\ndiv zero error!\n");
            }
            st.top--;
            break;
        default:
            d=0;
            while(ch >= '0'&&ch <= '9')
            {
                d = 10*d+ch-'0';
                ch = exp[t];
                t++;
            }
            st.top++;
            st.data[st.top] = d;
        }
        ch = exp[t];
        t++;
    }
    return st.data[st.top];
}


void print_float(float x)
{
    long i = (long)(x*1000);
    printf("%ld.%03ld\n", (long)(i/1000), (long)(i%1000));
}

int tinix_main()
{
    char str[MaxSize],exp[MaxSize];
    printf("Input  expression: ");
    scanf("%s", str);
    translate(str, exp); 
    printf("Suffix expression: %s\n", exp);
    printf("Result is: ");
    print_float(cal_value(exp));
    sys_exit();
    return 0;
}
