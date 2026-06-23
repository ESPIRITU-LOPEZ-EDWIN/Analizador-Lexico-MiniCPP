/*
 * ============================================================
 *  COMPILADOR MiniC++ – Lexico + Sintactico + Semantico
 *  AFD (switch-estado) + Descendente recursivo
 *  INF 3631 "A" – Diseno de Compiladores
 * ============================================================
 *
 *  Entrada : programa2.txt  (codigo fuente MiniC++)
 *
 *  Salida  :
 *    [1] Tabla  LEXEMA | TIPO | CATEGORIA | LINEA
 *    [2] Lista de errores lexicos
 *    [3] RESULTADO SINTACTICO: VALIDO / ERROR
 *    [4] Tabla de simbolos  NOMBRE | TIPO | LINEA | INIC | USADA
 *    [5] Lista de errores semanticos
 *    [6] RESULTADO SEMANTICO: VALIDO / ERROR
 *
 *  Errores semanticos detectados:
 *    [E1] Variable no declarada
 *    [E2] Variable declarada dos veces
 *    [E3] Tipo incompatible en asignacion (int <- float o viceversa)
 *    [E4] Variable usada antes de ser inicializada
 *    [E5] Variable declarada pero nunca usada  (advertencia)
 *    [E6] Tipos incompatibles en condicion
 *    [E7] Tipos incompatibles en expresion aritmetica
 *
 *  Gramatica EBNF (MiniC++):
 *    programa     = bloque
 *    bloque       = instruccion { instruccion }
 *    instruccion  = declaracion | asignacion | entrada | salida | si
 *    declaracion  = tipo lista_id ";"
 *    tipo         = "int" | "float"
 *    lista_id     = id { "," id }
 *    entrada      = "cin" ">>" id ";"
 *    salida       = "cout" "<<" valor_salida ";"
 *    valor_salida = id | cadena
 *    asignacion   = id "=" expresion ";"
 *    expresion    = valor [ op_arit valor ]
 *    op_arit      = "+" | "-" | "*" | "/"
 *    valor        = id | numero
 *    si           = "if" "(" condicion ")" "{" bloque "}"
 *                   [ "else" "{" bloque "}" ]
 *    condicion    = valor op_rel valor
 *    op_rel       = "<" | ">" | "<=" | ">=" | "=="
 * ============================================================
 */
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
using namespace std;

#define TK_INT        1
#define TK_FLOAT      2
#define TK_CIN       10
#define TK_COUT      11
#define TK_IF        20
#define TK_ELSE      21
#define TK_ID      1000
#define TK_NUM     2000
#define TK_STRING  3000
#define TK_PUNTOCOMA  40
#define TK_COMA       41
#define TK_PAR_AB     42
#define TK_PAR_CE     43
#define TK_LLAVE_AB   44
#define TK_LLAVE_CE   45
#define TK_IGUAL      60
#define TK_MAS        70
#define TK_MENOS      71
#define TK_POR        72
#define TK_DIV        73
#define TK_MENOR      80
#define TK_MAYOR      81
#define TK_MENORI     82
#define TK_MAYORI     83
#define TK_IGUAL2     84
#define TK_MAYOR2     90
#define TK_MENOR2     91
#define TK_EOF        -1

struct Token {
    string lexema;
    int    tipo;
    int    linea;
};

vector<Token>   tokens;
vector<string>  erroresLex;

bool esLetra(char c)  { return isalpha((unsigned char)c); }
bool esDigito(char c) { return (c >= '0' && c <= '9'); }
bool esCI(char c) {
    return isspace((unsigned char)c) || c == '\0' ||
           c == ';' || c == ',' || c == '(' || c == ')' ||
           c == '{' || c == '}' || c == '"' ||
           c == '=' || c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '<' || c == '>';
}
string nombreCategoria(int tipo) {
    if (tipo == 1 || tipo == 2)           return "RESERVADA";
    if (tipo == 10 || tipo == 11)         return "RESERVADA";
    if (tipo == 20 || tipo == 21)         return "RESERVADA";
    if (tipo == 1000)                     return "IDENTIFICADOR";
    if (tipo == 2000)                     return "NUMERO";
    if (tipo == 3000)                     return "CADENA";
    if (tipo == 40 || tipo == 41)         return "SIMBOLO";
    if (tipo >= 42 && tipo <= 45)         return "SIMBOLO";
    if (tipo == 60)                       return "OPERADOR";
    if (tipo >= 70 && tipo <= 73)         return "OPERADOR";
    if (tipo >= 80 && tipo <= 84)         return "OPERADOR";
    if (tipo == 90 || tipo == 91)         return "OPERADOR";
    return "DESCONOCIDO";
}
void separador(int n = 55) { for (int i = 0; i < n; i++) cout << '-'; cout << '\n'; }

