
// try to simulate the spread of a virus and maybe we'll include a 
// bit to do with the social distancing everyone is talking about 
// now due to the coronavirus
//
// compile with:
// g++ `root-config --cflags --glibs` virus.cxx
// 
// joey turko, 20/3/2020
//

#include<iostream>
#include<cmath>
#include<fstream>

#include "TH1F.h"
#include "THStack.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TROOT.h"
#include "TStyle.h"

double xdim = 150;
double ydim = 100;
const int npeople = 500;
double infectionlength = 300;
double socialdistancing = 0.9;
double stepsize = 0.5;
double contactsize = 0.5;
double quarantinetime = 500;
double randomdirectioninc = 0.2;
double infectionprobability = 0.5;
int casesuntilquarantine = 50;
double storeprobability = 0.001;
double quarantineeasingfactor = 0.01;

double xstore = xdim/2.;
double ystore = ydim/2.;
int maxstorebound = npeople/2;

enum condition
{
    healthy,
    sick,
    immune
};

std::string pcond(condition status)
{   
    switch(status) 
    {
        case healthy : return "healthy";
        case sick : return "sick";
        case immune : return "immune";
    }
}

double rando() { return std::rand()/double(RAND_MAX); }

void printparameters() {
    std::cout<<"====================================="<<std::endl
             <<"virus simulation "<<std::endl
             <<"npeople:\t\t"<<npeople<<std::endl
             <<"dimx:\t\t\t"<<xdim<<std::endl
             <<"dimy:\t\t\t"<<ydim<<std::endl
             <<"contact size:\t\t"<<contactsize<<std::endl
             <<"step size:\t\t"<<stepsize<<std::endl
             <<"infection duration:\t"<<infectionlength<<std::endl
             <<"social dist. duration:\t"<<quarantinetime<<std::endl
             <<"social dist. factor:\t"<<socialdistancing<<std::endl
             <<"infection prob.:\t"<<infectionprobability<<std::endl
             <<"cases pre-quarantine:\t"<<casesuntilquarantine<<std::endl
             <<"prob. of storebound:\t"<<storeprobability<<std::endl
             <<"max people storebound:\t"<<maxstorebound<<std::endl
             <<"quarantine easing fact:\t"<<quarantineeasingfactor<<std::endl
             <<"====================================="<<std::endl;
}

class person
{
    public:
        person(int id, double x, double y, double direction, condition status) {
            fId = id;
            fX = x;
            fY = y;
            fStatus = status;
            fDirection = direction;
            fInfectionTime = 1e100;
            fStorebound = false;
        }
        person(int id) {
            fId = id;
            fX = xdim*rando();
            fY = ydim*rando();
            fStatus = healthy;
            fDirection = 360.*rando();
            fInfectionTime = 1e100;
            fStorebound = false;
        }

        double id() { return fId; }
        void id(double id) { fId = id; }
        
        double x() { return fX; }
        void x(double xx) { fX = xx; }
        double y() { return fY; }
        void y(double yy) { fY = yy; }
        
        condition status() { return fStatus; }
        void status(condition ss) { fStatus = ss; }
        
        double direction() { return fDirection; }
        void direction(double direction) { fDirection = direction; }

        double infectiontime() { return fInfectionTime; }
        void infectiontime(double time) { fInfectionTime = time; }

        bool storebound() { return fStorebound; }
        void storebound(bool val) { fStorebound = val; }

        void print() { 
            std::cout<< "id: "<<fId<<",\t x: "<<fX<<",\t y: "<<fY<<",\t direction: "<<fDirection<<",\t status: "<<pcond(fStatus);
            if(fStatus!=healthy) std::cout<<",\t infection time: "<<fInfectionTime<<std::endl; else std::cout<<std::endl;
        }
        bool contact(person other) { 
            if( std::sqrt( std::pow(x()-other.x(),2.) + std::pow(y()-other.y(),2.) ) < contactsize ) return true; 
            else return false; 
        }
        void walk() {
            if(storebound()) { 
                fDirection = std::atan((fY-ystore)/(fX-xstore));
                if(fX>xstore) fDirection += M_PI;
            }
            // check if beyond boundary, move to it and change dir if we have
            if     ( x() <= 0. )     { x(0.);      direction(2.*M_PI*rando()); }
            else if( x() >= xdim )   { x(xdim);    direction(2.*M_PI*rando()); }
            else if( y() <= 0. )     { y(0.);      direction(2.*M_PI*rando()); }
            else if( y() >= ydim )   { y(ydim);    direction(2.*M_PI*rando()); } 
            x( x()+stepsize*std::cos(direction()) );
            y( y()+stepsize*std::sin(direction()) );
        }
        void randomdirection() { fDirection += (1.-2.*rando())*randomdirectioninc; }
      
