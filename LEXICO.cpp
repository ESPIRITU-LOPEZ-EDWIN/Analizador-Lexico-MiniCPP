/*
 * ============================================================
 *  COMPILADOR MiniC++ – Lexico + Sintactico
 *  AFD (switch-estado) + Descendente recursivo
 *  INF 3631 "A" – Diseno de Compiladores
 * ============================================================
 *
 *  Entrada : programa1.txt  (codigo fuente MiniC++)
 *
 *  Salida  :
 *    [1] Tabla  LEXEMA | TIPO | CATEGORIA | LINEA
 *    [2] Lista de errores lexicos
 *    [3] RESULTADO: VALIDO
 *        RESULTADO: ERROR en linea N: se esperaba 'X', se encontro 'Y'
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

// ════════════════════════════════════════════════════════════
//  CODIGOS DE TOKEN
// ════════════════════════════════════════════════════════════
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
#define TK_MAYOR2     90   // >>
#define TK_MENOR2     91   // <<
#define TK_EOF        -1

// ════════════════════════════════════════════════════════════
//  STRUCT TOKEN UNIFICADO  (orden del lexico)
// ════════════════════════════════════════════════════════════
struct Token {
    string lexema;
    int    tipo;
    int    linea;
};

// ════════════════════════════════════════════════════════════
//  VECTORES GLOBALES  (el lexico los llena, el sintactico los lee)
// ════════════════════════════════════════════════════════════
vector<Token>   tokens;
vector<string>  erroresLex;

// ════════════════════════════════════════════════════════════
//  ─────────── ANALIZADOR LEXICO ───────────
//  (estructura original intacta)
// ════════════════════════════════════════════════════════════

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
    if (tipo == 1 || tipo == 2)             return "RESERVADA";
    if (tipo == 10 || tipo == 11)           return "RESERVADA";
    if (tipo == 20 || tipo == 21)           return "RESERVADA";
    if (tipo == 1000)                       return "IDENTIFICADOR";
    if (tipo == 2000)                       return "NUMERO";
    if (tipo == 3000)                       return "CADENA";
    if (tipo == 40 || tipo == 41)           return "SIMBOLO";
    if (tipo >= 42 && tipo <= 45)           return "SIMBOLO";
    if (tipo == 60)                         return "OPERADOR";
    if (tipo >= 70 && tipo <= 73)           return "OPERADOR";
    if (tipo >= 80 && tipo <= 84)           return "OPERADOR";
    if (tipo == 90 || tipo == 91)           return "OPERADOR";
    return "DESCONOCIDO";
}

void separador(int n = 55) { for (int i = 0; i < n; i++) cout << '-'; cout << '\n'; }

// ── AFD con switch-estado (firma original, recibe por referencia) ──
void analizarLexico(const string& entrada, vector<Token>& tokens, vector<string>& errores)
{
    int    i      = 0;
    int    estado = 0;
    int    linea  = 1;
    string lexema = "";
    char   c      = '\0';

    auto leer = [&]() {
        if (i < (int)entrada.size()) {
            c = entrada[i++];
            if (c == '\n') linea++;
        } else {
            c = '\0';
        }
    };

    leer();

    while (true)
    {
        switch (estado)
        {
        case 0:
            lexema = "";
            if (c == '\0') return;
            if (isspace((unsigned char)c)) { leer(); break; }
            if (esLetra(c)) {
                lexema += c; leer();
                if (lexema=="i" && c=='n') { lexema+=c; leer(); estado=-1;    break; }
                if (lexema=="f" && c=='l') { lexema+=c; leer(); estado=-2;    break; }
                if (lexema=="c" && c=='i') { lexema+=c; leer(); estado=-10;   break; }
                if (lexema=="c" && c=='o') { lexema+=c; leer(); estado=-11;   break; }
                if (lexema=="i" && c=='f') { lexema+=c; leer(); estado=-20;   break; }
                if (lexema=="e" && c=='l') { lexema+=c; leer(); estado=-21;   break; }
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
            errores.push_back("Error lexico (linea " + to_string(linea) +
                              "): caracter invalido -> " + string(1, c));
            leer(); break;

        case -1:
            if (c=='t') { lexema+=c; leer();
                if (esCI(c)) { estado=1; break; }
            }
            estado=-1000; break;
        case 1:  tokens.push_back({"int",   1,  linea}); estado=0; break;

        case -2:
            if (c=='o') { lexema+=c; leer();
            if (c=='a') { lexema+=c; leer();
            if (c=='t') { lexema+=c; leer();
                if (esCI(c)) { estado=2; break; }
            }}}
            estado=-1000; break;
        case 2:  tokens.push_back({"float", 2,  linea}); estado=0; break;

        case -10:
            if (c=='n') { lexema+=c; leer();
                if (esCI(c)) { estado=10; break; }
            }
            estado=-1000; break;
        case 10: tokens.push_back({"cin",   10, linea}); estado=0; break;

        case -11:
            if (c=='u') { lexema+=c; leer();
            if (c=='t') { lexema+=c; leer();
                if (esCI(c)) { estado=11; break; }
            }}
            estado=-1000; break;
        case 11: tokens.push_back({"cout",  11, linea}); estado=0; break;

        case -20:
            if (esCI(c)) { estado=20; break; }
            estado=-1000; break;
        case 20: tokens.push_back({"if",    20, linea}); estado=0; break;

        case -21:
            if (c=='s') { lexema+=c; leer();
            if (c=='e') { lexema+=c; leer();
                if (esCI(c)) { estado=21; break; }
            }}
            estado=-1000; break;
        case 21: tokens.push_back({"else",  21, linea}); estado=0; break;

        case -1000:
            if (esLetra(c)) { lexema+=c; leer(); estado=-1000; }
            else            { estado=1000; }
            break;
        case 1000: tokens.push_back({lexema, 1000, linea}); estado=0; break;

        case -2000:
            if (esDigito(c)) { lexema+=c; leer(); estado=-2000; }
            else if (c=='.') {
                if (i < (int)entrada.size() && esDigito(entrada[i]))
                    { lexema+=c; leer(); estado=-2001; }
                else {
                    errores.push_back("Error lexico (linea " + to_string(linea) +
                                      "): numero mal formado -> " + lexema + ".");
                    leer(); estado=0;
                }
            }
            else if (esLetra(c)) {
                while (esLetra(c) || esDigito(c) || c=='.')
                    { lexema+=c; leer(); }
                errores.push_back("Error lexico (linea " + to_string(linea) +
                                  "): ID no puede empezar con numero -> " + lexema);
                estado=0;
            }
            else { estado=2000; }
            break;
        case -2001:
            if (esDigito(c)) { lexema+=c; leer(); estado=-2001; }
            else if (esCI(c)) { estado=2000; }
            else {
                errores.push_back("Error lexico (linea " + to_string(linea) +
                                  "): numero mal formado -> " + lexema);
                estado=0;
            }
            break;
        case 2000: tokens.push_back({lexema, 2000, linea}); estado=0; break;

        case -3000:
            if (c=='\0') {
                errores.push_back("Error lexico (linea " + to_string(linea) +
                                  "): cadena sin cerrar -> \"" + lexema);
                estado=0;
            } else if (c=='"') {
                leer(); estado=3000;
            } else {
                lexema+=c; leer(); estado=-3000;
            }
            break;
        case 3000: tokens.push_back({"\""+lexema+"\"", 3000, linea}); estado=0; break;

        case -40: tokens.push_back({";",  40, linea}); estado=0; break;
        case -41: tokens.push_back({",",  41, linea}); estado=0; break;
        case -42: tokens.push_back({"(",  42, linea}); estado=0; break;
        case -43: tokens.push_back({")",  43, linea}); estado=0; break;
        case -44: tokens.push_back({"{",  44, linea}); estado=0; break;
        case -45: tokens.push_back({"}",  45, linea}); estado=0; break;
        case -70: tokens.push_back({"+",  70, linea}); estado=0; break;
        case -71: tokens.push_back({"-",  71, linea}); estado=0; break;
        case -72: tokens.push_back({"*",  72, linea}); estado=0; break;
        case -73: tokens.push_back({"/",  73, linea}); estado=0; break;

        case -60:
            if (c=='=') { lexema+="="; leer(); tokens.push_back({"==", 84, linea}); }
            else        { tokens.push_back({"=",  60, linea}); }
            estado=0; break;
        case -80:
            if (c=='=') { lexema+="="; leer(); tokens.push_back({"<=", 82, linea}); }
            else if (c=='<') { lexema+="<"; leer(); tokens.push_back({"<<", 91, linea}); }
            else             { tokens.push_back({"<",  80, linea}); }
            estado=0; break;
        case -81:
            if (c=='=') { lexema+="="; leer(); tokens.push_back({">=", 83, linea}); }
            else if (c=='>') { lexema+=">"; leer(); tokens.push_back({">>", 90, linea}); }
            else             { tokens.push_back({">",  81, linea}); }
            estado=0; break;

        default: estado=0; break;
        }
    }
}

// ════════════════════════════════════════════════════════════
//  ─────────── ANALIZADOR SINTACTICO ───────────
//  (estructura original intacta)
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
    if (pos < (int)tokens.size())
        tk = tokens[pos++];
    else
        tk = {"EOF", TK_EOF, 0};  // <-- unico ajuste: orden {lexema, tipo, linea}
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

void bloque();
void instruccion();
void declaracion();
void listaID();
void entrada();
void salida();
void valor_salida();
void asignacion();
void expresion();
void valor();
void si();
void condicion();

void bloque() {
    instruccion();
    while (!hayError &&
           (tk.tipo == TK_INT   || tk.tipo == TK_FLOAT ||
            tk.tipo == TK_CIN   || tk.tipo == TK_COUT  ||
            tk.tipo == TK_IF    || tk.tipo == TK_ID))
    {
        instruccion();
    }
}

void instruccion() {
    switch (tk.tipo) {
        case TK_INT:
        case TK_FLOAT:  declaracion(); break;
        case TK_CIN:    entrada();     break;
        case TK_COUT:   salida();      break;
        case TK_IF:     si();          break;
        case TK_ID:     asignacion();  break;
        default:
            error("int | float | cin | cout | if | identificador");
            break;
    }
}

void declaracion() {
    if (tk.tipo == TK_INT || tk.tipo == TK_FLOAT) leer();
    else error("int | float");
    listaID();
    verificar(TK_PUNTOCOMA);
}

void listaID() {
    verificar(TK_ID);
    while (!hayError && tk.tipo == TK_COMA) {
        leer();
        verificar(TK_ID);
    }
}

void entrada() {
    verificar(TK_CIN);
    verificar(TK_MAYOR2);
    verificar(TK_ID);
    verificar(TK_PUNTOCOMA);
}

void salida() {
    verificar(TK_COUT);
    verificar(TK_MENOR2);
    valor_salida();
    verificar(TK_PUNTOCOMA);
}

void valor_salida() {
    if (tk.tipo == TK_ID || tk.tipo == TK_STRING) leer();
    else error("identificador | cadena");
}

void asignacion() {
    verificar(TK_ID);
    verificar(TK_IGUAL);
    expresion();
    verificar(TK_PUNTOCOMA);
}

void expresion() {
    valor();
    if (!hayError &&
        (tk.tipo == TK_MAS  || tk.tipo == TK_MENOS ||
         tk.tipo == TK_POR  || tk.tipo == TK_DIV))
    {
        leer();
        valor();
    }
}

void valor() {
    if (tk.tipo == TK_ID || tk.tipo == TK_NUM) leer();
    else error("identificador | numero");
}

void si() {
    verificar(TK_IF);
    verificar(TK_PAR_AB);
    condicion();
    verificar(TK_PAR_CE);
    verificar(TK_LLAVE_AB);
    bloque();
    verificar(TK_LLAVE_CE);
    if (!hayError && tk.tipo == TK_ELSE) {
        leer();
        verificar(TK_LLAVE_AB);
        bloque();
        verificar(TK_LLAVE_CE);
    }
}

void condicion() {
    valor();
    if (!hayError &&
        (tk.tipo == TK_MENOR  || tk.tipo == TK_MAYOR  ||
         tk.tipo == TK_MENORI || tk.tipo == TK_MAYORI ||
         tk.tipo == TK_IGUAL2))
    {
        leer();
        valor();
    }
    else if (!hayError) {
        error("< | > | <= | >= | ==");
    }
}

// ════════════════════════════════════════════════════════════
//  MAIN  –  dos fases sobre el mismo vector<Token>
// ════════════════════════════════════════════════════════════
int main()
{
    // ── 1. Leer codigo fuente ─────────────────────────────
    ifstream archivo("../programa1.txt");
    if (!archivo) {
        cout << "No se pudo abrir programa1.txt\n";
        return 1;
    }
    string fuente = "", linea_in;
    while (getline(archivo, linea_in))
        fuente += linea_in + "\n";
    archivo.close();

    // ── 2. Fase lexica ────────────────────────────────────
    analizarLexico(fuente, tokens, erroresLex);

    cout << "\n\t=== TOKENS ===\n";
    separador();
    cout << left
         << setw(22) << "LEXEMA"
         << setw(8)  << "TIPO"
         << setw(16) << "CATEGORIA"
         << setw(6)  << "LINEA" << '\n';
    separador();
    if (tokens.empty())
        cout << "\tVACIO\n";
    else
        for (auto& t : tokens)
            cout << left
                 << setw(22) << t.lexema
                 << setw(8)  << t.tipo
                 << setw(16) << nombreCategoria(t.tipo)
                 << setw(6)  << t.linea << '\n';

    cout << "\n\t=== ERRORES LEXICOS ===\n";
    separador();
    if (erroresLex.empty())
        cout << "\tVACIO\n";
    else
        for (auto& e : erroresLex)
            cout << "  [!] " << e << '\n';
    separador();

    // ── 3. Fase sintactica ────────────────────────────────
    cout << "\n\t=== ANALIZADOR SINTACTICO ===\n";
    separador();

    if (!erroresLex.empty()) {
        cout << "RESULTADO: OMITIDO (hay errores lexicos)\n";
        separador();
        return 0;
    }

    leer();
    bloque();

    if (!hayError && tk.tipo != TK_EOF)
        error("fin de programa");

    if (!hayError)
        cout << "RESULTADO: VALIDO\n";
    else
        cout << "RESULTADO: ERROR " << msgError << "\n";

    separador();
    return 0;
}