#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    std::cout << "smash: got ctrl-Z" << std::endl;

}

void ctrlCHandler(int sig_num) {
    std::cout << "smash: got ctrl-C" << std::endl;
    SmallShell& smash= SmallShell::getInstance() ;
    if (smash.pid_in_fg==0){
        return;
    }
    if (kill(smash.pid_in_fg, SIGKILL)==-1){
        perror("smash error: kill failed");
        return;
    }
    cout << "smash: process " << smash.pid_in_fg << " was killed" << endl;
    smash.pid_in_fg = 0;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

