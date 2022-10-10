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
#include <ctype.h>
#include <string.h>

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

int nLinhasCompiladas = 1;

/** Tabela de símbolos, armazenando identificadores e palavras reservadas da linguagem */
class Tabela_simbolos {
    public:
        /** Tabela hash que mapeia lexema para token */
        unordered_map<string, int> tab_simbolos; 
        // int inserir(string, int);

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

        /** Verifica se um lexema já existe na tabela
         *  Se nao existir, insere este lexema
         */
        string pesquisar(string lexema);

};

Tabela_simbolos tab_simbolos;

/** Verifica se o caracter lido é válido 
 * letras, dígitos, espaço, sublinhado, ponto, vírgula, ponto-e-vírgula, 
 * dois-pontos, parênteses, colchetes, chaves, mais, menos, aspas, apóstrofo,
 * barra, barra em pé, arroba, e-comercial, porcentagem, exclamação, interrogação,
 * maior, menor e igual
 * --- NAO INCLUSOS ABAIXO: quebra de linha (bytes 0Dh e 0Ah) ---
*/
bool isValidChar(char c) {
    if ((c >= 65 && c <= 90)    || // maiusculas
        (c >= 97 && c <= 122)   || // minusculas
        (c >= 48 && c <= 57)    || // digitos
        isspace(c)              || // quebra de linha e espaço
        cin.eof()               ||
        c == 95 || c == 46 || c == 44 || c == 59 ||
        c == 58 || c == 40 || c == 41 || c == 42 || c == 91 ||
        c == 45 || c == 34 || c == 39 || c == 47 ||
        c == 33 || c == 63 || c == 60 || c == 61 || c == 62 ||
        c == 124 || c == 64 || c == 38 || c == 37 ||
        c == 93 || c == 123 || c == 125 || c == 43)
            return true;

    return false;      
}


/** Verifica se o caracter lido é uma letra */
bool isLetter(char c) {
    return ((c >= 65 && c <= 90) || (c >= 97 && c <= 122));
}


/** Verifica se o caracter lido é um número */
bool isNumber(char c) {
    return (c >= 48 && c <= 57);
}