void analizarLexico(const string& entrada, vector<Token>& tokens, vector<string>& errores)
{
    int    i = 0, estado = 0, linea = 1;
    string lexema = "";
    char   c = '\0';
    auto leer = [&]() {
        if (i < (int)entrada.size()) { c = entrada[i++]; if (c == '\n') linea++; }
        else c = '\0';
    };
    leer();
    while (true) {
        switch (estado) {
        case 0:
            lexema = "";
            if (c == '\0') return;
            if (isspace((unsigned char)c)) { leer(); break; }
            if (esLetra(c)) {
                lexema += c; leer();
                if (lexema=="i" && c=='n') { lexema+=c; leer(); estado=-1;  break; }
                if (lexema=="f" && c=='l') { lexema+=c; leer(); estado=-2;  break; }
                if (lexema=="c" && c=='i') { lexema+=c; leer(); estado=-10; break; }
                if (lexema=="c" && c=='o') { lexema+=c; leer(); estado=-11; break; }
                if (lexema=="i" && c=='f') { lexema+=c; leer(); estado=-20; break; }
                if (lexema=="e" && c=='l') { lexema+=c; leer(); estado=-21; break; }
                estado = -1000; break;
            }
            if (esDigito(c)) { lexema+=c; leer(); estado=-2000; break; }
            if (c=='"')      { leer(); estado=-3000; break; }
            if (c==';') { lexema=";"; leer(); estado=-40; break; }
            if (c==',') { lexema=","; leer(); estado=-41; break; }
            if (c=='(') { lexema="("; leer(); estado=-42; break; }
            if (c==')') { lexema=")"; leer(); estado=-43; break; }
            if (c=='{') { lexema="{"; leer(); estado=-44; break; }
            if (c=='}') { lexema="}"; leer(); estado=-45; break; }
            if (c=='+') { lexema="+"; leer(); estado=-70; break; }
            if (c=='-') { lexema="-"; leer(); estado=-71; break; }
            if (c=='*') { lexema="*"; leer(); estado=-72; break; }
            if (c=='/') { lexema="/"; leer(); estado=-73; break; }
            if (c=='=') { lexema="="; leer(); estado=-60; break; }
            if (c=='<') { lexema="<"; leer(); estado=-80; break; }
            if (c=='>') { lexema=">"; leer(); estado=-81; break; }
            errores.push_back("Error lexico (linea " + to_string(linea) + "): caracter invalido -> " + string(1,c));
            leer(); break;
        case -1:
            if (c=='t') { lexema+=c; leer(); if (esCI(c)) { estado=1; break; } }
            estado=-1000; break;
        case 1:  tokens.push_back({"int",  1,  linea}); estado=0; break;
        case -2:
            if (c=='o') { lexema+=c; leer();
            if (c=='a') { lexema+=c; leer();
            if (c=='t') { lexema+=c; leer();
                if (esCI(c)) { estado=2; break; }
            }}}
            estado=-1000; break;
        case 2:  tokens.push_back({"float",2,  linea}); estado=0; break;
        case -10:
            if (c=='n') { lexema+=c; leer(); if (esCI(c)) { estado=10; break; } }
            estado=-1000; break;
        case 10: tokens.push_back({"cin",  10, linea}); estado=0; break;
        case -11:
            if (c=='u') { lexema+=c; leer();
            if (c=='t') { lexema+=c; leer();
                if (esCI(c)) { estado=11; break; }
            }}
            estado=-1000; break;
        case 11: tokens.push_back({"cout", 11, linea}); estado=0; break;
        case -20:
            if (esCI(c)) { estado=20; break; }
            estado=-1000; break;
        case 20: tokens.push_back({"if",   20, linea}); estado=0; break;
        case -21:
            if (c=='s') { lexema+=c; leer();
            if (c=='e') { lexema+=c; leer();
                if (esCI(c)) { estado=21; break; }
            }}
            estado=-1000; break;
        case 21: tokens.push_back({"else", 21, linea}); estado=0; break;
        case -1000:
            if (esLetra(c)) { lexema+=c; leer(); estado=-1000; }
            else             { estado=1000; }
            break;
        case 1000: tokens.push_back({lexema,1000,linea}); estado=0; break;
        case -2000:
            if (esDigito(c)) { lexema+=c; leer(); estado=-2000; }
            else if (c=='.') {
                if (i < (int)entrada.size() && esDigito(entrada[i]))
                    { lexema+=c; leer(); estado=-2001; }
                else {
                    errores.push_back("Error lexico (linea " + to_string(linea) + "): numero mal formado -> " + lexema + ".");
                    leer(); estado=0;
                }
            }
            else if (esLetra(c)) {
                while (esLetra(c) || esDigito(c) || c=='.') { lexema+=c; leer(); }
                errores.push_back("Error lexico (linea " + to_string(linea) + "): ID no puede empezar con numero -> " + lexema);
                estado=0;
            }
            else { estado=2000; }
            break;
        case -2001:
            if (esDigito(c)) { lexema+=c; leer(); estado=-2001; }
            else if (esCI(c)) { estado=2000; }
            else {
                errores.push_back("Error lexico (linea " + to_string(linea) + "): numero mal formado -> " + lexema);
                estado=0;
            }
            break;
        case 2000: tokens.push_back({lexema,2000,linea}); estado=0; break;
        case -3000:
            if (c=='\0') {
                errores.push_back("Error lexico (linea " + to_string(linea) + "): cadena sin cerrar -> \"" + lexema);
                estado=0;
            } else if (c=='"') { leer(); estado=3000; }
            else { lexema+=c; leer(); estado=-3000; }
            break;
        case 3000: tokens.push_back({"\""+lexema+"\"",3000,linea}); estado=0; break;
        case -40: tokens.push_back({";", 40,linea}); estado=0; break;
        case -41: tokens.push_back({",", 41,linea}); estado=0; break;
        case -42: tokens.push_back({"(", 42,linea}); estado=0; break;
        case -43: tokens.push_back({")", 43,linea}); estado=0; break;
        case -44: tokens.push_back({"{", 44,linea}); estado=0; break;
        case -45: tokens.push_back({"}", 45,linea}); estado=0; break;
        case -70: tokens.push_back({"+", 70,linea}); estado=0; break;
        case -71: tokens.push_back({"-", 71,linea}); estado=0; break;
        case -72: tokens.push_back({"*", 72,linea}); estado=0; break;
        case -73: tokens.push_back({"/", 73,linea}); estado=0; break;
        case -60:
            if (c=='=') { lexema+="="; leer(); tokens.push_back({"==",84,linea}); }
            else         { tokens.push_back({"=",60,linea}); }
            estado=0; break;
        case -80:
            if (c=='=') { lexema+="="; leer(); tokens.push_back({"<=",82,linea}); }
            else if (c=='<') { lexema+="<"; leer(); tokens.push_back({"<<",91,linea}); }
            else              { tokens.push_back({"<",80,linea}); }
            estado=0; break;
        case -81:
            if (c=='=') { lexema+="="; leer(); tokens.push_back({">=",83,linea}); }
            else if (c=='>') { lexema+=">"; leer(); tokens.push_back({">>",90,linea}); }
            else              { tokens.push_back({">",81,linea}); }
            estado=0; break;
        default: estado=0; break;
        }
    }
}

