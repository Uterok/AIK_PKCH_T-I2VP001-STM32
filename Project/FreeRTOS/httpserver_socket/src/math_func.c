#include "math_func.h"

float float_abs(float number)
{
  if(number > 0)
  {
    return number;
  }
  else if(number < 0)
  {
    return 0 - number;
  }
  else
  {
    return 0;
  }
}

long int lInt_abs(long int number)
{
  if(number > 0)
  {
    return number;
  }
  else if(number < 0)
  {
    return 0 - number;
  }
  else
  {
    return 0;
  }
}