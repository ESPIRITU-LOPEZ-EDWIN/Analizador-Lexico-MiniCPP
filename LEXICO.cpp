#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
using namespace std;

struct Token {
    string lexema;
    int    tipo;
};

bool esLetra(char c)
{
    return isalpha((unsigned char)c); 
}

bool esDigito(char c)
{
    return (c >= '0' && c <= '9');
}

// C.I. = caracter de interrupcion (todo lo que NO es parte del token actual)  
bool esCI(char c){
    return isspace((unsigned char)c) || c=='\0' ||   
           c==';' || c==',' || c=='(' || c==')' ||
           c=='{' || c=='}' || c=='"' ||
           c=='=' || c=='+' || c=='-' || c=='*' || c=='/' ||
           c=='<' || c=='>';
}

void linea(int n=40){ for(int i=0;i<n;i++) cout<<'-'; cout<<'\n'; }

// ─────────────────────────────────────────
//  ANALIZADOR LEXICO (AFD con switch-estado)
// ─────────────────────────────────────────
void analizarLexico(const string& entrada, vector<Token>& tokens, vector<string>& errores)
{
    int    i      = 0;
    int    estado = 0;
    string lexema = "";
    char   c      = '\0';

    auto leer = [&](){
        if(i < (int)entrada.size())
        {
            c = entrada[i];
            i++;
        }
        else
            c = '\0';
    };

    leer();

    while(true)
    {
        switch(estado)
        {
        // ══════════════════════════════════
        // ESTADO 0 — inicial
        // ══════════════════════════════════
        case 0:
            lexema = "";

            if(c == '\0') return;

            // espacios -> ignorar
            if(isspace((unsigned char)c))
            {
                leer(); break; 
            }

            // palabras reservadas y identificadores — empiezan con letra
            if(esLetra(c)){
                // detectar palabras reservadas comparando prefijo
                // acumulamos hasta CI y luego decidimos
                lexema += c; leer();
                // int
                if(lexema=="i" && c=='n'){ lexema+=c;leer(); estado=-1; break; }
                // float
                if(lexema=="f" && c=='l'){ lexema+=c;leer(); estado=-2; break; }
                // cin / cout
                if(lexema=="c" && c=='i'){ lexema+=c;leer(); estado=-10; break; }
                if(lexema=="c" && c=='o'){ lexema+=c;leer(); estado=-11; break; }
                // if
                if(lexema=="i" && c=='f'){ lexema+=c;leer(); estado=-20; break; }
                // else
                if(lexema=="e" && c=='l'){ lexema+=c;leer(); estado=-21; break; }
                // identificador generico
                estado = -1000;
                break;
            }

            // digito → numero
            if(esDigito(c))
            { 
                lexema+=c; leer(); estado=-2000; break; 
            }

            // comilla → cadena
            if(c=='"'){ leer(); estado=-3000; break; }

            // simbolos simples
            if(c==';'){ lexema=";"; leer(); estado=-40; break; }
            if(c==','){ lexema=","; leer(); estado=-41; break; }
            if(c=='('){ lexema="("; leer(); estado=-42; break; }
            if(c==')'){ lexema=")"; leer(); estado=-43; break; }
            if(c=='{'){ lexema="{"; leer(); estado=-44; break; }
            if(c=='}'){ lexema="}"; leer(); estado=-45; break; }
            if(c=='+'){ lexema="+"; leer(); estado=-70; break; }
            if(c=='-'){ lexema="-"; leer(); estado=-71; break; }
            if(c=='*'){ lexema="*"; leer(); estado=-72; break; }
            if(c=='/'){ lexema="/"; leer(); estado=-73; break; }

            // operadores dobles (lookahead)
            if(c=='='){ lexema="="; leer(); estado=-60; break; }
            if(c=='<'){ lexema="<"; leer(); estado=-80; break; }
            if(c=='>'){ lexema=">"; leer(); estado=-81; break; }

            // carácter inválido
            errores.push_back("Error lexico: caracter invalido -> " + string(1,c));
            leer();
            break;

        // ══════════════════════════════════
        // PALABRAS RESERVADAS
        // ══════════════════════════════════

        // int → -1 → 1
        case -1:
            if(c=='t'){ lexema+=c; leer();
                if(esCI(c)){ estado=1; break; }
            }
            // no es "int" → identificador
            estado=-1000; break;

        case 1:
            tokens.push_back({"int",1});
            estado=0; break;

        // float → -2 → 2
        case -2:
            if(c=='o'){ lexema+=c; leer();
            if(c=='a'){ lexema+=c; leer();
            if(c=='t'){ lexema+=c; leer();
                if(esCI(c)){ estado=2; break; }
            }}}
            estado=-1000; break;

        case 2:
            tokens.push_back({"float",2});
            estado=0; break;

        // cin → -10 → 10
        case -10:
            if(c=='n'){ lexema+=c; leer();
                if(esCI(c)){ estado=10; break; }
            }
            estado=-1000; break;

        case 10:
            tokens.push_back({"cin",10});
            estado=0; break;

        // cout → -11 → 11
        case -11:
            if(c=='u'){ lexema+=c; leer();
            if(c=='t'){ lexema+=c; leer();
                if(esCI(c)){ estado=11; break; }
            }}
            estado=-1000; break;

        case 11:
            tokens.push_back({"cout",11});
            estado=0; break;

        // if → -20 → 20
        case -20:
            if(esCI(c)){ estado=20; break; }
            estado=-1000; break;

        case 20:
            tokens.push_back({"if",20});
            estado=0; break;

        // else → -21 → 21
        case -21:
            if(c=='s'){ lexema+=c; leer();
            if(c=='e'){ lexema+=c; leer();
                if(esCI(c)){ estado=21; break; }
            }}
            estado=-1000; break;

        case 21:
            tokens.push_back({"else",21});
            estado=0; break;

        // ══════════════════════════════════
        // IDENTIFICADOR  letra → -1000 → 1000
        // ══════════════════════════════════
        case -1000: // inta
            if(esLetra(c)){ lexema+=c; leer(); estado=-1000; }
            else           { estado=1000; }
            break;

        case 1000:
            tokens.push_back({lexema,1000});
            estado=0; break;

        // ══════════════════════════════════
        // NUMERO  digito → -2000 → 2000
        //                ↘ '.' → -2001 → 2000
        // ══════════════════════════════════
        case -2000:
            if(esDigito(c)){ lexema+=c; leer(); estado=-2000; }
            else if(c=='.'){
                // verificar que el siguiente sea dígito
                if(i<(int)entrada.size() && esDigito(entrada[i])){
                    lexema+=c; leer(); estado=-2001;
                } else {
                    errores.push_back("Error lexico: numero mal formado -> "+lexema+".");
                    leer(); estado=0;
                }
            }
            else if(esLetra(c)){
                // número pegado a letra → error
                while(esLetra(c) || esDigito(c) || c=='.')
                {
                    lexema+=c;leer();
                }
                errores.push_back("Error lexico: ID no puede empezar con numero -> "+lexema);
                estado=0;
            }
            else { estado=2000; }
            break;

        case -2001:
            if(esDigito(c)){ lexema+=c; leer(); estado=-2001; }
            else if(esCI(c)){ estado=2000; }
            else {
                errores.push_back("Error lexico: numero mal formado -> "+lexema);
                estado=0;
            }
            break;

        case 2000:
            tokens.push_back({lexema,2000});
            estado=0; break;

        // ══════════════════════════════════
        // CADENA  " → -3000 → 3000
        // Acumula todo incluyendo espacios hasta "
        // ══════════════════════════════════
        case -3000:
            if(c=='\0'){
                errores.push_back("Error lexico: cadena sin cerrar -> \""+lexema);
                estado=0;
            } else if(c=='"'){
                leer(); estado=3000;
            } else {
                lexema+=c; leer(); estado=-3000;
            }
            break;

        case 3000:
            tokens.push_back({"\""+lexema+"\"",3000});
            estado=0; break;

        // ══════════════════════════════════
        // SIMBOLOS SIMPLES
        // ══════════════════════════════════
        case -40: tokens.push_back({";",40}); estado=0; break;
        case -41: tokens.push_back({",",41}); estado=0; break;
        case -42: tokens.push_back({"(",42}); estado=0; break;
        case -43: tokens.push_back({")",43}); estado=0; break;
        case -44: tokens.push_back({"{",44}); estado=0; break;
        case -45: tokens.push_back({"}",45}); estado=0; break;
        case -70: tokens.push_back({"+",70}); estado=0; break;
        case -71: tokens.push_back({"-",71}); estado=0; break;
        case -72: tokens.push_back({"*",72}); estado=0; break;
        case -73: tokens.push_back({"/",73}); estado=0; break;

        // ══════════════════════════════════
        // OPERADORES DOBLES (lookahead)
        // ══════════════════════════════════

        // = → -60: si sigue '=' → ==84, sino → =60
        case -60:
            if(c=='='){ lexema+="="; leer(); tokens.push_back({"==",84}); }
            else       { tokens.push_back({"=",60}); }
            estado=0; break;

        // < → -80: si '=' → <=82, si '<' → <<91, sino → <80
        case -80:
            if(c=='='){ lexema+="="; leer(); tokens.push_back({"<=",82}); }
            else if(c=='<'){ lexema+="<"; leer(); tokens.push_back({"<<",91}); }
            else           { tokens.push_back({"<",80}); }
            estado=0; break;

        // > → -81: si '=' → >=83, si '>' → >>90, sino → >81
        case -81:
            if(c=='='){ lexema+="="; leer(); tokens.push_back({">=",83}); }
            else if(c=='>'){ lexema+=">"; leer(); tokens.push_back({">>",90}); }
            else           { tokens.push_back({">",81}); }
            estado=0; break;

        default:
            estado=0; break;
        }
    }
}

int main()
{
    vector<Token>  tokens;
    vector<string> errores;

    string programa = "";
    string linea_in;

    ifstream archivo("programa3.txt");

    if(!archivo){
        cout << "No se pudo abrir el archivo." << '\n';
        return 1;
    }

    while(getline(archivo, linea_in)){
        programa += linea_in + " ";
    }

    archivo.close();
    
    analizarLexico(programa, tokens, errores);

    cout << "\n\t=== TOKENS ===\n";
    linea();
    cout << left << setw(20) << "LEXEMA" << setw(10) << "TOKEN" << '\n';
    linea();
    if(tokens.empty()) cout << "\tVACIO\n";
    else for(auto& t : tokens)
        cout << left << setw(20) << t.lexema << setw(10) << t.tipo << '\n';

    cout << "\n\t=== ERRORES ===\n";
    linea();
    if(errores.empty()) cout << "\tVACIO\n";
    else for(auto& e : errores) cout << e << '\n';
    linea();

    return 0;
}