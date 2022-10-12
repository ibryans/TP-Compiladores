/**
 * Trabalho Prático de Compiladores
 * Parte 1 e 2 - Analisador Léxico e Sintático
 * 
 * Bryan Santos e Igor Reis - 02/2022
 */

#include <iostream>
#include <string>
#include <unordered_map>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

using namespace std;

/** Tipos de Erro */
#define INVALID_CHAR    1
#define INVALID_LEX     2
#define INVALID_EOF     3
#define INVALID_TOKEN   4

/** Definindo e Enumerando os tokens */
#define TK_const            1
#define TK_int              2
#define TK_char             3
#define TK_while            4
#define TK_if               5
#define TK_float            6
#define TK_else             7
#define TK_and              8
#define TK_or               9
#define TK_not              10
#define TK_atrib            11
#define TK_igualdade        12
#define TK_abreParentese    13
#define TK_fechaParentese   14
#define TK_menor            15
#define TK_maior            16
#define TK_diferente        17
#define TK_maiorIgual       18
#define TK_menorIgual       19
#define TK_virgula          20
#define TK_menos            21
#define TK_mais             22
#define TK_asterisco        23
#define TK_barra            24
#define TK_pontoevirgula    25
#define TK_abreChave        26
#define TK_fechaChave       27
#define TK_readln           28
#define TK_div              29
#define TK_string           30
#define TK_write            31
#define TK_writeln          32
#define TK_mod              33
#define TK_abreColchete     34
#define TK_fechaColchete    35
#define TK_true             36
#define TK_false            37
#define TK_boolean          38
#define TK_id               39
#define TK_eof              40


/** Tabela de símbolos, armazenando identificadores e palavras reservadas da linguagem */
class Tabela_simbolos {
    public:
        /** Tabela hash que mapeia lexema para token */
        unordered_map<string, int> tab_simbolos; 
        // int inserir(string, int);

        /** Construtor: Insere as palavras reservadas na tabela de simbolos */
        Tabela_simbolos() {
            inserir("const" , TK_const);
            inserir("int" , TK_int);
            inserir("char" , TK_char);
            inserir("while" , TK_while);
            inserir("if" , TK_if);
            inserir("float" , TK_float);
            inserir("else" , TK_else);
            inserir("&&" , TK_and);
            inserir("||" , TK_or);
            inserir("!" , TK_not);
            inserir(":=" , TK_atrib);
            inserir("=" , TK_igualdade);
            inserir("(" , TK_abreParentese);
            inserir(")" , TK_fechaParentese);
            inserir("<" , TK_menor);
            inserir(">" , TK_maior);
            inserir("!=" , TK_diferente);
            inserir(">=" , TK_maiorIgual);
            inserir("<=" , TK_menorIgual);
            inserir("," , TK_virgula);
            inserir("-" , TK_menos);
            inserir("+", TK_mais);
            inserir("*" , TK_asterisco);
            inserir("/" , TK_barra);
            inserir(";" , TK_pontoevirgula);
            inserir("{" , TK_abreChave);
            inserir("}" , TK_fechaChave);
            inserir("readln" , TK_readln);
            inserir("div" , TK_div);
            inserir("string" , TK_string);
            inserir("write" , TK_write);
            inserir("writeln" , TK_writeln);
            inserir("mod" , TK_mod);
            inserir("[" , TK_abreColchete);
            inserir("]" , TK_fechaColchete);
            inserir("true" , TK_true);
            inserir("false" , TK_false);
            inserir("boolean" , TK_boolean);
        }

        /**
         * Insere um [lex,tok] na tabela de símbolos
        */
        void inserir(string lexema, int token) {
            tab_simbolos.insert(make_pair(lexema, token));
        }

        /** 
         * Verifica se um lexema (identificador) já existe na tabela
         * Se nao existir, insere este lexema
         * Se existir, retorna o endereço
         */
        int pesquisar(string lexema) {
            int endereco = TK_id;
            if (tab_simbolos[lexema] == 0){ // novo identificador, insere na tabela
                inserir(lexema, TK_id);
            } else {    // ja existe na tabela, retorna 
                endereco = tab_simbolos[lexema];
            }
            return endereco;
        };

};

