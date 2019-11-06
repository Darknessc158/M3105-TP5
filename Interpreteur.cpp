#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
#include <typeinfo>
#include <vector>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr) {
}

void Interpreteur::analyse() {
    m_arbre = programme(); // on lance l'analyse de la première règle
}

void Interpreteur::tester(const string & symboleAttendu) const {
    // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
    static char messageWhat[256];
    if (m_lecteur.getSymbole() != symboleAttendu) {
        sprintf(messageWhat,
                "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
                m_lecteur.getLigne(), m_lecteur.getColonne(),
                symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
        throw SyntaxeException(messageWhat);
    }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) {
    // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
    tester(symboleAttendu);
    m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const {
    // Lève une exception contenant le message et le symbole courant trouvé
    // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
    static char messageWhat[256];
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() {
    // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    testerEtAvancer("procedure");
    testerEtAvancer("principale");
    testerEtAvancer("(");
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    testerEtAvancer("finproc");
    tester("<FINDEFICHIER>");
    return sequence;
}

Noeud* Interpreteur::seqInst() {
    // <seqInst> ::= <inst> { <inst> }
    NoeudSeqInst* sequence = new NoeudSeqInst();
    do {
        sequence->ajoute(inst());
    } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "si" || m_lecteur.getSymbole() == "tantque" || m_lecteur.getSymbole() == "repeter" || m_lecteur.getSymbole() == "pour"|| m_lecteur.getSymbole() == "ecrire"|| m_lecteur.getSymbole() == "lire");
    // Tant que le symbole courant est un début possible d'instruction...
    // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction

    return sequence;
}

Noeud* Interpreteur::inst() {
    // <inst> ::= <affectation>  ; | <instSi>
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        Noeud *affect = affectation();
        testerEtAvancer(";");
        return affect;
    } else if (m_lecteur.getSymbole() == "si")
        return instSiRiche();
    else if (m_lecteur.getSymbole() == "tantque")
        return instTantQue();
    else if (m_lecteur.getSymbole() == "repeter")
        return instRepeter();
    else if (m_lecteur.getSymbole() == "pour")
        return instPour();
    else if (m_lecteur.getSymbole() == "ecrire")
        return instEcrire();
    else if (m_lecteur.getSymbole() == "lire")
        return instLire();
        // Compléter les alternatives chaque fois qu'on rajoute une nouvelle instruction
    else {
        erreur("Instruction incorrecte");
        return nullptr;
    }
}

Noeud* Interpreteur::affectation() {
    // <affectation> ::= <variable> = <expression>
    tester("<VARIABLE>");
    Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
    m_lecteur.avancer();
    testerEtAvancer("=");
    Noeud* exp = expression(); // On mémorise l'expression trouvée
    return new NoeudAffectation(var, exp); // On renvoie un noeud affectation
}

Noeud* Interpreteur::expression() {
    //<expression> ::= <expEt> {ou <expEt> } 
    //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
    Noeud* et = expEt();
    while ( m_lecteur.getSymbole() == "ou" ){
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* etDroit = expEt(); // On mémorise l'opérande droit
        et = new NoeudOperateurBinaire(operateur, et, etDroit); // Et on construuit un noeud opérateur binaire
    }
    return et; // On renvoie fact qui pointe sur la racine de l'expression
}
Noeud*  Interpreteur::expEt(){
    //    <expEt> ::= <expComp> {et <expComp> } 
    Noeud* comp = expComp();
    while(m_lecteur.getSymbole() == "et"){
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* compDroit = expComp();
        comp = new NoeudOperateurBinaire(operateur, comp, compDroit);
    }
    return comp;
   
}        
Noeud*  Interpreteur::expComp(){
    //    <expComp> ::= <expAdd> {==|!=|<|<=|>|>= <expAdd> }
    Noeud* add = expAdd();
    while(m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
          m_lecteur.getSymbole() == "<" || m_lecteur.getSymbole() == "<=" ||
          m_lecteur.getSymbole() == ">" || m_lecteur.getSymbole() == ">=" ||  ){
        
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* addDroit = expAdd();
        add = new NoeudOperateurBinaire(operateur, add, addDroit);
    }
    return add;

}      
Noeud*  Interpreteur::expAdd(){
    //    <expAdd> ::= <expMult> {+|-<expMult> }
    Noeud* mult = expMult();
    while(m_lecteur.getSymbole() == "+" || m_lecteur.getSymbole() == "-"){
        
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* multDroit = expMult();
        mult = new NoeudOperateurBinaire(operateur, mult, multDroit);
    }
    return mult;
} 
Noeud*  Interpreteur::expMult() {
     //    <expMult>::= <facteur> {*|/<facteur> } 
     Noeud* fact = facteur();
    while(m_lecteur.getSymbole() == "*" || m_lecteur.getSymbole() == "/" ){
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* factDroit = facteur();
        fact = new NoeudOperateurBinaire(operateur, fact, factDroit);
    }
    return fact;
}    

Noeud* Interpreteur::facteur() {
    // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
    Noeud* fact = nullptr;
    if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
        fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
        m_lecteur.avancer();
    } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), facteur());
    } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("non"), facteur(), nullptr);
    } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
        m_lecteur.avancer();
        fact = expression();
        testerEtAvancer(")");
    } else
        erreur("Facteur incorrect");
    return fact;
}

