#include "mod_func.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PROLOG_SIZE 19 
#define EPILOG_SIZE 6
#define BODY_SIZE 39
#define FUNCTION_CALL_SIZE 12   // 2 para movabs, 8 para endereço e 2 para call

static void constroi_instrucao_mov_32_bits(char* buf, int order, int valor);
static void constroi_instrucao_mov_64_bits(char* buf, int order, void* valor);
static void constroi_instrucao_desref_ptr_para_int(char* buf, int order);
static void constroi_instrucao_desref_ptr_para_ptr(char* buf, int order);
static int conta_quantidade_de_params_amarrados(ParamDescription params[], int n);


/**
 *
 *
 */
void mod_func(void* f, ParamDescription params[], int paramsCount, unsigned char codigo[]) {
    if (paramsCount < 1 || paramsCount > 3) {
        printf("O número mínimo de parâmetros é 1, e o máximo é 3!\n");
        return;
    }

    char prolog[PROLOG_SIZE] = {
        /* Equivalente a pushq %rbp */
        0x48, 0x83, 0xec, 0x08, // subq $8, %rsp
        0x48, 0x89, 0x2c, 0x24, // movq %rbp, (%rsp)
        
        0x48, 0x89, 0xe5, // mov %rsp, %rbp
    
        /* Cria espaço para registradores callee-saved e eventuais variáveis locais */
        0x48, 0x83, 0xec, 0x08, // subq $8, %rsp
        0x48, 0x89, 0x1c, 0x24 // movq %rbx, (%rsp)
    };

    char* body = malloc(BODY_SIZE * sizeof(char));
    if (!body) {
        printf("Erro de memória!\n");
        return;
    }

    int offset = 0;
    int params_amarrados = conta_quantidade_de_params_amarrados(params, paramsCount);
    
    /**
     * Quando a função original tiver exatamente 2 parâmetros e amarra apenas um, em particular,
     * o primeiro, temos um caso especial.
     * Mapeia-se o valor do primeiro argumento (rdi) da função gerada 
     * para o segundo argumento (rsi) da função original
     */
    if (paramsCount == 2 && params_amarrados == 1 && (params[0].value_origin == FIX || params[0].value_origin == IND)) {
        char caso_especial_de_localizacao_de_registrador[3] = {
            // movq %rdi, %rsi
            0x48, 0x89, 0xfe  
        };

        for (int j = 0; j < sizeof(caso_especial_de_localizacao_de_registrador); j++) {
            body[offset + j] = caso_especial_de_localizacao_de_registrador[j];
        }
        offset += sizeof(caso_especial_de_localizacao_de_registrador);
        
    } 
    /**
     * Quando a função original tem exatamente 3 parâmetros, há mais 4 casos especiais.
     */
    else if (paramsCount == 3) {
        if (params_amarrados == 1) {
            if (params[0].value_origin == FIX || params[0].value_origin == IND) {
                /**
                 * 1) Um parâmetro é amarrado, em especial, o primeiro. O mapeamento
                 * feito consiste em salvar temporariamente o segundo argumento da função gerada (rsi),
                 * copiar o primeiro argumento da gerada (rdi) no segundo argumento da original (rsi) e,
                 * por fim, colocar o valor salvo do segundo argumento da gerada no terceiro argumento (rdx) da original
                 */
                char caso_especial_de_localizacao_de_registrador[9] = {
                    // movq %rsi, %rbx
                    0x48, 0x89, 0xf3,
                    // movq %rdi, %rsi
                    0x48, 0x89, 0xfe,
                    // movq %rbx, %rdx
                    0x48, 0x89, 0xda 
                };

                for (int j = 0; j < sizeof(caso_especial_de_localizacao_de_registrador); j++) {
                    body[offset + j] = caso_especial_de_localizacao_de_registrador[j];
                }
                offset += sizeof(caso_especial_de_localizacao_de_registrador);
            } else if (params[1].value_origin == FIX || params[1].value_origin == IND) {
                /**
                 * 2) Um parâmetro é amarrado, em especial, o segundo.
                 * O mapeamento feito é do segundo argumento da gerada para o terceiro da original.
                 */
                char caso_especial_de_localizacao_de_registrador[3] = {
                    // movq %rsi, %rdx
                    0x48, 0x89, 0xf2 
                };

                for (int j = 0; j < sizeof(caso_especial_de_localizacao_de_registrador); j++) {
                    body[offset + j] = caso_especial_de_localizacao_de_registrador[j];
                }
                offset += sizeof(caso_especial_de_localizacao_de_registrador);
            }
        } else if (params_amarrados == 2) {
            if ((params[0].value_origin == FIX || params[0].value_origin == IND) && (params[1].value_origin == FIX || params[1].value_origin == IND)) {
                /**
                 * 3) Dois parâmetros são amarrados, em especial, os dois primeiros.
                 * Nesse caso, mapeia-se o primeiro argumento da gerada para o terceiro da original.
                 */
                char caso_especial_de_localizacao_de_registrador[3] = {
                    // movq %rdi, %rdx
                    0x48, 0x89, 0xfa
                };

                for (int j = 0; j < sizeof(caso_especial_de_localizacao_de_registrador); j++) {
                    body[offset + j] = caso_especial_de_localizacao_de_registrador[j];
                }
                offset += sizeof(caso_especial_de_localizacao_de_registrador);
            } else if ((params[0].value_origin == FIX || params[0].value_origin == IND) && (params[2].value_origin == FIX || params[2].value_origin == IND)) {
                /**
                 * 4) Dois parâmetros são amarrados, em especial, o primeiro e o último.
                 * Nesse caso, mapeia-se o primeiro argumento da gerada para o segundo da original.
                 */
                char caso_especial_de_localizacao_de_registrador[3] = {
                    // movq %rdi, %rsi
                    0x48, 0x89, 0xfe
                };
                for (int j = 0; j < sizeof(caso_especial_de_localizacao_de_registrador); j++) {
                    body[offset + j] = caso_especial_de_localizacao_de_registrador[j];
                }
                offset += sizeof(caso_especial_de_localizacao_de_registrador);
            }
        }
    };

    
    for (int i = 0; i < paramsCount; i++) {
        ParamDescription param = params[i];
        ValueType tp = param.value_type;
        ValueOrigin ov = param.value_origin;

        if (ov == PARAM) {
            continue;
        } else if (ov == FIX) {
            if (tp == INT_PAR) {
                int valor_a_guardar = param.value.v_int;
                char instruction[5];

                constroi_instrucao_mov_32_bits(instruction, i, valor_a_guardar);
                for (int j = 0; j < sizeof(instruction); j++) {
                    body[offset + j] = instruction[j];
                }
                offset += sizeof(instruction);
            } else if (tp == PTR_PAR) {
                void* ptr_a_guardar = param.value.v_ptr;
                char instruction[10];

                constroi_instrucao_mov_64_bits(instruction, i, ptr_a_guardar);
                for (int j = 0; j < sizeof(instruction); j++) {
                    body[offset + j] = instruction[j];
                }
                offset += sizeof(instruction);
            } else {
                printf("Erro inesperado!\n");
                return;
            }
        } else if (ov == IND) {
            if (tp == INT_PAR) {
                int* ptr_para_valor_a_guardar = (int*) param.value.v_ptr;
                char mov_instruction[10];
                char desref_instruction[2];

                if (!ptr_para_valor_a_guardar) {
                    printf("Ponteiro IND NULL!\n");
                    return;
                }
                
                constroi_instrucao_mov_64_bits(mov_instruction, i, ptr_para_valor_a_guardar);
                for (int j = 0; j < sizeof(mov_instruction); j++) {
                    body[offset + j] = mov_instruction[j];
                }
                offset += sizeof(mov_instruction);

                constroi_instrucao_desref_ptr_para_int(desref_instruction, i);
                for (int j = 0; j < sizeof(desref_instruction); j++) {
                    body[offset + j] = desref_instruction[j];
                }
                offset += sizeof(desref_instruction);
            } else if (tp == PTR_PAR) {
                void* ptr_a_guardar = param.value.v_ptr;
                char instruction[10];
                char desref_instruction[3];

                constroi_instrucao_mov_64_bits(instruction, i, ptr_a_guardar);
                for (int j = 0; j < sizeof(instruction); j++) {
                    body[offset + j] = instruction[j];
                }
                offset += sizeof(instruction);

                constroi_instrucao_desref_ptr_para_ptr(desref_instruction, i);
                for (int j = 0; j < sizeof(desref_instruction); j++) {
                    body[offset + j] = desref_instruction[j];
                }
                offset += sizeof(desref_instruction);
            } else {
                printf("Erro inesperado!\n");
                return;
            }
        } else {
            printf("Erro inesperado!\n");
            return;
        }
    }

    uintptr_t original_address = (uintptr_t) f;

    char original_function_call[FUNCTION_CALL_SIZE] = {
        // movabs f, %rax
        0x48, 0xb8,

        (original_address & 0xFF),           
        (original_address >> 8) & 0xFF,      
        (original_address >> 16) & 0xFF,     
        (original_address >> 24) & 0xFF,     
        (original_address >> 32) & 0xFF,     
        (original_address >> 40) & 0xFF,     
        (original_address >> 48) & 0xFF,     
        (original_address >> 56) & 0xFF,     

        //TODO: faz call indireto para permitir endereços dinâmicos - call *%rax
        0xff, 0xd0
    };
    
    // Desfazendo RA + restaurando valores + repassando controle para chamadora
    char epilog[EPILOG_SIZE] = {
        // Equivalente a popq %rbx
        0x48, 0x8b, 0x1c, 0x24, // movq (%rsp), %rbx 
        // leave
        0xc9,
        // ret
        0xc3
    };
    
    memcpy(codigo, prolog, PROLOG_SIZE);
    memcpy(codigo + PROLOG_SIZE, body, offset);
    memcpy(codigo + PROLOG_SIZE + offset, original_function_call, FUNCTION_CALL_SIZE);
    memcpy(codigo + PROLOG_SIZE + offset + FUNCTION_CALL_SIZE, epilog, EPILOG_SIZE);

    free(body);
}

