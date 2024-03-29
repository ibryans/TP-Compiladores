/**
 * Trabalho Prático de Compiladores
 * Completo - Analisador Léxico, Sintático, Semântico e Gerador de Código
 * 
 * Bryan Santos e Igor Reis - 02/2022
 */

#include <iostream>
#include <string>
#include <unordered_map>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <fstream>

using namespace std;

/** Tipos de Erro */
#define INVALID_CHAR         111
#define INVALID_LEX          222
#define INVALID_EOF          333
#define INVALID_TOKEN        444
#define INVALID_STR_SIZE     555
#define DUPLICATE_ID         666
#define INCOMPATIBLE_TYPES   777
#define INCOMPATIBLE_CLASSES 888
#define UNDECLARED_ID        999

/** Definindo e Enumerando os tokens */
#define TK_CONST            1
#define TK_INT              2
#define TK_CHAR             3
#define TK_WHILE            4
#define TK_IF               5
#define TK_FLOAT            6
#define TK_ELSE             7
#define TK_AND              8
#define TK_OR               9
#define TK_NOT              10
#define TK_ATRIBUICAO       11
#define TK_IGUAL            12
#define TK_ABRE_PARENTESE   13
#define TK_FECHA_PARENTESE  14
#define TK_MENOR            15
#define TK_MAIOR            16
#define TK_DIFERENTE        17
#define TK_MAIOR_IGUAL      18
#define TK_MENOR_IGUAL      19
#define TK_VIRGULA          20
#define TK_MENOS            21
#define TK_MAIS             22
#define TK_ASTERISCO        23
#define TK_BARRA            24
#define TK_PONTO_E_VIRGULA  25
#define TK_ABRE_CHAVE       26
#define TK_FECHA_CHAVE      27
#define TK_READLN           28
#define TK_DIV              29
#define TK_STRING           30
#define TK_WRITE            31
#define TK_WRITELN          32
#define TK_MOD              33
#define TK_ABRE_COLCHETE    34
#define TK_FECHA_COLCHETE   35

#define TK_VERDADEIRO       36
#define TK_FALSO            37

#define TK_BOOLEAN          38
#define TK_ID               39
#define TK_EOF              40

/** Tipos de Variáveis e Constantes */
#define TIPO_VAZIO          41
#define TIPO_INTEIRO        42
#define TIPO_REAL           43
#define TIPO_CARACTER       44
#define TIPO_STRING         45
#define TIPO_BOOLEAN        46

/** Tipos de classes */
#define CLASSE_VAZIA        47
#define CLASSE_VARIAVEL     48
#define CLASSE_CONSTANTE    49

/** Simbolo inserido na TS */
struct Simbolo {
    int token;
    int tipo;
    int classe;
    int tamanho;
    int endereco;
};

/** String que irá armazenar todo o codigo em assembly gerado */
string codigo_asm;

/** Primeiras linhas de código em assembly, para inicializar o programa */
void inicializa_asm() {
    codigo_asm.append("\nsection .data\t\t; Seção de dados\n");
    codigo_asm.append("M:\t\t\t; Rótulo que marca o início da seção de dados\n");
    codigo_asm.append("resb 10000h\t\t\t; Reserva de temporários\n");
    codigo_asm.append("section .text\t\t; Seção de código\n");
    codigo_asm.append("global _start\t\t; Ponto inicial do programa\n");
    codigo_asm.append("_start:\t\t\t; Início do programa\n");
}

/** Últimas linhas de código em assembly, para finalizar o programa*/
void finaliza_asm() {
    codigo_asm.append("\n; Halt\n");
    codigo_asm.append("mov rax, 60 \t\t; Chamada de saída\n");
    codigo_asm.append("mov rdi, 0 \t\t\t; Código de saída sem erros\n");
    codigo_asm.append("syscall \t\t\t; Chama o kernel\n");

    // faz arquivo .asm
    ofstream arquivo_asm("codigo.asm");
    arquivo_asm << codigo_asm;
    arquivo_asm.close();
}

/** contadores de rotulos, dados e temporarios **/
int rotulo = 1;
int count_dados = 65536; // 10000h em decimal
int count_temporarios = 0;

/** Aumenta o contador de Rótulos e retorna um novo */
string novo_rotulo() {
    string novo_rotulo = "rotulo" + to_string(rotulo++);
    return novo_rotulo;
}

/** Aumenta o contador de dados e retorna o próximo endereço */
int novo_dado(int num_bytes){
    int endereco = count_dados;
    count_dados += num_bytes;
    return endereco;
}

/** Aumenta o contador de temporários e retorna o próximo endereço */
int novo_temp(int num_bytes){
    int endereco = count_temporarios;
    count_temporarios += num_bytes;
    return endereco;
}

/** Transforma inteiro em Hexadecimal */
string int2hex(int number){
    ostringstream ss;
    ss << "0x" << hex << number;
    return ss.str();
}

/** Tabela de símbolos, armazenando identificadores e palavras reservadas da linguagem */
class Tabela_simbolos {
    public:
        /** Tabela hash que mapeia lexema para Simbolo */
        unordered_map<string, Simbolo> tab_simbolos; 

        /** Construtor: Insere as palavras reservadas na tabela de simbolos */
        Tabela_simbolos() {
            inserir("const" , TK_CONST);
            inserir("int" , TK_INT);
            inserir("char" , TK_CHAR);
            inserir("while" , TK_WHILE);
            inserir("if" , TK_IF);
            inserir("float" , TK_FLOAT);
            inserir("else" , TK_ELSE);
            inserir("&&" , TK_AND);
            inserir("||" , TK_OR);
            inserir("!" , TK_NOT);
            inserir(":=" , TK_ATRIBUICAO);
            inserir("=" , TK_IGUAL  );
            inserir("(" , TK_ABRE_PARENTESE);
            inserir(")" , TK_FECHA_PARENTESE);
            inserir("<" , TK_MENOR);
            inserir(">" , TK_MAIOR);
            inserir("!=" , TK_DIFERENTE);
            inserir(">=" , TK_MAIOR_IGUAL);
            inserir("<=" , TK_MENOR_IGUAL);
            inserir("," , TK_VIRGULA);
            inserir("-" , TK_MENOS);
            inserir("+", TK_MAIS);
            inserir("*" , TK_ASTERISCO);
            inserir("/" , TK_BARRA);
            inserir(";" , TK_PONTO_E_VIRGULA);
            inserir("{" , TK_ABRE_CHAVE);
            inserir("}" , TK_FECHA_CHAVE);
            inserir("readln" , TK_READLN);
            inserir("div" , TK_DIV);
            inserir("string" , TK_STRING);
            inserir("write" , TK_WRITE);
            inserir("writeln" , TK_WRITELN);
            inserir("mod" , TK_MOD);
            inserir("[" , TK_ABRE_COLCHETE);
            inserir("]" , TK_FECHA_COLCHETE);
            inserir("true" , TK_VERDADEIRO);
            inserir("false" , TK_FALSO);
            inserir("boolean" , TK_BOOLEAN);
        }

        /**
         * Insere um [lex, Simbolo] na tabela de símbolos
        */
        void inserir(string lexema, int token) {
            int tipo;
            
            // Se o lexema for um booleano, insere na TS com esse tipo
            if (lexema.compare("true") == 0 || lexema.compare("false") == 0)
                tipo = TIPO_BOOLEAN;
            else
                tipo = TIPO_VAZIO;   // Se não, insere como tipo nulo mesmo

            tab_simbolos.insert(make_pair(lexema, Simbolo{token, tipo, CLASSE_VAZIA, 0, 0}));
        }

        /** 
         * Verifica se um lexema (identificador) já existe na tabela
         * Se nao existir, insere o simbolo com esse lexema e o retorna
         * Se existir, retorna o simbolo da tabela
         */
        Simbolo pesquisar(string lexema) {
            Simbolo s;
            s.token = TK_ID;
            auto iterator = tab_simbolos.find(lexema);
            if (iterator == tab_simbolos.end()){ // novo identificador, insere na tabela
                inserir(lexema, TK_ID);
            } else {    // ja existe na tabela, retorna
                s = tab_simbolos[lexema];
            }
            return s;
        };

        /**
         * Faz um update em um símbolo existente da Tabela de Símbolos
        */
        void update(string lexema, Simbolo novo_simbolo) {
            auto it = tab_simbolos.find(lexema);
            if (it != tab_simbolos.end())
                it->second = novo_simbolo;
        }

};

/** Estrutura para carregar o registro léxico */
struct RegLex{
    string lexema;
    int token;
    int tipo;
    int tamanho;
};

/* Variáveis (globais) do analisador léxico */
int nLinhasCompiladas = 1;
Tabela_simbolos tab_simbolos;
RegLex registroLexico;
bool erro = false;

/** Método para atualizar o Registro Léxico */
void updateRegLex(string lexema, int token, int tipo, int tamanho) {
    registroLexico.lexema = lexema;
    registroLexico.token = token;
    registroLexico.tipo = tipo;
    registroLexico.tamanho = tamanho;
}

/** Verifica se o caracter lido é válido para a linguagem
 * Isso inclui quebra de linha, espaço, fim do arquivo, e todos os caracteres permitidos
 */
bool isValidChar(char c) {
    return (
        (c >= 65 && c <= 90)    || // maiusculas
        (c >= 97 && c <= 122)   || // minusculas
        (c >= 48 && c <= 57)    || // digitos
        isspace(c)              || // quebra de linha e espaço
        cin.eof()               || // fim do arquivo

        // Demais caracteres permitidos
        c == 95 || c == 46 || c == 44 || c == 59  ||
        c == 58 || c == 40 || c == 41 || c == 42  || 
        c == 91 || c == 45 || c == 34 || c == 39  || 
        c == 47 || c == 33 || c == 63 || c == 60  || 
        c == 61 || c == 62 || c == 124 || c == 64 || 
        c == 38 || c == 37 || c == 93 || c == 123 || 
        c == 125 || c == 43
    );
}


/** Verifica se o caracter lido é uma letra */
bool isLetter(char c) {
    return ((c >= 65 && c <= 90) || (c >= 97 && c <= 122));
}


/** Verifica se o caracter lido é um número */
bool isNumber(char c) {
    return (c >= 48 && c <= 57);
}

/** Mostra a mensagem de erro de acordo com o tipo dela */
void showError(int type, string lex) {
    cout << nLinhasCompiladas << endl;
    switch (type) {
        case INVALID_CHAR: 
            cout << "caractere invalido." << endl;
            break;
        case INVALID_LEX: 
            cout << "lexema nao identificado [" << lex << "]." << endl;
            break;
        case INVALID_EOF: 
            cout << "fim de arquivo nao esperado." << endl;
            break;
        case INVALID_TOKEN: 
            cout << "token nao esperado [" << lex << "]." << endl;
            break;
        case INVALID_STR_SIZE:
            cout << "str maior que o permitido (" << lex.length() << " bytes)." << endl;
            break;
        case DUPLICATE_ID:
            cout << "identificador ja declarado [" << lex << "]." << endl;
            break;
        case INCOMPATIBLE_TYPES:
            cout << "tipos incompativeis." << endl;
            break;
        case INCOMPATIBLE_CLASSES:
            cout << "classe de identificador incompatível [" << lex << "]." << endl;
            break;
        case UNDECLARED_ID:
            cout << "identificador nao declarado [" << lex << "]." << endl;
            break;
    }
    exit(0);
}