/** Estrutura para armazenar o registro léxico */
struct RegLex{
    string lexema;
    int token;
    int tipo;
};

/* Variáveis (globais) do analisador léxico */
int nLinhasCompiladas = 1;
Tabela_simbolos tab_simbolos;
RegLex registroLexico;
bool erro = false;

/** Método para atualizar o Registro Léxico */
void updateRegLex(string lexema, int token) {
    registroLexico.lexema = lexema;
    registroLexico.token = token;
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
                        updateRegLex(lex, TK_virgula);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '+'){
                        lex = c;
                        updateRegLex(lex, TK_mais);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '-'){
                        lex = c;
                        updateRegLex(lex, TK_menos);
                        estado_atual = estado_final;
                    } 

                    else if(c == '='){
                        lex = c;
                        updateRegLex(lex, TK_igualdade);
                        estado_atual = estado_final;
                    }
                    
                    else if(c == ';'){
                        lex = c;
                        updateRegLex(lex, TK_pontoevirgula);
                        estado_atual = estado_final;
                    }

                    else if(c == '*'){
                        lex = c;
                        updateRegLex(lex, TK_asterisco);
                        estado_atual = estado_final;
                    }                    
                    
                    else if(c == '('){
                        lex = c;
                        updateRegLex(lex, TK_abreParentese);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == ')'){
                        lex = c;
                        updateRegLex(lex, TK_fechaParentese);
                        estado_atual = estado_final;
                    }
                    
                    else if(c == '['){
                        lex = c;
                        updateRegLex(lex, TK_abreColchete);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == ']'){
                        lex = c;
                        updateRegLex(lex, TK_fechaColchete);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '{'){
                        lex = c;
                        updateRegLex(lex, TK_abreChave);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '}'){
                        lex = c;
                        updateRegLex(lex, TK_fechaChave);
                        estado_atual = estado_final;
                    }

                    else if(cin.eof()){
                        lex = "eof";
                        updateRegLex(lex, TK_eof);
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
                        int tok = tab_simbolos.pesquisar(lex);
                        updateRegLex(lex, tok);
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
                    else if (c == '\"') {
                        lex += c;
                        updateRegLex(lex, TK_const);
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

                        updateRegLex(lex, TK_const);
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
                        
                        updateRegLex(lex, TK_const);
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
                            tok = TK_maiorIgual;
                        else if (lex[0] == '<')
                            tok = TK_menorIgual;
                        else if (lex[0] == '!')
                            tok = TK_diferente;
                        updateRegLex(lex, tok);
                        estado_atual = estado_final;
                    }
                    else {
                        // > Devolve o caracter lido
                        if (!cin.eof()) {
                            cin.putback(c);
                            if (lex[0] == '>')
                                tok = TK_maior;
                            else if (lex[0] == '<')
                                tok = TK_menor;
                            else if (lex[0] == '!')
                                tok = TK_not;
                            updateRegLex(lex, tok);
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
                        updateRegLex(lex, TK_and);
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
                        updateRegLex(lex, TK_or);
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
                        updateRegLex(lex, TK_atrib);
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
                            updateRegLex(lex, TK_barra);
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
                        updateRegLex(lex, TK_const);
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
                        updateRegLex(lex, TK_const);
                        estado_atual = estado_final;
                    }
                    break;

                // Constante hexa - 1o digito
                case 17:
                    if ((isNumber(c) || (c >= 65 && c <= 70) || (c >= 97 && c <= 102)) && !cin.eof()){
                        lex += c; 
                        estado_atual = 18;
                    } else {
                        // ERRO -> não é um lexema válido (sem número depois do X)
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
                        updateRegLex(lex, TK_const);
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

                default:
                    lex += c;
                    erro = true;
                    showError(INVALID_LEX, lex);
            }

        }
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

/* Analisador Sintatico */

// Assinatura dos procedimentos
void S();
void Dec();
void Comandos();
void Exp();
void ExpS();
void T();
void F();

/** Procedimento casaToken: Verifica se o token recebido é igual ao esperado na linguagem **/
void casaToken(int token) {
    // if (erro) {
    //     exit(0);
    // }

    // cout << "esperado: " << token << endl;

    if (token == registroLexico.token) {
        // cout << "recebeu válido: " << registroLexico.lexema << endl;
        // pega proximo token apenas se nao eh fim do arquivo
        if (token != TK_eof)
            getNextToken();
    }
    else {  // Não era o token esperado, mostrar erro.
        // cout << "recebeu inválido: " << registroLexico.lexema << endl;
        if (registroLexico.token == TK_eof)
            showError(INVALID_EOF, "");
        else
            showError(INVALID_TOKEN, registroLexico.lexema);
    }
}


/* Procedimento S (primeiro da gramatica)
  S -> {Dec | Comandos} EOF
*/
void S(){
    // cout << "S()" << "\n\n";
    // Enquanto token pertencer ao first de Dec ou Comandos
    while ( registroLexico.token == TK_int || registroLexico.token == TK_float || registroLexico.token == TK_char || registroLexico.token == TK_string || registroLexico.token == TK_boolean ||
            registroLexico.token == TK_const || registroLexico.token == TK_id || registroLexico.token == TK_readln || registroLexico.token == TK_write || registroLexico.token == TK_writeln ||
            registroLexico.token == TK_if || registroLexico.token == TK_while || registroLexico.token == TK_pontoevirgula){
                
                // first de Dec
                if(registroLexico.token == TK_int || registroLexico.token == TK_float || registroLexico.token == TK_char || registroLexico.token == TK_string || registroLexico.token == TK_boolean || registroLexico.token == TK_const){
                    Dec();
                }
                else Comandos();
    }// fim while

    // fim do programa
    casaToken(TK_eof);
}// fim S()


/* Procedimento Dec
  Dec -> 
  const id = [-] constante |
  (int | float | char | string) id [:= [-] constante] {, id [:= [-] constante]} |
  boolean id [:= (true | false)] {, id [:= (true | false)]}
*/
void Dec() {
    // cout << "Dec()" << "\n\n";
    // const id = [-] constante
    if(registroLexico.token == TK_const){
        casaToken(TK_const);
        casaToken(TK_id);
        casaToken(TK_igualdade);

        if(registroLexico.token == TK_menos){
            casaToken(TK_menos);
        }

        casaToken(TK_const);
    }
   
    // (int | float | char | string) id [:= [-] constante] {, id [:= [-] constante]}
    else if(registroLexico.token == TK_int || registroLexico.token == TK_float || registroLexico.token == TK_char || registroLexico.token == TK_string){
        if (registroLexico.token == TK_int)     
            casaToken(TK_int);
        else if(registroLexico.token == TK_float) 
            casaToken(TK_float);
        else if(registroLexico.token == TK_char) 
            casaToken(TK_char);
        else 
            casaToken(TK_string);

        casaToken(TK_id);

        if (registroLexico.token == TK_atrib){
            casaToken(TK_atrib);

            if(registroLexico.token == TK_menos) 
                casaToken(TK_menos);
            casaToken(TK_const);
        }
        
        while(registroLexico.token == TK_virgula){
            casaToken(TK_virgula);
            casaToken(TK_id);

            if(registroLexico.token == TK_atrib){
                casaToken(TK_atrib);

                if (registroLexico.token == TK_menos) 
                    casaToken(TK_menos);
                casaToken(TK_const);
            }
        }

    }
    
    // boolean id [:= (true | false)] {, id [:= (true | false)]}
    else {
        casaToken(TK_boolean);

        if(registroLexico.token == TK_atrib){
            casaToken(TK_atrib);
            if (registroLexico.token == TK_true) 
                casaToken(TK_true);
            else 
                casaToken(TK_false);
        }

        while(registroLexico.token == TK_virgula) {
            casaToken(TK_virgula);
            casaToken(TK_id);

            if (registroLexico.token == TK_atrib) {
                casaToken(TK_atrib);
                if (registroLexico.token == TK_true) 
                    casaToken(TK_true);
                else 
                    casaToken(TK_false);
            }
        }
    }
}

/* Procedimento Comandos
* Comandos -> 
  (if "("Exp")" ( Comandos | "{" {Comandos} "}" ) [ else ( Comandos | "{" {Comandos} "}" ) ] | 
  while "("Exp")" ( Comandos | "{" {Comandos} "}" )) | 
  [ id [ "[" Exp "]" ] := Exp | readln "(" id ")" | (write | writeln) "(" Exp {, Exp} ")" ] ;
*/
void Comandos(){
    // cout << "Comandos()" << "\n\n";
    // while e if
    if (registroLexico.token == TK_if || registroLexico.token == TK_while){

        // if "("Exp")" ( Comandos | "{" {Comandos} "}" ) [ else ( Comandos | "{" {Comandos} "}" ) ]
        if (registroLexico.token == TK_if){
            casaToken(TK_if);
            casaToken(TK_abreParentese);
            Exp();
            casaToken(TK_fechaParentese);
            
            if(registroLexico.token == TK_abreChave){
                casaToken(TK_abreChave);
                // Enquanto token pertencer ao first de Comandos()
                while ( registroLexico.token == TK_if || registroLexico.token == TK_while ||
                        registroLexico.token == TK_id || registroLexico.token == TK_readln ||
                        registroLexico.token == TK_write || registroLexico.token == TK_writeln ||
                        registroLexico.token == TK_pontoevirgula){
                            Comandos();                            
                }
                casaToken(TK_fechaChave);
            } else{
                Comandos();
            }// fim if-else

            if(registroLexico.token == TK_else){
                if(registroLexico.token == TK_abreChave){
                    casaToken(TK_abreChave);
                    // Enquanto token pertencer ao first de Comandos()
                    while ( registroLexico.token == TK_if || registroLexico.token == TK_while ||
                            registroLexico.token == TK_id || registroLexico.token == TK_readln ||
                            registroLexico.token == TK_write || registroLexico.token == TK_writeln ||
                            registroLexico.token == TK_pontoevirgula){
                                Comandos();                            
                    }
                    casaToken(TK_fechaChave);
                } else{
                    Comandos();
                }// fim if-else

            }// fim if
            
        }// fim da producao de teste

        // while "("Exp")" ( Comandos | "{" {Comandos} "}" ) 
        else{
            casaToken(TK_while);
            casaToken(TK_abreParentese);
            Exp();
            casaToken(TK_fechaParentese);
            
            if(registroLexico.token == TK_abreChave){
                casaToken(TK_abreChave);
                // Enquanto token pertencer ao first de Comandos()
                while ( registroLexico.token == TK_if || registroLexico.token == TK_while ||
                        registroLexico.token == TK_id || registroLexico.token == TK_readln ||
                        registroLexico.token == TK_write || registroLexico.token == TK_writeln ||
                        registroLexico.token == TK_pontoevirgula){
                            Comandos();                            
                }
                casaToken(TK_fechaChave);
            } else{
                Comandos();
            }// fim if-else
        }// fim da producao de repeticao
    
    }// fim das producoes de teste e repeticao

    // producoes opcionais (seguidas de ;)
    else{
        // id [ "[" Exp "]" ] := Exp
        if(registroLexico.token == TK_id){
            casaToken(TK_id);
            
            if(registroLexico.token == TK_abreColchete){
                casaToken(TK_abreColchete);
                Exp();
                casaToken(TK_fechaColchete);
            }

            casaToken(TK_atrib);
            Exp();
        }// fim da producao de atribuicao
        
        // readln "(" id ")"
        else if(registroLexico.token == TK_readln){
            casaToken(TK_readln);
            casaToken(TK_abreParentese);
            casaToken(TK_id);  
            casaToken(TK_fechaParentese);  
        } // fim da producao de leitura
        
        // (write | writeln) "(" Exp {, Exp} ")"
        else if(registroLexico.token == TK_write || registroLexico.token == TK_writeln){
            if(registroLexico.token == TK_write){
                casaToken(TK_write);
            }
            else{
                casaToken(TK_writeln);
            }
            casaToken(TK_abreParentese);
            Exp();

            while(registroLexico.token == TK_virgula){
                casaToken(TK_virgula);
                Exp();
            }// fim while
            casaToken(TK_fechaParentese);
            
        }// fim da producao de escrita

        casaToken(TK_pontoevirgula);
    
    }// fim das producoes opcionais

}// fim Comandos()

/* Procedimento Exp
  Exp -> ExpS [ (= | != | < | > | <= | >=) ExpS]
*/
void Exp(){
    // cout << "Exp()" << "\n\n";
    ExpS();
    switch(registroLexico.token){
        case TK_igualdade:
            casaToken(TK_igualdade);
            ExpS();
        break;

        case TK_diferente:
            casaToken(TK_diferente);
            ExpS();
        break;

        case TK_maior:
            casaToken(TK_maior);
            ExpS();
        break;

        case TK_menor:
            casaToken(TK_menor);
            ExpS();
        break;

        case TK_menorIgual:
            casaToken(TK_menorIgual);
            ExpS();
        break;

        case TK_maiorIgual:
            casaToken(TK_maiorIgual);
            ExpS();
        break;
    
    default:
        break;

    }// fim switch
}// fim Exp()


/* Procedimento ExpS
  ExpS -> [-] T { (+ | - | "||") T}
*/
void ExpS() {
    // cout << "ExpS()" << "\n\n";
    if (registroLexico.token == TK_menos)
        casaToken(TK_menos);
    T();
    while (registroLexico.token == TK_mais || registroLexico.token == TK_menos || registroLexico.token == TK_or) {
        casaToken(registroLexico.token);
        T();
    }
}

/* Procedimento T
  T -> F { (* | && | / | div | mod) F}
*/
void T() {
    // cout << "T()" << "\n\n";
    F();
    while (
        registroLexico.token == TK_asterisco || 
        registroLexico.token == TK_and       || 
        registroLexico.token == TK_barra     || 
        registroLexico.token == TK_div       || 
        registroLexico.token == TK_mod
    ) {
        casaToken(registroLexico.token);
        F();
    }
}// fim T()

/* Procedimento F
  F -> ! F | (int | float) "(" Exp ")" | "(" Exp ")" | id [ "[" Exp "]" ] | constante
*/
void F() {
    // cout << "F()" << "\n\n";
    if (registroLexico.token == TK_not) {
        casaToken(TK_not);
        F();
    } else if (registroLexico.token == TK_int || registroLexico.token == TK_float) {
        if (registroLexico.token == TK_int)
            casaToken(TK_int);
        else
            casaToken(TK_float);
        casaToken(TK_abreParentese);
        Exp();
        casaToken(TK_fechaParentese);
    }
    else if (registroLexico.token == TK_abreParentese) {
        casaToken(TK_abreParentese);
        Exp();
        casaToken(TK_fechaParentese);
    }
    else if (registroLexico.token == TK_id) {
        casaToken(TK_id);
        if (registroLexico.token == TK_abreColchete) {
            casaToken(TK_abreColchete);
            Exp();
            casaToken(TK_fechaColchete);
        }
    }
    else{
        casaToken(TK_const);
    }
}// fim F()

int main(int argc, char const *argv[]) {
    char c;
    bool erro = false;

    getNextToken();
    S();

    if (!erro)
        cout << nLinhasCompiladas << " linhas compiladas.";

    return 0;
}