
// try to simulate the spread of a virus and maybe we'll include a 
// bit to do with the social distancing everyone is talking about 
// now due to the coronavirus
//
// joey turko, 20/3/2020

#include<iostream>
#include<cmath>
#include<fstream>

double xdim = 150;
double ydim = 100;
const int npeople = 200;
double infectionlength = 200;
double socialdistancing = 0.9;
double stepsize = 0.5;
double contactsize = 3.0;
double quarantinetime = 600;
double changedirectioninc = 0.2;
double infectionprobability = 0.05;

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
        }
        person(int id) {
            fId = id;
            fX = xdim*rando();
            fY = ydim*rando();
            fStatus = healthy;
            fDirection = 360.*rando();
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
            if( std::sqrt( std::pow(x()-other.x(),2.) + std::pow(y()-other.y(),2.) ) < contactsize ) return true; 
            else return false; 
        }
        void walk() {
            // check if beyond boundary, move to it and change dir if we have
            if     ( x() <= 0. )     { x(0.);      direction(2.*M_PI*rando()); }
            else if( x() >= xdim )   { x(xdim);    direction(2.*M_PI*rando()); }
            else if( y() <= 0. )     { y(0.);      direction(2.*M_PI*rando()); }
            else if( y() >= ydim )   { y(ydim);    direction(2.*M_PI*rando()); } 
            x( x()+stepsize*std::cos(direction()) );
            y( y()+stepsize*std::sin(direction()) );
        }
        void changedirection() { fDirection += (1.-2.*rando())*changedirectioninc; }
      
    private:
        int fId;
        double fX;
        double fY;
        double fInfectionTime;
        double fDirection;
        condition fStatus;
};

int main() {
    printparameters();

    srand(time(0));
    std::ofstream outfile;
    outfile.open("output.dat");    

    int nhealthy = npeople-1;
    int nsick = 1;
    int nimmune = 0;
    double time = 0.;
    
    // populate array with healthy people at random positions
    person * people[npeople];
    for(int i=0; i<npeople; i++) people[i] = new person(i);
    
    // make one persons sick
    people[0]->status(sick);
    people[0]->infectiontime(0);

    while(nimmune < npeople && nsick > 0) // this iterator is the passage of time
    {
        if(time > quarantinetime) socialdistancing = 0.;
        for(int i=0; i<npeople*(1.0-socialdistancing); i++) people[i]->walk(); // make them walk
        
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
                    if(people[i]->status()==sick && people[j]->status()==healthy && rando()<infectionprobability)
                    { 
                        people[j]->status(sick); 
                        people[j]->infectiontime(time);
                        nsick += 1;
                        nhealthy -= 1;
                    }
                    else if(people[i]->status()==healthy && people[j]->status()==sick && rando()<infectionprobability)
                    { 
                        people[i]->status(sick); 
                        people[i]->infectiontime(time);
                        nsick += 1;
                        nhealthy -= 1;
                    }
                }
            }
            people[i]->changedirection();
        }
        time += 1.;
        std::cout<<std::flush<<"time: "<<time<<", nsick: "<<nsick<<", nhealthy: "<<nhealthy<<", nimmune: "<<nimmune<<"                \r";
        outfile<<time<<"\t"<<nsick<<"\t"<<nhealthy<<"\t"<<nimmune<<std::endl;

        // save data for making a gif
        std::ofstream giffile;
        char buffer[50];
        sprintf(buffer,"gif_files/time_%.0f.dat",time);
        giffile.open(buffer);
        for(int i=0; i<npeople; i++) giffile<<people[i]->x()<<",\t"<<people[i]->y()<<",\t"<<people[i]->status()<<std::endl;
        giffile.close();
    }
    outfile.close();
    std::cout<<std::endl;
}