/** Implementando o autômato (analisador léxico) */
string getNextToken() {
    int estado_inicial = 0; 
    int estado_atual = estado_inicial; 
    int estado_final = 2;
    string lex = "";
    char c;
    bool erro = false;

    while (estado_atual != estado_final && !erro) {
        c = cin.get(); // leitura do prox caractere
        if (isValidChar(c)) {
            switch (estado_atual) {
            
                /** Estado Inicial **/
                case 0:
                    // Caractere em branco
                    if (isspace(c)) {
                        if (c == '\n')
                            nLinhasCompiladas++;
                    }

                    // Letra ou _ levam a um identificador ou palavra reservada
                    else if (isLetter(c) || c == '_') {
                        lex = c; estado_atual = 1; 
                    }

                    // " (leva a uma string)
                    else if (c == '\"') { 
                        lex = c; estado_atual = 3; 
                    }

                    // Um dígito exceto 0 (leva a num inteiro ou real)
                    else if (c != '0' && isNumber(c)) { 
                        lex = c; estado_atual = 4; 
                    }

                    // Digito 0 (pode levar a num inteiro, real ou hexadecimal)
                    else if (c == '0'){
                        lex = c; estado_atual = 16;
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

                    // / (início de comentário ou divisao)
                    else if (c == '/') { 
                        lex = c; estado_atual = 11; 
                    }

                    // ' (leva a um char)
                    else if (c == '\'') { 
                        lex = c; estado_atual = 14; 
                    }

                    // demais tokens
                    else if(c == ','){
                        lex = c;
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '+'){
                        lex = c;
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '-'){
                        lex = c;
                        estado_atual = estado_final;
                    } 

                    else if(c == '='){
                        lex = c;
                        estado_atual = estado_final;
                    }
                    
                    else if(c == ';'){
                        lex = c;
                        estado_atual = estado_final;
                    }

                    else if(c == '*'){
                        lex = c;
                        estado_atual = estado_final;
                    }                    
                    
                    else if(c == '('){
                        lex = c;
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == ')'){
                        lex = c;
                        estado_atual = estado_final;
                    }
                    
                    else if(c == '['){
                        lex = c;
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == ']'){
                        lex = c;
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '{'){
                        lex = c;
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '}'){
                        lex = c;
                        estado_atual = estado_final;
                    }

                    else if(cin.eof()){
                        lex = "eof";
                        estado_atual = estado_final;
                    }

                    else {
                        // Printando o erro -> lexema n identificado
                        lex += c;
                        erro = true;
                        cout << nLinhasCompiladas << endl;
                        cout << "lexema nao identificado [" << lex << "]" << endl;
                    }
                    break;

                // Letra, Dígito ou sublinhado
                case 1:
                    while(isLetter(c) || isNumber(c) || c == '_') {
                        lex += c;
                        c = cin.get();
                    }
                    //Devolve o caracter lido
                    if (!cin.eof()) cin.putback(c);

                    // > Pesquisa o lex na tabela de simbolos
                    // tab_simbolos.pesquisar(lex);
                    estado_atual = estado_final;
                    break;

                // Conteúdo da string até o fechamento das aspas
                case 3:
                    while (c != '\"' && c != '\n') {
                        lex += c;
                        c = cin.get();
                    }
                    if (c == '\"') {
                        lex += c;
                        estado_atual = estado_final;
                    } 
                    else if (c == '\n') {
                        // ERRO -> Caracter inválido
                        erro = true;
                        cout << nLinhasCompiladas << endl;
                        cout << "caractere invalido" << endl;
                    }
                    break;

                // Constante numérica
                case 4: 
                    while (isNumber(c)) {
                        lex += c;
                        c = cin.get();
                    }
                    if (c == '.') {
                        lex += c;
                        estado_atual = 5;
                    }
                    else {
                        // > Devolve o caracter lido
                        if (!cin.eof()) cin.putback(c);

                        estado_atual = estado_final;
                    }
                    break;

                // Parte decimal da constante numérica (apos o .)
                case 5:
                    while (isNumber(c)) {
                        lex += c;
                        c = cin.get();
                    }
                    // Terminou o número, verificar as 6 casas de precisão
                    // .123456 ou 12345.6 = 7 caracteres no máximo
                    if (lex.length() > 7) {
                        erro = true;
                        cout << nLinhasCompiladas << endl;
                        cout << "lexema nao identificado [" << lex << "]" << endl;
                    }
                    else {
                        // > Devolve o caracter lido
                        if (!cin.eof()) cin.putback(c);
                        estado_atual = estado_final;
                    }
                    break;

                // Parte decimal do número (após começar com '.')
                case 6:
                    if (isNumber(c)) {
                        lex += c;
                        estado_atual = 5;
                    } else {
                        // ERRO -> Não tem número depois do ponto
                        erro = true;
                        cout << nLinhasCompiladas << endl;
                        cout << "caractere invalido" << endl;
                    }
                    break;

                // >=, <= ou !=
                case 7:
                    if (c == '=') {
                        lex += c;
                        estado_atual = estado_final;
                    }
                    else {
                        // > Devolve o caracter lido
                        if (!cin.eof()) cin.putback(c);
                        estado_atual = estado_final;
                    }
                    break;

                // && (and)
                case 8:
                    if (c == '&') {
                        lex += c;
                        estado_atual = estado_final;
                    } else {
                        // Erro: Caracter inválido
                        erro = true;
                        cout << nLinhasCompiladas << endl;
                        cout << "caractere invalido" << endl;
                    }
                    break;

                // || (or)
                case 9:
                    if (c == '|') {
                        lex += c;
                        estado_atual = estado_final;
                    } else {
                        // Erro: Caracter inválido
                        erro = true;
                        cout << nLinhasCompiladas << endl;
                        cout << "caractere invalido" << endl;
                    }
                    break;

                // := (atribuição)
                case 10:
                    if (c == '=') {
                        lex += c;
                        estado_atual = estado_final;
                    } else {
                        // Erro: Caracter inválido
                        erro = true;
                        cout << nLinhasCompiladas << endl;
                        cout << "caractere invalido" << endl;
                    }
                    break;

                // Comentário (/*) ou divisao (/)
                case 11:
                    if (c == '*') { // comentario
                        lex += c;
                        estado_atual = 12;
                    }
                    else { // divisao
                        // > Devolve caracter lido
                        if (!cin.eof()) cin.putback(c);
                        estado_atual = estado_final;
                    }
                    break;

                // Comentário (*/)
                case 12:
                    while (c != '*') {
                        lex += c;
                        // Pode possuir quebras de linha dentro do comentário
                        if (c == '\n') nLinhasCompiladas++;
                        c = cin.get();
                    }
                    lex += c;
                    estado_atual = 13;
                    break;

                // Fechando comentário (*/) ou voltando p/ estado anterior
                case 13:
                    if (c == '/') {
                        lex = ""; // Reseta o lexema
                        estado_atual = estado_inicial;
                    } 
                    else {
                        lex += c;
                        estado_atual = 12;
                    }
                    break;

                // Constante char ('c')
                case 14:
                    lex += c;
                    estado_atual = 15;
                    break;

                // Constante char ('c')
                case 15:
                    if (c == '\'') {
                        lex += c;
                        estado_atual = estado_final;
                    } else {
                        // ERRO -> Não é uma constante char válida
                        erro = true;
                        cout << nLinhasCompiladas << endl;
                        cout << "caractere invalido" << endl;
                    }
                    break;

                // Primeiro digito = 0 -> pode ser inteiro, hexadecimal ou real
                case 16:
                    if (isNumber(c)) {  // Constante numérica
                        lex += c;
                        estado_atual = 4;
                    }
                    else if (c == 'x'){ // Constante hexa
                        lex += c; 
                        estado_atual = 17;
                    }
                    else if (c == '.'){ // Constante real iniciada com 0
                        lex += c; 
                        estado_atual = 5;
                    }
                    else {
                        // > Devolve caracter lido
                        if (!cin.eof()) cin.putback(c);
                        estado_atual = estado_final;
                    }
                    break;

                // Constante hexa - 1o digito
                case 17:
                    if (isNumber(c) || (c >= 65 && c <= 70)){
                        lex += c; 
                        estado_atual = 18;
                    } else {
                        // ERRO -> Caracter inválido
                        erro = true;
                        cout << nLinhasCompiladas << endl;
                        cout << "caractere invalido" << endl;
                    }
                    break;

                // Constante hexa - 2o digito
                case 18:
                    if (isNumber(c) || (c >= 65 && c <= 70)){
                        lex += c;
                        estado_atual = estado_final;
                    } else {
                        // ERRO -> Caracter inválido
                        erro = true;
                        cout << nLinhasCompiladas << endl;
                        cout << "caractere invalido" << endl;
                    }
                    break;

                default:
                    lex += c;
                    erro = true;
                    cout << nLinhasCompiladas << endl;
                    cout << "lexema nao identificado [" << lex << "]" << endl;
            }

        }
        else {
            erro = true;
            cout << nLinhasCompiladas << endl;
            cout << "caractere invalidonao reconhecido): " << c << endl;
        }
    }

    if (erro)
        return "-1";
    else
        return lex;
}

int main(int argc, char const *argv[]) {
    char c;

    c = cin.get();
    // Ler enquanto não chegar ao final do arquivo
    while (!cin.eof()){
        cin.putback(c);
        string token = getNextToken();
        if (strcmp(token.c_str(), "-1") == 0)
            break;
        // cout << "token: " << token << endl;
        c = cin.get();
    }

    getNextToken();

    cout << nLinhasCompiladas << " linhas compiladas.";

    return 0;
}