    private:
        int fId;
        double fX;
        double fY;
        double fInfectionTime;
        double fDirection;
        condition fStatus;
        bool fStorebound;
};

void plotforgif(int time, person ** people, bool quarantinestart, double quarantinestarttime, bool quarantinestop, double quarantinestoptime)
{
    gErrorIgnoreLevel = kError;

    TCanvas * c1 = new TCanvas("c1","c1",500,700);
    c1->Divide(1,2);
    
    // top pad
    c1->cd(1);
    TGraph * hhealthy = new TGraph; 
    TGraph * hsick = new TGraph;
    TGraph * himmune = new TGraph;
    for(int i=0; i<npeople; i++) {
        if(people[i]->status()==healthy) hhealthy->SetPoint(hhealthy->GetN(),people[i]->x(),people[i]->y());
        if(people[i]->status()==sick) hsick->SetPoint(hsick->GetN(),people[i]->x(),people[i]->y());
        if(people[i]->status()==immune) himmune->SetPoint(himmune->GetN(),people[i]->x(),people[i]->y());
    }
    TAxis * axis = hhealthy->GetXaxis();
    axis->SetLimits(-10.,160.);
    hhealthy->GetHistogram()->SetMaximum(110.);    
    hhealthy->GetHistogram()->SetMinimum(-10.);
    hhealthy->SetMarkerColor(kGray);
    hhealthy->SetMarkerSize(0.5);
    hhealthy->SetMarkerStyle(20);
    hsick->SetMarkerColor(kMagenta-4);
    hsick->SetMarkerSize(0.5);
    hsick->SetMarkerStyle(20);
    himmune->SetMarkerColor(kTeal-4);
    himmune->SetMarkerSize(0.5);
    himmune->SetMarkerStyle(20);
    if(hhealthy->GetN()>0) hhealthy->Draw("ap");
    if(hsick->GetN()>0) hsick->Draw("p same");
    if(himmune->GetN()>0) himmune->Draw("p same");
    
    // bottom pad
    c1->cd(2);
    gStyle->SetOptStat(0);
    std::ifstream infile;
    infile.open("output.dat");
    const int max_size = 10000;
    int size = 0;
    int times[max_size];
    int sick[max_size];
    int healthy[max_size];
    int immune[max_size];
    while(!infile.eof()) {
        infile >> times[size] >> sick[size] >> healthy[size] >> immune[size];
        size++;
    }
    infile.close();
    TH1F * hs = new TH1F("sick","sick",size,0,size);
    hs->SetFillColor(kMagenta-4);
    hs->GetYaxis()->SetRangeUser(0,npeople);
    TH1F * hh = new TH1F("healthy","healthy",size,0,size);
    hh->SetFillColor(kGray);
    hh->GetYaxis()->SetRangeUser(0,npeople);
    TH1F * hi = new TH1F("immune","immune",size,0,size);
    hi->SetFillColor(kTeal-4);
    hi->GetYaxis()->SetRangeUser(0,npeople);
    for(int i=0; i<size; i++) {
        hs->SetBinContent(i,sick[i]);
        hh->SetBinContent(i,healthy[i]);
        hi->SetBinContent(i,immune[i]);
    }
    THStack * stack = new THStack("stack","stack");
    stack->Add(hs);
    stack->Add(hh);
    stack->Add(hi);
    hi->SetTitle(Form("time = %d",time));
    hi->Draw();
    stack->Draw("same");
    stack->SetMaximum(npeople);
    if(quarantinestart) {
        TLine * qs = new TLine(quarantinestarttime,0,quarantinestarttime,npeople);
        qs->Draw("same");
    }
    if(quarantinestop) {
        TLine * qe = new TLine(quarantinestoptime,0,quarantinestoptime,npeople);
        qe->Draw("same");
    }

    c1->SaveAs(Form("gif_files/plot_%05d.gif",time));
    delete c1;
    c1 = NULL;
}

