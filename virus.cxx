
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
#include <sstream>

#include "TH1F.h"
#include "THStack.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TLegend.h"

#include "TError.h"

double xdim = 150;
double ydim = 100;
int npeople = 300;
double infectionduration = 400;
double quarantinefactor = 0.95;
double stepsize = 0.5;
double contactsize = 1.5;
double quarantineduration = 600;
double randomdirectioninc = 0.2;
double infectionprobability = 0.25;
int casesprequarantine = 80;
double quarantineeasingfactor = 0.01;
int immuneduration = 50;

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
             <<"infection prob.:\t"<<infectionprobability<<std::endl
             <<"infection duration:\t"<<infectionduration<<std::endl
	           <<"immune duration:\t"<<immuneduration<<std::endl
             <<"quarantine duration:\t"<<quarantineduration<<std::endl
             <<"cases pre-quarantine:\t"<<casesprequarantine<<std::endl
             <<"quarantine factor:\t"<<quarantinefactor<<std::endl
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
            fImmuneTime = 1e100;
        }
        person(int id) {
            fId = id;
            fX = xdim*rando();
            fY = ydim*rando();
            fStatus = healthy;
            fDirection = 360.*rando();
            fInfectionTime = 1e100;
            fImmuneTime = 1e100;
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

        double immunetime() { return fImmuneTime; }
        void immunetime(double time) { fImmuneTime = time; }

        void print() { 
            std::cout<< "id: "<<fId<<",\t x: "<<fX<<",\t y: "<<fY<<",\t direction: "<<fDirection<<",\t status: "<<pcond(fStatus);
            if(fStatus!=healthy) std::cout<<",\t infection time: "<<fInfectionTime<<std::endl; else std::cout<<std::endl;
        }
        bool contact(person other) { 
            if( std::sqrt( std::pow(fX-other.x(),2.) + std::pow(fY-other.y(),2.) ) < contactsize ) return true; 
            else return false; 
        }
        void walk() {
            // check if beyond boundary, move to it and change dir if we have
            if     ( fX <= 0. )     { fX = 0.;      fDirection = 2.*M_PI*rando(); }
            else if( fX >= xdim )   { fX = xdim;    fDirection = 2.*M_PI*rando(); }
            else if( fY <= 0. )     { fY = 0.;      fDirection = 2.*M_PI*rando(); }
            else if( fY >= ydim )   { fY = ydim;    fDirection = 2.*M_PI*rando(); } 
            fX += stepsize*std::cos(fDirection);
            fY += stepsize*std::sin(fDirection);
        }
        void randomdirection() { fDirection += (1.-2.*rando())*randomdirectioninc; }
      
    private:
        int fId;
        double fX;
        double fY;
        double fInfectionTime;
	      double fImmuneTime;
        double fDirection;
        condition fStatus;

};


int npixelx = 50;
int npixely = 50;

int getpixelx(person * guy) {
  double pct = guy->x() / xdim;
  int pixel = pct * (npixelx-1);
  return pixel;  
}

int getpixely(person * guy) {
  double pct = guy->y() / ydim;
  int pixel = pct * (npixely-1);
  return pixel;  
}

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
    axis->SetLimits(-1.,xdim+1.);
    hhealthy->GetHistogram()->SetMaximum(ydim+1.);    
    hhealthy->GetHistogram()->SetMinimum(-1.);
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
    TLegend * leg = new TLegend(0.1,0.7,0.3,0.9);
    leg->AddEntry(hhealthy,"healthy","p");
    leg->AddEntry(hsick,"sick","p");
    leg->AddEntry(himmune,"immune","p");
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->Draw("same");


    c1->SaveAs(Form("gif_files/plot_%05d.gif",time));
    delete c1;
    c1 = NULL;

    delete hs; delete hh; delete hi;
    delete hsick; delete hhealthy; delete himmune;
    delete leg;

}

