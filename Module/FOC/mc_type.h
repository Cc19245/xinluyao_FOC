#ifndef __MC_TYPE_H
#define __MC_TYPE_H

#include <stdint.h>
#include <stdbool.h>

/**
  * @brief Two components q, d type definition
  */
typedef struct
{
  int16_t q;
  int16_t d;
} qd_t;
/**
  * @brief Two components a,b type definition
  */
typedef struct
{
  int16_t a;
  int16_t b;
} ab_t;
/**
  * @brief Two components alpha, beta type definition
  */
typedef struct
{
  int16_t alpha;
  int16_t beta;
} alphabeta_t;


#endif