int main() {
    printparameters();

    srand(time(0));
    std::ofstream outfile;
    outfile.open("output.dat");    

    int nhealthy = npeople-1;
    int nsick = 1;
    int nimmune = 0;
    double time = 0.;
    double currentsocialdistancing = 0.;    
    double quarantinestarttime = 1000000;
    bool quarantinestarted = false;
    bool quarantinefinished = false;
    int nstorebound = 0;

    // populate array with healthy people at random positions
    person * people[npeople];
    for(int i=0; i<npeople; i++) people[i] = new person(i);
    
    // make one persons sick
    people[0]->status(sick);
    people[0]->infectiontime(0);

    while(nsick > 0) // this iterator is the passage of time
    {
        if(socialdistancing>0.) {
            if(nsick >= casesuntilquarantine && quarantinestarted==false) { 
                std::cout<<"---> quarantine started!"<<std::endl;
                currentsocialdistancing = socialdistancing;
                quarantinestarttime = time;
                quarantinestarted = true;
            }
            if((time-quarantinestarttime) > quarantinetime && quarantinefinished==false) { 
                std::cout<<"---> quarantine ended!"<<std::endl;
                quarantinefinished = true;
            }
            if(quarantinefinished && currentsocialdistancing > 0.) currentsocialdistancing -= quarantineeasingfactor;
        }
        for(int i=0; i<npeople*(1.0-currentsocialdistancing); i++) people[i]->walk(); // make them walk
        
        for(int i=0; i<npeople; i++) // loop over all people
        {
            if(nstorebound<maxstorebound) {
                if(rando()<storeprobability) { 
                    people[i]->storebound(true);
                    nstorebound++;
                }
                if(std::sqrt( std::pow(people[i]->x()-xstore,2.) + std::pow(people[i]->y()-ystore,2.) ) < contactsize) 
                    if(people[i]->storebound()) nstorebound--;
                    people[i]->storebound(false);            
            }

            if(time-people[i]->infectiontime() > infectionlength && people[i]->status() == sick) // check how long they have been infected 
            {
                people[i]->status(immune);
                nimmune += 1;
                nsick -= 1;
            }
            for(int j=people[i]->id()+1; j<npeople; j++) // check if anyone is in contact
            {                
                if(people[i]->contact(*people[j])) // the people are in contact
                {
                    if(people[i]->status()==sick && people[j]->status()==healthy)
                    { 
                        if(rando()<infectionprobability) {
                            people[j]->status(sick); 
                            people[j]->infectiontime(time);
                            nsick += 1;
                            nhealthy -= 1;
                        }
                        people[i]->randomdirection();
                        people[j]->randomdirection();
                    }
                    else if(people[i]->status()==healthy && people[j]->status()==sick)
                    { 
                        if(rando()<infectionprobability) {
                            people[i]->status(sick); 
                            people[i]->infectiontime(time);
                            nsick += 1;
                            nhealthy -= 1;
                        }
                        people[i]->randomdirection();
                        people[j]->randomdirection();
                    }
                }
            }
            if(!people[i]->storebound()) people[i]->randomdirection();
        }
        time += 1.;
        std::cout<<"\rtime: "<<time<<", nsick: "<<nsick<<", nhealthy: "<<nhealthy<<", nimmune: "<<nimmune<<", nstorebound: "<<nstorebound<<"  "<<std::flush;
        outfile<<time<<"\t"<<nsick<<"\t"<<nhealthy<<"\t"<<nimmune<<std::endl;

        // save data for making a gif
        std::ofstream giffile;
        char buffer[50];
        sprintf(buffer,"gif_files/time_%.0f.dat",time);
        giffile.open(buffer);
        for(int i=0; i<npeople; i++) giffile<<people[i]->x()<<",\t"<<people[i]->y()<<",\t"<<people[i]->status()<<std::endl;
        giffile.close();
        plotforgif(time,people,quarantinestarted,quarantinestarttime,quarantinefinished,quarantinestarttime+quarantinetime);
    }
    outfile.close();
    std::cout<<std::endl;
    
    system("gifsicle --delay=10 --loop=5 gif_files/plot_*.gif > anim.gif");
}