void parseargs(int argc, char *argv[])
{
    if((argc-1)%2==0 && argc>1) {
        for(int i=1; i<argc; i+=2) {
            std::stringstream ss;
            ss << argv[i+1];
            std::string thisarg = std::string(argv[i]);
            if(thisarg=="-xdim" || thisarg=="-dx")                         ss >> xdim;
            else if(thisarg=="-ydim" || thisarg=="-dy")                    ss >> ydim;
            else if(thisarg=="-npeople" || thisarg=="-N")                  ss >> npeople;
            else if(thisarg=="-infectionduration" || thisarg=="-id")       ss >> infectionduration;
            else if(thisarg=="-quarantinefactor" || thisarg=="-qf")        ss >> quarantinefactor;
            else if(thisarg=="-stepsize" || thisarg=="-ss")                ss >> stepsize;
            else if(thisarg=="-contactsize" || thisarg=="-cs")             ss >> contactsize;
            else if(thisarg=="-quarantineduration" || thisarg=="-qd")      ss >> quarantineduration;
            else if(thisarg=="-infectionprobability" || thisarg=="-ip")    ss >> infectionprobability;
            else if(thisarg=="-casesprequarantine" || thisarg=="-cpq")     ss >> casesprequarantine;
            else if(thisarg=="-quarantineeasingfactor" || thisarg=="-qef") ss >> quarantineeasingfactor;
	          else if(thisarg=="-immuneduration" || thisarg=="-rd")	         ss >> immuneduration;
            else std::cout<<"---> unknown argument: "<<thisarg<<std::endl;
        }
    }
} 