// ════════════════════════════════════════════════════════════
//  ANALIZADOR SINTACTICO
// ════════════════════════════════════════════════════════════
string nombreToken(int t) {
    switch (t) {
        case TK_INT:       return "int";
        case TK_FLOAT:     return "float";
        case TK_CIN:       return "cin";
        case TK_COUT:      return "cout";
        case TK_IF:        return "if";
        case TK_ELSE:      return "else";
        case TK_ID:        return "identificador";
        case TK_NUM:       return "numero";
        case TK_STRING:    return "cadena";
        case TK_PUNTOCOMA: return ";";
        case TK_COMA:      return ",";
        case TK_PAR_AB:    return "(";
        case TK_PAR_CE:    return ")";
        case TK_LLAVE_AB:  return "{";
        case TK_LLAVE_CE:  return "}";
        case TK_IGUAL:     return "=";
        case TK_MAS:       return "+";
        case TK_MENOS:     return "-";
        case TK_POR:       return "*";
        case TK_DIV:       return "/";
        case TK_MENOR:     return "<";
        case TK_MAYOR:     return ">";
        case TK_MENORI:    return "<=";
        case TK_MAYORI:    return ">=";
        case TK_IGUAL2:    return "==";
        case TK_MAYOR2:    return ">>";
        case TK_MENOR2:    return "<<";
        case TK_EOF:       return "EOF";
        default:           return "?";
    }
}

