#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "mod_func.h"

int mult(int x, int y) {
  return x * y;
}

void roda_testes_0();
void roda_testes_1();
void roda_testes_2();
void roda_testes_3();
void roda_testes_4();

int main() {
  roda_testes_1();
  printf("teste 1 passou!\n");
  roda_testes_2();
  printf("teste 2 passou!\n");
  roda_testes_3();
  printf("teste 3 passou!\n");
  //roda_testes_4();
  //printf("teste 4 passou!\n");

  return 0;
}


void roda_testes_1() {
  typedef int (*func_ptr) (int x);

  ParamDescription params[2];
  func_ptr f_mult;
  int i;
  unsigned char codigo[500];

  params[0].tipo_val = INT_PAR; /* o primeiro parãmetro de mult é int */
  params[0].orig_val = PARAM;   /* a nova função repassa seu parämetro */

  params[1].tipo_val = INT_PAR; /* o segundo parâmetro de mult é int */
  params[1].orig_val = FIX;     /* a nova função passa para mult a constante 10 */
  params[1].valor.v_int = 10;

  mod_func (mult, params, 2, codigo);
  // faz type cast para "converter" vetor com instruções em um tipo "chamável" (função)
  f_mult = (func_ptr) codigo;   

  for (i = 1; i <=10; i++) {
    int res =  f_mult(i);
    printf("%d\n", res); /* a nova função só recebe um argumento */
    assert(res == i*10);
  }
}

void roda_testes_2() {
  typedef int (*func_ptr) ();

  ParamDescription params[2];
  func_ptr f_mult;
  int i;
  unsigned char codigo[500];

  params[0].tipo_val = INT_PAR; /* a nova função passa para mult um valor inteiro */
  params[0].orig_val = IND;     /* que é o valor corrente da variavel i */
  params[0].valor.v_ptr = &i;

  params[1].tipo_val = INT_PAR; /* o segundo argumento passado para mult é a constante 10 */
  params[1].orig_val = FIX;
  params[1].valor.v_int = 10;

  mod_func (mult, params, 2, codigo);
  f_mult = (func_ptr) codigo;
  
  for (i = 1; i <=10; i++) {
    int res = f_mult();
    printf("%d\n", res); /* a nova função não recebe argumentos */
    assert(res == i*10);
  }
}

void roda_testes_3() {
  typedef int (*func_ptr) (void* candidata, size_t n);

  char fixa[] = "quero saber se a outra string é um prefixo dessa";
  ParamDescription params[3];
  func_ptr mesmo_prefixo;
  char s[] = "quero saber tudo";
  int tam;
  unsigned char codigo[500];

  params[0].tipo_val = PTR_PAR; /* o primeiro parâmetro de memcmp é um ponteiro para char */
  params[0].orig_val = FIX;     /* a nova função passa para memcmp o endereço da string "fixa" */
  params[0].valor.v_ptr = fixa;

  params[1].tipo_val = PTR_PAR; /* o segundo parâmetro de memcmp é também um ponteiro para char */
  params[1].orig_val = PARAM;   /* a nova função recebe esse ponteiro e repassa para memcmp */

  params[2].tipo_val = INT_PAR; /* o terceiro parâmetro de memcmp é um inteiro */
  params[2].orig_val = PARAM;   /* a nova função recebe esse inteiro e repassa para memcmp */

  mod_func (memcmp, params, 3, codigo);
  mesmo_prefixo = (func_ptr) codigo;

  tam = 12;
  printf ("'%s' tem mesmo prefixo-%d de '%s'? %s\n", s, tam, fixa, mesmo_prefixo (s, tam) ? "NAO" : "SIM");
  tam = strlen(s);
  printf ("'%s' tem mesmo prefixo-%d de '%s'? %s\n", s, tam, fixa, mesmo_prefixo (s, tam) ? "NAO" : "SIM");
}

void roda_testes_4() {
    //TODO: passar este teste
    assert(0 == 1);
    printf("teste 4 passou!\n");
}
