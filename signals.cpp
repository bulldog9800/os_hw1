#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    std::cout << "smash: got ctrl-Z" << std::endl;
    SmallShell& smash = SmallShell::getInstance();
    if (smash.pid_in_fg==0){
        return;
    }
    if (kill(smash.pid_in_fg, SIGSTOP)==-1){
        perror("smash error: kill failed");
        return;
    }
    JobEntry* our_job = smash.jobs_list.getJobByPid(smash.pid_in_fg);
    if(our_job!= nullptr){
        our_job->is_stopped = true;
        our_job->is_bg = false;
    }
    else{
        smash.jobs_list.addJob(smash.command_in_fg,smash.pid_in_fg, true);
    }

    cout << "smash: process " << smash.pid_in_fg << " was stopped" << endl;
    smash.pid_in_fg = 0;
    smash.command_in_fg = nullptr ;

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

