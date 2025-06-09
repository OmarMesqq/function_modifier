# [Dynamic function modifier](https://www.inf.puc-rio.br/~inf1018/2024.2/trabs/t2/trab2-20242.html)

implementar uma função em C `gera_func` que recebe o endereço de uma função original e uma descrição de seus parâmetros.
Tal descrição permite "amarrar" valores específicos aos argumentos da função original, de forma que se pode reduzir a quantidade
de parâmetros passados e dar maior flexibilidade de uso à função gerada.

Para isso, será necessário gerar, em runtime, o código de máquina correspodente à nova função, copiá-la
para uma região da pilha da memória e, enfim, executá-lo.

Devido a políticas de segurança de sistemas operacionais modernos, será necessário compilar o projeto com uma flag
adicional no `gcc` que permite que código localizado na pilha seja executável:

```bash
gcc -g -Wall -Wextra -no-pie -Wa,--execstack run_tests.c mod_func.c -o func_modifier
```

O protótipo da função é:
```c
void mod_func(void* f, ParamDescription params[], int paramsCount, unsigned char codigo[]);
```
onde `f` tem o endereço da função original a ser chamada pelo código gerado, o array `params`
contém a descrição dos parâmetros para chamar essa função, `paramsCount` é o número de parâmetros descritos por params (1 <= paramsCount <= 3) 
e `codigo` é um vetor onde deverá ser gravado o código gerado.

O tipo `ParamDescription` é definido da seguinte forma:

```c
typedef enum {INT_PAR, PTR_PAR} ValueType;
typedef enum {PARAM, FIX, IND} ValueOrigin;

typedef struct {
  ValueType    value_type;  /* indica o tipo do parametro (inteiro ou ponteiro) */
  ValueOrigin  value_origin;  /* indica a origem do valor do parametro */
  union {
    int v_int;
    void* v_ptr;
  } valor;         /* define o valor ou endereço do valor do parametro (quando amarrado/indireto) */
} ParamDescription;

```

O campo value_origin indica se o parâmetro deve ser "amarrado" ou não; ele pode conter os seguintes valores:

- `PARAM`: o parâmetro não é amarrado, ou seja, deve ser recebido pela nova função e repassado à função origin.
- `FIX`: o parâmetro deve ser amarrado a um valor constante, fornecido no campo valor.
- `IND`: o parâmetro deve ser amarrado a uma variável, cujo endereço é fornecido no campo valor. Isto é, deve ser passado à função original o valor corrente dessa variável.

