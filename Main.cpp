#include <iostream>
#include <list>

#include "Readline.h"
using namespace std;

int main(int argc, char **argv)
{
    Readline& rl = Readline::GetInstance();
    rl.Run();

    return 0;
}
