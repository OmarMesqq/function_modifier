#ifndef MOD_FUNC_H
#define MOD_FUNC_H

typedef enum {INT_PAR, PTR_PAR} ValueType;
typedef enum {PARAM, FIX, IND} ValueOrigin;

typedef struct {
  ValueType    value_type;  /* indica o tipo do parametro (inteiro ou ponteiro) */
  ValueOrigin  value_origin;  /* indica a origem do valor do parametro */
  union {
    int v_int;
    void* v_ptr;
  } value;         /* define o valor ou endereço do valor do parametro (quando amarrado/indireto) */
} ParamDescription;

void mod_func(void* f, ParamDescription params[], int paramsCount, unsigned char codigo[]);

#endif