int    pos      = 0;
Token  tk;
bool   hayError = false;
string msgError = "";

void leer() {
    if (pos < (int)tokens.size()) tk = tokens[pos++];
    else tk = {"EOF", TK_EOF, 0};
}
void error(const string& esperado) {
    if (!hayError) {
        hayError = true;
        msgError = "en linea " + to_string(tk.linea) +
                   ": se esperaba '" + esperado +
                   "', se encontro '" + tk.lexema + "'";
    }
}
void verificar(int tipo) {
    if (tk.tipo == tipo) leer();
    else error(nombreToken(tipo));
}

void bloque(); void instruccion(); void declaracion(); void listaID();
void entrada(); void salida();    void valor_salida(); void asignacion();
void expresion(); void valor();   void si();           void condicion();

void bloque() {
    instruccion();
    while (!hayError &&
           (tk.tipo==TK_INT || tk.tipo==TK_FLOAT || tk.tipo==TK_CIN ||
            tk.tipo==TK_COUT || tk.tipo==TK_IF   || tk.tipo==TK_ID))
        instruccion();
}
void instruccion() {
    switch (tk.tipo) {
        case TK_INT: case TK_FLOAT: declaracion(); break;
        case TK_CIN:  entrada();  break;
        case TK_COUT: salida();   break;
        case TK_IF:   si();       break;
        case TK_ID:   asignacion(); break;
        default: error("int | float | cin | cout | if | identificador"); break;
    }
}
void declaracion() {
    if (tk.tipo==TK_INT || tk.tipo==TK_FLOAT) leer(); else error("int | float");
    listaID(); verificar(TK_PUNTOCOMA);
}
void listaID() {
    verificar(TK_ID);
    while (!hayError && tk.tipo==TK_COMA) { leer(); verificar(TK_ID); }
}
void entrada()  { verificar(TK_CIN);  verificar(TK_MAYOR2); verificar(TK_ID); verificar(TK_PUNTOCOMA); }
void salida()   { verificar(TK_COUT); verificar(TK_MENOR2); valor_salida();   verificar(TK_PUNTOCOMA); }
void valor_salida() {
    if (tk.tipo==TK_ID || tk.tipo==TK_STRING) leer();
    else error("identificador | cadena");
}
void asignacion() { verificar(TK_ID); verificar(TK_IGUAL); expresion(); verificar(TK_PUNTOCOMA); }
void expresion() {
    valor();
    if (!hayError && (tk.tipo==TK_MAS || tk.tipo==TK_MENOS ||
                      tk.tipo==TK_POR || tk.tipo==TK_DIV))
    { leer(); valor(); }
}
void valor() {
    if (tk.tipo==TK_ID || tk.tipo==TK_NUM) leer();
    else error("identificador | numero");
}
void si() {
    verificar(TK_IF); verificar(TK_PAR_AB); condicion();
    verificar(TK_PAR_CE); verificar(TK_LLAVE_AB); bloque(); verificar(TK_LLAVE_CE);
    if (!hayError && tk.tipo==TK_ELSE) {
        leer(); verificar(TK_LLAVE_AB); bloque(); verificar(TK_LLAVE_CE);
    }
}
void condicion() {
    valor();
    if (!hayError && (tk.tipo==TK_MENOR  || tk.tipo==TK_MAYOR  ||
                      tk.tipo==TK_MENORI || tk.tipo==TK_MAYORI ||
                      tk.tipo==TK_IGUAL2))
    { leer(); valor(); }
    else if (!hayError) error("< | > | <= | >= | ==");
}

// ════════════════════════════════════════════════════════════
//  ANALIZADOR SEMANTICO
// ════════════════════════════════════════════════════════════
struct Simbolo { // para guardar inf de las variables
    string nombre;
    string tipo;         // "int" o "float"
    int    linea;        // linea de declaracion
    bool   inicializada; // recibio valor via cin o asignacion
    bool   usada;        // aparece en expresion, condicion o cout
};

vector<Simbolo>  tablaSimbolos;
vector<string>   erroresSem;

int buscarSimbolo(const string& nombre) {
    for (int i = 0; i < (int)tablaSimbolos.size(); i++)
        if (tablaSimbolos[i].nombre == nombre) return i;
    return -1;
}

// ── Helpers ───────────────────────────────────────────────────

// Tipo de un token valor: "int", "float" o "?"
string getTipoValor(const Token& t) {
    if (t.tipo == TK_NUM) {
        for (char ch : t.lexema) if (ch == '.') return "float";
        return "int";
    }
    if (t.tipo == TK_ID) {
        int idx = buscarSimbolo(t.lexema);
        if (idx != -1) return tablaSimbolos[idx].tipo;
    }
    return "?";
}

