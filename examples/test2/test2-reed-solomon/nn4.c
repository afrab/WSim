/* I found these sources somewhere on the net.
   I use it for my remote control stuff.
   It is a good example for debugger as well. 
   2002 diwil@...
   */
/* $Id: nn4.c,v 1.1 2006-05-19 11:06:12 afraboul Exp $ */

// #include <stdio.h>
// #include <stdlib.h>
#include "nn4.h"

static const gf Pp[5] = { 1, 1, 0, 0, 1 };
static const gf Alpha_to[16] = { 1,2,4,8,3,6,12,11,5,10,7,14,15,13,9,0 };
static const gf Index_of[16] = { 15,0,1,4,2,8,5,10,3,14,9,7,6,13,11,12 };
static const gf Gg[6] = { 0,1,5,2,7,0 };


int __zz;

#ifdef TEST_SOURCE
int L__main()
{
	int res;
	dtype data[] = {1,2,3,15,5,15,11,15,15,15,0,0,0,0,0,0};
	dtype erasepos[5];
	
	
	/* Encode gata in GF(15,10) */
	encode_rs(&data[0],&data[10]);
	
	erasepos[0] = 5;
	data[0] = 3;
	data[11]= 0;
	data[5] = 0;
	/* Rstore data */
	res = eras_dec_rs(data, erasepos, 1);
	
	return 0;
}


int main()
{
	return L__main();
}

#endif


static inline unsigned int
modnn(gf x)
{
   return x%15;
}


gf
encode_rs(dtype *d, dtype *bb)
{
   register int i, j;
   gf feedback;
   dtype *data = d;

   bb[0] = 0;
   bb[1] = 0;                     
   bb[2] = 0;                     
   bb[3] = 0;                     
   bb[4] = 0;                     
   
   for (i = 9; i >= 0; i--)
   {
      if(data[i] > 15 ) return -1;	///////////////////////

      feedback = Index_of[data[i] ^ bb[4]];
      if(feedback != 15)
      {
         for (j = 4; j > 0; j--)
	 {
            if (Gg[j] != 15 )
               bb[j] = bb[j - 1] ^ Alpha_to[(unsigned int)modnn(Gg[j] + feedback)];
            else
               bb[j] = bb[j - 1];
	 }
         bb[0] = Alpha_to[(unsigned int)modnn(feedback)];
      }
      else
      {
         for (j = 4; j > 0; j--)
            bb[j] = bb[j - 1];
         bb[0] = 0;
      }
   }
   return 0;
}



