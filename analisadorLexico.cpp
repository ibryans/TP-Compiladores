/**
 * Trabalho Prático de Compiladores
 * Parte 1 - Analisador Léxico
 * 
 * @authors Bryan Santos e Igor Reis - 02/2022
 */

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

/** Definindo e Enumerando os tokens */
#define TK_const            0
#define TK_int              1
#define TK_char             2
#define TK_while            3
#define TK_if               4
#define TK_float            5
#define TK_else             6
#define TK_and              7
#define TK_or               8
#define TK_not              9
#define TK_atrib            10
#define TK_igualdade        11
#define TK_abreParentese    12
#define TK_fechaParentese   13
#define TK_menor            14
#define TK_maior            15
#define TK_diferente        16
#define TK_maiorIgual       17
#define TK_menorIgual       18
#define TK_virgula          19
#define TK_hifen            20
#define TK_asterisco        21
#define TK_barra            22
#define TK_pontoevirgula    23
#define TK_abreChave        24
#define TK_fechaChave       25
#define TK_readln           26
#define TK_div              27
#define TK_string           28
#define TK_write            29
#define TK_writeln          30
#define TK_mod              31
#define TK_abreColchete     32
#define TK_fechaColchete    33
#define TK_true             34
#define TK_false            35
#define TK_boolean          36


/** Tabela de símbolos, armazenando identificadores e palavras reservadas da linguagem */
class Tabela_simbolos {
    public:
        /** Tabela hash que mapeia lexema para token */
        unordered_map<string, int> tab_simbolos; 

        Tabela_simbolos();
        int inserir(string, int);

        /** Construtor: Insere as palavras reservadas na tabela de simbolos */
        Tabela_simbolos() {
            tab_simbolos.insert(make_pair("const" , TK_const));
            tab_simbolos.insert(make_pair("int" , TK_int));
            tab_simbolos.insert(make_pair("char" , TK_char));
            tab_simbolos.insert(make_pair("while" , TK_while));
            tab_simbolos.insert(make_pair("if" , TK_if));
            tab_simbolos.insert(make_pair("float" , TK_float));
            tab_simbolos.insert(make_pair("else" , TK_else));
            tab_simbolos.insert(make_pair("&&" , TK_and));
            tab_simbolos.insert(make_pair("||" , TK_or));
            tab_simbolos.insert(make_pair("!" , TK_not));
            tab_simbolos.insert(make_pair(":=" , TK_atrib));
            tab_simbolos.insert(make_pair("=" , TK_igualdade));
            tab_simbolos.insert(make_pair("(" , TK_abreParentese));
            tab_simbolos.insert(make_pair(")" , TK_fechaParentese));
            tab_simbolos.insert(make_pair("<" , TK_menor));
            tab_simbolos.insert(make_pair(">" , TK_maior));
            tab_simbolos.insert(make_pair("!=" , TK_diferente));
            tab_simbolos.insert(make_pair(">=" , TK_maiorIgual));
            tab_simbolos.insert(make_pair("<=" , TK_menorIgual));
            tab_simbolos.insert(make_pair("," , TK_virgula));
            tab_simbolos.insert(make_pair("-" , TK_hifen));
            tab_simbolos.insert(make_pair("*" , TK_asterisco));
            tab_simbolos.insert(make_pair("/" , TK_barra));
            tab_simbolos.insert(make_pair(";" , TK_pontoevirgula));
            tab_simbolos.insert(make_pair("{" , TK_abreChave));
            tab_simbolos.insert(make_pair("}" , TK_fechaChave));
            tab_simbolos.insert(make_pair("readln" , TK_readln));
            tab_simbolos.insert(make_pair("div" , TK_div));
            tab_simbolos.insert(make_pair("string" , TK_string));
            tab_simbolos.insert(make_pair("write" , TK_write));
            tab_simbolos.insert(make_pair("writeln" , TK_writeln));
            tab_simbolos.insert(make_pair("mod" , TK_mod));
            tab_simbolos.insert(make_pair("[" , TK_abreColchete));
            tab_simbolos.insert(make_pair("]" , TK_fechaColchete));
            tab_simbolos.insert(make_pair("true" , TK_true));
            tab_simbolos.insert(make_pair("false" , TK_false));
            tab_simbolos.insert(make_pair("boolean" , TK_boolean));
        }

        /** Verifica se um lexema já existe na tabela */
        string pesquisar(string lexema);

};


/** Verifica se o caracter lido é válido */
bool isValidChar(char c) {}


/** Verifica se o caracter lido é uma letra */
bool isLetter(char c) {}


/** Verifica se o caracter lido é um número */
bool isNumber(char c) {}