void marcarUsada(const string& nombre) {
    int idx = buscarSimbolo(nombre);
    if (idx != -1) tablaSimbolos[idx].usada = true;
}

void marcarInicializada(const string& nombre) {
    int idx = buscarSimbolo(nombre);
    if (idx != -1) tablaSimbolos[idx].inicializada = true;
}

// [E1] verifica declaracion
void verificarDeclarado(const Token& t) {
    if (buscarSimbolo(t.lexema) == -1)
        erroresSem.push_back(
            "Error semantico [E1] (linea " + to_string(t.linea) +
            "): variable '" + t.lexema + "' no fue declarada");
}

// [E4] verifica inicializacion antes de uso
void verificarInicializada(const Token& t) {
    if (t.tipo != TK_ID) return;
    int idx = buscarSimbolo(t.lexema);
    if (idx != -1 && !tablaSimbolos[idx].inicializada)
        erroresSem.push_back(
            "Error semantico [E4] (linea " + to_string(t.linea) +
            "): variable '" + t.lexema + "' usada antes de ser inicializada");
}

void analizarSemantico()
{
    // ══ PASADA 1: registrar declaraciones + [E2] doble declaracion ══════
    for (int i = 0; i < (int)tokens.size(); i++) {
        if (tokens[i].tipo == TK_INT || tokens[i].tipo == TK_FLOAT) {
            string tipoActual = tokens[i].lexema;
            i++;
            while (i < (int)tokens.size() && tokens[i].tipo != TK_PUNTOCOMA) {
                if (tokens[i].tipo == TK_ID) {
                    if (buscarSimbolo(tokens[i].lexema) != -1)
                        erroresSem.push_back(
                            "Error semantico [E2] (linea " + to_string(tokens[i].linea) +
                            "): variable '" + tokens[i].lexema + "' ya fue declarada");
                    else
                        tablaSimbolos.push_back(
                            {tokens[i].lexema, tipoActual, tokens[i].linea, false, false});
                }
                i++;
            }
        }
    }

    // ══ PASADA 2: uso, tipo e inicializacion ════════════════════════════
    int i = 0;
    while (i < (int)tokens.size()) {

        // ── Declaracion: saltar ──────────────────────────────────────────
        if (tokens[i].tipo == TK_INT || tokens[i].tipo == TK_FLOAT) {
            while (i < (int)tokens.size() && tokens[i].tipo != TK_PUNTOCOMA) i++;
            i++;
        }

        // ── cin >> id ; ─────────────────────────────────────────────────
        else if (tokens[i].tipo == TK_CIN) {
            i++;
            if (i < (int)tokens.size() && tokens[i].tipo == TK_MAYOR2) i++;
            if (i < (int)tokens.size() && tokens[i].tipo == TK_ID) {
                verificarDeclarado(tokens[i]);           // [E1] Variable no declarada
                marcarInicializada(tokens[i].lexema);
                marcarUsada(tokens[i].lexema);
                i++;
            }
            if (i < (int)tokens.size() && tokens[i].tipo == TK_PUNTOCOMA) i++;
        }

        // ── cout << (id | cadena) ; ─────────────────────────────────────
        else if (tokens[i].tipo == TK_COUT) {
            i++;
            if (i < (int)tokens.size() && tokens[i].tipo == TK_MENOR2) i++;
            if (i < (int)tokens.size()) {
                if (tokens[i].tipo == TK_ID) {
                    verificarDeclarado(tokens[i]);       // [E1] Variable no declarada
                    verificarInicializada(tokens[i]);    // [E4] Variable usada sin inicializar
                    marcarUsada(tokens[i].lexema);
                }
                i++;
            }
            if (i < (int)tokens.size() && tokens[i].tipo == TK_PUNTOCOMA) i++;
        }

        // ── id = expresion ; ────────────────────────────────────────────
        else if (tokens[i].tipo == TK_ID && // x = 
                 i+1 < (int)tokens.size() && tokens[i+1].tipo == TK_IGUAL) {
            Token idLeft = tokens[i];
            verificarDeclarado(idLeft);                  // [E1] Variable no declarada
            string tipoLeft = getTipoValor(idLeft);
            i += 2;

            string tipoExpr = "?";
            if (i < (int)tokens.size() &&
                (tokens[i].tipo == TK_ID || tokens[i].tipo == TK_NUM)) {
                Token val1 = tokens[i];  // a
                if (val1.tipo == TK_ID) { 
                    verificarDeclarado(val1);            // [E1] Variable no declarada
                    verificarInicializada(val1);         // [E4] Variable usada sin inicializar
                    marcarUsada(val1.lexema);
                }
                tipoExpr = getTipoValor(val1);
                i++;

                if (i < (int)tokens.size() &&
                    (tokens[i].tipo==TK_MAS || tokens[i].tipo==TK_MENOS ||
                     tokens[i].tipo==TK_POR || tokens[i].tipo==TK_DIV)) {
                    i++;
                    if (i < (int)tokens.size() &&
                        (tokens[i].tipo==TK_ID || tokens[i].tipo==TK_NUM)) {
                        Token val2 = tokens[i];  // b
                        if (val2.tipo == TK_ID) { 
                            verificarDeclarado(val2);    // [E1]
                            verificarInicializada(val2); // [E4]
                            marcarUsada(val2.lexema);
                        }
                        string tipoVal2 = getTipoValor(val2);
                        // [E7] tipos incompatibles en expresion
                        if (tipoExpr!="?" && tipoVal2!="?" && tipoExpr!=tipoVal2)
                            erroresSem.push_back(
                                "Error semantico [E7] (linea " + to_string(val2.linea) +
                                "): tipos incompatibles en expresion ('" +
                                tipoExpr + "' op '" + tipoVal2 + "')");
                        if (tipoVal2 == "float") tipoExpr = "float";
                        i++;
                    }
                }
            }
            // [E3] tipo incompatible en asignacion
            if (tipoLeft!="?" && tipoExpr!="?" && tipoLeft!=tipoExpr)
                erroresSem.push_back(
                    "Error semantico [E3] (linea " + to_string(idLeft.linea) +
                    "): tipo incompatible en asignacion: '" + idLeft.lexema +
                    "' es '" + tipoLeft + "' pero se asigna valor de tipo '" + tipoExpr + "'");
            marcarInicializada(idLeft.lexema);
            if (i < (int)tokens.size() && tokens[i].tipo == TK_PUNTOCOMA) i++;
        }

        // ── if ( val op_rel val ) { ─────────────────────────────────────
        else if (tokens[i].tipo == TK_IF) {
            i++;
            if (i < (int)tokens.size() && tokens[i].tipo == TK_PAR_AB) i++;
            string tipo1 = "?", tipo2 = "?";
            if (i < (int)tokens.size() &&
                (tokens[i].tipo==TK_ID || tokens[i].tipo==TK_NUM)) {
                Token val1 = tokens[i];
                if (val1.tipo == TK_ID) {
                    verificarDeclarado(val1);            // [E1]
                    verificarInicializada(val1);         // [E4]
                    marcarUsada(val1.lexema);
                }
                tipo1 = getTipoValor(val1); i++;
            }
            if (i < (int)tokens.size() &&
                (tokens[i].tipo==TK_MENOR  || tokens[i].tipo==TK_MAYOR  ||
                 tokens[i].tipo==TK_MENORI || tokens[i].tipo==TK_MAYORI ||
                 tokens[i].tipo==TK_IGUAL2)) i++;
            if (i < (int)tokens.size() &&
                (tokens[i].tipo==TK_ID || tokens[i].tipo==TK_NUM)) {
                Token val2 = tokens[i];
                if (val2.tipo == TK_ID) {
                    verificarDeclarado(val2);            // [E1]
                    verificarInicializada(val2);         // [E4]
                    marcarUsada(val2.lexema);
                }
                tipo2 = getTipoValor(val2);
                // [E6] tipos incompatibles en condicion
                if (tipo1!="?" && tipo2!="?" && tipo1!=tipo2)
                    erroresSem.push_back(
                        "Error semantico [E6] (linea " + to_string(val2.linea) +
                        "): tipos incompatibles en condicion ('" +
                        tipo1 + "' comparado con '" + tipo2 + "')");
                i++;
            }
            if (i < (int)tokens.size() && tokens[i].tipo == TK_PAR_CE)  i++;
            if (i < (int)tokens.size() && tokens[i].tipo == TK_LLAVE_AB) i++;
            // el cuerpo del if lo procesa el while externo
        }

        // ── else { ──────────────────────────────────────────────────────
        else if (tokens[i].tipo == TK_ELSE) {
            i++;
            if (i < (int)tokens.size() && tokens[i].tipo == TK_LLAVE_AB) i++;
        }

        // ── } ────────────────────────────────────────────────────────────
        else if (tokens[i].tipo == TK_LLAVE_CE) { i++; }

        else { i++; }
    }

    // ══ PASADA 3: [E5] variable declarada pero nunca usada ══════════════
    for (int j = 0; j < (int)tablaSimbolos.size(); j++) {
        if (!tablaSimbolos[j].usada)
            erroresSem.push_back(
                "Advertencia semantica [E5] (linea " + to_string(tablaSimbolos[j].linea) +
                "): variable '" + tablaSimbolos[j].nombre + "' declarada pero nunca usada");
    }
}

