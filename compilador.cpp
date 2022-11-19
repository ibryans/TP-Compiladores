/**
 * Trabalho Prático de Compiladores
 * Parte 1, 2, e 3 - Analisador Léxico, Sintático e Semântico
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
#define INVALID_CHAR         1
#define INVALID_LEX          2
#define INVALID_EOF          3
#define INVALID_TOKEN        4
#define INVALID_STR_SIZE     5
#define DUPLICATE_ID         6
#define INCOMPATIBLE_TYPES   7
#define INCOMPATIBLE_CLASSES 8
#define UNDECLARED_ID        9

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

#define tipo_nulo           41
#define tipo_inteiro        42
#define tipo_float          43
#define tipo_caractere      44
#define tipo_string         45
#define tipo_boolean        46

#define classe_nula         47
#define classe_variavel     48
#define classe_constante    49

// simbolo inserido na TS
struct Simbolo {
    int token;
    int tipo;
    int classe;
};


/** Tabela de símbolos, armazenando identificadores e palavras reservadas da linguagem */
class Tabela_simbolos {
    public:
        /** Tabela hash que mapeia lexema para token */
        unordered_map<string, Simbolo> tab_simbolos; 
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
            tab_simbolos.insert(make_pair(lexema, Simbolo{token, tipo_nulo, classe_nula}));
        }

        /** 
         * Verifica se um lexema (identificador) já existe na tabela
         * Se nao existir, insere o simbolo com esse lexema e o retorna
         * Se existir, retorna o simbolo da tabela
         */
        Simbolo pesquisar(string lexema) {
            Simbolo s;
            s.token = TK_id;
            if (tab_simbolos[lexema].token == 0){ // novo identificador, insere na tabela
                inserir(lexema, TK_id);
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

/** Estrutura para armazenar o registro léxico */
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
            cout << "str maior que o permitido (" << lex.length() << " bytes)" << endl;
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
                        updateRegLex(lex, TK_virgula, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '+'){
                        lex = c;
                        updateRegLex(lex, TK_mais, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '-'){
                        lex = c;
                        updateRegLex(lex, TK_menos, -99, 0);
                        estado_atual = estado_final;
                    } 

                    else if(c == '='){
                        lex = c;
                        updateRegLex(lex, TK_igualdade, -99, 0);
                        estado_atual = estado_final;
                    }
                    
                    else if(c == ';'){
                        lex = c;
                        updateRegLex(lex, TK_pontoevirgula, -99, 0);
                        estado_atual = estado_final;
                    }

                    else if(c == '*'){
                        lex = c;
                        updateRegLex(lex, TK_asterisco, -99, 0);
                        estado_atual = estado_final;
                    }                    
                    
                    else if(c == '('){
                        lex = c;
                        updateRegLex(lex, TK_abreParentese, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == ')'){
                        lex = c;
                        updateRegLex(lex, TK_fechaParentese, -99, 0);
                        estado_atual = estado_final;
                    }
                    
                    else if(c == '['){
                        lex = c;
                        updateRegLex(lex, TK_abreColchete, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == ']'){
                        lex = c;
                        updateRegLex(lex, TK_fechaColchete, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '{'){
                        lex = c;
                        updateRegLex(lex, TK_abreChave, -99, 0);
                        estado_atual = estado_final;
                    } 
                    
                    else if(c == '}'){
                        lex = c;
                        updateRegLex(lex, TK_fechaChave, -99, 0);
                        estado_atual = estado_final;
                    }

                    else if(cin.eof()){
                        lex = "eof";
                        updateRegLex(lex, TK_eof, -99, 0);
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
                        int tok = tab_simbolos.pesquisar(lex).token;
                        updateRegLex(lex, tok, -99, 0);
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
                        updateRegLex(lex, TK_const, tipo_string, lex.length()-2);
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

                        updateRegLex(lex, TK_const, tipo_inteiro, 4);
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
                        
                        updateRegLex(lex, TK_const, tipo_float, 4);
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
                        updateRegLex(lex, tok, -99, 0);
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
                        updateRegLex(lex, TK_and, -99, 0);
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
                        updateRegLex(lex, TK_or, -99, 0);
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
                        updateRegLex(lex, TK_atrib, -99, 0);
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
                            updateRegLex(lex, TK_barra, -99, 0);
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
                        updateRegLex(lex, TK_const, tipo_caractere, 1);
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
                        updateRegLex(lex, TK_const, tipo_inteiro, 4);
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
                        updateRegLex(lex, TK_const, tipo_caractere, 1);
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

    RegLex rl_const; // regLex auxiliar p/ salvar copia do RL atual
    bool negativo = false; // flag que indica se houve o token '-'
    
    // const id = [-] constante
    if(registroLexico.token == TK_const){
        casaToken(TK_const);
        string lex_id = registroLexico.lexema;
        casaToken(TK_id);

        Simbolo simb_id = tab_simbolos.pesquisar(lex_id);

        // checa se id ja foi declarado
        if (simb_id.classe == classe_nula) {
            simb_id.classe = classe_constante; // id ainda nao declarado
        } 
        // id ja declarado
        else showError(DUPLICATE_ID, lex_id);

        casaToken(TK_igualdade);

        if(registroLexico.token == TK_menos){
            casaToken(TK_menos);
            negativo = true;
        }

        memcpy(&rl_const, &registroLexico, sizeof(registroLexico));

        casaToken(TK_const);

        if (negativo) {
            // ERRO se a constante nao for um numero
            if (rl_const.tipo != tipo_inteiro && rl_const.tipo != tipo_float){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // atribui tipo ao simbolo
            else if (rl_const.tipo == tipo_inteiro){
                simb_id.tipo = tipo_inteiro;
            }
            else simb_id.tipo = tipo_float;
        } 
        // nao houve sinal negativo:
        else if (rl_const.tipo == tipo_inteiro) simb_id.tipo = tipo_inteiro;
        else if (rl_const.tipo == tipo_float) simb_id.tipo = tipo_float;
        else if (rl_const.tipo == tipo_string) simb_id.tipo = tipo_string;
        else if (rl_const.tipo == tipo_caractere) simb_id.tipo = tipo_caractere;
        else if (rl_const.tipo == tipo_boolean) simb_id.tipo = tipo_boolean;
        else showError(INCOMPATIBLE_TYPES, "");

        // atualiza TS
        tab_simbolos.update(rl_const.lexema, simb_id);
    }
   
    // (int | float | char | string) id [:= [-] constante] {, id [:= [-] constante]}
    else if(registroLexico.token == TK_int || registroLexico.token == TK_float || registroLexico.token == TK_char || registroLexico.token == TK_string){
        int tipo_id = -99; // variavel auxiliar

        if (registroLexico.token == TK_int){
            tipo_id = tipo_inteiro;
            casaToken(TK_int);
        }
        else if(registroLexico.token == TK_float){
            tipo_id = tipo_float;
            casaToken(TK_float);
        }
        else if(registroLexico.token == TK_char){
            tipo_id = tipo_caractere;
            casaToken(TK_char);
        }
        else{
            tipo_id = tipo_string;
            casaToken(TK_string);
        }
        
        string lex_id = registroLexico.lexema;
        casaToken(TK_id);

        Simbolo simb_id = tab_simbolos.pesquisar(lex_id);
        // checa se id ja foi declarado
        if (simb_id.classe == classe_nula) {
            // id ainda nao declarado
            simb_id.classe = classe_variavel;
            simb_id.tipo = tipo_id;
            tab_simbolos.update(lex_id, simb_id); // atualiza TS
        }
        // id ja declarado
        else showError(DUPLICATE_ID, lex_id);

        if (registroLexico.token == TK_atrib){
            casaToken(TK_atrib);

            if(registroLexico.token == TK_menos){
                casaToken(TK_menos);
                // ERRO se a constante nao for um numero
                if (simb_id.tipo != tipo_inteiro && simb_id.tipo != tipo_float){
                    showError(INCOMPATIBLE_TYPES, "");
                }
                negativo = true;
            }
            // salva copia do reg lexico antes de casar o prox token
            memcpy(&rl_const, &registroLexico, sizeof(registroLexico));
            casaToken(TK_const);

            // ERRO se nao for numero e possuir o token '-'
            if ((rl_const.tipo != tipo_inteiro && rl_const.tipo != tipo_float) && negativo){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // ERRO se o id (simb_id) for float e a constante (rl_const) nao for um numero
            else if (simb_id.tipo == tipo_float){
                if (rl_const.tipo != tipo_inteiro && rl_const.tipo != tipo_float){
                    showError(INCOMPATIBLE_TYPES, "");
                }
            }
            // se id n eh float, ERRO se os tipos forem diferentes
            else if (simb_id.tipo != rl_const.tipo){
                showError(INCOMPATIBLE_TYPES, "");
            }
        }// fim if
        
        while(registroLexico.token == TK_virgula){
            casaToken(TK_virgula);
            lex_id = registroLexico.lexema;
            casaToken(TK_id);

            simb_id = tab_simbolos.pesquisar(lex_id);

            // checa se id ja foi declarado
            if (simb_id.classe == classe_nula) {
                // id ainda nao declarado
                simb_id.classe = classe_variavel;
                simb_id.tipo = tipo_id;
                tab_simbolos.update(lex_id, simb_id); // atualiza TS
            } 
            // id ja declarado
            else showError(DUPLICATE_ID, lex_id);

            negativo = false;
            if(registroLexico.token == TK_atrib){
                casaToken(TK_atrib);

                if (registroLexico.token == TK_menos){ 
                    casaToken(TK_menos);

                    // ERRO se id nao for numerico
                    if (simb_id.tipo != tipo_inteiro && simb_id.tipo != tipo_float){
                        showError(INCOMPATIBLE_TYPES, "");
                    }
                    negativo = true;
                }

                // copia reglex
                memcpy(&rl_const, &registroLexico, sizeof(registroLexico));
                casaToken(TK_const);

                // ERRO se nao for numero e possuir o token '-'
                if ((rl_const.tipo != tipo_inteiro && rl_const.tipo != tipo_float) && negativo){
                    showError(INCOMPATIBLE_TYPES, "");
                }
                // ERRO se o id (simb_id) for float e a constante (rl_const) nao for um numero
                else if (simb_id.tipo == tipo_float){
                    if (rl_const.tipo != tipo_inteiro && rl_const.tipo != tipo_float){
                        showError(INCOMPATIBLE_TYPES, "");
                    }
                }
                // se id n eh float, ERRO se os tipos forem diferentes
                else if (simb_id.tipo != rl_const.tipo){
                    showError(INCOMPATIBLE_TYPES, "");
                }
            }
        }// fim while

    }
    
    // boolean id [:= (true | false)] {, id [:= (true | false)]}

    else {
        casaToken(TK_boolean);
        string lex_id = registroLexico.lexema;
        casaToken(TK_id);
        // guarda simbolo da TS
        Simbolo simb_id = tab_simbolos.pesquisar(lex_id);

        /* (38) */
        // checa se id ja foi declarado
        if (simb_id.classe == classe_nula){
            // id ainda nao declarado
            simb_id.classe = classe_variavel;
            simb_id.tipo = tipo_boolean;
            tab_simbolos.update(lex_id, simb_id); // atualiza TS
        }
        // id ja declarado
        else showError(DUPLICATE_ID, lex_id);

        if(registroLexico.token == TK_atrib){
            casaToken(TK_atrib);
            if (registroLexico.token == TK_true) 
                casaToken(TK_true);
            else 
                casaToken(TK_false);
        }

        while(registroLexico.token == TK_virgula) {
            casaToken(TK_virgula);
            string lex_id = registroLexico.lexema;
            casaToken(TK_id);
            // guarda simbolo da TS
            Simbolo simb_id = tab_simbolos.pesquisar(lex_id);

            // checa se id ja foi declarado
            if (simb_id.classe == classe_nula){
                // id ainda nao declarado
                simb_id.classe = classe_variavel;
                simb_id.tipo = tipo_boolean;
                tab_simbolos.update(lex_id, simb_id); // atualiza TS
            }
            // id ja declarado
            else showError(DUPLICATE_ID, lex_id);

            if (registroLexico.token == TK_atrib) {
                casaToken(TK_atrib);
                if (registroLexico.token == TK_true) 
                    casaToken(TK_true);
                else 
                    casaToken(TK_false);
            }
        }// fim while
    }
}

/* Procedimento Comandos
* Comandos -> 
  (if "("Exp")" ( Comandos | "{" {Comandos} "}" ) [ else ( Comandos | "{" {Comandos} "}" ) ] | 
  while "("Exp")" ( Comandos | "{" {Comandos} "}" )) | 
  [ id [ "[" Exp "]" ] := Exp | readln "(" id ")" | (write | writeln) "(" Exp {, Exp} ")" ] ;
*/
void Comandos(){
    int tipo_Exp;
    // cout << "Comandos()" << "\n\n";
    // while e if
    if (registroLexico.token == TK_if || registroLexico.token == TK_while){

        // if "("Exp")" ( Comandos | "{" {Comandos} "}" ) [ else ( Comandos | "{" {Comandos} "}" ) ]
        if (registroLexico.token == TK_if){
            casaToken(TK_if);
            casaToken(TK_abreParentese);
            Exp(&tipo_Exp);
            casaToken(TK_fechaParentese);

            // ERRO se o tipo n for logico
            if (tipo_Exp != tipo_boolean){
                showError(INCOMPATIBLE_TYPES, "");
            }
            
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
            Exp(&tipo_Exp);
            casaToken(TK_fechaParentese);
            
            // ERRO se o tipo n for logico
            if (tipo_Exp != tipo_boolean){
                showError(INCOMPATIBLE_TYPES, "");
            } 

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
        string lex_id;
        Simbolo simb_id;
        // id [ "[" Exp "]" ] := Exp
        if(registroLexico.token == TK_id){
            bool is_char_array = false;

            lex_id = registroLexico.lexema; // guarda lexema do identificador
            casaToken(TK_id);
            simb_id = tab_simbolos.pesquisar(lex_id); // pega o simbolo desse identificador na TS

            // checa se id eh constante
            if (simb_id.classe == classe_constante){
                showError(INCOMPATIBLE_CLASSES, lex_id);
            }
            // checa se o identificador ja foi declarado
            else if (simb_id.classe == classe_nula){
                showError(UNDECLARED_ID, lex_id);
            }

            if(registroLexico.token == TK_abreColchete){
                casaToken(TK_abreColchete);

                // ERRO se o id nao for string
                if (simb_id.tipo != tipo_string){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                Exp(&tipo_Exp);

                // ERRO se o indice nao for um numero inteiro
                if (tipo_Exp != tipo_inteiro){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                is_char_array = true;
                casaToken(TK_fechaColchete);
            }

            casaToken(TK_atrib);
            Exp(&tipo_Exp);

            // checando os tipos de id (simb_id) e Exp (tipo_Exp)
            // se id for string
            if (simb_id.tipo == tipo_string){
                // se id for do tipo indice de um char (id[numero]):
                if (is_char_array){
                    if (tipo_Exp != tipo_caractere){
                        showError(INCOMPATIBLE_TYPES, "");
                    }
                }
                // se id for uma string:
                else if (tipo_Exp != tipo_string){
                    showError(INCOMPATIBLE_TYPES, "");
                }
                
            }
            // se id for char e Exp n for
            else if (simb_id.tipo == tipo_caractere && tipo_Exp != tipo_caractere){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // se id for int e Exp nao for int:
            else if (simb_id.tipo == tipo_inteiro && tipo_Exp != tipo_inteiro){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // se id for float e Exp n for numero:
            else if (simb_id.tipo == tipo_float){
                if (tipo_Exp != tipo_inteiro && tipo_Exp != tipo_float){
                    showError(INCOMPATIBLE_TYPES, "");
                }
            }

        }// fim da producao de atribuicao
        
        // readln "(" id ")"
        else if(registroLexico.token == TK_readln){
            casaToken(TK_readln);
            casaToken(TK_abreParentese);
            lex_id = registroLexico.lexema; // guarda lexema do identificador p/ a busca na TS
            casaToken(TK_id);

            simb_id = tab_simbolos.pesquisar(lex_id); // busca na TS

            // checa se foi declarado
            if (simb_id.classe == classe_nula){
                showError(UNDECLARED_ID, lex_id);
            }
            // checa se eh uma constante
            else if (simb_id.classe == classe_constante){
                showError(INCOMPATIBLE_CLASSES, lex_id);
            }

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
            Exp(&tipo_Exp);

            // o tipo nao pode ser logico
            if (tipo_Exp == tipo_boolean){
                showError(INCOMPATIBLE_TYPES, "");
            }

            while(registroLexico.token == TK_virgula){
                casaToken(TK_virgula);
                Exp(&tipo_Exp);

                // o tipo nao pode ser logico
                if (tipo_Exp == tipo_boolean){
                    showError(INCOMPATIBLE_TYPES, "");
                }
            }// fim while

            casaToken(TK_fechaParentese);
            
        }// fim da producao de escrita

        casaToken(TK_pontoevirgula);
    
    }// fim das producoes opcionais

}// fim Comandos()

/* Procedimento Exp
  Exp -> ExpS [ (= | != | < | > | <= | >=) ExpS]
*/
void Exp(int* tipo_Exp){
    int tipo_ExpS, tipo_ExpS1;
    // cout << "Exp()" << "\n\n";
    ExpS(&tipo_ExpS);

    switch(registroLexico.token){
        case TK_igualdade:
            casaToken(TK_igualdade);
            ExpS(&tipo_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == tipo_caractere && tipo_ExpS1 != tipo_caractere){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == tipo_string && tipo_ExpS1 != tipo_string){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == tipo_inteiro || tipo_ExpS == tipo_float) && (tipo_ExpS1 != tipo_inteiro && tipo_ExpS1 != tipo_float)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = tipo_boolean;

        break;

        case TK_diferente:
            casaToken(TK_diferente);
            ExpS(&tipo_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == tipo_caractere && tipo_ExpS1 != tipo_caractere){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == tipo_string && tipo_ExpS1 == tipo_string){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == tipo_inteiro || tipo_ExpS == tipo_float) && (tipo_ExpS1 != tipo_inteiro && tipo_ExpS1 != tipo_float)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = tipo_boolean;
        break;

        case TK_maior:
            casaToken(TK_maior);
            ExpS(&tipo_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == tipo_caractere && tipo_ExpS1 != tipo_caractere){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == tipo_string && tipo_ExpS1 == tipo_string){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == tipo_inteiro || tipo_ExpS == tipo_float) && (tipo_ExpS1 != tipo_inteiro && tipo_ExpS1 != tipo_float)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = tipo_boolean;
        break;

        case TK_menor:
            casaToken(TK_menor);
            ExpS(&tipo_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == tipo_caractere && tipo_ExpS1 != tipo_caractere){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == tipo_string && tipo_ExpS1 == tipo_string){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == tipo_inteiro || tipo_ExpS == tipo_float) && (tipo_ExpS1 != tipo_inteiro && tipo_ExpS1 != tipo_float)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = tipo_boolean;
        break;

        case TK_menorIgual:
            casaToken(TK_menorIgual);
            ExpS(&tipo_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == tipo_caractere && tipo_ExpS1 != tipo_caractere){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == tipo_string && tipo_ExpS1 == tipo_string){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == tipo_inteiro || tipo_ExpS == tipo_float) && (tipo_ExpS1 != tipo_inteiro && tipo_ExpS1 != tipo_float)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = tipo_boolean;
        break;

        case TK_maiorIgual:
            casaToken(TK_maiorIgual);
            ExpS(&tipo_ExpS1);

            // verifica tipos dos termos a serem comparados
            // char - char
            if (tipo_ExpS == tipo_caractere && tipo_ExpS1 != tipo_caractere){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // string - string
            else if (tipo_ExpS == tipo_string && tipo_ExpS1 == tipo_string){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // numero - numero
            else if ((tipo_ExpS == tipo_inteiro || tipo_ExpS == tipo_float) && (tipo_ExpS1 != tipo_inteiro && tipo_ExpS1 != tipo_float)){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // nao houve erro, atribui tipo correto
            else tipo_ExpS = tipo_boolean;
        break;
    
    default:
        break;

    }// fim switch
    tipo_Exp = &tipo_ExpS;
}// fim Exp()


/* Procedimento ExpS
  ExpS -> [-] T { (+ | - | "||") T}
*/
void ExpS(int* tipo_ExpS) {
    int tipo_T, tipo_T1;
    bool subtracao = false;
    bool adicao = false;
    // cout << "ExpS()" << "\n\n";
    if (registroLexico.token == TK_menos){
        subtracao = true;
        casaToken(TK_menos);
    }
    T(&tipo_T);

    // se possui o token '-', deve ser um numero
    if (subtracao && (tipo_T != tipo_inteiro && tipo_T != tipo_float)){
        showError(INCOMPATIBLE_TYPES, "");
    }

    while (registroLexico.token == TK_mais || registroLexico.token == TK_menos || registroLexico.token == TK_or) {
        subtracao = adicao = false;

        switch (registroLexico.token){
            case TK_mais:
                casaToken(registroLexico.token);
                // o termo deve ser um numero
                if (tipo_T != tipo_inteiro && tipo_T != tipo_float){
                    showError(INCOMPATIBLE_TYPES, "");
                }
                adicao = true;
                break;

            case TK_menos:
                casaToken(registroLexico.token);
                // o termo deve ser um numero
                if (tipo_T != tipo_inteiro && tipo_T != tipo_float){
                    showError(INCOMPATIBLE_TYPES, "");
                }
                subtracao = true;
                break;

            case TK_or:
                casaToken(registroLexico.token);
                // o termo deve ser logico
                if (tipo_T != tipo_boolean){
                    showError(INCOMPATIBLE_TYPES, "");
                }
                break;
        }// fim switch
        
        T(&tipo_T1);

        // checa tipos para +, -, ||
        if (adicao || subtracao){
            if (tipo_T1 != tipo_inteiro && tipo_T1 != tipo_float){
                showError(INCOMPATIBLE_TYPES, "");
            }
            else if (tipo_T == tipo_inteiro && tipo_T1 == tipo_inteiro){
                tipo_T = tipo_inteiro;
            }
            else{
                tipo_T = tipo_float;
            }
        }
        // ERRO se um termo for logico e o outro nao
        else if (tipo_T == tipo_boolean && tipo_T1 != tipo_boolean){
            showError(INCOMPATIBLE_TYPES, "");
        }
    }// fim while

    tipo_ExpS = &tipo_T;
}

/* Procedimento T
  T -> F { (* | && | / | div | mod) F}
*/
void T(int* tipo_T) {
    int tipo_F, tipo_F1;
    bool multiplicacao = false;
    bool divisao = false;
    // cout << "T()" << "\n\n";
    F(&tipo_F);
    while (
        registroLexico.token == TK_asterisco || 
        registroLexico.token == TK_and       || 
        registroLexico.token == TK_barra     || 
        registroLexico.token == TK_div       || 
        registroLexico.token == TK_mod
    ) {
        multiplicacao = divisao = false;

        switch(registroLexico.token){
            case TK_barra:
                casaToken(registroLexico.token);

                // ERRO se fator nao for numero
                if (tipo_F != tipo_inteiro && tipo_F != tipo_float){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                divisao = true;
                break;
                
            case TK_asterisco:
                casaToken(registroLexico.token);

                // ERRO se fator nao for numero
                if (tipo_F != tipo_inteiro && tipo_F != tipo_float){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                multiplicacao = true;
                break;

            case TK_and:
                casaToken(registroLexico.token);

                // ERRO se fator nao for logico
                if (tipo_F != tipo_boolean){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                break;


            case TK_div:
                casaToken(registroLexico.token);

                // ERRO se fator nao for um numero inteiro
                if (tipo_F != tipo_inteiro){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                break;

            case TK_mod:
                casaToken(registroLexico.token);
                
                // ERRO se fator nao for um numero inteiro
                if (tipo_F != tipo_inteiro){
                    showError(INCOMPATIBLE_TYPES, "");
                }

                break;
        }// fim switch
        
        F(&tipo_F1);

        // checa *, /, &&, div, mod
        if (divisao || multiplicacao){
            // ERRO se n for um numero
            if (tipo_F1 != tipo_inteiro && tipo_F1 != tipo_float){
                showError(INCOMPATIBLE_TYPES, "");
            }
            // se for divisao o resultado sera um numero real
            else if (divisao){
                tipo_F = tipo_float;
            }
            // multiplicacao
            else if (tipo_F == tipo_inteiro && tipo_F1 == tipo_inteiro){
                // ambos fatores inteiros, resultado inteiro
                tipo_F = tipo_inteiro;
            }
            else{
                // se pelo menos um fator for real, resultado sera real
                tipo_F = tipo_float;
            }
        }
        // && exige que os 2 fatores sejam logicos
        else if (tipo_F == tipo_boolean && tipo_F1 != tipo_boolean){
            showError(INCOMPATIBLE_TYPES, "");
        }
        // div e mod exigem que os 2 fatores sejam inteiros
        else if (tipo_F == tipo_inteiro && tipo_F1 != tipo_inteiro){
            showError(INCOMPATIBLE_TYPES, "");
        }
    }// fim while

    tipo_T = &tipo_F;
}// fim T()

/* Procedimento F
  F -> constante | (int | float) "(" Exp ")" | "(" Exp ")" | id [ "[" Exp "]" ] | ! F
*/
void F(int* tipo_F) {
    int tipo_EXP;

    // constante
    if (registroLexico.token == TK_const) {
        casaToken(TK_const);
        *tipo_F = registroLexico.tipo;
    }

    // (int | float) "(" Exp ")"
    else if (registroLexico.token == TK_int || registroLexico.token == TK_float) {
        if (registroLexico.token == TK_int) {
            casaToken(TK_int);
            *tipo_F = tipo_inteiro;
        }
        else {
            casaToken(TK_float);
            *tipo_F = tipo_float;
        }
        casaToken(TK_abreParentese);
        
        Exp(&tipo_EXP);
        if (tipo_EXP != *tipo_F)
            showError(INCOMPATIBLE_TYPES, "");

        casaToken(TK_fechaParentese);
    }

    // "(" Exp ")"
    else if (registroLexico.token == TK_abreParentese) {
        casaToken(TK_abreParentese);
        
        Exp(&tipo_EXP);
        *tipo_F = tipo_EXP;

        casaToken(TK_fechaParentese);
    }

    // id [ "[" Exp "]" ]
    else if (registroLexico.token == TK_id) {
        string lex_id = registroLexico.lexema;
        casaToken(TK_id);
        
        // Verifica se é um id que ainda não foi declarado
        Simbolo simb_id = tab_simbolos.pesquisar(lex_id);
        if (simb_id.classe == classe_nula) {
            showError(UNDECLARED_ID, "");
        } else {
            // Se foi declarado, pega o tipo dele
            *tipo_F = simb_id.tipo;
        }

        if (registroLexico.token == TK_abreColchete) {
            casaToken(TK_abreColchete);

            // Se vai usar id[], o id tem que ser string
            if (simb_id.tipo != tipo_string)
                showError(INCOMPATIBLE_TYPES, "");

            Exp(&tipo_EXP);

            // Posição da string tem que ser um numero inteiro
            if (tipo_EXP != tipo_inteiro)
                showError(INCOMPATIBLE_TYPES, "");
            else
                *tipo_F = tipo_caractere;

            casaToken(TK_fechaColchete);
        }
    }

    // ! F1
    else if (registroLexico.token == TK_not) {
        int tipo_F1;
        casaToken(TK_not);
        
        F(&tipo_F1);

        // O tipo de F1 tem que ser lógico
        if (tipo_F1 != tipo_boolean)
            showError(INCOMPATIBLE_TYPES, "");
        else
            *tipo_F = tipo_F1;
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