/** Implementando o autômato (analisador léxico) */
string getNextToken(char c) {
    int estado_inicial = 0; 
    int estado_atual = estado_inicial; 
    int estado_final = 2;
    string lex = "";
    
    while (estado_atual != estado_final) {
        if (isValidChar(c)) {
            switch (estado_atual) {
            
                /** Estado Inicial **/
                case 0:

                    // Caracter em branco
                    if (c == ' ');

                    // Letra ou _ levam a um identificador ou palavra reservada
                    else if (isLetter(c) || c == '_') { 
                        lex = c; estado_atual = 1; 
                    }

                    // " (leva a uma string)
                    else if (c == '\"') { 
                        lex = c; estado_atual = 3; 
                    }

                    // Um dígito
                    else if (isNumber(c)) { 
                        lex = c; estado_atual = 4; 
                    }

                    // Ponto (leva a um número real ".456")
                    else if (c == '.') { 
                        lex = c; estado_atual = 6; 
                    }

                    // >, < ou ! (Pode vir seguido de = ou não)
                    else if (c == '>' || c == '<' || c == '!') { 
                        lex = c; estado_atual = 7; 
                    }

                    // & (AND)
                    else if (c == '&') { 
                        lex = c; estado_atual = 8; 
                    }

                    // | (OR)
                    else if (c == '|') { 
                        lex = c; estado_atual = 9; 
                    }

                    // : (vem seguido de '=' -> atribuição)
                    else if (c == ':') { 
                        lex = c; estado_atual = 10; 
                    }

                    // / (início de comentário)
                    else if (c == '/') { 
                        lex = c; estado_atual = 11; 
                    }

                    // ' (leva a um char)
                    else if (c == '\'') { 
                        lex = c; estado_atual = 12; 
                    }

                    // Caracteres independentes (+, -, ;, (), [], {}, etc...)
                    else {
                        lex = c;
                        estado_atual = estado_final;
                    }

                    break;

                // Letra, Dígito ou sublinhado
                case 1:
                    while(isLetter(c) || isNumber(c) || c == '_') {
                        lex += c;
                        cin >> c;
                    }
                    // > Devolve o caracter lido
                    // > Pesquisa o lex na tabela de simbolos
                    estado_atual = estado_final;
                    break;

                // Conteúdo da string até o fechamento das aspas
                case 3:
                    while (c != '\"' && c != '\n') {
                        lex += c;
                        cin >> c;
                    }
                    if (c == '\"') {
                        lex += c;
                        estado_atual = estado_final;
                    }
                    break;

                // Constante numérica
                case 4: 
                    while (isNumber(c)) {
                        lex += c;
                        cin >> c;
                    }
                    if (c == '.') {
                        lex += c;
                        estado_atual = 5;
                    }
                    else {
                        // > Devolve o caracter lido
                        estado_atual = estado_final;
                    }
                    break;

                // Parte decimal da constante numérica
                case 5:
                    while (isNumber(c)) {
                        lex += c;
                        cin >> c;
                    }
                    // > Devolve o caracter lido
                    estado_atual = estado_final;
                    break;

                // Parte decimal do número após começar com '.'
                case 6:
                    if (isNumber(c)) {
                        lex += c;
                        estado_atual = 5;
                    }
                    break;

                
                case 7:
                    if (c == '=') {
                        lex += c;
                        estado_atual = estado_final;
                    }
                    else {
                        // > Devolve o caracter lido
                        estado_atual = estado_final;
                    }
                    break;

                // && (and)
                case 8:
                    if (c == '&') {
                        lex += c;
                        estado_atual = estado_final;
                    }
                    break;

                // || (or)
                case 9:
                    if (c == '|') {
                        lex += c;
                        estado_atual = estado_final;
                    }
                    break;

                // := (atribuição)
                case 10:
                    if (c == '=') {
                        lex += c;
                        estado_atual = estado_final;
                    }
                    break;

                // Começando comentário (/*)
                case 11:
                    if (c == '*') {
                        lex += c;
                        estado_atual = 12;
                    }
                    break;

                // Fechando comentário (*/)
                case 12:
                    while (c != '*') {
                        lex += c;
                        cin >> c;
                    }
                    lex += c;
                    estado_atual = 13;
                    break;

                // Fechando comentário (*/) ou voltando
                case 13:
                    if (c == '/') {
                        estado_atual = estado_inicial;
                    } 
                    else {
                        lex += c;
                        estado_atual = 12;
                    }
                    break;

                // Constante char ('c')
                case 14:
                    if (isLetter(c)) {
                        lex += c;
                        estado_atual = 15;
                    }
                    break;

                // Constante char ('c')
                case 15:
                    if (c == '\'') {
                        lex += c;
                        estado_atual = estado_final;
                    }
                    break;

            }

        }

        // Lendo o próximo caracter
        cin >> c;
    }
}

int main(int argc, char const *argv[]) {
    Tabela_simbolos tab_simbolos;
    char c;

    // Ler enquanto não chegar ao final do arquivo
    while (cin >> c) getNextToken(c);

    return 0;
}