// ════════════════════════════════════════════════════════════
//  GENERADOR DE CUARTETOS 
// ════════════════════════════════════════════════════════════
struct Cuarteto {
    string op;
    string arg1;
    string arg2;
    string resultado;
};

vector<Cuarteto> cuartetos;
int contTemp  = 0;
int contLabel = 0;

string nuevoTemp()  { return "t" + to_string(++contTemp); }
string nuevoLabel() { return "L" + to_string(++contLabel); }

void emitirC(const string& op, const string& a1,
             const string& a2, const string& res) {
    cuartetos.push_back({op, a1, a2, res});
}

int   posC = 0;
Token tkC;

void leerC() {
    if (posC < (int)tokens.size()) tkC = tokens[posC++];
    else tkC = {"EOF", TK_EOF, 0};
}

void cuartetoBloque();
void cuartetoInstruccion();

// Procesa expresion y retorna el nombre del resultado (variable o temporal)
string cuartetoExpresion() { // a + b -> t1
    string val1 = tkC.lexema; leerC();   // primer valor
    if (tkC.tipo == TK_MAS || tkC.tipo == TK_MENOS ||
        tkC.tipo == TK_POR || tkC.tipo == TK_DIV) {
        string op   = tkC.lexema; leerC(); // op
        string val2 = tkC.lexema; leerC(); // segundo valor
        string t = nuevoTemp();
        emitirC(op, val1, val2, t);
        return t;
    }
    return val1;   // expresion simple sin operador
}

