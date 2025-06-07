#ifndef MOD_FUNC_H
#define MOD_FUNC_H

typedef enum {INT_PAR, PTR_PAR} ValueType;
typedef enum {PARAM, FIX, IND} ValueOrigin;

typedef struct {
  ValueType    tipo_val;  /* indica o tipo do parametro (inteiro ou ponteiro) */
  ValueOrigin  orig_val;  /* indica a origem do valor do parametro */
  union {
    int v_int;
    void* v_ptr;
  } valor;         /* define o valor ou endereço do valor do parametro (quando amarrado/indireto) */
} ParamDescription;

void cria_func (void* f, ParamDescription params[], int paramsCount, unsigned char codigo[]);

#endif