/** Implementando o autômato (analisador léxico) */
string getNextToken() {
    int estado_inicial = 0; 
    int estado_atual = estado_inicial; 
    int estado_final = 2;
    string lex = "";
    char c;

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
                        updateRegLex(lex, TK_VIRGULA, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '+'){
                        lex = c;
                        updateRegLex(lex, TK_MAIS, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '-'){
                        lex = c;
                        updateRegLex(lex, TK_MENOS, -99, 0);
                        estado_atual = estado_final;
                    } 

                    else if(c == '='){
                        lex = c;
                        updateRegLex(lex, TK_IGUAL  , -99, 0);
                        estado_atual = estado_final;
                    }
                    
                    else if(c == ';'){
                        lex = c;
                        updateRegLex(lex, TK_PONTO_E_VIRGULA, -99, 0);
                        estado_atual = estado_final;
                    }

                    else if(c == '*'){
                        lex = c;
                        updateRegLex(lex, TK_ASTERISCO, -99, 0);
                        estado_atual = estado_final;
                    }                    
                    
                    else if(c == '('){
                        lex = c;
                        updateRegLex(lex, TK_ABRE_PARENTESE, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == ')'){
                        lex = c;
                        updateRegLex(lex, TK_FECHA_PARENTESE, -99, 0);
                        estado_atual = estado_final;
                    }
                    
                    else if(c == '['){
                        lex = c;
                        updateRegLex(lex, TK_ABRE_COLCHETE, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == ']'){
                        lex = c;
                        updateRegLex(lex, TK_FECHA_COLCHETE, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '{'){
                        lex = c;
                        updateRegLex(lex, TK_ABRE_CHAVE, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '}'){
                        lex = c;
                        updateRegLex(lex, TK_FECHA_CHAVE, -99, 0);
                        estado_atual = estado_final;
                    }

                    else if(cin.eof()){
                        lex = "eof";
                        updateRegLex(lex, TK_EOF, -99, 0);
                        estado_atual = estado_final;
                    }

                    else {
                        lex += c;
                        erro = true;
                        showError(INVALID_CHAR, lex);
                    }
                    break;

                // Letra, Dígito ou sublinhado
                case 1:
                    while(isLetter(c) || isNumber(c) || c == '_') {
                        lex += c;
                        c = cin.get();
                    }
                    // Checa se excedeu tamanho maximo permitido
                    if(lex.length() > 32){
                        erro = true;
                        showError(INVALID_LEX, lex);
                    }
                    else{
                        // Devolve o caracter lido
                        if (!cin.eof()) cin.putback(c);
                        
                        // Desconsiderando maiúsculas e minúsculas
                        for(int i=0; i < lex.length(); i++)
                            lex[i] = tolower(lex[i]);

                        // Pesquisa o lex na tabela de simbolos pra ver se é uma palavra reservada ou um identificador
                        Simbolo s = tab_simbolos.pesquisar(lex);
                        int tok = s.token;
                        int tipo = s.tipo;
                        updateRegLex(lex, tok, tipo, 0);
                        estado_atual = estado_final;
                    }
                    
                    break;

                // Conteúdo da string até o fechamento das aspas
                case 3:
                    while (isValidChar(c) && c != '\"' && c != '\n' && !cin.eof()) {
                        lex += c;
                        c = cin.get();
                    }
                    if (!isValidChar(c)) {
                        erro = true;
                        showError(INVALID_CHAR, lex);
                    }
                    else if (lex.length() > 256) {
                        erro = true;
                        showError(INVALID_STR_SIZE, lex);
                    }
                    else if (c == '\"') {
                        lex += c;
                        updateRegLex(lex, TK_CONST, TIPO_STRING, lex.length()-2);
                        estado_atual = estado_final;
                    } 
                    else if (c == '\n') {
                        erro = true;
                        showError(INVALID_LEX, lex);
                    }
                    else if (cin.eof()) {
                        erro = true;
                        showError(INVALID_EOF, lex);
                    }
                    break;

                // Constante numérica
                case 4: 
                    while (isNumber(c)) {
                        lex += c;
                        c = cin.get();
                    }
                    if (c == '.') { // Numero float
                        lex += c;
                        estado_atual = 5;
                    }
                    else {
                        // > Devolve o caracter lido
                        if (!cin.eof()) cin.putback(c);

                        updateRegLex(lex, TK_CONST, TIPO_INTEIRO, 4);
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
                        showError(INVALID_LEX, lex);
                    }
                    else {
                        // > Devolve o caracter lido
                        if (!cin.eof()) cin.putback(c);
                        
                        updateRegLex(lex, TK_CONST, TIPO_REAL, 4);
                        estado_atual = estado_final;
                    }
                    break;

                // Parte decimal do número (após começar com '.')
                case 6:
                    if (isNumber(c)) {
                        lex += c;
                        estado_atual = 5;
                    } 
                    else if (cin.eof()) {
                        erro = true;
                        showError(INVALID_EOF, lex);
                    }
                    else {
                        // ERRO -> Não tem número depois do ponto
                        erro = true;
                        showError(INVALID_LEX, lex);
                    }
                    break;

                // >=, <= ou !=
                case 7:
                    int tok;
                    if (c == '=') {
                        lex += c;
                        if (lex[0] == '>')
                            tok = TK_MAIOR_IGUAL;
                        else if (lex[0] == '<')
                            tok = TK_MENOR_IGUAL;
                        else if (lex[0] == '!')
                            tok = TK_DIFERENTE;
                        updateRegLex(lex, tok, -99, 0);
                        estado_atual = estado_final;
                    }
                    else {
                        // > Devolve o caracter lido
                        if (!cin.eof()) {
                            cin.putback(c);
                            if (lex[0] == '>')
                                tok = TK_MAIOR;
                            else if (lex[0] == '<')
                                tok = TK_MENOR;
                            else if (lex[0] == '!')
                                tok = TK_NOT;
                            updateRegLex(lex, tok, -99, 0);
                            estado_atual = estado_final;
                        } else {
                            erro = true;
                            showError(INVALID_EOF, lex);
                        }
                    }
                    break;

                // && (and)
                case 8:
                    if (c == '&') {
                        lex += c;
                        updateRegLex(lex, TK_AND, -99, 0);
                        estado_atual = estado_final;
                    } else {
                        // Erro: Não existe '&' na linguagem
                        erro = true;
                        showError(INVALID_LEX, lex);
                    }
                    break;

                // || (or)
                case 9:
                    if (c == '|') {
                        lex += c;
                        updateRegLex(lex, TK_OR, -99, 0);
                        estado_atual = estado_final;
                    } else {
                        // Erro: Não existe '|' na linguagem
                        erro = true;
                        showError(INVALID_LEX, lex);
                    }
                    break;

                // := (atribuição)
                case 10:
                    if (c == '=') {
                        lex += c;
                        updateRegLex(lex, TK_ATRIBUICAO, -99, 0);
                        estado_atual = estado_final;
                    } else {
                        // Erro: Não existe só ':' na linguagem
                        erro = true;
                        showError(INVALID_LEX, lex);
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
                        if (!cin.eof()) {
                            cin.putback(c);
                            updateRegLex(lex, TK_BARRA, -99, 0);
                            estado_atual = estado_final;
                        } else {
                            erro = true;
                            showError(INVALID_EOF, lex);
                        }
                    }
                    break;

                // Comentário (*/)
                case 12:
                    while (c != '*' && !cin.eof() && isValidChar(c)) {
                        lex += c;
                        // Pode possuir quebras de linha dentro do comentário
                        if (c == '\n') nLinhasCompiladas++;
                        c = cin.get();
                    }
                    if (!cin.eof()) {
                        lex += c;
                        estado_atual = 13;
                    } else {
                        erro = true;
                        showError(INVALID_EOF, lex);
                    }
                    break;

                // Fechando comentário (*/) ou voltando p/ estado anterior
                case 13:
                    if (c == '/') {
                        lex = ""; // Reseta o lexema
                        estado_atual = estado_inicial;
                    } 
                    else {
                        if (c == '*') {
                            lex += c;
                        } else if (!cin.eof()) {
                            lex += c;
                            estado_atual = 12;
                        } else {
                            erro = true;
                            showError(INVALID_EOF, lex);
                        }
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
                        updateRegLex(lex, TK_CONST, TIPO_CARACTER, 1);
                        estado_atual = estado_final;
                    } else {
                        // ERRO -> Não é uma constante char válida
                        erro = true;
                        showError(INVALID_LEX, lex);
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
                        updateRegLex(lex, TK_CONST, TIPO_INTEIRO, 4);
                        estado_atual = estado_final;
                    }
                    break;

                // Constante hexa - 1o digito
                case 17:
                    if ((isNumber(c) || (c >= 65 && c <= 70) || (c >= 97 && c <= 102)) && !cin.eof()){
                        lex += c; 
                        estado_atual = 18;
                    } else {
                        // ERRO -> não é um lexema válido (sem número depois do x)
                        erro = true;
                        if (cin.eof())
                            showError(INVALID_EOF, lex);
                        else
                            showError(INVALID_LEX, lex);
                    }
                    break;

                // Constante hexa - 2o digito
                case 18:
                    if (isNumber(c) || (c >= 65 && c <= 70) || (c >= 97 && c <= 102)){
                        lex += c;
                        updateRegLex(lex, TK_CONST, TIPO_CARACTER, 1);
                        estado_atual = estado_final;
                    } else {
                        // ERRO -> não é um lexema válido (sem o ultimo número)
                        erro = true;
                        if (cin.eof())
                            showError(INVALID_EOF, lex);
                        else
                            showError(INVALID_LEX, lex);
                    }
                    break;

                // Se não entrar em nenhum dos casos, não é um lexema válido para a linguagem
                default:
                    lex += c;
                    erro = true;
                    showError(INVALID_LEX, lex);
            }

        }

        // Caractere inválido
        else {
            erro = true;
            showError(INVALID_CHAR, lex);
        }
    }

    if (erro)
        return "-1";
    else
        return lex;
}

/** Analisador Sintatico **/

// Assinatura dos procedimentos
void S();
void Dec();
void Comandos();
void Exp(int*, int*, int*);
void ExpS(int*, int*, int*);
void T(int*, int*, int*);
void F(int*, int*, int*);

/** Procedimento casaToken: Verifica se o token recebido é igual ao esperado na linguagem **/
void casaToken(int token) {
    if (token == registroLexico.token) {
        // pega proximo token apenas se nao eh fim do arquivo
        if (token != TK_EOF)
            getNextToken();
    }
    else {  // Não era o token esperado, mostrar erro.
        if (registroLexico.token == TK_EOF)
            showError(INVALID_EOF, "");
        else
            showError(INVALID_TOKEN, registroLexico.lexema);
    }
}


/* Procedimento S (primeiro da gramatica)
  S -> {Dec | Comandos} EOF
*/
void S(){
    // Enquanto token pertencer ao first de Dec ou Comandos
    while ( registroLexico.token == TK_INT || registroLexico.token == TK_FLOAT || registroLexico.token == TK_CHAR || registroLexico.token == TK_STRING || registroLexico.token == TK_BOOLEAN ||
            registroLexico.token == TK_CONST || registroLexico.token == TK_ID || registroLexico.token == TK_READLN || registroLexico.token == TK_WRITE || registroLexico.token == TK_WRITELN ||
            registroLexico.token == TK_IF || registroLexico.token == TK_WHILE || registroLexico.token == TK_PONTO_E_VIRGULA){
                
                // first de Dec
                if(registroLexico.token == TK_INT || registroLexico.token == TK_FLOAT || registroLexico.token == TK_CHAR || registroLexico.token == TK_STRING || registroLexico.token == TK_BOOLEAN || registroLexico.token == TK_CONST){
                    Dec();
                }
                else Comandos();
    }// fim while

    // fim do programa
    casaToken(TK_EOF);
}// fim S()


/* Procedimento Dec
  Dec -> 
  const id = [-] (constante | true | false) |
  (int | float | char | string) id [:= [-] constante] {, id [:= [-] constante]} |
  boolean id [:= (true | false)] {, id [:= (true | false)]}
*/
void Dec() {
    RegLex rl_const; // regLex auxiliar p/ salvar copia do RL atual
    bool negativo = false; // flag que indica se houve o token '-'
    int end_aux;
    
    // const id = [-] (constante | true | false)
    if(registroLexico.token == TK_CONST){
        casaToken(TK_CONST);
        string lex_id = registroLexico.lexema;
        casaToken(TK_ID);

        Simbolo simb_id = tab_simbolos.pesquisar(lex_id);

        // checa se id ja foi declarado
        if (simb_id.classe == CLASSE_VAZIA) {
            simb_id.classe = CLASSE_CONSTANTE; // id ainda nao declarado
        } 
        // id ja declarado
        else showError(DUPLICATE_ID, lex_id);

        casaToken(TK_IGUAL  );

        if(registroLexico.token == TK_MENOS){
            casaToken(TK_MENOS);
            negativo = true;
        }

        rl_const = registroLexico;

        if (registroLexico.token == TK_CONST)
            casaToken(TK_CONST);
        else if (registroLexico.token == TK_VERDADEIRO)
            casaToken(TK_VERDADEIRO);
        else
            casaToken(TK_FALSO);

        if (negativo) {
            // ERRO se a constante nao for um numero
            if (rl_const.tipo != TIPO_INTEIRO && rl_const.tipo != TIPO_REAL){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // atribui tipo ao simbolo
            else if (rl_const.tipo == TIPO_INTEIRO){
                simb_id.tipo = TIPO_INTEIRO;
            }
            else simb_id.tipo = TIPO_REAL;
        } 
        // nao houve sinal negativo:
        else if (rl_const.tipo == TIPO_INTEIRO) simb_id.tipo = TIPO_INTEIRO;
        else if (rl_const.tipo == TIPO_REAL) simb_id.tipo = TIPO_REAL;
        else if (rl_const.tipo == TIPO_STRING) simb_id.tipo = TIPO_STRING;
        else if (rl_const.tipo == TIPO_CARACTER) simb_id.tipo = TIPO_CARACTER;
        else if (rl_const.tipo == TIPO_BOOLEAN) simb_id.tipo = TIPO_BOOLEAN;
        else showError(INCOMPATIBLE_TYPES, "");
        
        codigo_asm.append("\nsection .data \t\t; seção de dados\n");
        // id eh numero (int ou float)
        if (simb_id.tipo == TIPO_INTEIRO || simb_id.tipo == TIPO_REAL){
            // adiciona 0 se o numero comecar com .
            if (rl_const.lexema[0] == '.') rl_const.lexema = '0' + rl_const.lexema;

            if(negativo){
                codigo_asm.append("\tdd -" + rl_const.lexema + " \t\t; reserva 4 bytes e escreve a constante\n");
            } else codigo_asm.append("\tdd " + rl_const.lexema + " \t\t; reserva 4 bytes e escreve a constante\n");
            end_aux = novo_dado(4); // salva endereco disponivel e atualiza proximo endereco disponivel
        }
        // id eh caractere ou booleano
        else if (simb_id.tipo == TIPO_CARACTER || simb_id.tipo == TIPO_BOOLEAN){
            codigo_asm.append("\tdb " + rl_const.lexema + "\t\t; reserva 1 byte e atribui valor\n");
            end_aux = novo_dado(1);
        }
        // id eh string
        else{
            codigo_asm.append("\tdb " + rl_const.lexema + ", 0 \t\t; reserva tamanho da string + indicador de fim da string\n");
            end_aux = novo_dado(rl_const.tamanho+1); // +1 em funcao do ',0' (indicador de fim da string)
            simb_id.tamanho = rl_const.tamanho;
        }
        codigo_asm.append("\nsection .text \t\t; voltando p/ seção de código\n");
        simb_id.endereco = end_aux;
        // atualiza TS
        tab_simbolos.update(lex_id, simb_id);
    }
   
    // (int | float | char | string) id [:= [-] constante] {, id [:= [-] constante]}
    else if(registroLexico.token == TK_INT || registroLexico.token == TK_FLOAT || registroLexico.token == TK_CHAR || registroLexico.token == TK_STRING){
        int end_aux, tipo_id = -999; // auxiliares
        
        if (registroLexico.token == TK_INT){
            tipo_id = TIPO_INTEIRO;
            casaToken(TK_INT);
        }
        else if(registroLexico.token == TK_FLOAT){
            tipo_id = TIPO_REAL;
            casaToken(TK_FLOAT);
        }
        else if(registroLexico.token == TK_CHAR){
            tipo_id = TIPO_CARACTER;
            casaToken(TK_CHAR);
        }
        else{
            tipo_id = TIPO_STRING;
            casaToken(TK_STRING);
        }
        
        string lex_id = registroLexico.lexema;
        casaToken(TK_ID);

        Simbolo simb_id = tab_simbolos.pesquisar(lex_id);
        // checa se id ja foi declarado
        if (simb_id.classe == CLASSE_VAZIA) {
            // id ainda nao declarado
            simb_id.classe = CLASSE_VARIAVEL;
            simb_id.tipo = tipo_id;

        }
        // id ja declarado
        else showError(DUPLICATE_ID, lex_id);

        // faz reserva de memoria p/ id declarado
        codigo_asm.append("\nsection .data \t\t; seção de dados\n");
        // id tipo int ou float
        if (simb_id.tipo == TIPO_INTEIRO || simb_id.tipo == TIPO_REAL) {        
            end_aux = novo_dado(4);
            codigo_asm.append("\tresd 1 \t\t; reserva 4 bytes\n");
        }
        // id tipo char
        else if (simb_id.tipo == TIPO_CARACTER) {
            end_aux = novo_dado(1);
            codigo_asm.append("\tresb 1 \t\t; reserva 1 byte\n"); 
        }
        // id tipo string
        else {
            end_aux = novo_dado(256);
            codigo_asm.append("\tresb 100h \t\t; reserva 256 bytes\n"); 
        }
        codigo_asm.append("\nsection .text \t\t; voltando p/ seção de código\n");

        simb_id.endereco = end_aux;
        simb_id.tamanho = 0;
        tab_simbolos.update(lex_id, simb_id); // atualiza TS


        if (registroLexico.token == TK_ATRIBUICAO){
            casaToken(TK_ATRIBUICAO);

            if(registroLexico.token == TK_MENOS){
                casaToken(TK_MENOS);
                // ERRO se a constante nao for um numero
                if (simb_id.tipo != TIPO_INTEIRO && simb_id.tipo != TIPO_REAL){
                    showError(INCOMPATIBLE_TYPES, "");
                }
                negativo = true;
            }
            // rl_const = {};
            // salva copia do reg lexico antes de casar o prox token
            rl_const = registroLexico;
            casaToken(TK_CONST);


            // ERRO se nao for numero e possuir o token '-'
            if ((rl_const.tipo != TIPO_INTEIRO && rl_const.tipo != TIPO_REAL) && negativo){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // ERRO se o id (simb_id) for float e a constante (rl_const) nao for um numero
            else if (simb_id.tipo == TIPO_REAL){
                if (rl_const.tipo != TIPO_INTEIRO && rl_const.tipo != TIPO_REAL){
                    showError(INCOMPATIBLE_TYPES, "");
                }
            }
            // se id n eh float, ERRO se os tipos forem diferentes
            else if (simb_id.tipo != rl_const.tipo){
                showError(INCOMPATIBLE_TYPES, "");
            }

            // id inteiro
            if(simb_id.tipo == TIPO_INTEIRO){
                codigo_asm.append("\tmov eax," + rl_const.lexema + " \t\t; carrega a constante em eax\n");
                if (negativo) {
                    codigo_asm.append("\tneg eax \t\t; inverte sinal de eax\n");
                }
                codigo_asm.append("\tmov [qword M+" + int2hex(simb_id.endereco) + "],eax \t\t; id recebe o valor da constante\n");
            }
            // id float
            else if(simb_id.tipo == TIPO_REAL){
                codigo_asm.append("\nsection .data \t\t; seção de dados\n");
                // adiciona 0 se o numero comecar com .
                if (rl_const.lexema[0] == '.') rl_const.lexema = '0' + rl_const.lexema;

                if(negativo){
                    codigo_asm.append("\tdd -" + rl_const.lexema + " \t\t; reserva 4 bytes e escreve a constante\n");
                } else codigo_asm.append("\tdd " + rl_const.lexema + " \t\t; reserva 4 bytes e escreve a constante\n");

                codigo_asm.append("\nsection .text \t\t; voltando p/ seção de código\n");
                int end_float = novo_dado(4);
                codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_float) + "] \t\t; carrega a constante em xmm0\n");
                codigo_asm.append("\tmovss [qword M+" + int2hex(simb_id.endereco) + "],xmm0 \t\t; id recebe o valor da constante\n");
            }
            // id char
            else if (simb_id.tipo == TIPO_CARACTER){
                codigo_asm.append("\tmov al," + rl_const.lexema + " \t\t; carrega a constante em eax\n");
                codigo_asm.append("\tmov [qword M+" + int2hex(simb_id.endereco) + "], al \t\t; id recebe o valor da constante\n");
            }
            // id string
            else{
                codigo_asm.append("\nsection .data \t\t; seção de dados\n");
                codigo_asm.append("\tdb " + rl_const.lexema + ", 0 \t\t; reserva tamanho da string + indicador de fim da string\n");
                codigo_asm.append("\nsection .text \t\t; voltando p/ seção de código\n");

                int end_str = novo_dado(rl_const.tamanho+1); // +1 em funcao do ',0' (indicador de fim da string)
                simb_id.tamanho = rl_const.tamanho;
                tab_simbolos.update(lex_id, simb_id); // atualiza TS

                codigo_asm.append("\tmov rsi,qword M+" + int2hex(end_str) + "\t\t; carrega endereco da constante em rsi\n");
                codigo_asm.append("\tmov rdi,qword M+" + int2hex(simb_id.endereco) + "\t\t; carrega endereco de id em rdi\n");
                string next_char = novo_rotulo();
                codigo_asm.append(next_char + ": \t\t; rotulo p/ ler proximo char da str\n");
                codigo_asm.append("\tmov al, [rsi] \t\t; guarda o char atual da constante\n");
                codigo_asm.append("\tmov [rdi], al \t\t; carrega esse char em id\n");
                codigo_asm.append("\tadd rsi, 1 \t\t; avanca p/ proximo char da constante\n");
                codigo_asm.append("\tadd rdi, 1 \t\t; avanca p/ proximo char de id\n");
                codigo_asm.append("\tcmp al,0 \t\t; checa se eh fim da str\n");
                codigo_asm.append("\tjne " + next_char + "\t\t; se nao eh fim, continua o loop\n");
            }

        }// fim if
        
        while(registroLexico.token == TK_VIRGULA){
            casaToken(TK_VIRGULA);
            lex_id = registroLexico.lexema;
            casaToken(TK_ID);

            simb_id = tab_simbolos.pesquisar(lex_id);

            // checa se id ja foi declarado
            if (simb_id.classe == CLASSE_VAZIA) {
                // id ainda nao declarado
                simb_id.classe = CLASSE_VARIAVEL;
                simb_id.tipo = tipo_id;
                tab_simbolos.update(lex_id, simb_id); // atualiza TS
            } 
            // id ja declarado
            else showError(DUPLICATE_ID, lex_id);

            // faz reserva de memoria p/ id declarado
            codigo_asm.append("\nsection .data \t\t; seção de dados\n");
            // id tipo int ou float
            if (simb_id.tipo == TIPO_INTEIRO || simb_id.tipo == TIPO_REAL) {        
                end_aux = novo_dado(4);
                codigo_asm.append("\tresd 1 \t\t; reserva 4 bytes\n");
            }
            // id tipo char
            else if (simb_id.tipo == TIPO_CARACTER) {
                end_aux = novo_dado(1);
                codigo_asm.append("\tresb 1 \t\t; reserva 1 byte\n"); 
            }
            // id tipo string
            else {
                end_aux = novo_dado(256);
                codigo_asm.append("\tresb 100h \t\t; reserva 256 bytes\n"); 
            }
            codigo_asm.append("\nsection .text \t\t; voltando p/ seção de código\n");

            simb_id.endereco = end_aux;
            simb_id.tamanho = 0;
            tab_simbolos.update(lex_id, simb_id); // atualiza TS


            negativo = false;
            if(registroLexico.token == TK_ATRIBUICAO){
                casaToken(TK_ATRIBUICAO);

                if (registroLexico.token == TK_MENOS){ 
                    casaToken(TK_MENOS);

                    // ERRO se id nao for numerico
                    if (simb_id.tipo != TIPO_INTEIRO && simb_id.tipo != TIPO_REAL){
                        showError(INCOMPATIBLE_TYPES, "");
                    }
                    negativo = true;
                }

                // copia reglex
                rl_const = registroLexico;
                casaToken(TK_CONST);

                // ERRO se nao for numero e possuir o token '-'
                if ((rl_const.tipo != TIPO_INTEIRO && rl_const.tipo != TIPO_REAL) && negativo){
                    showError(INCOMPATIBLE_TYPES, "");
                }
                // ERRO se o id (simb_id) for float e a constante (rl_const) nao for um numero
                else if (simb_id.tipo == TIPO_REAL){
                    if (rl_const.tipo != TIPO_INTEIRO && rl_const.tipo != TIPO_REAL){
                        showError(INCOMPATIBLE_TYPES, "");
                    }
                }
                // se id n eh float, ERRO se os tipos forem diferentes
                else if (simb_id.tipo != rl_const.tipo){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                // id inteiro
                if(simb_id.tipo == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax," + rl_const.lexema + " \t\t; carrega a constante em eax\n");
                    if (negativo) {
                        codigo_asm.append("\tneg eax \t\t; inverte sinal de eax\n");
                    }
                    codigo_asm.append("\tmov [qword M+" + int2hex(simb_id.endereco) + "],eax \t\t; id recebe o valor da constante\n");
                }
                // id float
                else if(simb_id.tipo == TIPO_REAL){
                    codigo_asm.append("\nsection .data \t\t; seção de dados\n");
                    // adiciona 0 se o numero comecar com .
                    if (rl_const.lexema[0] == '.') rl_const.lexema = '0' + rl_const.lexema;

                    if(negativo){
                        codigo_asm.append("\tdd -" + rl_const.lexema + " \t\t; reserva 4 bytes e escreve a constante\n");
                    } else codigo_asm.append("\tdd " + rl_const.lexema + " \t\t; reserva 4 bytes e escreve a constante\n");
                    codigo_asm.append("\nsection .text \t\t; voltando p/ seção de código\n");
                    int end_float = novo_dado(4);
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_float) + "] \t\t; carrega a constante em xmm0\n");
                    codigo_asm.append("\tmovss [qword M+" + int2hex(simb_id.endereco) + "],xmm0 \t\t; id recebe o valor da constante\n");
                }
                // id char
                else if (simb_id.tipo == TIPO_CARACTER){
                    codigo_asm.append("\tmov al," + rl_const.lexema + " \t\t; carrega a constante em eax\n");
                    codigo_asm.append("\tmov [qword M+" + int2hex(simb_id.endereco) + "], al \t\t; id recebe o valor da constante\n");
                }
                // id string
                else{
                    codigo_asm.append("\nsection .data \t\t; seção de dados\n");
                    codigo_asm.append("\tdb " + rl_const.lexema + ", 0 \t\t; reserva tamanho da string + indicador de fim da string\n");
                    codigo_asm.append("\nsection .text \t\t; voltando p/ seção de código\n");

                    int end_str = novo_dado(rl_const.tamanho+1); // +1 em funcao do ',0' (indicador de fim da string)
                    simb_id.tamanho = rl_const.tamanho;
                    tab_simbolos.update(lex_id, simb_id); // atualiza TS

                    codigo_asm.append("\tmov rsi,qword M+" + int2hex(end_str) + "\t\t; carrega endereco da constante em rsi\n");
                    codigo_asm.append("\tmov rdi,qword M+" + int2hex(simb_id.endereco) + "\t\t; carrega endereco de id em rdi\n");
                    string next_char = novo_rotulo();
                    codigo_asm.append(next_char + ": \t\t; rotulo p/ ler proximo char da str\n");
                    codigo_asm.append("\tmov al, [rsi] \t\t; guarda o char atual da constante\n");
                    codigo_asm.append("\tmov [rdi], al \t\t; carrega esse char em id\n");
                    codigo_asm.append("\tadd rsi, 1 \t\t; avanca p/ proximo char da constante\n");
                    codigo_asm.append("\tadd rdi, 1 \t\t; avanca p/ proximo char de id\n");
                    codigo_asm.append("\tcmp al,0 \t\t; checa se eh fim da str\n");
                    codigo_asm.append("\tjne " + next_char + "\t\t; se nao eh fim, continua o loop\n");
                }
            }
        }// fim while
    }
    
    // boolean id [:= (true | false)] {, id [:= (true | false)]}
    else {
        casaToken(TK_BOOLEAN);
        string lex_id = registroLexico.lexema;
        casaToken(TK_ID);
        // guarda o simbolo da TS
        Simbolo simb_id = tab_simbolos.pesquisar(lex_id);

        /* (38) */
        // checa se id ja foi declarado
        if (simb_id.classe == CLASSE_VAZIA){
            // id ainda nao declarado
            simb_id.classe = CLASSE_VARIAVEL;
            simb_id.tipo = TIPO_BOOLEAN;

        }
        // id ja declarado
        else showError(DUPLICATE_ID, lex_id);

        // faz reserva de memoria p/ o booleano declarado
        codigo_asm.append("\nsection .data \t\t; seção de dados\n");
        end_aux = novo_dado(1);
        codigo_asm.append("\tresb 1 \t\t; reserva 1 byte\n"); 
        codigo_asm.append("\nsection .text \t\t; voltando p/ seção de código\n");

        simb_id.endereco = end_aux;
        simb_id.tamanho = 0;
        tab_simbolos.update(lex_id, simb_id); // atualiza TS

        if(registroLexico.token == TK_ATRIBUICAO){
            casaToken(TK_ATRIBUICAO);
            rl_const = registroLexico;

            if (registroLexico.token == TK_VERDADEIRO){
                casaToken(TK_VERDADEIRO);
            }
            else if(registroLexico.token == TK_FALSO){ 
                casaToken(TK_FALSO);
            }
            else{
                showError(INCOMPATIBLE_TYPES, "");
            }
        
            codigo_asm.append("\tmov al," + rl_const.lexema + " \t\t; carrega a constante em eax\n");
            codigo_asm.append("\tmov [qword M+" + int2hex(simb_id.endereco) + "], al \t\t; id recebe o valor da constante\n");
        }


        while(registroLexico.token == TK_VIRGULA) {
            casaToken(TK_VIRGULA);
            string lex_id = registroLexico.lexema;
            casaToken(TK_ID);
            // guarda simbolo da TS
            Simbolo simb_id = tab_simbolos.pesquisar(lex_id);

            // checa se id ja foi declarado
            if (simb_id.classe == CLASSE_VAZIA){
                // id ainda nao declarado
                simb_id.classe = CLASSE_VARIAVEL;
                simb_id.tipo = TIPO_BOOLEAN;
                tab_simbolos.update(lex_id, simb_id); // atualiza TS
            }
            // id ja declarado
            else showError(DUPLICATE_ID, lex_id);

            // faz reserva de memoria p/ o booleano declarado
            codigo_asm.append("\nsection .data \t\t; seção de dados\n");
            end_aux = novo_dado(1);
            codigo_asm.append("\tresb 1 \t\t; reserva 1 byte\n"); 
            codigo_asm.append("\nsection .text \t\t; voltando p/ seção de código\n");

            simb_id.endereco = end_aux;
            simb_id.tamanho = 0;
            tab_simbolos.update(lex_id, simb_id); // atualiza TS

            if(registroLexico.token == TK_ATRIBUICAO){
                casaToken(TK_ATRIBUICAO);
                rl_const = registroLexico;
                if (registroLexico.token == TK_VERDADEIRO){
                    casaToken(TK_VERDADEIRO);
                }
                else if(registroLexico.token == TK_FALSO){ 
                    casaToken(TK_FALSO);
                }
                else{
                    showError(INCOMPATIBLE_TYPES, "");
                }

                codigo_asm.append("\tmov al," + rl_const.lexema + " \t\t; carrega a constante em eax\n");
                codigo_asm.append("\tmov [qword M+" + int2hex(simb_id.endereco) + "], al \t\t; id recebe o valor da constante\n");
            }
        }// fim while
    }

    // ????
    casaToken(TK_PONTO_E_VIRGULA);
}// fim Dec()

/* Procedimento Comandos
* Comandos -> 
  (if "("Exp")" ( Comandos | "{" {Comandos} "}" ) [ else ( Comandos | "{" {Comandos} "}" ) ] | 
  while "("Exp")" ( Comandos | "{" {Comandos} "}" )) | 
  [ id [ "[" Exp "]" ] := Exp | readln "(" id ")" | (write | writeln) "(" Exp {, Exp} ")" ] ;
*/
void Comandos(){
    int tipo_Exp, tamanho_Exp, end_Exp;

    // while e if
    if (registroLexico.token == TK_IF || registroLexico.token == TK_WHILE){

        // if "("Exp")" ( Comandos | "{" {Comandos} "}" ) [ else ( Comandos | "{" {Comandos} "}" ) ]
        if (registroLexico.token == TK_IF){
            casaToken(TK_IF);
            casaToken(TK_ABRE_PARENTESE);
            string rot_falso = novo_rotulo();
            string rot_fim = novo_rotulo();
            // limpa area de temporarios
            count_temporarios = 0;
            Exp(&tipo_Exp, &tamanho_Exp, &end_Exp);
            casaToken(TK_FECHA_PARENTESE);

            // ERRO se o tipo n for logico
            if (tipo_Exp != TIPO_BOOLEAN){
                showError(INCOMPATIBLE_TYPES, "");
            }

            codigo_asm.append("\tmov eax, [qword M+" +int2hex(end_Exp) + "]\t\t; carrega [Exp.end] em eax\n");
            codigo_asm.append("\tcmp eax,1 \t\t; faz o teste\n");
            codigo_asm.append("\tjne " + rot_falso + "\t\t; se teste deu falso, jump p/ rot_falso\n");
            
            if(registroLexico.token == TK_ABRE_CHAVE){
                casaToken(TK_ABRE_CHAVE);
                // Enquanto token pertencer ao first de Comandos()
                while ( registroLexico.token == TK_IF || registroLexico.token == TK_WHILE ||
                        registroLexico.token == TK_ID || registroLexico.token == TK_READLN ||
                        registroLexico.token == TK_WRITE || registroLexico.token == TK_WRITELN ||
                        registroLexico.token == TK_PONTO_E_VIRGULA){
                            Comandos();                            
                }
                casaToken(TK_FECHA_CHAVE);
                codigo_asm.append("\tjmp " + rot_fim + "\t\t; jump p/ rot_fim (fim do if)\n");
            } else{
                Comandos();
                codigo_asm.append("\tjmp " + rot_fim + "\t\t; jump p/ rot_fim (fim do if)\n");
            }// fim if-else

            codigo_asm.append(rot_falso + ": \t\t; rot_falso\n");

            if(registroLexico.token == TK_ELSE){
                if(registroLexico.token == TK_ABRE_CHAVE){
                    casaToken(TK_ABRE_CHAVE);
                    // Enquanto token pertencer ao first de Comandos()
                    while ( registroLexico.token == TK_IF || registroLexico.token == TK_WHILE ||
                            registroLexico.token == TK_ID || registroLexico.token == TK_READLN ||
                            registroLexico.token == TK_WRITE || registroLexico.token == TK_WRITELN ||
                            registroLexico.token == TK_PONTO_E_VIRGULA){
                                Comandos();                            
                    }
                    casaToken(TK_FECHA_CHAVE);
                } else{
                    Comandos();
                }// fim if-else

            }// fim if
            codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
        }// fim da producao de teste (if)

        // while "("Exp")" ( Comandos | "{" {Comandos} "}" ) 
        else{
            casaToken(TK_WHILE);
            casaToken(TK_ABRE_PARENTESE);

            string rot_inicio = novo_rotulo();
            string rot_fim = novo_rotulo();
            codigo_asm.append(rot_inicio + ": \t\t; rot_inicio\n");

            count_temporarios = 0; // limpa area de temporarios
            Exp(&tipo_Exp, &tamanho_Exp, &end_Exp);
            
            casaToken(TK_FECHA_PARENTESE);
            
            // ERRO se o tipo n for logico
            if (tipo_Exp != TIPO_BOOLEAN){
                showError(INCOMPATIBLE_TYPES, "");
            } 

            codigo_asm.append("\tmov eax, [qword M+" +int2hex(end_Exp) + "]\t\t; carrega [Exp.end] em eax\n");
            codigo_asm.append("\tcmp eax,1 \t\t; faz o teste\n");
            codigo_asm.append("\tjne " + rot_fim+ "\t\t; se teste deu falso, jump p/ rot_fim\n");

            if(registroLexico.token == TK_ABRE_CHAVE){
                casaToken(TK_ABRE_CHAVE);
                // Enquanto token pertencer ao first de Comandos()
                while ( registroLexico.token == TK_IF || registroLexico.token == TK_WHILE ||
                        registroLexico.token == TK_ID || registroLexico.token == TK_READLN ||
                        registroLexico.token == TK_WRITE || registroLexico.token == TK_WRITELN ||
                        registroLexico.token == TK_PONTO_E_VIRGULA){
                            Comandos();                            
                }
                casaToken(TK_FECHA_CHAVE);
            } else{
                Comandos();
            }// fim if-else

            codigo_asm.append("\tjmp " + rot_inicio + "\t\t; jump p/ rot_inicio\n");
            codigo_asm.append(rot_fim+ ": \t\t; rot_fim\n");

        }// fim da producao de repeticao (while)
    
    }// fim das producoes de teste e repeticao

    // producoes opcionais (seguidas de ;)
    else{
        string lex_id;
        Simbolo simb_id;
        // id [ "[" Exp "]" ] := Exp
        if(registroLexico.token == TK_ID){
            bool is_char_array = false;
            int posicao_string;

            lex_id = registroLexico.lexema; // guarda lexema do identificador
            casaToken(TK_ID);
            simb_id = tab_simbolos.pesquisar(lex_id); // pega o simbolo desse identificador na TS

            // checa se id eh constante
            if (simb_id.classe == CLASSE_CONSTANTE){
                showError(INCOMPATIBLE_CLASSES, lex_id);
            }
            // checa se o identificador ja foi declarado
            else if (simb_id.classe == CLASSE_VAZIA){
                showError(UNDECLARED_ID, lex_id);
            }

            // limpa area de temporarios
            count_temporarios = 0;
            posicao_string = 0;
            if(registroLexico.token == TK_ABRE_COLCHETE){
                casaToken(TK_ABRE_COLCHETE);

                // ERRO se o id nao for string
                if (simb_id.tipo != TIPO_STRING){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                Exp(&tipo_Exp, &tamanho_Exp, &end_Exp);

                // ERRO se o indice nao for um numero inteiro
                if (tipo_Exp != TIPO_INTEIRO){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                is_char_array = true;
                casaToken(TK_FECHA_COLCHETE);

                posicao_string = novo_temp(4);
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega a posicao da string em eax\n");
                codigo_asm.append("\tmov [qword M+" + int2hex(posicao_string) + "],eax \t\t; carrega eax em um temporario\n");
            }

            casaToken(TK_ATRIBUICAO);
            Exp(&tipo_Exp, &tamanho_Exp, &end_Exp);

            // checando os tipos de id (simb_id) e Exp (tipo_Exp)
            // se id for string
            if (simb_id.tipo == TIPO_STRING){
                // se houver indice da string (id[numero]), Exp DEVE ser char:
                if (is_char_array){
                    if (tipo_Exp != TIPO_CARACTER){
                        showError(INCOMPATIBLE_TYPES, "");
                    }
                }
                // se Exp n for uma string:
                else if (tipo_Exp != TIPO_STRING){
                    showError(INCOMPATIBLE_TYPES, "");
                }
            }
            // se id for char e Exp n for char:
            else if (simb_id.tipo == TIPO_CARACTER && tipo_Exp != TIPO_CARACTER){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // se id for int e Exp nao for int:
            else if (simb_id.tipo == TIPO_INTEIRO && tipo_Exp != TIPO_INTEIRO){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // se id for float e Exp n for numero:
            else if (simb_id.tipo == TIPO_REAL){
                if (tipo_Exp != TIPO_INTEIRO && tipo_Exp != TIPO_REAL){
                    showError(INCOMPATIBLE_TYPES, "");
                }
            }

            /* GERAÇÃO DE CÓDIGO */
            // id:=Exp
            // id inteiro, Exp inteiro
            if (simb_id.tipo == TIPO_INTEIRO){
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega [Exp.end] em eax\n");
                codigo_asm.append("\tmov [qword M+" + int2hex(simb_id.endereco) + "],eax \t\t; carrega id.end com Exp.end\n");
            }
            // id char, Exp char
            else if (simb_id.tipo == TIPO_CARACTER){
                codigo_asm.append("\tmov al, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega [Exp.end] em al\n");
                codigo_asm.append("\tmov [qword M+" + int2hex(simb_id.endereco) + "], al \t\t; carrega id.end com Exp.end\n");
            }
            // id float
            else if (simb_id.tipo == TIPO_REAL){
                // id float, Exp inteiro
                if (tipo_Exp == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega [Exp.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm0, eax \t\t; converte p/ float e guarda em xmm0\n");
                    codigo_asm.append("\tmovss [qword M+" + int2hex(simb_id.endereco) + "],xmm0 \t\t; carrega id.end com Exp.end\n");
                }
                // id float, Exp float
                else{
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega [Exp.end] em xmm0\n");
                    codigo_asm.append("\tmovss [qword M+" + int2hex(simb_id.endereco) + "],xmm0 \t\t; carrega id.end com Exp.end\n");
                }
            }
            // id string
            else{
                // id string, recebe um char na posicao Exp
                if(is_char_array) {
                    codigo_asm.append("\tmov rsi,qword M+" + int2hex(simb_id.endereco) + " \t\t; carrega [id.end] (inicio da string) em rsi\n");
                    codigo_asm.append("\tmov rbx,0 \t\t; rbx:=0\n");
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(posicao_string) + "] \t\t; rbx recebe posicao do char\n");
                    codigo_asm.append("\tadd rsi,rbx \t\t; soma inicio da string com posicao do char\n");
                    codigo_asm.append("\tmov al, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega [Exp.end] em al\n");
                    codigo_asm.append("\tmov [rsi], al \t\t; salva [Exp.end] em id[pos]\n");  
                }
                // id string, Exp string
                else{
                    codigo_asm.append("\tmov rsi,qword M+" + int2hex(end_Exp) + "\t\t; carrega endereco de Exp em rsi\n");
                    codigo_asm.append("\tmov rdi,qword M+" + int2hex(simb_id.endereco) + "\t\t; carrega endereco de id em rdi\n");
                    string next_char = novo_rotulo();
                    codigo_asm.append(next_char + ": \t\t; rotulo p/ ler proximo char da str\n");
                    codigo_asm.append("\tmov al, [rsi] \t\t; guarda o char atual de Exp\n");
                    codigo_asm.append("\tmov [rdi], al \t\t; carrega esse char em id\n");
                    codigo_asm.append("\tadd rsi, 1 \t\t; avanca p/ proximo char de Exp\n");
                    codigo_asm.append("\tadd rdi, 1 \t\t; avanca p/ proximo char de id\n");
                    codigo_asm.append("\tcmp al,0 \t\t; checa se eh fim da str\n");
                    codigo_asm.append("\tjne " + next_char + "\t\t; se nao eh fim, continua o loop\n");

                    simb_id.tamanho = tamanho_Exp; // tamanho do id agora eh o tamanho de Exp
                    tab_simbolos.update(lex_id, simb_id); // atualiza TS
                }
            }

        }// fim da producao de atribuicao
        
        // readln "(" id ")"
        else if(registroLexico.token == TK_READLN){
            int end_aux;
            
            casaToken(TK_READLN);
            casaToken(TK_ABRE_PARENTESE);
            lex_id = registroLexico.lexema; // guarda lexema do identificador p/ a busca na TS
            casaToken(TK_ID);

            simb_id = tab_simbolos.pesquisar(lex_id); // busca na TS

            // checa se foi declarado
            if (simb_id.classe == CLASSE_VAZIA){
                showError(UNDECLARED_ID, lex_id);
            }
            // checa se eh uma constante
            else if (simb_id.classe == CLASSE_CONSTANTE){
                showError(INCOMPATIBLE_CLASSES, lex_id);
            }

            casaToken(TK_FECHA_PARENTESE);

            /* GERAÇÃO DE CÓDIGO */
            count_temporarios = 0; // limpa area de temporarios
            end_aux = novo_temp(256); // tamanho maximo pois pode ser string
            codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; endereco do buffer\n");
            codigo_asm.append("\tmov rdx, 100h \t\t; tamanho do buffer\n");
            codigo_asm.append("\tmov rax, 0 \t\t; chamada para leitura\n");
            codigo_asm.append("\tmov rdi, 0 \t\t; leitura do teclado\n");
            codigo_asm.append("\tsyscall \t\t\n");
            
            // ler valor inteiro
            if(simb_id.tipo == TIPO_INTEIRO){
                codigo_asm.append("\tmov eax, 0 \t\t; acumulador:=0\n");
                codigo_asm.append("\tmov ebx, 0 \t\t; caractere\n");
                codigo_asm.append("\tmov ecx, 10 \t\t; base 10\n");
                codigo_asm.append("\tmov dx, 1 \t\t; sinal\n");
                codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; endereco do buffer\n");
                codigo_asm.append("\tmov bl, [rsi] \t\t; carrega caractere\n");
                codigo_asm.append("\tcmp bl, '-' \t\t; sinal - ?\n");
                string R0 = novo_rotulo();
                codigo_asm.append("\tjne " + R0 + " \t\t; se dif -, salta\n");
                codigo_asm.append("\tmov dx, -1 \t\t; senão, armazena -\n");
                codigo_asm.append("\tadd rsi, 1 \t\t; inc. ponteiro string\n");
                codigo_asm.append("\tmov bl, [rsi] \t\t; carrega caractere\n\n");

                codigo_asm.append(R0 + ": \t\t; R0\n");
                codigo_asm.append("\tpush dx \t\t; empilha sinal\n");
                codigo_asm.append("\tmov edx, 0 \t\t; reg. multiplicação\n\n");

                string R1 = novo_rotulo();
                codigo_asm.append(R1 + ": \t\t; R1\n");
                codigo_asm.append("\tcmp bl, 0Ah \t\t; verifica fim string\n");
                string R2 = novo_rotulo();
                codigo_asm.append("\tje " + R2 + " \t\t; salta se fim string\n");
                codigo_asm.append("\timul ecx \t\t; mult. eax por 10\n");
                codigo_asm.append("\tsub bl, '0' \t\t; converte caractere\n");
                codigo_asm.append("\tadd eax, ebx \t\t; soma valor caractere\n");
                codigo_asm.append("\tadd rsi, 1 \t\t; incrementa base\n");
                codigo_asm.append("\tmov bl, [rsi] \t\t; carrega caractere\n");
                codigo_asm.append("\tjmp " + R1 + " \t\t; loop\n\n");

                codigo_asm.append(R2 + ": \t\t; R2\n");
                codigo_asm.append("\tpop cx \t\t; desempilha sinal\n");
                codigo_asm.append("\tcmp cx, 0\t\t; verifica sinal\n");
                string R3 = novo_rotulo();
                codigo_asm.append("\tjg " + R3 + " \t\t\n");
                codigo_asm.append("\tneg eax \t\t; mult. sinal\n\n");

                codigo_asm.append(R3 + ": \t\t; R3\n");

                codigo_asm.append("\tmov [qword M+" +int2hex(simb_id.endereco) + "], eax\t\t; id recebe o novo int\n");
            }
            // ler valor real
            else if (simb_id.tipo == TIPO_REAL){
                codigo_asm.append("\tmov rax, 0 \t\t; acumul. parte int.\n");
                codigo_asm.append("\tsubss xmm0,xmm0 \t\t; acumul. parte frac.\n");
                codigo_asm.append("\tmov rbx, 0 \t\t; caractere\n");
                codigo_asm.append("\tmov rcx, 10 \t\t; base 10\n");
                codigo_asm.append("\tcvtsi2ss xmm3,rcx \t\t; base 10\n");
                codigo_asm.append("\tmovss xmm2,xmm3 \t\t;potência de 10\n");
                codigo_asm.append("\tmov rdx, 1 \t\t;sinal\n");     
                codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t;end. buffer\n");
                codigo_asm.append("\tmov bl, [rsi] \t\t;carrega caractere\n");
                codigo_asm.append("\tcmp bl, '-' \t\t;sinal - ?\n");
                string R0 = novo_rotulo();
                codigo_asm.append("\tjne " + R0 + " \t\t;se dif -, salta\n");
                codigo_asm.append("\tmov rdx, -1 \t\t;senao, armazena -\n");
                codigo_asm.append("\tadd rsi, 1 \t\t;inc. ponteiro stringcarrega caractere\n");
                codigo_asm.append("\tmov bl, [rsi] \t\t;carrega caractere\n");

                
                codigo_asm.append(R0 + ": \t\t; R0\n");
                codigo_asm.append("\tpush rdx \t\t;empilha sinal\n");
                codigo_asm.append("\tmov rdx, 0 \t\t;reg. multiplicação\n");
                
                string R1 = novo_rotulo();
                string R2 = novo_rotulo();
                string R3 = novo_rotulo();
                codigo_asm.append(R1 + ": \t\t; R1\n");
                codigo_asm.append("\tcmp bl, 0Ah \t\t;verifica fim string\n");
                codigo_asm.append("\tje " + R2 + " \t\t;salta se fim string\n");
                codigo_asm.append("\tcmp bl, '.' \t\t;verifica ponto\n");
                codigo_asm.append("\tje " + R3 + " \t\t;salta se salta se ponto\n");
                codigo_asm.append("\timul ecx \t\t;mult. eax por 10\n");
                codigo_asm.append("\tsub bl, '0' \t\t;converte caractere\n");
                codigo_asm.append("\tadd eax, ebx \t\t;soma valor caractere\n");
                codigo_asm.append("\tadd rsi, 1 \t\t;incrementa base\n");
                codigo_asm.append("\tmov bl, [rsi] \t\t;carrega caractere\n");
                codigo_asm.append("\tjmp " + R1 + " \t\t;loop\n");

                codigo_asm.append(R3 + ": \t\t; R3\n");
                codigo_asm.append("\t\t; calcula parte fracion'aria em xmm0\n");

                codigo_asm.append("\tadd rsi, 1 \t\t; inc. ponteiro string\n");
                codigo_asm.append("\tmov bl, [rsi] \t\t ; carrega caractere\n");
                codigo_asm.append("\tcmp bl, 0Ah \t\t; verifica fim string\n");
                codigo_asm.append("\tje " + R2 + " \t\t;salta se fim string\n");
                codigo_asm.append("\tsub bl, '0' \t\t; converte caractere\n");
                codigo_asm.append("\tcvtsi2ss xmm1, rbx \t\t;conv real\n");
                codigo_asm.append("\tdivss xmm1, xmm2 \t\t; transf casa decimal\n");
                codigo_asm.append("\taddss xmm0, xmm1 \t\t; soma acumul\n");
                codigo_asm.append("\tmulss xmm2,xmm3 \t\t; atualiza potencia\n");
                codigo_asm.append("\tjmp " + R3 + " \t\t;loop\n");
                
                codigo_asm.append(R2 + ": \t\t; R2\n");
                codigo_asm.append("\tcvtsi2ss xmm1, rax \t\t; conv parte inteira\n");
                codigo_asm.append("\taddss xmm0, xmm1\t\t; soma parte frac.\n");     
                codigo_asm.append("\tpop rcx \t\t; desempilha sinal\n");
                codigo_asm.append("\tcvtsi2ss xmm1, rcx\t\t; conv sinal\n");
                codigo_asm.append("\tmulss xmm0, xmm1 \t\t; mult. sinal\n");        

                codigo_asm.append("\tmovss [qword M+" +int2hex(simb_id.endereco) + "], xmm0\t\t; id recebe o novo float\n");
            }
            // ler valor do tipo caractere
            else if(simb_id.tipo == TIPO_CARACTER){
                codigo_asm.append("\tmov al, [M+" + int2hex(end_aux) + "] \t\t; carrega char no registrador temporario\n");
                codigo_asm.append("\tmov [qword M+" +int2hex(simb_id.endereco) + "], al\t\t; id recebe o novo char\n");
            }
            // ler valor tipo boolean
            else if(simb_id.tipo == TIPO_BOOLEAN){
                codigo_asm.append("\tmov al, [M+" + int2hex(end_aux) + "] \t\t; carrega booleano no registrador temporario\n");
                codigo_asm.append("\tmov [qword M+" +int2hex(simb_id.endereco) + "], al\t\t; id recebe o novo booleano\n");
            }
            // ler valor tipo string
            else {
                codigo_asm.append("\tmov rsi, qword M+" + int2hex(end_aux) + "\t\t; rsi recebe endereco do novo_temp\n");
                codigo_asm.append("\tmov rdi, qword M+" + int2hex(simb_id.endereco) + "\t\t; rdi recebe endereco de id\n");
                string next_char = novo_rotulo();
                codigo_asm.append(next_char + ": \t\t; next_char: rotulo p/ ler proximo char da str\n");
                codigo_asm.append("\tmov al, [rsi] \t\t; guarda o char atual da string lida\n");
                codigo_asm.append("\tmov [rdi], al \t\t; salva esse char no id\n");
                codigo_asm.append("\tadd rsi, 1 \t\t; proximo char (string lida)\n");
                codigo_asm.append("\tadd rdi, 1 \t\t; proximo char (id)\n");
                codigo_asm.append("\tcmp al, 0Ah \t\t; checa pelo 'Enter' (quebra de linha = fim da string)\n");
                codigo_asm.append("\tjne " + next_char + "\t\t; se não eh o fim, continua o loop (jump p/ next_char)\n");
                
                // leu string toda
                codigo_asm.append("\tsub rdi, 1 \t\t; retira ultimo byte lido da str (quebra de linha)\n");
                codigo_asm.append("\tmov bl, 0 \t\t; carrega delimitador de fim de string (0) em bl\n");
                codigo_asm.append("\tmov [rdi], bl \t\t; carrega bl no fim da string (troca quebra de linha pelo 0)\n");

                simb_id.tamanho = 255; // maior tamanho permitido (0 no fim da string)
                tab_simbolos.update(lex_id, simb_id);
            }

        } // fim da producao de leitura
        
        // (write | writeln) "(" Exp {, Exp} ")"
        else if(registroLexico.token == TK_WRITE || registroLexico.token == TK_WRITELN){
            int end_aux;
            bool writeln = false;

            if(registroLexico.token == TK_WRITE){
                casaToken(TK_WRITE);
            }
            else{
                casaToken(TK_WRITELN);
                writeln = true;
            }
            casaToken(TK_ABRE_PARENTESE);
            count_temporarios = 0; // limpa area de temporarios
            Exp(&tipo_Exp, &tamanho_Exp, &end_Exp);

            // o tipo nao pode ser logico
            if (tipo_Exp == TIPO_BOOLEAN){
                showError(INCOMPATIBLE_TYPES, "");
            }

            while(registroLexico.token == TK_VIRGULA){
                casaToken(TK_VIRGULA);
                // escrever valor inteiro
                if(tipo_Exp == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_Exp) + "] \t\t; inteiro a ser convertido\n");
                    end_aux = novo_temp(256);
                    codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t;end. string ou temp\n");
                    codigo_asm.append("\tmov rcx, 0 \t\t;contador pilha\n");
                    codigo_asm.append("\tmov rdi, 0 \t\t;tam. string convertido\n");
                    codigo_asm.append("\tcmp eax, 0 \t\t;verifica sinal\n");
                    string R0 = novo_rotulo();
                    codigo_asm.append("\tjge " + R0 + "\t\t;salta se número positivo\n");
                    codigo_asm.append("\tmov bl, '-' \t\t;senão, escreve sinal –\n");
                    codigo_asm.append("\tmov [rsi], bl \t\t\n");
                    codigo_asm.append("\tadd rsi, 1 \t\t;incrementa índice\n");
                    codigo_asm.append("\tadd rdi, 1 \t\t;incrementa tamanho\n");
                    codigo_asm.append("\tneg eax \t\t;toma módulo do número\n");

                    codigo_asm.append(R0 + ": \t\t\n");
                    codigo_asm.append("\tmov ebx, 10 \t\t;divisor\n");

                    string R1 = novo_rotulo();
                    codigo_asm.append(R1 + ": \t\t\n");
                    codigo_asm.append("\tadd rcx, 1 \t\t;incrementa contador\n");
                    codigo_asm.append("\tcdq \t\t;estende edx:eax p/ div.\n");
                    codigo_asm.append("\tidiv ebx \t\t;divide edx:eax por ebx\n");
                    codigo_asm.append("\tpush dx \t\t;empilha valor do resto\n");
                    codigo_asm.append("\tcmp eax, 0 \t\t;verifica se quoc. é 0\n");
                    codigo_asm.append("\tjne " + R1 + "\t\t;se não é 0, continua\n");
                    codigo_asm.append("\tadd rdi, rcx \t\t;atualiza tam. string\n");
                    
                    codigo_asm.append(";agora, desemp. os valores e escreve o string\n");

                    string R2 = novo_rotulo();
                    codigo_asm.append(R2 + ": \t\t\n");
                    codigo_asm.append("\tpop dx \t\t;desempilha valor\n");
                    codigo_asm.append("\tadd dl, '0' \t\t;transforma em caractere\n");
                    codigo_asm.append("\tmov [rsi], dl \t\t;escreve caractere\n");
                    codigo_asm.append("\tadd rsi, 1 \t\t;incrementa base\n");
                    codigo_asm.append("\tsub rcx, 1 \t\t;decrementa contador\n");
                    codigo_asm.append("\tcmp rcx, 0 \t\t;verifica pilha vazia\n");
                    codigo_asm.append("\tjne " + R2 + "\t\t;se não pilha vazia, loop\n");
                    

                    codigo_asm.append("\n;executa interrupção de saída\n\n");

                    codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                    codigo_asm.append("\tmov rdx, rdi\t\t; tamanho da string\n");
                    codigo_asm.append("\tmov rax, 1\t\t; chamada para saida\n");
                    codigo_asm.append("\tmov rdi, 1\t\t; saida para tela\n");
                    codigo_asm.append("\tsyscall \t\t; chama o kernel\n");
                }
                // escrever valor real
                else if(tipo_Exp == TIPO_REAL){
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_Exp) + "] \t\t; real a ser convertido\n");
                    end_aux = novo_temp(256);
                    codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t;end. temporário\n");
                    codigo_asm.append("\tmov rcx, 0 \t\t;contador pilha\n");
                    codigo_asm.append("\tmov rdi, 6 \t\t;precisao 6 casas compart\n");
                    codigo_asm.append("\tmov rbx, 10 \t\t;divisor\n");
                    codigo_asm.append("\tcvtsi2ss xmm2, rbx \t\t;divisor real\n");
                    codigo_asm.append("\tsubss xmm1, xmm1 \t\t;zera registrador\n");
                    codigo_asm.append("\tcomiss xmm0, xmm1 \t\t;verifica sinal\n");
                    string R0 = novo_rotulo();
                    codigo_asm.append("\tjae " + R0 + "\t\t;salta se número positivo\n");
                    codigo_asm.append("\tmov dl, '-' \t\t;senão, escreve sinal –\n");
                    codigo_asm.append("\tmov [rsi], dl \t\t\n");
                    codigo_asm.append("\tmov rdx, -1 \t\t; rdx := -1\n");
                    codigo_asm.append("\tcvtsi2ss xmm1, rdx \t\t;Converte para real\n");
                    codigo_asm.append("\tmulss xmm0, xmm1 \t\t;Toma módulo\n");
                    codigo_asm.append("\tadd rsi, 1 \t\t;incrementa índice\n");

                    codigo_asm.append(R0 + ": \t\t\n");
                    codigo_asm.append("\troundss xmm1, xmm0, 0b0011 \t\t;parte inteira xmm1\n");
                    codigo_asm.append("\tsubss xmm0, xmm1 \t\t;parte frac xmm0\n");
                    codigo_asm.append("\tcvtss2si rax, xmm1 \t\t;convertido para int\n");

                    codigo_asm.append(";converte parte inteira que está em rax\n");

                    string R1 = novo_rotulo();
                    codigo_asm.append(R1 + ": \t\t\n");
                    codigo_asm.append("\tadd rcx, 1 \t\t;incrementa contador\n");
                    codigo_asm.append("\tcdq \t\t;estende edx:eax p/ div.\n");
                    codigo_asm.append("\tidiv ebx \t\t;divide edx:eax por ebx\n");
                    codigo_asm.append("\tpush dx \t\t;empilha valor do resto\n");
                    codigo_asm.append("\tcmp eax, 0 \t\t;verifica se quoc. é 0\n");
                    codigo_asm.append("\tjne " + R1 + "\t\t;se não é 0, continua\n");
                    codigo_asm.append("\tsub rdi, rcx \t\t;decrementa precisao\n");

                    codigo_asm.append(";agora, desemp valores e escreve parte int\n");

                    string R2 = novo_rotulo();
                    codigo_asm.append(R2 + ": \t\t\n");
                    codigo_asm.append("\tpop dx \t\t;desempilha valor\n");
                    codigo_asm.append("\tadd dl, '0' \t\t;transforma em caractere\n");
                    codigo_asm.append("\tmov [rsi], dl \t\t;escreve caractere\n");
                    codigo_asm.append("\tadd rsi, 1 \t\t;incrementa base\n");
                    codigo_asm.append("\tsub rcx, 1 \t\t;decrementa contador\n");
                    codigo_asm.append("\tcmp rcx, 0 \t\t;verifica pilha vazia\n");
                    codigo_asm.append("\tjne " + R2 + "\t\t;se não pilha vazia, loop\n");

                    codigo_asm.append("\tmov dl, '.' \t\t;escreve ponto decimal\n");
                    codigo_asm.append("\tmov [rsi], dl \t\t;escreve ponto decimal\n");
                    codigo_asm.append("\tadd rsi, 1 \t\t;incrementa base\n");

                    codigo_asm.append(";converte parte fracionaria que está em xmm0\n");

                    string R3 = novo_rotulo();
                    string R4 = novo_rotulo();
                    codigo_asm.append(R3 + ": \t\t\n");
                    codigo_asm.append("\tcmp rdi, 0 \t\t;verifica precisao\n");
                    codigo_asm.append("\tjle " + R4 + " \t\t; terminou precisao ?\n");
                    codigo_asm.append("\tmulss xmm0,xmm2 \t\t;desloca para esquerda\n");
                    codigo_asm.append("\troundss xmm1,xmm0,0b0011 \t\t;parte inteira xmm1\n");
                    codigo_asm.append("\tsubss xmm0,xmm1 \t\t;atualiza xmm0\n");
                    codigo_asm.append("\tcvtss2si rdx, xmm1 \t\t;convertido para int\n");
                    codigo_asm.append("\tadd dl, '0' \t\t;transforma em caractere\n");
                    codigo_asm.append("\tmov [rsi], dl \t\t;escreve caractere\n");
                    codigo_asm.append("\tadd rsi, 1 \t\t;incrementa base\n");
                    codigo_asm.append("\tsub rdi, 1 \t\t;decrementa precisao\n");
                    codigo_asm.append("\tjmp " + R3 + "\t\t;loop\n");

                    codigo_asm.append("; impressão\n");
                    codigo_asm.append(R4 + ": \t\t\n");

                    codigo_asm.append("\tmov rdx, rsi \t\t;calc tam str convertido\n");
                    codigo_asm.append("\tmov rbx, M+" + int2hex(end_aux) + "\t\t;end. string ou temp\n");
                    codigo_asm.append("\tsub rdx, rbx \t\t;tam=rsi-M-buffer.end\n");
                    

                    codigo_asm.append("\n;executa interrupção de saída\n\n");

                    codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                    codigo_asm.append("\t\t\t; tamanho da string ja está em rdx\n");
                    codigo_asm.append("\tmov rax, 1\t\t; chamada para saida\n");
                    codigo_asm.append("\tmov rdi, 1\t\t; saida para tela\n");
                    codigo_asm.append("\tsyscall \t\t; chama o kernel\n");
                }
                // escreve char
                else if (tipo_Exp == TIPO_CARACTER){
                    end_aux = novo_temp(2); // 1 byte a mais p/ quebra de linha
                    codigo_asm.append("\tmov rcx,0 \t\t; zera contador\n");

                    codigo_asm.append("\tmov al, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega char no registrador temporario\n");
                    codigo_asm.append("\tmov [qword M+" +int2hex(end_aux) + "], al\t\t; novo_temp recebe o char\n");
                    codigo_asm.append("\tmov ecx,1 \t\t; cont:=1 byte\n");

                    
                    codigo_asm.append("\n;executa interrupção de saída\n\n");
                    
                    codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                    codigo_asm.append("\tmov rdx, rcx\t\t; tamanho do char\n");
                    codigo_asm.append("\tmov rax, 1\t\t; chamada para saida\n");
                    codigo_asm.append("\tmov rdi, 1\t\t; saida para tela\n");
                    codigo_asm.append("\tsyscall \t\t; chama o kernel\n");
                }
                // escreve boolean
                else if(tipo_Exp == TIPO_BOOLEAN){
                    codigo_asm.append("\tmov rax,0 \t\t; zera contador\n");
                    end_aux = novo_temp(2); // 1 byte a mais p/ quebra de linha

                    codigo_asm.append("\tmov al, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega char no registrador temporario\n");
                    codigo_asm.append("\tmov [qword M+" +int2hex(end_aux) + "], al\t\t; novo_temp recebe o char\n");
                    codigo_asm.append("\tmov rax,1 \t\t; cont:=1 byte\n");


                    codigo_asm.append("\n;executa interrupção de saída\n\n");

                    codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                    codigo_asm.append("\tmov rdx, rax\t\t; tamanho do booleano\n");
                    codigo_asm.append("\tmov rax, 1\t\t; chamada para saida\n");
                    codigo_asm.append("\tmov rdi, 1\t\t; saida para tela\n");
                    codigo_asm.append("\tsyscall \t\t; chama o kernel\n");
                }
                // escreve string
                else {
                    codigo_asm.append("\tmov rcx,0 \t\t; zera contador\n");
                    end_aux = novo_temp(tamanho_Exp+1); // 1 byte a mais p/ quebra de linha

                    codigo_asm.append("\tmov rsi, qword M+" + int2hex(end_Exp) + "\t\t; rsi recebe endereco de Exp\n");
                    codigo_asm.append("\tmov rdi, qword M+" + int2hex(end_aux) + "\t\t; rdi recebe endereco de novo_temp\n");
                    string next_char = novo_rotulo();
                    codigo_asm.append(next_char + ": \t\t; next_char: rotulo p/ ler proximo char da str\n");
                    codigo_asm.append("\tmov al, [rsi] \t\t; guarda o char atual da string\n");
                    codigo_asm.append("\tmov [rdi], al \t\t; salva esse char no temp\n");
                    codigo_asm.append("\tadd rcx, 1 \t\t; contador++\n");
                    codigo_asm.append("\tadd rsi, 1 \t\t; proximo char (Exp)\n");
                    codigo_asm.append("\tadd rdi, 1 \t\t; proximo char (temp)\n");
                    codigo_asm.append("\tcmp al, 0 \t\t; checa se eh fim da str\n");
                    codigo_asm.append("\tjne " + next_char + "\t\t; se não eh o fim, continua o loop (jump p/ next_char)\n");
                    codigo_asm.append("\tsub rcx, 1 \t\t; contador-- (remove o fim da str)\n");


                    codigo_asm.append("\n;executa interrupção de saída\n\n");
                    
                    codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                    codigo_asm.append("\tmov rdx, rcx\t\t; tamanho da str\n");
                    codigo_asm.append("\tmov rax, 1\t\t; chamada para saida\n");
                    codigo_asm.append("\tmov rdi, 1\t\t; saida para tela\n");
                    codigo_asm.append("\tsyscall \t\t; chama o kernel\n");
                }

                count_temporarios = 0; // limpa area de temporarios
                Exp(&tipo_Exp, &tamanho_Exp, &end_Exp);

                // Exp nao pode ser logico
                if (tipo_Exp == TIPO_BOOLEAN){
                    showError(INCOMPATIBLE_TYPES, "");
                }
            }// fim while

            casaToken(TK_FECHA_PARENTESE);

            // escrever valor inteiro
            if(tipo_Exp == TIPO_INTEIRO){
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_Exp) + "] \t\t; inteiro a ser convertido\n");
                end_aux = novo_temp(256);
                codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t;end. string ou temp\n");
                codigo_asm.append("\tmov rcx, 0 \t\t;contador pilha\n");
                codigo_asm.append("\tmov rdi, 0 \t\t;tam. string convertido\n");
                codigo_asm.append("\tcmp eax, 0 \t\t;verifica sinal\n");
                string R0 = novo_rotulo();
                codigo_asm.append("\tjge " + R0 + "\t\t;salta se número positivo\n");
                codigo_asm.append("\tmov bl, '-' \t\t;senão, escreve sinal –\n");
                codigo_asm.append("\tmov [rsi], bl \t\t\n");
                codigo_asm.append("\tadd rsi, 1 \t\t;incrementa índice\n");
                codigo_asm.append("\tadd rdi, 1 \t\t;incrementa tamanho\n");
                codigo_asm.append("\tneg eax \t\t;toma módulo do número\n");

                codigo_asm.append(R0 + ": \t\t\n");
                codigo_asm.append("\tmov ebx, 10 \t\t;divisor\n");

                string R1 = novo_rotulo();
                codigo_asm.append(R1 + ": \t\t\n");
                codigo_asm.append("\tadd rcx, 1 \t\t;incrementa contador\n");
                codigo_asm.append("\tcdq \t\t;estende edx:eax p/ div.\n");
                codigo_asm.append("\tidiv ebx \t\t;divide edx:eax por ebx\n");
                codigo_asm.append("\tpush dx \t\t;empilha valor do resto\n");
                codigo_asm.append("\tcmp eax, 0 \t\t;verifica se quoc. é 0\n");
                codigo_asm.append("\tjne " + R1 + "\t\t;se não é 0, continua\n");
                codigo_asm.append("\tadd rdi, rcx \t\t;atualiza tam. string\n");
                
                codigo_asm.append(";agora, desemp. os valores e escreve o string\n");

                string R2 = novo_rotulo();
                codigo_asm.append(R2 + ": \t\t\n");
                codigo_asm.append("\tpop dx \t\t;desempilha valor\n");
                codigo_asm.append("\tadd dl, '0' \t\t;transforma em caractere\n");
                codigo_asm.append("\tmov [rsi], dl \t\t;escreve caractere\n");
                codigo_asm.append("\tadd rsi, 1 \t\t;incrementa base\n");
                codigo_asm.append("\tsub rcx, 1 \t\t;decrementa contador\n");
                codigo_asm.append("\tcmp rcx, 0 \t\t;verifica pilha vazia\n");
                codigo_asm.append("\tjne " + R2 + "\t\t;se não pilha vazia, loop\n");
                
                // se o comando foi writeln, inserir quebra de linha no str antes de imprimi-lo
                if(writeln){
                    codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                    codigo_asm.append("\tadd rsi, rdi\t\t; inicio da string += tamanho da string\n");
                    codigo_asm.append("\tadd rdi, 1\t\t; tamanho da string += 1\n");
                    codigo_asm.append("\tmov al, 0xA\t\t; reg. al recebe a quebre de linha\n");
                    codigo_asm.append("\tmov [rsi], al\t\t; ultima posicao ([rsi]) recebe quebra de linha\n");
                }

                codigo_asm.append("\n;executa interrupção de saída\n\n");

                codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                codigo_asm.append("\tmov rdx, rdi\t\t; tamanho da string\n");
                codigo_asm.append("\tmov rax, 1\t\t; chamada para saida\n");
                codigo_asm.append("\tmov rdi, 1\t\t; saida para tela\n");
                codigo_asm.append("\tsyscall \t\t; chama o kernel\n");
            }
            // escrever valor real
            else if(tipo_Exp == TIPO_REAL){
                codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_Exp) + "] \t\t; real a ser convertido\n");
                end_aux = novo_temp(256);
                codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t;end. temporário\n");
                codigo_asm.append("\tmov rcx, 0 \t\t;contador pilha\n");
                codigo_asm.append("\tmov rdi, 6 \t\t;precisao 6 casas compart\n");
                codigo_asm.append("\tmov rbx, 10 \t\t;divisor\n");
                codigo_asm.append("\tcvtsi2ss xmm2, rbx \t\t;divisor real\n");
                codigo_asm.append("\tsubss xmm1, xmm1 \t\t;zera registrador\n");
                codigo_asm.append("\tcomiss xmm0, xmm1 \t\t;verifica sinal\n");
                string R0 = novo_rotulo();
                codigo_asm.append("\tjae " + R0 + "\t\t;salta se número positivo\n");
                codigo_asm.append("\tmov dl, '-' \t\t;senão, escreve sinal –\n");
                codigo_asm.append("\tmov [rsi], dl \t\t\n");
                codigo_asm.append("\tmov rdx, -1 \t\t; rdx := -1\n");
                codigo_asm.append("\tcvtsi2ss xmm1, rdx \t\t;Converte para real\n");
                codigo_asm.append("\tmulss xmm0, xmm1 \t\t;Toma módulo\n");
                codigo_asm.append("\tadd rsi, 1 \t\t;incrementa índice\n");

                codigo_asm.append(R0 + ": \t\t\n");
                codigo_asm.append("\troundss xmm1, xmm0, 0b0011 \t\t;parte inteira xmm1\n");
                codigo_asm.append("\tsubss xmm0, xmm1 \t\t;parte frac xmm0\n");
                codigo_asm.append("\tcvtss2si rax, xmm1 \t\t;convertido para int\n");

                codigo_asm.append(";converte parte inteira que está em rax\n");

                string R1 = novo_rotulo();
                codigo_asm.append(R1 + ": \t\t\n");
                codigo_asm.append("\tadd rcx, 1 \t\t;incrementa contador\n");
                codigo_asm.append("\tcdq \t\t;estende edx:eax p/ div.\n");
                codigo_asm.append("\tidiv ebx \t\t;divide edx:eax por ebx\n");
                codigo_asm.append("\tpush dx \t\t;empilha valor do resto\n");
                codigo_asm.append("\tcmp eax, 0 \t\t;verifica se quoc. é 0\n");
                codigo_asm.append("\tjne " + R1 + "\t\t;se não é 0, continua\n");
                codigo_asm.append("\tsub rdi, rcx \t\t;decrementa precisao\n");

                codigo_asm.append(";agora, desemp valores e escreve parte int\n");

                string R2 = novo_rotulo();
                codigo_asm.append(R2 + ": \t\t\n");
                codigo_asm.append("\tpop dx \t\t;desempilha valor\n");
                codigo_asm.append("\tadd dl, '0' \t\t;transforma em caractere\n");
                codigo_asm.append("\tmov [rsi], dl \t\t;escreve caractere\n");
                codigo_asm.append("\tadd rsi, 1 \t\t;incrementa base\n");
                codigo_asm.append("\tsub rcx, 1 \t\t;decrementa contador\n");
                codigo_asm.append("\tcmp rcx, 0 \t\t;verifica pilha vazia\n");
                codigo_asm.append("\tjne " + R2 + "\t\t;se não pilha vazia, loop\n");

                codigo_asm.append("\tmov dl, '.' \t\t;escreve ponto decimal\n");
                codigo_asm.append("\tmov [rsi], dl \t\t;escreve ponto decimal\n");
                codigo_asm.append("\tadd rsi, 1 \t\t;incrementa base\n");

                codigo_asm.append(";converte parte fracionaria que está em xmm0\n");

                string R3 = novo_rotulo();
                string R4 = novo_rotulo();
                codigo_asm.append(R3 + ": \t\t\n");
                codigo_asm.append("\tcmp rdi, 0 \t\t;verifica precisao\n");
                codigo_asm.append("\tjle " + R4 + " \t\t; terminou precisao ?\n");
                codigo_asm.append("\tmulss xmm0,xmm2 \t\t;desloca para esquerda\n");
                codigo_asm.append("\troundss xmm1,xmm0,0b0011 \t\t;parte inteira xmm1\n");
                codigo_asm.append("\tsubss xmm0,xmm1 \t\t;atualiza xmm0\n");
                codigo_asm.append("\tcvtss2si rdx, xmm1 \t\t;convertido para int\n");
                codigo_asm.append("\tadd dl, '0' \t\t;transforma em caractere\n");
                codigo_asm.append("\tmov [rsi], dl \t\t;escreve caractere\n");
                codigo_asm.append("\tadd rsi, 1 \t\t;incrementa base\n");
                codigo_asm.append("\tsub rdi, 1 \t\t;decrementa precisao\n");
                codigo_asm.append("\tjmp " + R3 + "\t\t;loop\n");

                codigo_asm.append("; impressão\n");
                codigo_asm.append(R4 + ": \t\t\n");

                codigo_asm.append("\tmov rdx, rsi \t\t;calc tam str convertido\n");
                codigo_asm.append("\tmov rbx, M+" + int2hex(end_aux) + "\t\t;end. string ou temp\n");
                codigo_asm.append("\tsub rdx, rbx \t\t;tam=rsi-M-buffer.end\n");
                
                if(writeln){
                    codigo_asm.append("\tmov dl, 0xA \t\t; reg. dl recebe quebra de linha\n");
                    codigo_asm.append("\tmov [rsi], dl \t\t; quebra de linha no final do float\n");
                    codigo_asm.append("\tadd rsi, 1 \t\t;incrementa base\n");
                }

                codigo_asm.append("\n;executa interrupção de saída\n\n");

                codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                codigo_asm.append("\t\t\t; tamanho da string ja está em rdx\n");
                codigo_asm.append("\tmov rax, 1\t\t; chamada para saida\n");
                codigo_asm.append("\tmov rdi, 1\t\t; saida para tela\n");
                codigo_asm.append("\tsyscall \t\t; chama o kernel\n");
            }
            // escreve char
            else if (tipo_Exp == TIPO_CARACTER){
                codigo_asm.append("\tmov rcx,0 \t\t; zera contador\n");
                end_aux = novo_temp(2); // 1 byte a mais p/ quebra de linha

                codigo_asm.append("\tmov al, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega char no registrador temporario\n");
                codigo_asm.append("\tmov [qword M+" +int2hex(end_aux) + "], al\t\t; novo_temp recebe o char\n");
                codigo_asm.append("\tmov ecx,1 \t\t; cont:=1 byte\n");

                if(writeln){
                    codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                    codigo_asm.append("\tadd rsi, rcx \t\t; soma contador ao inicio\n");
                    codigo_asm.append("\tmov al, 0xA \t\t; reg. al recebe quebra de linha\n");
                    codigo_asm.append("\tmov [rsi], al \t\t; quebra de linha no final do char\n");
                    codigo_asm.append("\tadd rcx, 1 \t\t; contador++ p/ n dar pau com a quebra de linha\n");
                }
                
                codigo_asm.append("\n;executa interrupção de saída\n\n");
                
                codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                codigo_asm.append("\tmov rdx, rcx\t\t; tamanho do char\n");
                codigo_asm.append("\tmov rax, 1\t\t; chamada para saida\n");
                codigo_asm.append("\tmov rdi, 1\t\t; saida para tela\n");
                codigo_asm.append("\tsyscall \t\t; chama o kernel\n");
            }
            // escreve boolean
            else if(tipo_Exp == TIPO_BOOLEAN){
                codigo_asm.append("\tmov rax,0 \t\t; zera contador\n");
                end_aux = novo_temp(2); // 1 byte a mais p/ quebra de linha

                codigo_asm.append("\tmov al, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega char no registrador temporario\n");
                codigo_asm.append("\tmov [qword M+" +int2hex(end_aux) + "], al\t\t; novo_temp recebe o char\n");
                codigo_asm.append("\tmov rax,1 \t\t; cont:=1 byte\n");

                if(writeln){
                    codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                    codigo_asm.append("\tadd rsi, rax \t\t; soma contador ao inicio\n");
                    codigo_asm.append("\tmov al, 0xA \t\t; reg. al recebe quebra de linha\n");
                    codigo_asm.append("\tmov [rsi], al \t\t; quebra de linha no final do char\n");
                    codigo_asm.append("\tadd rax, 1 \t\t; contador++ p/ n dar pau com a quebra de linha\n");
                }

                codigo_asm.append("\n;executa interrupção de saída\n\n");

                codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                codigo_asm.append("\tmov rdx, rax\t\t; tamanho do booleano\n");
                codigo_asm.append("\tmov rax, 1\t\t; chamada para saida\n");
                codigo_asm.append("\tmov rdi, 1\t\t; saida para tela\n");
                codigo_asm.append("\tsyscall \t\t; chama o kernel\n");
            }
            // escreve string
            else{
                codigo_asm.append("\tmov rcx,0 \t\t; zera contador\n");
                end_aux = novo_temp(tamanho_Exp+1); // 1 byte a mais p/ quebra de linha

                codigo_asm.append("\tmov rsi, qword M+" + int2hex(end_Exp) + "\t\t; rsi recebe endereco de Exp\n");
                codigo_asm.append("\tmov rdi, qword M+" + int2hex(end_aux) + "\t\t; rdi recebe endereco de novo_temp\n");
                string next_char = novo_rotulo();
                codigo_asm.append(next_char + ": \t\t; next_char: rotulo p/ ler proximo char da str\n");
                codigo_asm.append("\tmov al, [rsi] \t\t; guarda o char atual da string\n");
                codigo_asm.append("\tmov [rdi], al \t\t; salva esse char no temp\n");
                codigo_asm.append("\tadd rcx, 1 \t\t; contador++\n");
                codigo_asm.append("\tadd rsi, 1 \t\t; proximo char (Exp)\n");
                codigo_asm.append("\tadd rdi, 1 \t\t; proximo char (temp)\n");
                codigo_asm.append("\tcmp al, 0 \t\t; checa se eh fim da str\n");
                codigo_asm.append("\tjne " + next_char + "\t\t; se não eh o fim, continua o loop (jump p/ next_char)\n");
                codigo_asm.append("\tsub rcx, 1 \t\t; contador-- (remove o fim da str)\n");

                if(writeln){
                    codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                    codigo_asm.append("\tadd rsi, rcx \t\t; soma contador ao inicio\n");
                    codigo_asm.append("\tmov al, 0xA \t\t; reg. al recebe quebra de linha\n");
                    codigo_asm.append("\tmov [rsi], al \t\t; quebra de linha no final do char\n");
                    codigo_asm.append("\tadd rcx, 1 \t\t; contador++ p/ n dar pau com a quebra de linha\n");
                }

                codigo_asm.append("\n;executa interrupção de saída\n\n");
                
                codigo_asm.append("\tmov rsi, M+" + int2hex(end_aux) + "\t\t; inicio da string\n");
                codigo_asm.append("\tmov rdx, rcx\t\t; tamanho da str\n");
                codigo_asm.append("\tmov rax, 1\t\t; chamada para saida\n");
                codigo_asm.append("\tmov rdi, 1\t\t; saida para tela\n");
                codigo_asm.append("\tsyscall \t\t; chama o kernel\n");
            }

        }// fim da producao de escrita

        casaToken(TK_PONTO_E_VIRGULA);
    
    }// fim das producoes opcionais

}// fim Comandos()