static void constroi_instrucao_mov_32_bits(char* buf, int order, int valor) {
    char mov_registers_32[3] = { 
        0xbf, // edi
        0xbe, // esi
        0xba,  // edx
    };

    // Configura a instrução movl para o devido registrador
    buf[0] = mov_registers_32[order];

    // Guarda os bytes do inteiro. Assume sistema LE
    buf[1] = valor & 0xFF;           // Byte 0 (byte menos significativo)
    buf[2] = (valor >> 8) & 0xFF;    // Byte 1
    buf[3] = (valor >> 16) & 0xFF;   // Byte 2
    buf[4] = (valor >> 24) & 0xFF;   // Byte 3 (byte mais significativo)
}

static void constroi_instrucao_mov_64_bits(char* buf, int order, void* valor) {
    char mov_registers_64[3] = {
        0xbf, // movabs $ptr, %rdi
        0xbe, // movabs $ptr, %rsi
        0xba  // movabs $ptr, %rdx
    };

    // Configura a instrução movabs para o devido registrador
    // Guarda o ptr no registrador da devida ordem
    buf[0] = 0x48;
    buf[1] = mov_registers_64[order];

    // Armazena endereço no buffer. Assume LE.
    uintptr_t address = (uintptr_t)valor;
    buf[2] = address & 0xFF;           // Byte 0 (byte menos significativo)
    buf[3] = (address >> 8) & 0xFF;    // Byte 1
    buf[4] = (address >> 16) & 0xFF;   // Byte 2
    buf[5] = (address >> 24) & 0xFF;   // Byte 3
    buf[6] = (address >> 32) & 0xFF;   // Byte 4
    buf[7] = (address >> 40) & 0xFF;   // Byte 5
    buf[8] = (address >> 48) & 0xFF;   // Byte 6
    buf[9] = (address >> 56) & 0xFF;   // Byte 7 (byte mais significativo)
}

