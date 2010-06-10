
int f(int a)
{
  if (a==0)
    return 1;
  return a*f(a-1);
}

int a;

int main(void)
{
  int b;
  a=4;
  while (1)
    {
      b=f(a);
      a=a+b;
    }
  return a;
}