/* Procedimento Exp
  Exp -> ExpS [ (= | != | < | > | <= | >=) ExpS]
*/
void Exp(int* tipo_Exp, int* tamanho_Exp, int* end_Exp){
    int tipo_ExpS, tamanho_ExpS, end_ExpS, tipo_ExpS_aux;
    int tipo_ExpS1, tamanho_ExpS1, end_ExpS1;

    ExpS(&tipo_ExpS, &tamanho_ExpS, &end_ExpS);

    tipo_ExpS_aux = tipo_ExpS;
    switch(registroLexico.token){
        case TK_IGUAL   :
            casaToken(TK_IGUAL  );
            ExpS(&tipo_ExpS1, &tamanho_ExpS1, &end_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == TIPO_CARACTER && tipo_ExpS1 != TIPO_CARACTER){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == TIPO_STRING && tipo_ExpS1 != TIPO_STRING){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == TIPO_INTEIRO || tipo_ExpS == TIPO_REAL) && (tipo_ExpS1 != TIPO_INTEIRO && tipo_ExpS1 != TIPO_REAL)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = TIPO_BOOLEAN;


            /* GERAÇÃO DE CÓDIGO (=) */
            // ambos operandos sao inteiros
            if(tipo_ExpS_aux == TIPO_INTEIRO && tipo_ExpS1 == TIPO_INTEIRO){
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em ebx\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; faz comparacao (ExpS e ExpS1)\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tje " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; guarda resultado\n");
            }
            // pelo menos um operando eh float
            if(tipo_ExpS_aux == TIPO_REAL || tipo_ExpS1 == TIPO_REAL){
                // ExpS eh inteiro
                if(tipo_ExpS_aux == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm0, eax\t\t; converte p/ real\n");
                }
                // ExpS eh real
                else {
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em xmm0\n");
                }
                
                // ExpS1 eh inteiro
                if(tipo_ExpS1 == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm1, eax \t\t; converte p/ real\n");
                }
                // ExpS1 eh real
                else {
                    codigo_asm.append("\tmovss xmm1, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end em xmm1\n");
                }

                // faz (=)
                codigo_asm.append("\tcomiss xmm0, xmm1\t\t; faz comparacao (ExpS e ExpS1)\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tje " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega ExpS.end\n");
            }

            // ambos operandos sao sgtring
            if(tipo_ExpS_aux == TIPO_STRING && tipo_ExpS1 == TIPO_STRING){

                codigo_asm.append("\tmov eax,0 \t\t; eax:=0\n");
                codigo_asm.append("\tmov ebx,0 \t\t; ebx:=0\n");
                codigo_asm.append("\tmov rax, qword M+" + int2hex(end_ExpS) + "\t\t; guarda ExpS.end em rax\n");
                codigo_asm.append("\tmov rbx, qword M+" + int2hex(end_ExpS1) + "\t\t; guarda ExpS1.end em rbx\n");
                string next_char = novo_rotulo();
                codigo_asm.append(next_char + ": \t\t; next_char: rotulo p/ ler proximo char da str\n");
                codigo_asm.append("\tmov al, [rax] \t\t; guarda o char atual de ExpS\n");
                codigo_asm.append("\tmov bl, [rbx] \t\t; guarda o char atual de ExpS1\n");
                codigo_asm.append("\tadd rax, 1 \t\t; aponta p/ o char seguinte (anda 1 byte p/ frente)\n");
                codigo_asm.append("\tadd rbx, 1 \t\t; aponta p/ o char seguinte (anda 1 byte p/ frente)\n");
                codigo_asm.append("\tcmp al, bl \t\t; compara chars\n");
                string rot_falso = novo_rotulo();
                codigo_asm.append("\tjne " + rot_falso + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tcmp eax,0\t\t; checa se eh o fim da string\n");
                codigo_asm.append("\tjne " + next_char + "\t\t; se não eh o fim, continua o loop (jump p/ next_char)\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_falso + ": \t\t; rot_falso\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega true ou false em [ExpS.end]\n");
            }

            // ambos operandos sao caracteres
            if(tipo_ExpS_aux == TIPO_CARACTER && tipo_ExpS1 == TIPO_CARACTER){
                codigo_asm.append("\tmov eax,0 \t\t; eax:=0\n");
                codigo_asm.append("\tmov ebx,0 \t\t; ebx:=0\n");
                codigo_asm.append("\tmov al, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega ExpS.end\n");
                codigo_asm.append("\tmov bl, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; comparar resultado\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tje " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega ExpS.end\n");
            }

        break;

        case TK_DIFERENTE:
            casaToken(TK_DIFERENTE);
            ExpS(&tipo_ExpS1, &tamanho_ExpS1, &end_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == TIPO_CARACTER && tipo_ExpS1 != TIPO_CARACTER){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == TIPO_STRING && tipo_ExpS1 != TIPO_STRING){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == TIPO_INTEIRO || tipo_ExpS == TIPO_REAL) && (tipo_ExpS1 != TIPO_INTEIRO && tipo_ExpS1 != TIPO_REAL)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = TIPO_BOOLEAN;

            /* GERAÇÃO DE CÓDIGO (!=) */
            // ambos operandos sao inteiros
            if(tipo_ExpS_aux == TIPO_INTEIRO && tipo_ExpS1 == TIPO_INTEIRO){
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em ebx\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; faz comparacao (ExpS e ExpS1)\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjne " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; guarda resultado\n");
            }
            // pelo menos um operando eh float
            if(tipo_ExpS_aux == TIPO_REAL || tipo_ExpS1 == TIPO_REAL){
                // ExpS eh inteiro
                if(tipo_ExpS_aux == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm0, eax\t\t; converte p/ real\n");
                }
                // ExpS eh real
                else {
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em xmm0\n");
                }

                // ExpS1 eh inteiro
                if(tipo_ExpS1 == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm1, eax \t\t; converte p/ real\n");
                }
                // ExpS1 eh real
                else {
                    codigo_asm.append("\tmovss xmm1, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end em xmm1\n");
                }

                // faz (!=)
                codigo_asm.append("\tcomiss xmm0, xmm1\t\t; faz comparacao (ExpS e ExpS1)\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjne " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega ExpS.end\n");
            }

            if(tipo_ExpS_aux == TIPO_CARACTER && tipo_ExpS1 == TIPO_CARACTER){
                codigo_asm.append("\tmov eax,0 \t\t; eax:=0\n");
                codigo_asm.append("\tmov ebx,0 \t\t; ebx:=0\n");
                codigo_asm.append("\tmov al, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega ExpS.end\n");
                codigo_asm.append("\tmov bl, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; comparar resultado\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjne " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega ExpS.end\n");
            }

        break;

        case TK_MAIOR:
            casaToken(TK_MAIOR);
            ExpS(&tipo_ExpS1, &tamanho_ExpS1, &end_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == TIPO_CARACTER && tipo_ExpS1 != TIPO_CARACTER){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == TIPO_STRING && tipo_ExpS1 != TIPO_STRING){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == TIPO_INTEIRO || tipo_ExpS == TIPO_REAL) && (tipo_ExpS1 != TIPO_INTEIRO && tipo_ExpS1 != TIPO_REAL)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = TIPO_BOOLEAN;

            /* GERAÇÃO DE CÓDIGO (>) */
            // ambos operandos sao inteiros
            if(tipo_ExpS_aux == TIPO_INTEIRO && tipo_ExpS1 == TIPO_INTEIRO){
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em ebx\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; faz comparacao (ExpS e ExpS1)\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjg " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; guarda resultado\n");
            }
            // pelo menos um operando eh float
            if(tipo_ExpS_aux == TIPO_REAL || tipo_ExpS1 == TIPO_REAL){
                // ExpS eh inteiro
                if(tipo_ExpS_aux == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm0, eax\t\t; converte p/ real\n");
                }
                // ExpS eh real
                else {
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em xmm0\n");
                }
                
                // ExpS1 eh inteiro
                if(tipo_ExpS1 == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm1, eax \t\t; converte p/ real\n");
                }
                // ExpS1 eh real
                else {
                    codigo_asm.append("\tmovss xmm1, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end em xmm1\n");
                }

                // faz (>)
                codigo_asm.append("\tcomiss xmm0, xmm1\t\t; faz comparacao (ExpS e ExpS1)\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tja " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega ExpS.end\n");
            }
            // ambos operandos sao caracteres
            if(tipo_ExpS_aux == TIPO_CARACTER && tipo_ExpS1 == TIPO_CARACTER){
                codigo_asm.append("\tmov eax,0 \t\t; eax:=0\n");
                codigo_asm.append("\tmov ebx,0 \t\t; ebx:=0\n");
                codigo_asm.append("\tmov al, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega ExpS.end\n");
                codigo_asm.append("\tmov bl, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; comparar resultado\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjg " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega ExpS.end\n");
            }

        break;

        case TK_MENOR:
            casaToken(TK_MENOR);
            ExpS(&tipo_ExpS1, &tamanho_ExpS1, &end_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == TIPO_CARACTER && tipo_ExpS1 != TIPO_CARACTER){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == TIPO_STRING && tipo_ExpS1 != TIPO_STRING){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == TIPO_INTEIRO || tipo_ExpS == TIPO_REAL) && (tipo_ExpS1 != TIPO_INTEIRO && tipo_ExpS1 != TIPO_REAL)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = TIPO_BOOLEAN;

            /* GERAÇÃO DE CÓDIGO (<) */
            // ambos inteiros
            if(tipo_ExpS_aux == TIPO_INTEIRO && tipo_ExpS1 == TIPO_INTEIRO){
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em ebx\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; faz comparacao (ExpS e ExpS1)\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjl " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; guarda resultado\n");
            }

            // pelo menos um operando eh float
            if(tipo_ExpS_aux == TIPO_REAL || tipo_ExpS1 == TIPO_REAL){
                // ExpS eh inteiro
                if(tipo_ExpS_aux == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm0, eax\t\t; converte p/ real\n");
                }
                // ExpS eh real
                else {
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em xmm0\n");
                }
                
                // ExpS1 eh inteiro
                if(tipo_ExpS1 == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm1, eax \t\t; converte p/ real\n");
                }
                // ExpS1 eh real
                else {
                    codigo_asm.append("\tmovss xmm1, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end em xmm1\n");
                }

                // faz (<)
                codigo_asm.append("\tcomiss xmm0, xmm1\t\t; comparar resultado\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjb " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; armazenar exp1.end\n");
            }
            // ambos operandos sao caracteres
            if(tipo_ExpS_aux == TIPO_CARACTER && tipo_ExpS1 == TIPO_CARACTER){
                codigo_asm.append("\tmov eax,0 \t\t; eax:=0\n");
                codigo_asm.append("\tmov ebx,0 \t\t; ebx:=0\n");
                codigo_asm.append("\tmov al, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega ExpS.end\n");
                codigo_asm.append("\tmov bl, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; comparar resultado\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjl " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega ExpS.end\n");
            }

        break;

        case TK_MENOR_IGUAL:
            casaToken(TK_MENOR_IGUAL);
            ExpS(&tipo_ExpS1, &tamanho_ExpS1, &end_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == TIPO_CARACTER && tipo_ExpS1 != TIPO_CARACTER){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == TIPO_STRING && tipo_ExpS1 != TIPO_STRING){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == TIPO_INTEIRO || tipo_ExpS == TIPO_REAL) && (tipo_ExpS1 != TIPO_INTEIRO && tipo_ExpS1 != TIPO_REAL)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = TIPO_BOOLEAN;

            /* GERAÇÃO DE CÓDIGO (<=) */
            // ambos operandos sao inteiros
            if(tipo_ExpS_aux == TIPO_INTEIRO && tipo_ExpS1 == TIPO_INTEIRO){
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em ebx\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; faz comparacao (ExpS e ExpS1)\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjle " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; guarda resultado\n");
            }

            // pelo menos um operando eh float
            if(tipo_ExpS_aux == TIPO_REAL || tipo_ExpS1 == TIPO_REAL){
                // ExpS eh inteiro
                if(tipo_ExpS_aux == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm0, eax\t\t; converte p/ real\n");
                }
                // ExpS eh real
                else {
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em xmm0\n");
                }
                
                // ExpS1 eh inteiro
                if(tipo_ExpS1 == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm1, eax \t\t; converte p/ real\n");
                }
                // ExpS1 eh real
                else {
                    codigo_asm.append("\tmovss xmm1, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end em xmm1\n");
                }

                // faz (<=)
                codigo_asm.append("\tcomiss xmm0, xmm1\t\t; faz comparacao (ExpS e ExpS1)\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjbe " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega ExpS.end\n");
            }

            // ambos operandos sao caracteres
            if(tipo_ExpS_aux == TIPO_CARACTER && tipo_ExpS1 == TIPO_CARACTER){
                codigo_asm.append("\tmov eax,0 \t\t; eax:=0\n");
                codigo_asm.append("\tmov ebx,0 \t\t; ebx:=0\n");
                codigo_asm.append("\tmov al, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega ExpS.end\n");
                codigo_asm.append("\tmov bl, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; comparar resultado\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjle " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega ExpS.end\n");
            }

        break;

        case TK_MAIOR_IGUAL:
            casaToken(TK_MAIOR_IGUAL);
            ExpS(&tipo_ExpS1, &tamanho_ExpS1, &end_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == TIPO_CARACTER && tipo_ExpS1 != TIPO_CARACTER){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == TIPO_STRING && tipo_ExpS1 != TIPO_STRING){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == TIPO_INTEIRO || tipo_ExpS == TIPO_REAL) && (tipo_ExpS1 != TIPO_INTEIRO && tipo_ExpS1 != TIPO_REAL)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = TIPO_BOOLEAN;

            /* GERAÇÃO DE CÓDIGO (>=) */
            // ambos operandos sao inteiros
            if(tipo_ExpS_aux == TIPO_INTEIRO && tipo_ExpS1 == TIPO_INTEIRO){
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em ebx\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; faz comparacao (ExpS e ExpS1)\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjge " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; guarda resultado\n");
            }

            // pelo menos um operando eh float
            if(tipo_ExpS_aux == TIPO_REAL || tipo_ExpS1 == TIPO_REAL){
                // ExpS eh inteiro
                if(tipo_ExpS_aux == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm0, eax\t\t; converte p/ real\n");
                }
                // ExpS eh real
                else {
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega [ExpS.end] em xmm0\n");
                }
                
                // ExpS1 eh inteiro
                if(tipo_ExpS1 == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega [ExpS1.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm1, eax \t\t; converte p/ real\n");
                }
                // ExpS1 eh real
                else {
                    codigo_asm.append("\tmovss xmm1, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end em xmm1\n");
                }

                // faz (>=)
                codigo_asm.append("\tcomiss xmm0, xmm1\t\t; faz comparacao (ExpS e ExpS1)\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjae " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega ExpS.end\n");
            }

            // ambos operandos sao caracteres
            if(tipo_ExpS_aux == TIPO_CARACTER && tipo_ExpS1 == TIPO_CARACTER){
                codigo_asm.append("\tmov eax,0 \t\t; eax:=0\n");
                codigo_asm.append("\tmov ebx,0 \t\t; ebx:=0\n");
                codigo_asm.append("\tmov al, [qword M+" + int2hex(end_ExpS) + "]\t\t; carrega ExpS.end\n");
                codigo_asm.append("\tmov bl, [qword M+" + int2hex(end_ExpS1) + "]\t\t; carrega ExpS1.end\n");
                codigo_asm.append("\tcmp eax,ebx\t\t; comparar resultado\n");
                string rot_verdadeiro = novo_rotulo();
                codigo_asm.append("\tjge " + rot_verdadeiro + "\t\t; pula p/ rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,0 \t\t; eax:=false\n");
                string rot_fim = novo_rotulo();
                codigo_asm.append("\tjmp " + rot_fim + " \t\t; pula p/ rot_fim\n");
                codigo_asm.append(rot_verdadeiro + ": \t\t; rot_verdadeiro\n");
                codigo_asm.append("\tmov eax,1 \t\t; eax:=true\n");
                codigo_asm.append(rot_fim + ": \t\t; rot_fim\n");
                end_ExpS = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_ExpS) + "], eax\t\t; carrega ExpS.end\n");
            }

        break;
    
    default:
        break;

    }// fim switch
    *tipo_Exp = tipo_ExpS;
    *tamanho_Exp = tamanho_ExpS;
    *end_Exp = end_ExpS;
}// fim Exp()


/* Procedimento ExpS	
  ExpS -> [-] T { (+ | - | "||") T}	
*/	
void ExpS(int* tipo_ExpS, int* tamanho_ExpS, int* end_ExpS) {	
    int tipo_T, tamanho_T, end_T, tipo_T_aux;
    int tipo_T1, tamanho_T1, end_T1;
    int end_aux;

    bool adicao = false;
    bool subtracao = false;	

    if (registroLexico.token == TK_MENOS){	
        subtracao = true;	
        casaToken(TK_MENOS);
    }

    T(&tipo_T, &tamanho_T, &end_T);

    // se possui o token '-', precisa ser um numero	
    if (subtracao) {

        // Se não for um número, dá erro
        if (tipo_T != TIPO_INTEIRO && tipo_T != TIPO_REAL)
            showError(INCOMPATIBLE_TYPES, "");

        // Se for, inverte o sinal e salva o resultado
        else {
            end_aux = novo_temp(4);
            if (tipo_T == TIPO_INTEIRO) {
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_T) + "] \t\t; carrega [T.end] em eax\n");
                codigo_asm.append("\tneg eax \t\t; nega valor de eax \n");
                codigo_asm.append("\tmov [qword M+" + int2hex(end_aux) + "],eax \t\t; copia resultado p/ novo endereco\n");
            }
            else{
                codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_T) + "] \t\t; carrega [T.end] em xmm0\n");
                codigo_asm.append("\tmov eax,-1 \t\t; carrega eax com -1\n");
                codigo_asm.append("\tcvtsi2ss xmm1, eax \t\t; converte eax p/ real\n");
                codigo_asm.append("\tmulss xmm0, xmm1 \t\t; multiplica xmm0 por -1\n");
                codigo_asm.append("\tmovss [qword M+" + int2hex(end_aux) + "],xmm0 \t\t; copia resultado p/ novo endereco\n");
            }
            end_T = end_aux;
        }
    }

    while (registroLexico.token == TK_MAIS || registroLexico.token == TK_MENOS || registroLexico.token == TK_OR) {	
        subtracao = adicao = false;	
        switch (registroLexico.token){	
            case TK_MAIS:	
                casaToken(registroLexico.token);	
                // o termo deve ser um numero	
                if (tipo_T != TIPO_INTEIRO && tipo_T != TIPO_REAL){	
                    showError(INCOMPATIBLE_TYPES, "");	
                }	
                adicao = true;	
                break;	
            case TK_MENOS:	
                casaToken(registroLexico.token);	
                // o termo deve ser um numero	
                if (tipo_T != TIPO_INTEIRO && tipo_T != TIPO_REAL){	
                    showError(INCOMPATIBLE_TYPES, "");	
                }	
                subtracao = true;	
                break;	
            case TK_OR:	
                casaToken(registroLexico.token);	
                // o termo deve ser logico	
                if (tipo_T != TIPO_BOOLEAN){	
                    showError(INCOMPATIBLE_TYPES, "");	
                }	
                break;	
        }// fim switch	
        	
        T(&tipo_T1, &tamanho_T1, &end_T1);

        tipo_T_aux = tipo_T; // guarda tipo de T (usado na geracao de codigo)

        // checa tipos para +, -, ||	
        if (adicao || subtracao){	
            if (tipo_T1 != TIPO_INTEIRO && tipo_T1 != TIPO_REAL){	
                showError(INCOMPATIBLE_TYPES, "");	
            }	
            else if (tipo_T == TIPO_INTEIRO && tipo_T1 == TIPO_INTEIRO){	
                tipo_T = TIPO_INTEIRO;	
            }	
            else{	
                tipo_T = TIPO_REAL;	
            }	
        }	
        // checa tipos p/ || 
        // ERRO se um termo for logico e o outro nao	
        else if (tipo_T == TIPO_BOOLEAN && tipo_T1 != TIPO_BOOLEAN){	
            showError(INCOMPATIBLE_TYPES, "");	
        }

        /* GERAÇÃO DE CÓDIGO (+, -, ||) */
        end_aux = novo_temp(4);
        if (adicao || subtracao){
            // se resultado da operacao for um inteiro, ambos operandos (T e T1) sao inteiros
            if (tipo_T == TIPO_INTEIRO){
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_T) + "]\t\t; carrega [T.end] em eax\n");
                codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_T1) + "]\t\t; carrega [T1.end] em ebx\n");
                
                if (adicao){
                    codigo_asm.append("\tadd eax,ebx\t\t; faz adicao\n");
                }
                // subtracao
                else{
                    codigo_asm.append("\tsub eax,ebx\t\t; faz subtracao\n");
                } 
                codigo_asm.append("\tmov [qword M+" + int2hex(end_aux) + "],eax \t\t; copia resultado p/ novo endereco\n");
            }
            // se resultado eh float, pelo menos um operando eh float
            else{
                // T eh inteiro
                if(tipo_T_aux == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_T) + "]\t\t; carrega [T.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm0, eax\t\t; converte eax p/ real\n");
                }
                // T eh real
                else{
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_T) + "]\t\t; carrega [T.end] em xmm0\n");
                }
                // T1 eh inteiro
                if(tipo_T1 == TIPO_INTEIRO){
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_T1) + "]\t\t; carrega [T1.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm1, eax\t\t; converte reg para real\n");
                }
                // T1 eh real
                else{
                    codigo_asm.append("\tmovss xmm1, [qword M+" + int2hex(end_T1) + "]\t\t; carrega [T1.end] em xmm1\n");
                }

                // faz a soma ou subtracao de T com T1
                if(adicao) codigo_asm.append("\taddss xmm0, xmm1 \t\t; faz adicao\n");
                else codigo_asm.append("\tsubss xmm0, xmm1 \t\t; faz subtracao\n");
            }
        }
        // se a operacao for ||
        else{
            codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_T) + "]\t\t; carrega [T.end] em eax\n");
            codigo_asm.append("\tneg eax \t\t; inverte o sinal\n");
            codigo_asm.append("\tadd eax,1 \t\t; faz o complemento de 2\n");
            codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_T1) + "]\t\t; carrega [T1.end] em ebx\n");
            codigo_asm.append("\tneg ebx \t\t; inverte o sinal\n");
            codigo_asm.append("\tadd ebx,1 \t\t; faz o complemento de 2\n");
            codigo_asm.append("\tmov edx,0 \t\t; limpa edx p/ multiplicacao, caso haja overflow\n");
            codigo_asm.append("\timul ebx \t\t; faz imul, que executa o papel do ||\n");
            codigo_asm.append("\tneg eax \t\t; inverte o sinal\n");
            codigo_asm.append("\tadd eax,1 \t\t; soma 1 em eax p/ finalizar o ||\n");
            codigo_asm.append("\tmov [qword M+" + int2hex(end_aux) + "],eax \t\t; copia resultado p/ novo endereco\n");
        }

        end_T = end_aux;

    }// fim while	
    
    *tipo_ExpS = tipo_T;

    *tamanho_ExpS = tamanho_T;
    *end_ExpS = end_T;
}