void cuartetoInstruccion() {
    // declaracion: int/float id {, id} ;
    if (tkC.tipo == TK_INT || tkC.tipo == TK_FLOAT) {
        string tipo = tkC.lexema; leerC();
        while (tkC.tipo != TK_PUNTOCOMA) {
            if (tkC.tipo == TK_ID)
                emitirC("DECL", tipo, "-", tkC.lexema);
            leerC();
        }
        leerC(); // ;
    }
    // cin >> id ;
    else if (tkC.tipo == TK_CIN) {
        leerC(); leerC();               // cin >>
        string id = tkC.lexema; leerC();
        leerC();                        // ;
        emitirC("READ", "-", "-", id);
    }
    // cout << (id | cadena) ;
    else if (tkC.tipo == TK_COUT) {
        leerC(); leerC();               // cout <<
        string val = tkC.lexema; leerC();
        leerC();                        // ;
        emitirC("WRITE", val, "-", "-");
    }
    // id = expresion ;
    else if (tkC.tipo == TK_ID) {
        string idLeft = tkC.lexema; leerC();  // id
        leerC();                              // =
        string res = cuartetoExpresion();
        leerC();                              // ;
        emitirC("=", res, "-", idLeft);
    }
    // if ( val op_rel val ) { bloque } [ else { bloque } ]
    else if (tkC.tipo == TK_IF) {
        leerC(); leerC();               // if (
        string val1  = tkC.lexema; leerC();
        string opRel = tkC.lexema; leerC();
        string val2  = tkC.lexema; leerC();
        leerC(); leerC();               // ) {

        string t     = nuevoTemp();
        string lThen = nuevoLabel();
        string lEnd  = nuevoLabel();

        emitirC(opRel,   val1,  val2, t);
        emitirC("IF",    t,     "-",  lThen);
        emitirC("GOTO",  "-",   "-",  lEnd);
        emitirC("LABEL", lThen, "-",  "-");

        cuartetoBloque();
        leerC(); // }

        if (tkC.tipo == TK_ELSE) {
            string lAfter = nuevoLabel();
            emitirC("GOTO",  "-",    "-",   lAfter);
            emitirC("LABEL", lEnd,   "-",   "-");
            leerC(); leerC();           // else {
            cuartetoBloque();
            leerC();                    // }
            emitirC("LABEL", lAfter,  "-",  "-");
        } else {
            emitirC("LABEL", lEnd, "-", "-");
        }
    }
    else { leerC(); }
}