static void constroi_instrucao_desref_ptr_para_int(char* buf, int order) {
    char desref_opcodes_32_bits[3][2] = {
        {0x8b, 0x3f}, // movl (%rdi), %edi
        {0x8b, 0x36}, // movl (%rsi), %esi
        {0x8b, 0x12}, // movl (%rdx), %edx
    };

    // Gera código para desreferenciar o ponteiro
    // da devida ordem e guardar o valor no seu equivalente de 32 bits
    buf[0] = desref_opcodes_32_bits[order][0];
    buf[1] = desref_opcodes_32_bits[order][1];
}

static int conta_quantidade_de_params_amarrados(ParamDescription params[], int n) {
    int params_amarrados = 0;
    for (int i = 0; i < n; i++) {
        ParamDescription param = params[i];
        ValueOrigin ov = param.value_origin;
        if (ov == FIX || ov == IND) {
            params_amarrados++;
        }
    }
    return params_amarrados;
}

static void constroi_instrucao_desref_ptr_para_ptr(char* buf, int order) {
    char desref_opcodes_64_bits[3][3] = {
        {0x48, 0x8b, 0x3f}, // movq (%rdi), %rdi
        {0x48, 0x8b, 0x36}, // movq (%rsi), %rsi
        {0x48, 0x8b, 0x12}, // movq (%rdx), %rdx
    };

    // Gera código para desreferenciar o ponteiro
    // e guardar seu valor no próprio registrador
    buf[0] = desref_opcodes_64_bits[order][0];
    buf[1] = desref_opcodes_64_bits[order][1];
    buf[2] = desref_opcodes_64_bits[order][2];
}