gf
eras_dec_rs(dtype *data1, dtype *eras_pos, gf no_eras)
{
   gf deg_lambda, el, deg_omega;
   gf r;
   dtype *data = data1;
   
   register int i, j, tmp, count;
   
   dtype u,q,num1,num2,den,discr_r;
   dtype recd[15];
   dtype lambda[6], s[6], b[6], t[6], omega[6];
   dtype root[5 ], reg[6], loc[5];
   gf syn_error;


   for (i = 14; i >= 0; i--)
   {
      if(data[i] > 15 )   return -1; ///////////////////////////////////////
      recd[i] = Index_of[data[i]];
   }


   syn_error = 0;
   for (i = 1; i <= 5 ; i++)
   {
      tmp = 0;
      for (j = 0; j < 15 ; j++) if (recd[j] != 15 )  tmp ^= Alpha_to[modnn(recd[j] + i*j)];
      syn_error |= tmp;
      s[i] = Index_of[tmp];
   }
   
   if (!syn_error) return 0;


   for(i=5;i>0;i--) lambda[i] = 0;
   lambda[0] = 1;

   if (no_eras > 0)
   {
      lambda[1] = Alpha_to[eras_pos[0]];
      for (i = 1; i < no_eras; i++)
      {
         u = eras_pos[i];
         for (j = i+1; j > 0; j--)
         {
            tmp = Index_of[lambda[j - 1]];
            if(tmp != 15 )
               lambda[j] ^= Alpha_to[modnn(u + tmp)];
         }
      }
   }

   for(i=0; i< 15 - 10 + 1; i++) b[i] = Index_of[lambda[i]];
   
   r = no_eras;
   el = no_eras;
   
   while (++r <= 5 )
   {
      discr_r = 0;
      for (i = 0; i < r; i++)
      {
         if ((lambda[i] != 0) && (s[r - i] != 15 ))
            discr_r ^= Alpha_to[modnn(Index_of[lambda[i]] + s[r - i])];
      }

      discr_r = Index_of[(dtype)discr_r];
      if (discr_r == 15 )
      {

         for(i=4;i>=0;i--) (&b[1])[i] = b[i];
         b[0] = 15;
      }
      else
      {
         t[0] = lambda[0];
         for (i = 0 ; i < 5; i++)
         {
            if(b[i] != 15  )
               t[i+1] = lambda[i+1] ^ Alpha_to[modnn(discr_r + b[i])];
            else
               t[i+1] = lambda[i+1];
         }
         if (2 * el <= r + no_eras - 1)
         {
            el = r + no_eras - el;
            for (i = 0; i <= 5 ; i++)
            {
                b[i] = (lambda[i] == 0) ? 15   : modnn(Index_of[lambda[i]] - discr_r + 15 );
            /*	
            	if(lambda[i] == 0)
            		b[i] = 15;
            	else
            	{
            		b[i] =  modnn(Index_of[lambda[i]] - discr_r + 15 );
            	}
            */
            }	
          __zz = 0;
         }
         else
         {

            for(i=4;i>=0;i--) (&b[1])[i] = b[i];
            b[0] = 15  ;
         }
         for(i=5;i >=0;i--) lambda[i]=t[i];
      }
   }


   deg_lambda = 0;
   for(i=0;i<6;i++)
   {
      lambda[i] = Index_of[lambda[i]];
      if(lambda[i] != 15) deg_lambda = i;
   }

   for(i=5;i >0;i--) reg[i] = lambda[i];

   count = 0;
   for (i = 1; i <= 15 ; i++)
   {
      q = 1;
      for (j = deg_lambda; j > 0; j--)
         if (reg[j] != 15 )
         {
            reg[j] = modnn(reg[j] + j);
            q ^= Alpha_to[reg[j]];
         }
      if (!q)
      {
         root[count] = i;
         loc[count] = 15  - i;
         count++;
      }
   }

   if (deg_lambda != count)
   {
      return -1;
   }

   deg_omega = 0;
   for (i = 0; i < 5;i++)
   {
      tmp = 0;
      j = (deg_lambda < i) ? deg_lambda : i;
      for(;j >= 0; j--)
      {
         if ((s[i + 1 - j] != 15 ) && (lambda[j] != 15))
            tmp ^= Alpha_to[modnn(s[i + 1 - j] + lambda[j])];
      }
      if(tmp != 0)  deg_omega = i;
      omega[i] = Index_of[tmp];
   }
   omega[5] = 5;

   for (j = count-1; j >=0; j--)
   {
      num1 = 0;
      for (i = deg_omega; i >= 0; i--)
      {
         if (omega[i] != 15 )
            num1  ^= Alpha_to[modnn(omega[i] + i * root[j])];
      }
      num2 = Alpha_to[0];
      den = 0;

      i = ( deg_lambda<4 ? deg_lambda : 4)  & ~1;
      
      for (; i >= 0; i -=2)
      {
         if(lambda[i+1] != 15 ) den ^= Alpha_to[modnn(lambda[i+1] + i * root[j])];
      }
      
      if (den == 0) return -1;

      if (num1 != 0)
      {
         data[loc[j]] ^= Alpha_to[modnn(Index_of[num1] + Index_of[num2] + 15  - Index_of[den])];
      }
   }
   return count;
}
