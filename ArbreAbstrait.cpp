#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"



////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}

int NoeudSeqInst::executer() {
    for (unsigned int i = 0; i < m_instructions.size(); i++)
        m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
    if (instruction != nullptr) m_instructions.push_back(instruction);
}

void NoeudSeqInst::traduitEnCPP(ostream& cout, unsigned int indentation) const {
    for (unsigned int i = 0; i < m_instructions.size(); i++)
        m_instructions[i]->traduitEnCPP(cout,0); // on exécute chaque instruction de la séquence

}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
    int valeur = m_expression->executer(); // On exécute (évalue) l'expression
    ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudAffectation::traduitEnCPP(ostream& cout, unsigned int indentation) const {
    cout << setw(4*(indentation)) << " " ;
    m_variable->traduitEnCPP(cout,0);
    cout <<" = ";
    m_expression->traduitEnCPP(cout,0); cout << ";"endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
    int og, od, valeur;
    if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
    if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
    // Et on combine les deux opérandes en fonctions de l'opérateur
    if (this->m_operateur == "+") valeur = (og + od);
    else if (this->m_operateur == "-") valeur = (og - od);
    else if (this->m_operateur == "*") valeur = (og * od);
    else if (this->m_operateur == "==") valeur = (og == od);
    else if (this->m_operateur == "!=") valeur = (og != od);
    else if (this->m_operateur == "<") valeur = (og < od);
    else if (this->m_operateur == ">") valeur = (og > od);
    else if (this->m_operateur == "<=") valeur = (og <= od);
    else if (this->m_operateur == ">=") valeur = (og >= od);
    else if (this->m_operateur == "et") valeur = (og && od);
    else if (this->m_operateur == "ou") valeur = (og || od);
    else if (this->m_operateur == "non") valeur = (!og);
    else if (this->m_operateur == "/") {
        if (od == 0) throw DivParZeroException();
        valeur = og / od;
    }
    return valeur; // On retourne la valeur calculée
}

void NoeudOperateurBinaire::traduitEnCPP(ostream& cout, unsigned int indentation) const {
    m_operandeGauche->traduitEnCPP(cout,0.0);
    cout << " "<<m_operateur.getChaine() << " ";
    m_operandeDroit->traduitEnCPP(cout,0.0);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstSi::executer() {
    if (m_condition->executer()) m_sequence->executer();
    return 0; // La valeur renvoyée ne représente rien !
}
void NoeudInstSi::traduitEnCPP(ostream& cout, unsigned int indentation) const {
    cout << setw(4 * indentation) << ""<<"if (";
    m_condition->traduitEnCPP(cout,0);
    cout << ") {"<<endl;
    m_sequence->traduitEnCPP(cout,indentation+1);
    cout << setw(4 * indentation) << ""<<"}" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTantQue::executer() {
    while (m_condition->executer()) m_sequence->executer();
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstTantQue::traduitEnCPP(ostream& cout, unsigned int indentation) const {
    cout << setw(4 * indentation) << ""<<"while (";
    m_condition->traduitEnCPP(cout,0);
    cout << ") {"<<endl;
    m_sequence->traduitEnCPP(cout,indentation+1);
    cout << setw(4 * indentation) << ""<<"}" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSiRiche
////////////////////////////////////////////////////////////////////////////////

NoeudInstSiRiche::NoeudInstSiRiche(vector<Noeud * > expressions,vector<Noeud * > sequences)
: m_expressions(expressions),m_sequences(sequences) {
}

int NoeudInstSiRiche::executer() {
    bool test = false;
    for(int i = 0; i < m_expressions.size() && !test; i++){
        if (m_expressions[i]->executer()) {
            test = true;
            m_sequences[i]->executer();
        }

    }
    if (m_sequences.size() == m_expressions.size() + 1 && !test) {
        m_sequences.back()->executer();

    }

    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstSiRiche::traduitEnCPP(ostream& cout, unsigned int indentation) const {
  //IF
  for (Noeud * uneCondition : m_expressions){
    if (i == 0){
      cout << setw(4 * indentation) << ""<<"if (";
      uneCondition->traduitEnCPP(cout,indentation);
      cout << ") {"<<endl;
    }
    else if (m_expressions.size()==2){
      cout << setw(4 * indentation) << ""<<"else (";
      uneCondition->traduitEnCPP(cout,indentation);
      cout << ") {"<<endl;
    }
  else {
    cout << setw(4 * indentation) << ""<<"else if (";
    m_condition->traduitEnCPP(cout,indentation);
    cout << ") {"<<endl;
  }
  m_sequences[i]->traduitEnCPP(cout,indentation+1);
  cout << setw(4 * indentation) << ""<<"}" << endl;
  i++;
  }
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudInstRepeter::NoeudInstRepeter(Noeud* sequence, Noeud* condition)
: m_sequence(sequence), m_condition(condition) {
}

int NoeudInstRepeter::executer() {
    do {
        m_sequence->executer();


    } while (m_condition->executer());
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstRepeter::traduitEnCPP(ostream& cout, unsigned int indentation) const {
    cout << "traduction pas encore faite repeter";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(Noeud* sequence, Noeud* condition,Noeud* affectation1,Noeud* affectation2)
: m_sequence(sequence), m_condition(condition),m_affectation1(affectation1),m_affectation2(affectation2) {
}

int NoeudInstPour::executer() {
    for (m_affectation1->executer();m_condition->executer();m_affectation2->executer()){
        m_sequence->executer();
    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstPour::traduitEnCPP(ostream& cout, unsigned int indentation) const {
    cout << "traduction pas encore faite InstPour";
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstEcrire
////////////////////////////////////////////////////////////////////////////////
//TestInstEcrire.txt
NoeudInstEcrire::NoeudInstEcrire(vector<Noeud * > chaineouexpr)
: m_chaineouexpr(chaineouexpr) {
}

int NoeudInstEcrire::executer() {
    string s ;
    for(auto p : m_chaineouexpr){
        if ( (typeid(*p)==typeid(SymboleValue) &&  *((SymboleValue*)p)== "<CHAINE>")){
            s = ((SymboleValue*)p)->getChaine();
            cout << s.substr(1,s.length()-2);
        }else {
            cout << p->executer();
        }

    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstEcrire::traduitEnCPP(ostream& cout, unsigned int indentation) const {
    cout << "traduction pas encore faite Ecrire";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstLire
////////////////////////////////////////////////////////////////////////////////
//TestInstLire.txt
NoeudInstLire::NoeudInstLire(vector<Noeud * > variable)
: m_variable(variable) {
}

int NoeudInstLire::executer() {
    string s ;
    for(auto p : m_variable){

        if ( (typeid(*p)==typeid(SymboleValue) &&  *((SymboleValue*)p)== "<VARIABLE>")){
            int valvar;
            cin>>valvar;
            ((SymboleValue*)p)->setValeur(valvar);
        }

    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstLire::traduitEnCPP(ostream& cout, unsigned int indentation) const {
    cout << "traduction pas encore faite Lire";
}
