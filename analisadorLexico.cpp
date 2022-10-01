#include<iostream>
#include<string>
#include<unordered_map>
#include<vector>
// #include<tuple>

using namespace std;

#define tk_const            0
#define tk_int              1
#define tk_char             2
#define tk_while            3
#define tk_if               4
#define tk_float            5
#define tk_else             6
#define tk_and              7
#define tk_or               8
#define tk_not              9
#define tk_atrib            10
#define tk_igualdade        11
#define tk_abreParentese    12
#define tk_fechaParentese   13
#define tk_menor            14
#define tk_maior            15
#define tk_diferente        16
#define tk_maiorIgual       17
#define tk_menorIgual       18
#define tk_virgula          19
#define tk_hifen            20
#define tk_asterisco        21
#define tk_barra            22
#define tk_pontoevirgula    23
#define tk_abreChave        24
#define tk_fechaChave       25
#define tk_readln           26
#define tk_div              27
#define tk_string           28
#define tk_write            29
#define tk_writeln          30
#define tk_mod              31
#define tk_abreColchete     32
#define tk_fechaColchete    33
#define tk_true             34
#define tk_false            35
#define tk_boolean          36

// typedef vector< tuple<string,string> > tuple_list;

// struct Token
// {
//     int token;
// };


class Tabela_simbolos{
    public:
        // tabela hash que mapeia lexema para token
        unordered_map<string, int> tab_simbolos; 

        Tabela_simbolos(); // construtor
        int inserir(string, int);

    // insere as palavras reservadas na tabela de simbolos
    Tabela_simbolos()
    {
        tab_simbolos.insert(make_pair("const" , tk_const));
        tab_simbolos.insert(make_pair("int" , tk_int));
        tab_simbolos.insert(make_pair("char" , tk_char));
        tab_simbolos.insert(make_pair("while" , tk_while));
        tab_simbolos.insert(make_pair("if" , tk_if));
        tab_simbolos.insert(make_pair("float" , tk_float));
        tab_simbolos.insert(make_pair("else" , tk_else));
        tab_simbolos.insert(make_pair("&&" , tk_and));
        tab_simbolos.insert(make_pair("||" , tk_or));
        tab_simbolos.insert(make_pair("!" , tk_not));
        tab_simbolos.insert(make_pair(":=" , tk_atrib));
        tab_simbolos.insert(make_pair("=" , tk_igualdade));
        tab_simbolos.insert(make_pair("(" , tk_abreParentese));
        tab_simbolos.insert(make_pair(")" , tk_fechaParentese));
        tab_simbolos.insert(make_pair("<" , tk_menor));
        tab_simbolos.insert(make_pair(">" , tk_maior));
        tab_simbolos.insert(make_pair("!=" , tk_diferente));
        tab_simbolos.insert(make_pair(">=" , tk_maiorIgual));
        tab_simbolos.insert(make_pair("<=" , tk_menorIgual));
        tab_simbolos.insert(make_pair("," , tk_virgula));
        tab_simbolos.insert(make_pair("-" , tk_hifen));
        tab_simbolos.insert(make_pair("*" , tk_asterisco));
        tab_simbolos.insert(make_pair("/" , tk_barra));
        tab_simbolos.insert(make_pair(";" , tk_pontoevirgula));
        tab_simbolos.insert(make_pair("{" , tk_abreChave));
        tab_simbolos.insert(make_pair("}" , tk_fechaChave));
        tab_simbolos.insert(make_pair("readln" , tk_readln));
        tab_simbolos.insert(make_pair("div" , tk_div));
        tab_simbolos.insert(make_pair("string" , tk_string));
        tab_simbolos.insert(make_pair("write" , tk_write));
        tab_simbolos.insert(make_pair("writeln" , tk_writeln));
        tab_simbolos.insert(make_pair("mod" , tk_mod));
        tab_simbolos.insert(make_pair("[" , tk_abreColchete));
        tab_simbolos.insert(make_pair("]" , tk_fechaColchete));
        tab_simbolos.insert(make_pair("true" , tk_true));
        tab_simbolos.insert(make_pair("false" , tk_false));
        tab_simbolos.insert(make_pair("boolean" , tk_boolean));

        // inserir("const" , tk_const);
        // inserir("int" , tk_int);
        // inserir("char" , tk_char);
        // inserir("while" , tk_while);
        // inserir("if" , tk_if);
        // inserir("float" , tk_float);
        // inserir("else" , tk_else);
        // inserir("&&" , tk_and);
        // inserir("||" , tk_or);
        // inserir("!" , tk_not);
        // inserir(":=" , tk_atrib);
        // inserir("=" , tk_igualdade);
        // inserir("(" , tk_abreParentese);
        // inserir(")" , tk_fechaParentese);
        // inserir("<" , tk_menor);
        // inserir(">" , tk_maior);
        // inserir("!=" , tk_diferente);
        // inserir(">=" , tk_maiorIgual);
        // inserir("<=" , tk_menorIgual);
        // inserir("," , tk_virgula);
        // inserir("-" , tk_hifen);
        // inserir("*" , tk_asterisco);
        // inserir("/" , tk_barra);
        // inserir(";" , tk_pontoevirgula);
        // inserir("{" , tk_abreChave);
        // inserir("}" , tk_fechaChave);
        // inserir("readln" , tk_readln);
        // inserir("div" , tk_div);
        // inserir("string" , tk_string);
        // inserir("write" , tk_write);
        // inserir("writeln" , tk_writeln);
        // inserir("mod" , tk_mod);
        // inserir("[" , tk_abreColchete);
        // inserir("]" , tk_fechaColchete);
        // inserir("true" , tk_true);
        // inserir("false" , tk_false);
        // inserir("boolean" , tk_boolean);
    }

    /*return pesquisar(string lexema)
    {

    }*/
    


};

string lexema;
int numLinhas = 0;

// AUTOMATO
string getNextToken(string lexema)
{
    int estado_atual = 0;
    int estado_final = (?);
    string lexema = "";
    char c;
    
    //SWITCH gigante
}

int main(int argc, char const *argv[])
{
    Tabela_simbolos tab_simbolos;

    while (cin >> lexema)
    {
        
        getNextToken(lexema);
    }


    return 0;
}