int main(int argc, char *argv[]) {
    parseargs(argc, argv);
    printparameters();

    srand((unsigned int)(time(0)));
    //std::ofstream outfile;
    //outfile.open("output.dat");    

    int nhealthy = npeople-1;
    int nsick = 1;
    int nimmune = 0;
    double time = 0.;
    double currentquarantinefactor = 0.;    
    double quarantinestarttime = 1000000;
    bool quarantinestarted = false;
    bool quarantinefinished = false;

    // populate array with healthy people at random positions
    person * people[(const int)npeople];
    for(int i=0; i<npeople; i++) people[i] = new person(i);
    
    // make one persons sick
    people[0]->status(sick);
    people[0]->infectiontime(0);

    std::vector<int> pixels[npixelx][npixely];

    while(nsick > 0) // this iterator is the passage of time
    {
        // clear pixels
        for(int xx=0; xx<npixelx; xx++ ) {
          for(int yy=0; yy<npixely; yy++ ) {
            pixels[xx][yy].clear();
          }
        }
        // assign pixels
        for(int i=0; i<npeople; i++) {
            int px = getpixelx(people[i]);
            int py = getpixely(people[i]);
            pixels[px][py].push_back(i);
        }

        if(quarantinefactor>0.) {
            if(nsick >= casesprequarantine && quarantinestarted==false) { 
                std::cout<<"---> quarantine started!"<<std::endl;
                currentquarantinefactor = quarantinefactor;
                quarantinestarttime = time;
                quarantinestarted = true;
            }
            if((time-quarantinestarttime) > quarantineduration && quarantinefinished==false) { 
                std::cout<<"---> quarantine ended!"<<std::endl;
                quarantinefinished = true;
            }
            if(quarantinefinished && currentquarantinefactor > 0.) currentquarantinefactor -= quarantineeasingfactor;
            if(currentquarantinefactor < 0.) currentquarantinefactor = 0.;
        }
        for(int i=0; i<npeople*(1.0-currentquarantinefactor); i++) people[i]->walk(); // make them walk

        //for(int i=0; i<npeople; i++) // loop over all people
        //{
        //    if(time-people[i]->infectiontime() > infectionduration && people[i]->status() == sick) // check how long they have been infected 
        //    {
        //        people[i]->status(immune);
        //        people[i]->immunetime(time);
        //        nimmune++;
        //        nsick--;
        //    }
	      //    if(time-people[i]->immunetime() > immuneduration && people[i]->status() == immune) // check how long they have been immune 
	      //    {
		    //        people[i]->status(healthy);
        //        people[i]->infectiontime(1e100);
        //        people[i]->immunetime(1e100);
        //        nimmune--;
        //        nhealthy++;
	      //    }
        //    for(int j=people[i]->id()+1; j<npeople; j++) // check if anyone is in contact
        //    {                
        //        if(people[i]->contact(*people[j])) // the people are in contact
        //        {
        //            if(people[i]->status()==sick && people[j]->status()==healthy)
        //            { 
        //                if(rando()<infectionprobability) {
        //                    people[j]->status(sick); 
        //                    people[j]->infectiontime(time);
        //                    nsick += 1;
        //                    nhealthy -= 1;
        //                }
        //            }
        //            else if(people[i]->status()==healthy && people[j]->status()==sick)
        //            { 
        //                if(rando()<infectionprobability) {
        //                    people[i]->status(sick); 
        //                    people[i]->infectiontime(time);
        //                    nsick += 1;
        //                    nhealthy -= 1;
        //                }
        //            }
        //        }
        //    }
        //    people[i]->randomdirection();
        //}
        
        // loop over all pixels
        for(int px=0; px<npixelx; px++) {
          for(int py=0; py<npixely; py++) {
            // loop over the people in the current pixel
            for(int pp1=0; pp1<pixels[px][py].size(); pp1++) {
              person * guy1 = people[pixels[px][py][pp1]];
              if(time-guy1->infectiontime() > infectionduration && guy1->status() == sick) // check how long they have been infected 
              {
                  guy1->status(immune);
                  guy1->immunetime(time);
                  nimmune++;
                  nsick--;
              }
	            if(time-guy1->immunetime() > immuneduration && guy1->status() == immune) // check how long they have been immune 
	            {
		              guy1->status(healthy);
                  guy1->infectiontime(1e100);
                  guy1->immunetime(1e100);
                  nimmune--;
                  nhealthy++;
	            }
              guy1->randomdirection();
              // loop over the other people in the current pixel
              for(int pp2=pp1+1; pp2<pixels[px][py].size(); pp2++) {
                person * guy2 = people[pixels[px][py][pp2]];
                //check if in contact
                if(guy1->contact(*guy2)) {
                    if(guy1->status()==sick && guy2->status()==healthy)
                    { 
                        if(rando()<infectionprobability) {
                            guy2->status(sick); 
                            guy2->infectiontime(time);
                            nsick++;
                            nhealthy--;
                        }
                    }
                    else if(guy1->status()==healthy && guy2->status()==sick)
                    { 
                        if(rando()<infectionprobability) {
                            guy1->status(sick); 
                            guy1->infectiontime(time);
                            nsick++;
                            nhealthy--;
                        }
                    }
                }
              }
            }
          }
        }

        time += 1.;
        std::cout<<"\rtime: "<<time<<", nsick: "<<nsick<<", nhealthy: "<<nhealthy<<", nimmune: "<<nimmune<<"  "<<std::flush;
        //outfile<<time<<"\t"<<nsick<<"\t"<<nhealthy<<"\t"<<nimmune<<std::endl;

        // save data for making a gif
        //std::ofstream giffile;
        //char buffer[50];
        //sprintf(buffer,"gif_files/time_%.0f.dat",time);
        //giffile.open(buffer);
        //for(int i=0; i<npeople; i++) giffile<<people[i]->x()<<",\t"<<people[i]->y()<<",\t"<<people[i]->status()<<std::endl;
        //giffile.close();
        //plotforgif(time,people,quarantinestarted,quarantinestarttime,quarantinefinished,quarantinestarttime+quarantineduration);
    }
    //outfile.close();
    //std::cout<<std::endl;
    //
    //system("gifsicle --delay=5 --loop=5 gif_files/plot_*.gif > anim.gif");
    //std::cout<<"gif file created: anim.gif"<<std::endl;
    //system("rm -r gif_files/*");    

    return 0;
}

