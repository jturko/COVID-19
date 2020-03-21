
// try to simulate the spread of a virus and maybe we'll include a 
// bit to do with the social distancing everyone is talking about 
// now due to the coronavirus
//
// joey turko, 20/3/2020

#include<iostream>
#include<cmath>
#include <fstream>

double xdim = 150;
double ydim = 150;
const int npeople = 200;
double infectionlength = 400;

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

        void print() { 
            std::cout<< "id: "<<fId<<",\t x: "<<fX<<",\t y: "<<fY<<",\t direction: "<<fDirection<<",\t status: "<<pcond(fStatus);
            if(fStatus!=healthy) std::cout<<",\t infection time: "<<fInfectionTime<<std::endl; else std::cout<<std::endl;
        }
        bool contact(person other) { 
            if( std::sqrt( std::pow(x()-other.x(),2.) + std::pow(y()-other.y(),2.) ) < 1.0 ) return true; 
            else return false; 
        }
        void walk() {
            // check if beyond boundary, move to it and change dir if we have
            if( x()<=0 )          { x(0.);      direction(360.*std::rand()/double(RAND_MAX)); }
            else if( x()>=xdim )  { x(xdim);    direction(360.*std::rand()/double(RAND_MAX)); }
            else if( y()<0 )      { y(0.);      direction(360.*std::rand()/double(RAND_MAX)); }
            else if( y()>ydim )   { y(ydim);    direction(360.*std::rand()/double(RAND_MAX)); } 
            x( x() + std::cos(direction()*M_PI/180.) );
            y( y() + std::sin(direction()*M_PI/180.) );
        }
        void changedirection() { direction(360.*std::rand()/double(RAND_MAX)); }
      
    private:
        int fId;
        double fX;
        double fY;
        double fInfectionTime;
        double fDirection;
        condition fStatus;
};

int main() {
    srand(time(0));
    std::ofstream outfile;
    outfile.open("output.dat");    

    int nhealthy = npeople-1;
    int nsick = 1;
    int nimmune = 0;
    double time = 0.;
    
    person * people[npeople];
    for(int i=0; i<npeople; i++) {
        people[i] = new person( i,   
                                xdim*std::rand()/double(RAND_MAX), 
                                ydim*std::rand()/double(RAND_MAX), 
                                360.*std::rand()/double(RAND_MAX),
                                healthy );
        people[i]->infectiontime(1e100);
    }
    people[0]->status(sick);
    people[0]->infectiontime(0);

    while(nimmune < npeople && nsick > 0) // this iterator is the passage of time
    {
        for(int i=0; i<npeople; i++) people[i]->walk(); // make them all walk
        
        for(int i=0; i<npeople; i++) // loop over all people
        {
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
                        people[j]->status(sick); 
                        people[j]->infectiontime(time);
                        nsick += 1;
                        nhealthy -= 1;
                    }
                    else if(people[i]->status()==healthy && people[j]->status()==sick)
                    { 
                        people[i]->status(sick); 
                        people[i]->infectiontime(time);
                        nsick += 1;
                        nhealthy -= 1;
                    }
                    people[i]->changedirection();
                    people[j]->changedirection();
                }
            }
        }
        time += 1.;
        std::cout<<"time: "<<time<<", nsick: "<<nsick<<", nhealthy: "<<nhealthy<<", nimmune: "<<nimmune<<"                \r";
        outfile<<time<<"\t"<<nsick<<"\t"<<nhealthy<<"\t"<<nimmune<<std::endl;
    }
    std::cout<<std::endl;
    outfile.close();
    //for(int i=0; i<npeople; i++) people[i]->print();
}
