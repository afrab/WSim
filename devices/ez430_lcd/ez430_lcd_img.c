/**
 *  \file   ez430_lcd_img.c
 *  \brief  ez430_lcd bitmap image definition
 *  \author Bernhard Dick
 *  \date   2011
 **/

#include <stdio.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "ez430_lcd_img.h"

#define BKG1 ' '

#define PM '-'
#define PM_DOT 0
#define ARROW_UP '~'
#define ARROW_DOWN '5'
#define SYM_CIRCLE 0

#define SEG_9A '+'
#define SEG_9B '%'
#define SEG_9C '1'
#define SEG_9D 'b'
#define SEG_9E '|'
#define SEG_9F ','
#define SEG_9G ']'

#define SEG_8A '@'
#define SEG_8B '&'
#define SEG_8C ':'
#define SEG_8D '9'
#define SEG_8E '2'
#define SEG_8F '\''
#define SEG_8G '^'

#define SEG_7A '#'
#define SEG_7B '*'
#define SEG_7C '<'
#define SEG_7D '0'
#define SEG_7E '3'
#define SEG_7F '>'
#define SEG_7G '/'

#define SEG_6A '$'
#define SEG_6B '='
#define SEG_6C '['
#define SEG_6D 'a'
#define SEG_6E '4'
#define SEG_6F ')'
#define SEG_6G '('

#define SYM_HEART 'e'
#define SYM_CLOCK 'd'
#define SYM_ROUND 'f'
#define SYM_BELL 'h'
#define SYM_S1 'j'
#define SYM_S2 'i'
#define SYM_S3 'g'

#define SEG_5A 'o'
#define SEG_5B 's'
#define SEG_5C 'K'
#define SEG_5D 'X'
#define SEG_5E 'S'
#define SEG_5F 'w'
#define SEG_5G 'F'

#define SEG_4A 'n'
#define SEG_4B 'r'
#define SEG_4C 'J'
#define SEG_4D 'W'
#define SEG_4E 'R'
#define SEG_4F 'v'
#define SEG_4G 'E'

#define SEG_3A 'm'
#define SEG_3B 'u'
#define SEG_3C 'Q'
#define SEG_3D 'Z'
#define SEG_3E 'P'
#define SEG_3F 'z'
#define SEG_3G 'I'

#define SEG_2A 'l'
#define SEG_2B 't'
#define SEG_2C 'O'
#define SEG_2D 'Y'
#define SEG_2E 'N'
#define SEG_2F 'y'
#define SEG_2G 'H'

#define SEG_1A 'k'
#define SEG_1B 'q'
#define SEG_1C 'M'
#define SEG_1D 'V'
#define SEG_1E 'L'
#define SEG_1F 'x'
#define SEG_1G 'D'

#define SEG_P 'p'

#define COL1 'B'
#define COL2 'C'
#define COL3 '!'

#define DP1 '`'
#define DP2 'c'
#define DP3 '.'

#define MI 'T'
#define KM 'G'
#define KCAL 'A'
#define BATTERY 'U'

#define PERCENT ';'
#define FT '{'
#define CHAR_M '6'
#define CHAR_I '}'
#define CHAR_K '_'
#define SLASH_H 'g'
#define SLASH_S '7'

#define TOTAL 0
#define AVG 0
#define MAX 0

#define NCOLORS 97


/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

struct ez430_lcd_img_t *ez430_lcd_img_create()
{
  int i, j, k, l;
  struct ez430_lcd_img_t *img;
  int ncolors, bpp;

  char segmapping[12][8] = {//LSB->HSB
    {PM, PM_DOT, ARROW_UP, ARROW_DOWN, COL1, COL3, DP3, SYM_ROUND}, //LCDMEM0
    {SEG_9F, SEG_9G, SEG_9E, SYM_HEART, SEG_9A, SEG_9B, SEG_9C, SEG_9D}, //LCDMEM1
    {SEG_8F, SEG_8G, SEG_8E, SYM_CLOCK, SEG_8A, SEG_8B, SEG_8C, SEG_8D}, //LCDMEM2
    {SEG_7F, SEG_7G, SEG_7E, SYM_BELL, SEG_7A, SEG_7B, SEG_7C, SEG_7D}, //LCDMEM3
    {COL2, SYM_CIRCLE, DP2, SYM_S1, PERCENT, FT, CHAR_K, SLASH_S}, //LCDMEM4
    {SEG_6F, SEG_6G, SEG_6E, SYM_S2, SEG_6A, SEG_6B, SEG_6C, SEG_6D}, //LCDMEM5
    {CHAR_I, CHAR_M, SLASH_H, SYM_S3, KCAL, KM, MI, BATTERY}, //LCDMEM6
    {SEG_5A, SEG_5B, SEG_5C, SEG_5D, SEG_5F, SEG_5G, SEG_5E, MAX}, //LCDMEM7
    {SEG_4A, SEG_4B, SEG_4C, SEG_4D, SEG_4F, SEG_4G, SEG_4E, DP1}, //LCDMEM8
    {SEG_3A, SEG_3B, SEG_3C, SEG_3D, SEG_3F, SEG_3G, SEG_3E, AVG}, //LCDMEM9
    {SEG_2A, SEG_2B, SEG_2C, SEG_2D, SEG_2F, SEG_2G, SEG_2E, TOTAL}, //LCDMEM10
    {SEG_1A, SEG_1B, SEG_1C, SEG_1D, SEG_1F, SEG_1G, SEG_1E, SEG_P} //LCDMEM11
  };

#include "ez430_lcd.xpm"

