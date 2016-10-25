#include<iostream>
#include<cstring>
#include<string>
#include<cstdlib> // system()
#include<unistd.h> // chdir()

using namespace std;

int main(int argc, char* argv[])
{
    /*
        check for 2 arguements.
        first : command name
        second : <db_name>
    */
    if(argc!=2)
    {
        cerr << "Usage " << argv[0] << " <db_name> \n";
        exit(0);
    }

    // second arguement is db_name
    string db_name = argv[1];

    // create s subdirectory for the database
    string command="mkdir ";
    command += db_name;
    ErrCode ec = system(command.c_str());

    if(ec != 0)
    {
        cerr << argv[0] <<": mkdir error for " << db_name << "\n";
        exit(ec);
    }

    if(chdir(db_name.c_str()) < 0)
    {
        cerr << argv[0] <<": chdir error to " << db_name << "\n";
        exit(1);
    }

    return 0;
}
