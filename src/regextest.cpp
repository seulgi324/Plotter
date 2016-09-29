#include <regex>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

map<string, string> latexer = { {"GenTau", "#tau"}, {"GenHadTau", "#tau_h"}, {"GenMuon", "#mu"}, {"TauJet", "#tau"}, {"Muon", "#mu"}, {"DiMuon", "#mu, #mu"}, {"DiTau", "#tau, #tau"}, {"Tau", "#tau"}, {"DiJet", "jj"}, {"Met", "#cancel{E_T}"}};
string listParticles(string);


int main(int argc, char* argv[]) {

  ifstream file;
  file.open("../hist_to_units.txt");

  string stringkey;

  string particle;

  while(getline(file, stringkey)) {

    smatch m;
    regex part ("(Di)?(Tau(Jet)?|Muon|Electron|Jet)");


    regex e ("^(.+?)(1|2)?Energy.+");
    regex n ("^N([^12[:space:]]+).*$");
    regex charge ("^(.+?)(1|2)?Charge.*");
    regex mass ("(.+?)(Not)?Mass.*");
    regex zeta ("(.+?)(P)?Zeta(1D|Vis)?.*");
    regex deltar ("(.+?)DeltaR.*");
    regex MetMt ("^(([^_]*?)(1|2)|[^_]+_(.+?)(1|2))MetMt.*");
    regex eta ("^(.+?)(Delta)?(Eta).*");
    regex phi ("^(([^_]*?)|[^_]+_(.+?))(Delta)?(Phi).+");
    regex cosdphi ("(.+?)(CosDphi)(.*)");
    regex pt ("^(.+?)(Delta)?(Pt)(Div)?.*");
    regex osls ("^(.+?)OSLS.*");
    regex zdecay ("^[^_]_(.+?)IsZdecay.*");

    
    if(regex_match(stringkey, m, e)) {
      particle = m[1].str();
      cout << "E(" + ((latexer[particle] == "") ? particle : latexer[particle]) + ") [GeV]" << endl;
    }
    else if(regex_match(stringkey, m, n)) {
      particle = m[1].str();
      cout << "N(" + ((latexer[particle] == "") ? particle : latexer[particle]) + ")" << endl;
    }
    else if(regex_match(stringkey, m, charge)) {
      particle = m[1].str();
           cout << "charge(" + ((latexer[particle] == "") ? particle : latexer[particle]) + ") [e]" << endl;
    }
    else if(regex_match(stringkey, m, mass)) {
      if(m[2].str() == "Not") cout << "Not Reconstructed ";
      cout << "M(" << listParticles(m[1].str()) << ") (GeV)" << endl;
    }
    else if(regex_match(stringkey, m, zeta)) {
      cout << m[2].str() << "#zeta";
      if(m[3].str() != "") cout << "_{" + m[3].str() + "}";
      cout << "(" << listParticles(m[1].str()) << ")" << endl;
    }
    else if(regex_match(stringkey, m, deltar)) {
           cout << "#DeltaR(" << listParticles(m[1].str()) << ")" << endl;
    }
    else if(regex_match(stringkey, m, MetMt)) {
      particle = m[2].str() + m[4].str();
      cout << "M_t(" + ((latexer[particle] == "") ? particle : latexer[particle]) + ")" << endl;
    }
    else if(regex_match(stringkey, m, eta))  {
      if(m[2].str() != "") cout << "#Delta";
           cout << "#eta(" << listParticles(m[1].str()) << ")" << endl;
    }
    else if(regex_match(stringkey, m, phi))  {
      if(m[4].str() != "") cout << "#Delta";
      cout << "#phi(" << listParticles(m[2].str()+m[3].str()) << ")" << endl;
    }
    else if(regex_match(stringkey, m, cosdphi)) {
      cout << "cos(#Delta#phi(" << listParticles(m[1].str()) << "))" << endl;
    }
    else if(regex_match(stringkey, m, pt)) {
      if(m[4].str() != "") cout << "#frac{#Deltap_T}{#Sigmap_T}(";
      else if(m[2] != "") cout << "#Deltap_T(";
      else cout << "p_T(";
      cout << listParticles(m[1].str()) << ") [GeV]" << endl;
    } else if(stringkey.find("Met") != string::npos) cout << "#cancel{E_T} [GeV]" << endl;
    else if(stringkey.find("MHT") != string::npos) cout << "#cancel{H_T} [GeV]" << endl;
    else if(stringkey.find("HT") != string::npos) cout << "H_T [GeV]" << endl;
    else if(stringkey.find("Meff") != string::npos) cout << "M_eff [GeV]" << endl;
    else if(regex_match(stringkey, m, osls)) {
      particle = m[1].str();
      while(regex_search(particle, m, part)) {
	cout << "q_{" + ((latexer[m[0]] == "") ? m[0] : latexer[m[0]]) << "}";
	particle = m.suffix().str();
      }
      cout << endl;
    } else if(regex_match(stringkey, m, zdecay)) {
      cout << listParticles(m[1].str()) << "is Z Decay" << endl;
    }
    else cout << stringkey << endl;
  }



}


string listParticles(string toParse) {
  smatch m;
  regex part ("(Di)?(Tau(Jet)?|Muon|Electron|Jet|Met)");
  bool first = true;
  string final = "";

  while(regex_search(toParse,m,part)) {
    if(first) first = false;
    else final += ", ";
    final += ((latexer[m[0]] == "") ? m[0] : latexer[m[0]]);
    toParse = m.suffix().str();
  }
  return final;
}