Noeud* Interpreteur::instSi() {
    // <instSi> ::= si ( <expression> ) <seqInst> finsi
    testerEtAvancer("si");
    testerEtAvancer("(");
    Noeud* condition = expression(); // On mémorise la condition
    testerEtAvancer(")");
    Noeud* sequence = seqInst(); // On mémorise la séquence d'instruction
    testerEtAvancer("finsi");
    return new NoeudInstSi(condition, sequence); // Et on renvoie un noeud Instruction Si
}

Noeud* Interpreteur::instTantQue() {
    // <instTantQue> ::= tantque( <expression> ) <seqInst> fintantque
    testerEtAvancer("tantque");
    testerEtAvancer("(");
    Noeud* condition = expression();
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    testerEtAvancer("fintantque");
    return new NoeudInstTantQue(condition, sequence);
}

Noeud* Interpreteur::instSiRiche() {
    // <instSiRiche> ::=si(<expression>) <seqInst> {sinonsi(<expression>) <seqInst> }[sinon <seqInst>]finsi
    vector<Noeud*>expressions;
    vector<Noeud*>sequences;
    testerEtAvancer("si");
    testerEtAvancer("(");
    Noeud* condition = expression(); // On mémorise la condition
    expressions.push_back(condition);
    testerEtAvancer(")");
    Noeud* sequence = seqInst(); // On mémorise la séquence d'instruction
    sequences.push_back(sequence);

    while (m_lecteur.getSymbole()=="sinonsi") {
    m_lecteur.avancer();
    testerEtAvancer("(");
    Noeud* condition = expression(); // On mémorise la condition
    expressions.push_back(condition);
    testerEtAvancer(")");
    Noeud* sequence = seqInst(); // On mémorise la séquence d'instruction
    sequences.push_back(sequence);
    }
    if (m_lecteur.getSymbole()=="sinon") {
        m_lecteur.avancer();
        Noeud* sequence = seqInst();
        sequences.push_back(sequence);
    }
    testerEtAvancer("finsi");
    return new NoeudInstSiRiche(expressions,sequences); // Et on renvoie un noeud Instruction Si
}

Noeud* Interpreteur::instRepeter() {
    // <instRepeter> ::=repeter <seqInst> jusqua( <expression> )
    testerEtAvancer("repeter");
    Noeud* sequence = seqInst();
    testerEtAvancer("jusqua");
    testerEtAvancer("(");
    Noeud* condition = expression();
    testerEtAvancer(")");
    return new NoeudInstRepeter(sequence, condition);
}

Noeud* Interpreteur::instPour() {
    // <instPour> ::= pour( [ <affectation> ] ; <expression> ;[ <affectation> ]) <seqInst> finpour
    Noeud* affectation1 = nullptr;
    Noeud* affectation2 = nullptr;


    testerEtAvancer("pour");
    testerEtAvancer("(");
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        //m_lecteur.avancer(); //il faudrait avancer pour voir si c'est bien un égal.
        affectation1 = affectation();
        testerEtAvancer(";");
    }    
    Noeud* condition1 = expression();
    if (m_lecteur.getSymbole() != ")") {
        testerEtAvancer(";");
        affectation2 = affectation();
    }
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    testerEtAvancer("finpour");
    return new NoeudInstPour(sequence, condition1,affectation1, affectation2);
}
//testInstPour.txt

Noeud* Interpreteur::instEcrire() {
    testerEtAvancer("ecrire");
    testerEtAvancer("(");

    vector <Noeud*> p;
    if (m_lecteur.getSymbole() == "<CHAINE>") {
        p.push_back(m_table.chercheAjoute(m_lecteur.getSymbole()));
        m_lecteur.avancer();
    } else {
        p.push_back(expression());
    }

    while (m_lecteur.getSymbole() == ",") {
        testerEtAvancer(",");

        if (m_lecteur.getSymbole() == "<CHAINE>") {
            p.push_back(m_table.chercheAjoute(m_lecteur.getSymbole()));
            m_lecteur.avancer();
        } else {
            p.push_back(expression());
        }


    }

    testerEtAvancer(")");

    return new NoeudInstEcrire(p);
}

Noeud* Interpreteur::instLire(){ //   <instLire>::=lire( <variable> {, <variable> })
testerEtAvancer("lire");
    testerEtAvancer("(");
    vector <Noeud*> v;
    v.push_back(m_table.chercheAjoute(m_lecteur.getSymbole()));
    m_lecteur.avancer();

    while (m_lecteur.getSymbole() == ",") {
        m_lecteur.avancer();
        v.push_back(m_table.chercheAjoute(m_lecteur.getSymbole()));
        m_lecteur.avancer();

    }
    testerEtAvancer(")");

   return new NoeudInstLire(v);
}

void Interpreteur::traduitEnCPP(ostream & cout,unsigned int indentation)const{

 cout << setw(4*indentation)<<""<<"int main() {"<< endl;
 getArbre()->traduitEnCPP(cout,indentation+1);
 cout << setw(4*(indentation+1))<<""<<"return 0;"<< endl ;
 cout << setw(4*indentation)<<"}" << endl ;

}