/* Procedimento T
  T -> F { (* | && | / | div | mod) F}
*/
void T(int* tipo_T, int* tamanho_T, int* end_T) {
    int tipo_F, tamanho_F, end_F;
    int tipo_F1, tamanho_F1, end_F1;

    bool multiplicacao, and_logico, divisao, div, mod;
    
    F(&tipo_F, &tamanho_F, &end_F);
    
    while (
        registroLexico.token == TK_ASTERISCO || 
        registroLexico.token == TK_AND       || 
        registroLexico.token == TK_BARRA     || 
        registroLexico.token == TK_DIV       || 
        registroLexico.token == TK_MOD
    ) {
        multiplicacao = and_logico = divisao = div = mod = false;

        switch(registroLexico.token){
            case TK_BARRA:
                casaToken(registroLexico.token);

                // ERRO se fator nao for numero
                if (tipo_F != TIPO_INTEIRO && tipo_F != TIPO_REAL){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                divisao = true;
                break;
                
            case TK_ASTERISCO:
                casaToken(registroLexico.token);

                // ERRO se fator nao for numero
                if (tipo_F != TIPO_INTEIRO && tipo_F != TIPO_REAL){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                multiplicacao = true;
                break;

            case TK_AND:
                casaToken(registroLexico.token);

                // ERRO se fator nao for logico
                if (tipo_F != TIPO_BOOLEAN){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                and_logico = true;
                break;


            case TK_DIV:
                casaToken(registroLexico.token);

                // ERRO se fator nao for um numero inteiro
                if (tipo_F != TIPO_INTEIRO){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                div = true;
                break;

            case TK_MOD:
                casaToken(registroLexico.token);
                
                // ERRO se fator nao for um numero inteiro
                if (tipo_F != TIPO_INTEIRO){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                mod = true;
                break;
        }// fim switch
        
        F(&tipo_F1, &tamanho_F1, &end_F1);

        // checa *, /, &&, div, mod
        if (divisao || multiplicacao){
            // ERRO se n for um numero
            if (tipo_F1 != TIPO_INTEIRO && tipo_F1 != TIPO_REAL){
                showError(INCOMPATIBLE_TYPES, "");
            }
            else if (divisao){
                // ambos operandos inteiros
                if (tipo_F == TIPO_INTEIRO && tipo_F1 == TIPO_INTEIRO){
                    
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_F) + "]\t\t; carrega [F.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm0, eax\t\t; converte p/ float e armazena em xmm0\n");
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_F1) + "]\t\t; carrega [F1.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm1, eax\t\t; converte p/ float e armazena em xmm1\n");
                    codigo_asm.append("\tdivss xmm0, xmm1\t\t; faz a divisao\n");
                    end_F = novo_temp(4);
                    codigo_asm.append("\tmovss [qword M+" + int2hex(end_F) + "],xmm0 \t\t; copia resultado p/ [F.end]\n");
                }
                // apenas F for float
                else if (tipo_F == TIPO_REAL && tipo_F1 != TIPO_REAL){
                    
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_F1) + "]\t\t; carrega [F1.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm1, eax\t\t; converte p/ float e armazena em xmm1\n");
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_F) + "]\t\t; carrega F.end em xmm0\n");
                    codigo_asm.append("\tdivss xmm0, xmm1\t\t; faz a divisao\n");
                    end_F = novo_temp(4);
                    codigo_asm.append("\tmovss [qword M+" + int2hex(end_F) + "],xmm0 \t\t; copia resultado p/ [F.end]\n");
                }
                // apenas F1 for float
                else if (tipo_F1 == TIPO_REAL && tipo_F != TIPO_REAL){

                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_F) + "]\t\t; carrega [F.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm0, eax\t\t; converte p/ float e armazena em xmm0\n");
                    codigo_asm.append("\tmovss xmm1, [qword M+" + int2hex(end_F1) + "]\t\t; carrega [F1.end] em xmm1\n");
                    codigo_asm.append("\tdivss xmm0, xmm1\t\t; faz a divisao\n");
                    end_F = novo_temp(4);
                    codigo_asm.append("\tmovss [qword M+" + int2hex(end_F) + "],xmm0 \t\t; copia resultado p/ [F.end]\n");
                }
                // ambos float
                else{
                    
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_F) + "]\t\t; carrega [F.end] em xmm0\n");
                    codigo_asm.append("\tmovss xmm1, [qword M+" + int2hex(end_F1) + "]\t\t; carrega [F1.end] em xmm1\n");
                    codigo_asm.append("\tdivss xmm0, xmm1\t\t; faz a divisao\n");
                    end_F = novo_temp(4);
                    codigo_asm.append("\tmovss [qword M+" + int2hex(end_F) + "],xmm0 \t\t; copia resultado p/ [F.end]\n");
                }
                
                // resultado eh real se for divisao
                tipo_F = TIPO_REAL;
            }
            // multiplicacao
            // ambos fatores inteiros
            else if (tipo_F == TIPO_INTEIRO && tipo_F1 == TIPO_INTEIRO){

                /* GERAÇÃO DE CÓDIGO */
                
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_F) + "]\t\t; carrega [F.end] em eax\n");
                codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_F1) + "]\t\t; carrega [F1.end] em ebx\n");
                codigo_asm.append("\tmov edx,0 \t\t; limpa edx p/ multiplicacao, caso haja overflow\n");
                codigo_asm.append("\timul ebx\t\t; multiplica\n");
                end_F = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_F) + "],eax \t\t; copia resultado p/ [F.end]\n");
                
                // resultado inteiro se ambos fatores inteiros
                tipo_F = TIPO_INTEIRO;
            }
            // pelo menos um fator real
            else{
                // apenas F for float
                if (tipo_F == TIPO_REAL && tipo_F1 != TIPO_REAL){
                    
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_F1) + "]\t\t; carrega [F1.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm1, eax\t\t; converte p/ float e armazena em xmm1\n");
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_F) + "]\t\t; carrega [F.end] em xmm0\n");
                    codigo_asm.append("\tmulss xmm0, xmm1\t\t; multiplica\n");
                    end_F = novo_temp(4);
                    codigo_asm.append("\tmovss [qword M+" + int2hex(end_F) + "],xmm0 \t\t; copia resultado p/ [F.end]\n");
                }
                // apenas F1 for float
                else if (tipo_F1 == TIPO_REAL && tipo_F != TIPO_REAL){
                    
                    codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_F) + "]\t\t; carrega [F.end] em eax\n");
                    codigo_asm.append("\tcvtsi2ss xmm0, eax\t\t; converte p/ float e armazena em xmm0\n");
                    codigo_asm.append("\tmovss xmm1, [qword M+" + int2hex(end_F1) + "]\t\t; carrega [F1.end] em xmm1\n");
                    codigo_asm.append("\tmulss xmm0, xmm1\t\t; multiplica\n");
                    end_F = novo_temp(4);
                    codigo_asm.append("\tmovss [qword M+" + int2hex(end_F) + "],xmm0 \t\t; copia resultado p/ [F.end]\n");
                }
                // ambos float
                else{
                    codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_F) + "]\t\t; carrega [F.end] em xmm0\n");
                    codigo_asm.append("\tmovss xmm1, [qword M+" + int2hex(end_F1) + "]\t\t; carrega [F1.end] em xmm1\n");
                    codigo_asm.append("\tmulss xmm0, xmm1\t\t; multiplica\n");
                    end_F = novo_temp(4);
                    codigo_asm.append("\tmovss [qword M+" + int2hex(end_F) + "],xmm0 \t\t; copia resultado p/ [F.end]\n");
                }
                
                // resultado eh real se pelo menos um fator for real
                tipo_F = TIPO_REAL;
            }
        }
        else if (and_logico){ 
            // && exige que os 2 fatores sejam logicos
            if (tipo_F == TIPO_BOOLEAN && tipo_F1 != TIPO_BOOLEAN){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // faz o &&
            else{
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_F) + "]\t\t; carrega [F.end] em eax\n");
                codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_F1) + "]\t\t; carrega [F1.end] em ebx\n");
                codigo_asm.append("\tmov edx,0 \t\t; limpa edx p/ multiplicacao, caso haja overflow\n");
                codigo_asm.append("\timul ebx\t\t; multiplica\n");
                end_F = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_F) + "],eax \t\t; copia resultado p/ [F.end]\n");
            }
        }
        else if (div || mod){ 
            // div e mod exigem que os 2 fatores sejam inteiros
            if (tipo_F == TIPO_INTEIRO && tipo_F1 != TIPO_INTEIRO){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // faz o div
            else if (div){
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_F) + "]\t\t; carrega [F.end] em eax\n");
                codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_F1) + "]\t\t; carrega [F1.end] em ebx\n");
                codigo_asm.append("\tmov edx,0 \t\t; limpa edx p/ multiplicacao, caso haja overflow\n");
                codigo_asm.append("\tidiv ebx\t\t; faz a divisao (pega o quociente da operacao na prox instrucao)\n");
                end_F = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_F) + "],eax \t\t; copia resultado p/ [F.end]\n");
            }
            // faz o mod
            else{
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_F) + "]\t\t; carrega [F.end] em eax\n");
                codigo_asm.append("\tmov ebx, [qword M+" + int2hex(end_F1) + "]\t\t; carrega [F1.end] em ebx\n");
                codigo_asm.append("\tmov edx,0 \t\t; limpa edx p/ multiplicacao, caso haja overflow\n");
                codigo_asm.append("\tidiv ebx\t\t; faz a divisao (pega o resto da operacao na prox instrucao)\n");
                end_F = novo_temp(4);
                codigo_asm.append("\tmov [qword M+" + int2hex(end_F) + "],edx \t\t; copia resultado p/ [F.end]\n");
            }
        }

    }// fim while

    *tipo_T = tipo_F;
    *tamanho_T = tamanho_F;
    *end_T = end_F;
}// fim T()

