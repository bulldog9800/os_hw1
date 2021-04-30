#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <assert.h>

using namespace std;

JobsList SmallShell::jobs_list;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell(): prompt("smash> "), last_working_dir("") {

}

SmallShell::~SmallShell() {

}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  // If first word contains & sign, remove it so we can compare it properly.
  if(firstWord[firstWord.size()-1]=='&'){
      firstWord.pop_back();
  }

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt") == 0) {
      return new ChangePromptCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) {
      return new ChangeDirCommand(cmd_line);
  }
  else if (firstWord.compare("jobs") == 0) {
      return new JobsCommand(cmd_line);
  }
  else if (firstWord.compare("bg") == 0) {
      return new BackgroundCommand(cmd_line);
  }
  else if (firstWord.compare("fg") == 0) {
      return new ForegroundCommand(cmd_line);
  }
  else{
      return new ExternalCommand(cmd_line);
  }


  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  Command* cmd = CreateCommand(cmd_line);
  cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

const string& SmallShell::getPrompt() {
    return prompt;
}

void SmallShell::changePrompt(const string &new_prompt) {
    prompt = new_prompt;
}

const string &SmallShell::getLWD() {
    return last_working_dir;
}

void SmallShell::changeLWD(const string &new_lwd) {
    last_working_dir = new_lwd;
}

/********************
 *
 *  CLASS COMMAND
 *
 ********************/

Command::Command(const char *cmd_line) {
    this->cmd_line = new char[strlen(cmd_line)+1];
    strcpy(this->cmd_line, cmd_line);
}

Command::~Command() {
    delete this->cmd_line;
}

char *Command::getCommand() {
    return cmd_line;
}

/**************************
 *
 * CLASS BUILT-IN COMMAND
 *
 **************************/

BuiltInCommand::BuiltInCommand(const char* cmd_line): Command(cmd_line){

}


/***************************
 *
 * CLASS CHANGE PROMPT COMMAND
 *
 ****************************/

ChangePromptCommand::ChangePromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line), new_prompt("smash> "),
num_of_args(0){
    args = new char*[20];
    char* line = new char[strlen(cmd_line)+1];
    strcpy(line, cmd_line);
    _removeBackgroundSign(line);
    num_of_args = _parseCommandLine(line, args);
    assert(strcmp(args[0],"chprompt")==0);
}

void ChangePromptCommand::execute() {
    if(num_of_args>1){
        new_prompt = string(args[1]);
        new_prompt += "> ";
    }
    else{
        assert(num_of_args==1);
        new_prompt = "smash> ";
    }

    SmallShell& smash = SmallShell::getInstance();
    smash.changePrompt(this->new_prompt);
}

ChangePromptCommand::~ChangePromptCommand() {
    delete args;
}

/************************************
 *
 * CLASS SHOW PID COMMAND
 *
 *************************************/

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line){

}



void GetCurrDirCommand::execute() {
    cout << string (getcwd(NULL,0)) << "\n" ;
}


/***************
    CHANGE DIR COMMAND (cd)
***************/

ChangeDirCommand::ChangeDirCommand(const char *cmd_line): BuiltInCommand(cmd_line), num_of_args(0) {
    args = new char*[20];
    char* line = new char[strlen(cmd_line)+1];
    strcpy(line, cmd_line);
    _removeBackgroundSign(line);
    num_of_args = _parseCommandLine(line, args);
    assert(strcmp(args[0],"cd")==0);
}

ChangeDirCommand::~ChangeDirCommand(){
    delete args;
}

void ChangeDirCommand::execute() {
    if (num_of_args > 2){
        cerr << "smash error: cd: too many arguments" << endl;
    }
    else if(num_of_args==1){
        return;
    }
    else{
        SmallShell& smash = SmallShell::getInstance();

        assert(num_of_args==2);
        if (strcmp(args[1],"-")==0) {
            if(smash.getLWD().empty()) {
                cerr << "smash error: cd: OLDPWD not set" << endl;
                return;
            }
            else {
                string temp = smash.getLWD();
                smash.changeLWD(string (getcwd(NULL,0)));
                if (chdir(temp.c_str())==-1){
                    perror("smash error: chdir failed");
                }
                return;
            }
        }
        else {
            smash.changeLWD(string (getcwd(NULL,0)));
            if(chdir(args[1])==-1){
                perror("smash error: chdir failed");
            }
            return;
        }

    }

}