  img = (struct ez430_lcd_img_t*) malloc(sizeof (struct ez430_lcd_img_t));

  sscanf(ez430_lcd_xpm[0], "%d %d %d %d", &(img->w), &(img->h), &ncolors, &bpp);

  //create array [x][y][LCDMEMx]
  img->img = (uint8_t***) malloc(img->w * sizeof (uint8_t**));

  for (i = 0; i < img->w; i++) {
    img->img[i] = (uint8_t**) malloc(img->h * sizeof (uint8_t*));
    for (j = 0; j < img->h; j++) {
      img->img[i][j] = (uint8_t*) malloc(12 * sizeof (uint8_t));
    }
  }

  for (i = 0; i < img->h; i++) //y
  {
    for (j = 0; j < img->w * 2; j += 2) //x
    {
      if (ez430_lcd_xpm[i + 1 + ncolors][j] == ' ' && ez430_lcd_xpm[i + 1 + ncolors][j + 1] == '.') { //TOTAL
        img->img[j / 2][i][10] = 1 << 7;
      } else if (ez430_lcd_xpm[i + 1 + ncolors][j] == '.' && ez430_lcd_xpm[i + 1 + ncolors][j + 1] == '.') { //AVG
        img->img[j / 2][i][9] = 1 << 7;
      } else if (ez430_lcd_xpm[i + 1 + ncolors][j] == '+' && ez430_lcd_xpm[i + 1 + ncolors][j + 1] == '.') { //MAX
        img->img[j / 2][i][7] = 1 << 7;
      } else if (ez430_lcd_xpm[i + 1 + ncolors][j] == '-' && ez430_lcd_xpm[i + 1 + ncolors][j + 1] == '.') { //PM DOT
        img->img[j / 2][i][0] = 1 << 1;
      } else if (ez430_lcd_xpm[i + 1 + ncolors][j] == '*' && ez430_lcd_xpm[i + 1 + ncolors][j + 1] == '.') { //SYM CIRCLE
        img->img[j / 2][i][4] = 1 << 1;
      } else {
        for (k = 0; k < 12; k++) //LCDMEMx
        {
          img->img[j / 2][i][k] = 0;
          for (l = 0; l < 8; l++) {
            if (ez430_lcd_xpm[i + 1 + ncolors][j] == segmapping[k][l]) {
              img->img[j / 2][i][k] = 1 << l;
            }
          }
        }
      }
    }
  }
  return img;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void ez430_lcd_img_delete(struct ez430_lcd_img_t *img)
{
  int i, j;
  for (i = 0; i < img->w; i++) {
    for (j = 0; j < 12; j++) {
      free(img->img[i][j]);
    }
    free(img->img[i]);
  }
  free(img->img);
  free(img);
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void ez430_lcd_img_draw(struct ez430_lcd_img_t *img, uint8_t mem[15], uint8_t bmem[15])
{
  int i, j, k;
  int pixel;

  pixel = (img->x + img->y * machine.ui.width) * machine.ui.bpp;
  for (i = 0; i < img->h; i++) {
    int pii = pixel;
    for (j = 0; j < img->w; j++) {
      setpixel(pii, 0x00, 0x00, 0x00); // bkg
      for (k = 0; k < 12; k++) {
        if ((mem[k] & img->img[j][i][k]) != 0) {
          if ((bmem[k] & img->img[j][i][k]) != 0) {
            setpixel(pii, 0x00, 0xee, 0x00); // blink
            break;
          } else {
            setpixel(pii, 0xee, 0x00, 0x00); // on
            break;
          }
        } else if (img->img[j][i][k] > 0) {
          setpixel(pii, 0x30, 0x30, 0x30); // off
        }
      }
      pii += 3;
    }
    pixel += machine.ui.width * machine.ui.bpp;
  }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