/* Procedimento F
  F -> constante | (int | float) "(" Exp ")" | "(" Exp ")" | id [ "[" Exp "]" ] | ! F
*/
void F(int* tipo_F, int* tamanho_F, int* end_F) {
    int tipo_Exp, tamanho_Exp, end_Exp;

    // constante
    if (registroLexico.token == TK_CONST) {
        RegLex rl_const = registroLexico;
        *tipo_F = registroLexico.tipo;
        casaToken(TK_CONST);

        /* GERAÇÃO DE CÓDIGO */	
        // faz a reserva de memoria p/ a constante	
        int end_aux;	
        if (*tipo_F == TIPO_INTEIRO) {	
            end_aux = novo_temp(4); 	
            codigo_asm.append("\tmov eax," + rl_const.lexema + "\t\t; carrega a constante inteira em eax\n");	
            codigo_asm.append("\tmov [qword M+" + int2hex(end_aux) + "], eax \t\t; o novo endereco recebe o valor da constante\n");	
        } else if (*tipo_F == TIPO_CARACTER) {	
            end_aux = novo_temp(1); 	
            codigo_asm.append("\tmov al," + rl_const.lexema + "\t\t; carrega a constante char em eax\n");	
            codigo_asm.append("\tmov [qword M+" + int2hex(end_aux) + "], al \t\t; o novo endereco recebe o valor da constante\n");	
        } else if (*tipo_F == TIPO_REAL) {	
            codigo_asm.append("\nsection .data \t\t; seção de dados\n");	
            // adiciona 0 se o numero comecar com .
            if (rl_const.lexema[0] == '.') rl_const.lexema = '0' + rl_const.lexema;	
            codigo_asm.append("\tdd " + rl_const.lexema + " \t\t; reserva 4 bytes e escreve a constante\n");	
            codigo_asm.append("\nsection .text \t\t; voltando p/ seção de código\n");	
            end_aux = novo_dado(4);
        } else {
            codigo_asm.append("\nsection .data \t\t; seção de dados\n");	
            codigo_asm.append("\tdb " + rl_const.lexema + ", 0 \t\t; reserva tamanho da string + indicador de fim da string\n");	
            codigo_asm.append("\nsection .text \t\t; voltando p/ seção de código\n");	
            
            end_aux = novo_dado(rl_const.tamanho+1);
            *tamanho_F = rl_const.tamanho; // tamanho da string	
        }// fim if else	

        *end_F = end_aux;

    }

    // (int | float) "(" Exp ")"
    else if (registroLexico.token == TK_INT || registroLexico.token == TK_FLOAT) {
        if (registroLexico.token == TK_INT) {
            casaToken(TK_INT);
            *tipo_F = TIPO_INTEIRO;
        }
        else {
            casaToken(TK_FLOAT);
            *tipo_F = TIPO_REAL;
        }
        casaToken(TK_ABRE_PARENTESE);
        
        Exp(&tipo_Exp, &tamanho_Exp, &end_Exp);
        if (tipo_Exp != TIPO_INTEIRO && tipo_Exp != TIPO_REAL)
            showError(INCOMPATIBLE_TYPES, "");

        casaToken(TK_FECHA_PARENTESE);

        /* GERAÇÃO DE CÓDIGO */
        // converte Exp p/ int ou float
        int end_aux = novo_temp(4);
        // convertendo p/ int
        if (*tipo_F == TIPO_INTEIRO) {
            // Exp eh int, apenas guarda no novo end
            if (tipo_Exp == TIPO_INTEIRO) {
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega [Exp.end] em eax\n");
                codigo_asm.append("\tmov [qword M+" + int2hex(end_aux) + "],eax \t\t; guarda valor no novo endereco\n");
            }
            // Exp eh float, converte p/ int
            else {
                codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega [Exp.end] em xmm0\n");
                codigo_asm.append("\troundss xmm1, xmm0, 0b0011 \t\t; trunca parte fracionaria e deixa somente a parte inteira em xmm1\n");
                codigo_asm.append("\tcvtss2si eax, xmm1 \t\t; converte p/ int e guarda em eax\n");
                codigo_asm.append("\tmov [qword M+" + int2hex(end_aux) + "],eax \t\t; guarda valor no novo endereco\n");
            }// fim if else
        }
        // convertendo p/ real
        else {
            // Exp eh int, converte p/ float
            if (tipo_Exp == TIPO_INTEIRO) {
                codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_Exp) + "]\t\t; carrega [Exp.end] em eax\n");
                codigo_asm.append("\tcvtsi2ss xmm0, eax \t\t; converte p/ real e guarda em xmm0\n");
                codigo_asm.append("\tmovss [qword M+" + int2hex(end_aux) + "],xmm0 \t\t; guarda valor no novo endereco\n");
            }
            // Exp eh float, apenas guarda no novo end
            else {
                codigo_asm.append("\tmovss xmm0, [qword M+" + int2hex(end_Exp) + "] \t\t; carrega [Exp.end] em xmm0\n");
                codigo_asm.append("\tmovss [qword M+" + int2hex(end_aux) + "],xmm0 \t\t; guarda valor no novo endereco\n");
            }// fim if else
        }// fim if else

        *end_F = end_aux; // atualiza endereco

    }

    // "(" Exp ")"
    else if (registroLexico.token == TK_ABRE_PARENTESE) {
        casaToken(TK_ABRE_PARENTESE);
        
        Exp(&tipo_Exp, &tamanho_Exp, &end_Exp);
        *tipo_F = tipo_Exp;

        // Endereço de F recebe o endereço de Exp
        *end_F = end_Exp;

        casaToken(TK_FECHA_PARENTESE);
    }

    // id [ "[" Exp "]" ]
    else if (registroLexico.token == TK_ID) {
        string lex_id = registroLexico.lexema;
        casaToken(TK_ID);
        
        // Verifica se é um id que ainda não foi declarado
        Simbolo simb_id = tab_simbolos.pesquisar(lex_id);
        if (simb_id.classe == CLASSE_VAZIA) {
            showError(UNDECLARED_ID, lex_id);
        } else {
            // Se foi declarado, pega as infos dele
            *tipo_F = simb_id.tipo;
            *tamanho_F = simb_id.tamanho;
            *end_F = simb_id.endereco;
        }

        if (registroLexico.token == TK_ABRE_COLCHETE) {
            casaToken(TK_ABRE_COLCHETE);

            // Se vai usar id[], o id tem que ser string
            if (simb_id.tipo != TIPO_STRING)
                showError(INCOMPATIBLE_TYPES, "");

            Exp(&tipo_Exp, &tamanho_Exp, &end_Exp);

            // Posição da string tem que ser um numero inteiro
            if (tipo_Exp != TIPO_INTEIRO)
                showError(INCOMPATIBLE_TYPES, "");
            else
                *tipo_F = TIPO_CARACTER;

            /* GERAÇÃO DE CÓDIGO */
            // guarda o novo char na posicao da string, indicada por Exp
            int end_aux = novo_temp(1);
            codigo_asm.append("\tmov rsi,qword M+" + int2hex(*end_F) + "\t\t; carrega [F.end] (inicio da string) em rsi\n");
            codigo_asm.append("\tmov rax,0 \t\t; rax:=0\n");
            codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_Exp) + "] \t\t; eax recebe posicao do char\n");
            codigo_asm.append("\tadd rsi, rax \t\t; soma inicio da string com posicao do char\n");
            codigo_asm.append("\tmov bl, [rsi] \t\t; guarda o char em bl\n");
            codigo_asm.append("\tmov [qword M+" + int2hex(end_aux) + "],bl \t\t; guarda char no novo endereco\n");

            *tamanho_F = 1; // tamanho de F = tamanho do char (1byte)
            *end_F = end_aux; 

            casaToken(TK_FECHA_COLCHETE);
        }
    }

    // ! F1
    else if (registroLexico.token == TK_NOT) {
        int tipo_F1, tamanho_F1, end_F1;
        casaToken(TK_NOT);
        
        F(&tipo_F1, &tamanho_F1, &end_F1);

        // O tipo de F1 tem que ser lógico
        if (tipo_F1 != TIPO_BOOLEAN) showError(INCOMPATIBLE_TYPES, "");
        else *tipo_F = tipo_F1;

        /* GERAÇÃO DE CÓDIGO */
        // guarda o novo_temp com valor de F negado
        int end_aux = novo_temp(4);
        codigo_asm.append("\tmov eax, [qword M+" + int2hex(end_F1) + "] \t\t; carrega [F1.end] em eax\n");
        codigo_asm.append("\tneg eax \t\t; inverte o sinal\n");
        codigo_asm.append("\tadd eax,1 \t\t; faz o complemento de 2\n");
        codigo_asm.append("\tmov [qword M+" + int2hex(end_aux) + "],eax \t\t; copia resultado p/ novo endereco\n");
        *tamanho_F = 0; // valor zerado pois n tem utilidade
        *end_F = end_aux; 
    } 
}// fim F()

int main(int argc, char const *argv[]) {
    char c;
    bool erro = false;

    inicializa_asm();

    getNextToken();
    S();

    if (!erro){
        cout << nLinhasCompiladas << " linhas compiladas.";
        finaliza_asm();
    } else {
        codigo_asm.append("; ---ERRO---\n; COMPILACAO INTERROMPIDA");
    }
    
    return 0;
}