/*****************************
 *
 *  EXTERNAL COMMAND CLASS
 *
 *****************************/


ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line),is_bg(false){
    is_bg= _isBackgroundComamnd(cmd_line);
    char* line =new char[strlen(cmd_line+1)];
            strcpy (line,cmd_line);
            _removeBackgroundSign(line);
    args = new char*[20];
    num_of_args = _parseCommandLine(line, args);
    command =new char[strlen(cmd_line)+1];
    strcpy(command,cmd_line);
    _removeBackgroundSign(command);


}
void ExternalCommand::execute() {
    SmallShell& smash = SmallShell::getInstance() ;
    pid_t pid = fork();
    if (pid<0){
        perror("smash error: fork failed");
        return;
    }
    if(pid==0){
        char* exec_args[4]={"bash","-c",command,NULL} ;
        if(setpgrp()==-1) {
            perror("smash error: setgrp failed");
        }
        execv("/bin/bash",exec_args);
        perror("smash error: execv failed");

        return;
    }
    else {
        if (is_bg){
            SmallShell::jobs_list.addJob(this, pid);
            waitpid(pid, nullptr, WNOHANG);
        }
        else {
            waitpid(pid, nullptr, WUNTRACED);
        }
    }

}


ExternalCommand::~ExternalCommand(){
    delete args ;
    delete command ;

}

JobsList::JobsList(): jobs(), max_job_id(1) {

}

bool JobEntry::operator==(const JobEntry& j) const{
    return this->job_id== j.job_id ;
}

JobEntry::JobEntry(int job_id, int pid, bool is_bg, bool is_stopped, string& command, time_t start):
        job_id(job_id), process_id(pid), is_bg(is_bg), is_stopped(is_stopped), command(command),
        start_time(start){

}

JobEntry * JobsList::getJobById(int jobId) {
    for (int i=0;i<jobs.size();i++){
        if (jobs[i]->job_id==jobId)
            return jobs[i];
    }
    return nullptr ;
}

void JobsList::removeJobById(int jobId) {
    for(int i=0; i<jobs.size(); i++){
        if (jobs[i]->job_id == jobId){
            JobEntry* temp = jobs[i];
            jobs.erase(jobs.begin()+i);
            delete temp;
        }
    }
}

JobEntry *JobsList::getLastJob(int *lastJobId) {
    if(jobs.empty()){
        *lastJobId = -1;
        return nullptr;
    }
    *lastJobId = jobs[jobs.size()-1]->job_id;
    return jobs[jobs.size()-1];
}
JobEntry * JobsList::getLastStoppedJob(int *jobId) {
    auto rit = jobs.rbegin();
    for(; rit!=jobs.rend(); ++rit){
        if((*rit)->is_stopped) {
            *jobId = (*rit)->job_id;
            return *rit;
        }
    }
    *jobId = -1;
    return nullptr ;
}

void JobsList::removeFinishedJobs() {
    for(int i=0 ;i<jobs.size();i++){
        int res=waitpid(jobs[i]->process_id,NULL,WNOHANG);
        if (res==-1){
            perror("smash error: waitpid failed");
        }
        if(res>0){

            JobEntry* temp = jobs[i];
            jobs.erase(jobs.begin()+i);
            delete temp;

        }
    }
}


void JobsList::killAllJobs() {
    for(int i=0 ;i<jobs.size();i++){
       if( killpg(jobs[i]->process_id,SIGKILL)<0){
           perror("smash error: killpg failed");
       }
        JobEntry* temp = jobs[i];
       jobs.erase(jobs.begin()+i);
        delete temp;

    }
    max_job_id = 1;
}

void JobsList::printJobsList() {
    removeFinishedJobs() ;
    for(int i=0;i<jobs.size();i++){
        if(jobs[i]->is_stopped){
            cout<< "[" << jobs[i]->job_id<<"] "<<jobs[i]->command<<" : "<<jobs[i]->process_id<< " "<<difftime(time(nullptr),jobs[i]->start_time)<< " secs (stopped)" << endl ;
        }
        else {
            cout<< "[" << jobs[i]->job_id<<"] "<<jobs[i]->command<<" : "<<jobs[i]->process_id<< " "<<difftime(time(nullptr),jobs[i]->start_time)<< " secs" << endl ;
        }
    }
}



void JobsList::addJob(Command *cmd, pid_t pid, bool isStopped) {
    removeFinishedJobs();
    int next_job_id;
    if (jobs.empty()){
        next_job_id = 1;
    }
    else{
        next_job_id = jobs[jobs.size()-1]->job_id + 1;
    }
    bool is_bg = ((ExternalCommand*)cmd)->is_bg;
    time_t start_time = time(nullptr);
    if(start_time == -1){
        perror("smash error: time failed");
    }
    string command = string(cmd->getCommand());
    auto* job_entry = new JobEntry(next_job_id, pid, is_bg, isStopped, command, start_time);

    jobs.push_back(job_entry);
}

/***************************
    jobs command
****************************/
JobsCommand::JobsCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void JobsCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.jobs_list.printJobsList();
}