void cuartetoBloque() {
    cuartetoInstruccion();
    while (tkC.tipo == TK_INT  || tkC.tipo == TK_FLOAT ||
           tkC.tipo == TK_CIN  || tkC.tipo == TK_COUT  ||
           tkC.tipo == TK_IF   || tkC.tipo == TK_ID)
        cuartetoInstruccion();
}

void generarCuartetos() {
    posC = 0;
    leerC();
    cuartetoBloque();
}

// ════════════════════════════════════════════════════════════
//  MAIN
// ════════════════════════════════════════════════════════════

int main()
{
    ifstream archivo("../programa2.txt");
    if (!archivo) { cout << "No se pudo abrir programa2.txt\n"; return 1; }
    string fuente = "", linea_in;
    while (getline(archivo, linea_in)) fuente += linea_in + "\n";
    archivo.close();

    // ── Lexico ────────────────────────────────────
    analizarLexico(fuente, tokens, erroresLex);
    cout << "\n\t=== ANALIZADOR LEXICO ===\n";
    separador();
    cout << left << setw(22)<<"LEXEMA" << setw(8)<<"TIPO" << setw(16)<<"CATEGORIA" << setw(6)<<"LINEA" << '\n';
    separador();
    if (tokens.empty()) cout << "\tVACIO\n";
    else for (auto& t : tokens)
        cout << left << setw(22)<<t.lexema << setw(8)<<t.tipo << setw(16)<<nombreCategoria(t.tipo) << setw(6)<<t.linea << '\n';
    cout << "\n\t=== ERRORES LEXICOS ===\n";
    separador();
    if (erroresLex.empty()) cout << "\tVACIO\n";
    else for (auto& e : erroresLex) cout << "  [!] " << e << '\n';
    separador();

    // ── Sintactico ────────────────────────────────
    cout << "\n\t=== ANALIZADOR SINTACTICO ===\n";
    separador();
    if (!erroresLex.empty()) {
        cout << "RESULTADO: OMITIDO (hay errores lexicos)\n";
        separador();
        return 0;
    }
    leer(); bloque();
    if (!hayError && tk.tipo != TK_EOF) error("fin de programa");
    if (!hayError) cout << "RESULTADO: VALIDO\n";
    else           cout << "RESULTADO: ERROR " << msgError << "\n";
    separador();
    if (hayError) return 0;

    // ── Semantico ────────────────────────────────
    analizarSemantico();
    cout << "\n\t=== TABLA DE SIMBOLOS ===\n";
    separador();
    cout << left << setw(20)<<"NOMBRE" << setw(10)<<"TIPO" << setw(8)<<"LINEA" << setw(8)<<"INIC" << setw(8)<<"USADA" << '\n';
    separador();
    if (tablaSimbolos.empty()) cout << "\tVACIO\n";
    else for (auto& s : tablaSimbolos)
        cout << left << setw(20)<<s.nombre << setw(10)<<s.tipo << setw(8)<<s.linea
             << setw(8)<<(s.inicializada?"SI":"NO") << setw(8)<<(s.usada?"SI":"NO") << '\n';
    cout << "\n\t=== ERRORES SEMANTICOS ===\n";
    separador();
    if (erroresSem.empty()) cout << "\tVACIO\n";
    else for (auto& e : erroresSem) cout << "  [!] " << e << '\n';
    separador();
    cout << "\n\t=== RESULTADO SEMANTICO ===\n";
    separador();
    bool hayErrorFatal = false;
    for (auto& e : erroresSem)
        if (e.find("Advertencia") == string::npos) { hayErrorFatal = true; break; }
    if (!hayErrorFatal) cout << "RESULTADO: VALIDO\n";
    else                cout << "RESULTADO: ERROR SEMANTICO\n";
    separador();
    if (hayErrorFatal) return 0;   // no generar codigo si hay error semantico

    // ── Cuartetos ─────────────────────────────────
    generarCuartetos();
    cout << "\n\t=== CODIGO INTERMEDIO (CUARTETOS) ===\n";
    separador(65);
    cout << left << setw(5)<<"#" << setw(10)<<"OP" << setw(14)<<"ARG1" << setw(14)<<"ARG2" << setw(14)<<"RESULTADO" << '\n';
    separador(65);
    for (int i = 0; i < (int)cuartetos.size(); i++) {
        auto& c = cuartetos[i];
        cout << left << setw(5)<<(i+1) << setw(10)<<c.op << setw(14)<<c.arg1 << setw(14)<<c.arg2 << setw(14)<<c.resultado << '\n';
    }
    separador(65);

    return 0;
}