/***************
    killcommand
***************/
KillCommand::KillCommand(const char *cmd_line): BuiltInCommand(cmd_line) {
    args = new char*[20];
    char* line = new char[strlen(cmd_line)+1];
    strcpy(line, cmd_line);
    _removeBackgroundSign(line);
    num_of_args = _parseCommandLine(line, args);
    delete[] line;

}
KillCommand::~KillCommand() {
    delete args;

}
void KillCommand::execute() {
    if(num_of_args!=3){
        std::cerr << "smash error: kill: invalid arguments" << endl ;
        return;
    }
    if (args[1][0]!='-'){
        std::cerr << "smash error: kill: invalid arguments" << endl ;
        return;
    }
    for (int i=1;i< strlen(args[1]);i++){
        if (!isdigit(args[1][i])){
            std::cerr << "smash error: kill: invalid arguments" << endl ;
            return;
        }
    }

    for (int i=0;i< strlen(args[2]);i++){
        if (!isdigit(args[2][i])){
            std::cerr << "smash error: kill: invalid arguments" << endl ;
            return;
        }

    }

    SmallShell& smash = SmallShell::getInstance();
    int job_id = atoi(args[2]);
    string signal_str = string(args[1]);
    signal_str.erase(signal_str.begin());
    int sig = stoi(signal_str);
    JobEntry* our_job =smash.jobs_list.getJobById(job_id);
    if(our_job== nullptr){
        std::cerr <<"smash error: kill: job-id " << job_id << " does not exist" << std::endl ;
        return;
    }
    if(killpg(our_job->process_id, sig) < 0) {
        perror("smash error: killpg failed");
        return;
    }
    std::cout << "signal number " << sig << " was sent to pid " << our_job->process_id<< std::endl;
    if(sig==SIGCONT){
        our_job->is_stopped= false ;
        our_job->is_bg= true ;
    }

}
/***************
    fgcommand
***************/

ForegroundCommand::ForegroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line){
    args = new char*[20];
    char* line = new char[strlen(cmd_line)+1];
    strcpy(line, cmd_line);
    _removeBackgroundSign(line);
    num_of_args = _parseCommandLine(line, args);
    delete[] line;
}

ForegroundCommand::~ForegroundCommand()   {
    delete args;
}

void ForegroundCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.jobs_list.removeFinishedJobs();
    if(num_of_args>2){
        std::cerr<<"smash error: fg: invalid arguments"<<endl;
        return;
    }

    ///no_args
    if (num_of_args==1){
       if(smash.jobs_list.jobs.empty()) {
           std::cerr<<"smash error: fg: jobs list is empty" <<endl ;
           return;
       }
       int* job_id ;
      JobEntry* our_job = smash.jobs_list.getLastJob(job_id);
      cout << our_job->command << " : " << our_job->process_id;
       if(kill(our_job->process_id,SIGCONT)==-1){
           perror("smash error: kill failed");
           return;
       }
       if(waitpid(our_job->process_id,NULL,WUNTRACED)==-1){
           perror("smash error: waitpid failed");
           return;
       }
    }
    ////with_args
    for (int i = 0; i < strlen(args[1]); i++) {
        if(!isdigit(args[1][i])){
            std::cerr<<"smash error: fg: invalid arguments"<<endl;
            return;

        }
        int job_id = atoi(args[1]);
       JobEntry* our_job = smash.jobs_list.getJobById(job_id);
        if(our_job== nullptr){
            cerr<<"smash error: fg: job-id "<<job_id<<" does not exist"<<endl;
            return;
        }
        cout << our_job->command << " : " << our_job->process_id;
        if(kill(our_job->process_id,SIGCONT)==-1){
            perror("smash error: kill failed");
            return;
        }
        if(waitpid(our_job->process_id,NULL,WUNTRACED)==-1){
            perror("smash error: waitpid failed");
            return;
        }

    }

}




/****************************
*
*     BACKGROUND COMMAND
*
*****************************/

BackgroundCommand::BackgroundCommand(const char *cmd_line): BuiltInCommand(cmd_line) {
    args = new char*[20];
    char* line = new char[strlen(cmd_line)+1];
    strcpy(line, cmd_line);
    _removeBackgroundSign(line);
    num_of_args = _parseCommandLine(line, args);
    delete[] line;
}

void BackgroundCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if(num_of_args>2){
        std::cerr << "smash error: bg: invalid arguments" << endl;
        return;
    }

    /// No args
    if (num_of_args==1){
        int* job_id;
        if(smash.jobs_list.getLastStoppedJob(job_id) == nullptr) {
            std::cerr<<"smash error: bg: there is no stopped jobs to resume" <<endl ;
            return;
        }
        JobEntry* our_job = smash.jobs_list.getLastJob(job_id);
        cout << our_job->command << " : " << our_job->process_id;
        if(kill(our_job->process_id,SIGCONT)==-1){
            perror("smash error: kill failed");
            return;
        }
    }

    /// With args
    for (int i = 0; i < strlen(args[1]); i++) {
        if(!isdigit(args[1][i])){
            std::cerr << "smash error: fg: invalid arguments" << endl;
            return;
        }
        int job_id = atoi(args[1]);
        JobEntry* our_job = smash.jobs_list.getJobById(job_id);
        if(our_job== nullptr){
            cerr<<" smash error: fg: job-id " << job_id << "does not exist" << endl;
            return;
        }
        cout << our_job->command << " : " << our_job->process_id;
        if(kill(our_job->process_id,SIGCONT)==-1){
            perror("smash error: kill failed");
            return;
        }
    }
}

