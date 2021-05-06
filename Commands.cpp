#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


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

SmallShell::SmallShell(): prompt("smash> "), last_working_dir(""), pid_in_fg(0), smash_pid(getpid()) {

}

SmallShell::~SmallShell() {

}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if(cmd_s.find('>') != string::npos ){
      return new RedirectionCommand(cmd_line);
  }
  if(cmd_s.find('|') != string::npos ){
      return new PipeCommand(cmd_line);
  }

  // If first word contains & sign, remove it so we can compare it properly.
  if(firstWord[firstWord.size()-1]=='&'){
      firstWord.pop_back();
  }


  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if(firstWord.compare("cat")==0){
        return new CatCommand(cmd_line);
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
  else if (firstWord.compare("kill") == 0) {
      return new KillCommand(cmd_line);
  }
  else if (firstWord.compare("quit") == 0) {
      return new QuitCommand(cmd_line);
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
    SmallShell& smash = SmallShell::getInstance();
    cout << "smash pid is " << smash.smash_pid << endl;
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
                    smash.changeLWD(temp);
                    perror("smash error: chdir failed");

                }
                return;
            }
        }
        else {
            string temp = smash.getLWD();
            smash.changeLWD(string (getcwd(NULL,0)));
            if(chdir(args[1])==-1){
                smash.changeLWD(temp);
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
    if (string(line).empty()) {
        command = (char *) string().c_str();
        return;
    }
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
        char* exec_args[4]={"/bin/bash","-c",command,NULL} ;
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
            smash.pid_in_fg = pid;
            smash.command_in_fg=this ;
            waitpid(pid, nullptr, WUNTRACED);
            smash.pid_in_fg = 0;
            smash.command_in_fg = nullptr;
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

JobEntry::JobEntry(int job_id, int pid, bool is_bg, bool is_stopped, string& command, time_t start, Command* command_obj):
        job_id(job_id), process_id(pid), is_bg(is_bg), is_stopped(is_stopped), command(command),
        start_time(start), command_obj(command_obj){

}

JobEntry * JobsList::getJobById(int jobId) {
    for (unsigned int i=0;i<jobs.size();i++){
        if (jobs[i]->job_id==jobId)
            return jobs[i];
    }
    return nullptr ;
}

void JobsList::removeJobById(int jobId) {
    for(unsigned int i=0; i<jobs.size(); i++){
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
    vector<int> to_be_removed;
    for(unsigned int i=0 ;i<jobs.size(); i++){
        int status;
        int res = waitpid(jobs[i]->process_id, &status,WNOHANG | WUNTRACED | WCONTINUED);
        if (res==-1){
            perror("smash error: Here waitpid failed");
            return;
        }
        if(res > 0 && !jobs[i]->is_stopped && !WIFCONTINUED(status)){
            to_be_removed.push_back(jobs[i]->job_id);
        }
    }
    for (unsigned int i=0; i<to_be_removed.size(); i++){
        removeJobById(to_be_removed[i]);
    }
}


void JobsList::killAllJobs() {
    for(unsigned int i=0 ;i<jobs.size();i++){
       if( kill(jobs[i]->process_id,SIGKILL)<0){
           perror("smash error: kill failed");
       }
        JobEntry* temp = jobs[i];
       jobs.erase(jobs.begin()+i);
        delete temp;

    }
    max_job_id = 1;
}

void JobsList::printJobsList() {
    removeFinishedJobs() ;
    for(unsigned int i=0;i<jobs.size();i++){
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
        return;
    }
    string command = string(cmd->getCommand());
    auto* job_entry = new JobEntry(next_job_id, pid, is_bg, isStopped, command, start_time, cmd);

    jobs.push_back(job_entry);
}

JobEntry *JobsList::getJobByPid(int pid) {
    for (unsigned int i=0;i<jobs.size();i++){
        if (jobs[i]->process_id==pid)
            return jobs[i];
    }
    return nullptr ;
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

    for (unsigned int i=1;i< strlen(args[1]);i++){
        if (!isdigit(args[1][i])){
            std::cerr << "smash error: kill: invalid arguments" << endl ;
            return;
        }

    }
    int num_of_minos=0;
    for (unsigned int i=0;i< strlen(args[2]);i++){
        if ((!isdigit(args[2][i]) && args[2][i] != '-')||num_of_minos>1){
            std::cerr << "smash error: kill: invalid arguments" << endl ;
            return;
        }
        if(args[1][i] == '-'){
            num_of_minos++;
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
    if(kill(our_job->process_id, sig) < 0) {
        perror("smash error: kill failed");
        return;
    }
    std::cout << "signal number " << sig << " was sent to pid " << our_job->process_id<< std::endl;
    if(sig==SIGKILL){
        smash.jobs_list.removeJobById(our_job->job_id);
    }
    if(sig==SIGCONT){
        our_job->is_stopped= false ;
        our_job->is_bg= true ;
    }
    if(sig == SIGSTOP){
        our_job->is_stopped= true ;
        our_job->is_bg= false ;
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
       int job_id ;
      JobEntry* our_job = smash.jobs_list.getLastJob(&job_id);
      cout << our_job->command << " : " << our_job->process_id << endl;
      if (our_job->is_stopped) {
          if (kill(our_job->process_id, SIGCONT) == -1) {
              perror("smash error: kill failed");
              return;
          }
          our_job->is_stopped = false;
      }
      int status;
      smash.pid_in_fg=our_job->process_id ;
      smash.command_in_fg = our_job->command_obj;
      if(waitpid(our_job->process_id,&status,WUNTRACED)==-1){
          perror("smash error: waitpid failed");
          return;
      }
      if (!our_job->is_stopped) {
          smash.jobs_list.removeJobById(our_job->job_id);
      }

      smash.pid_in_fg = 0;
    }
    ////with_args
    else {
        int num_of_minos=0;
        for (unsigned int i=0;i< strlen(args[1]);i++){
            if ((!isdigit(args[1][i]) && args[1][i] != '-')||num_of_minos>1){
                std::cerr << "smash error: fg: invalid arguments" << endl ;
                return;
            }
            if(args[1][i] == '-'){
                num_of_minos++;
            }

        }
            int job_id = atoi(args[1]);
            JobEntry *our_job = smash.jobs_list.getJobById(job_id);
            if (our_job == nullptr) {
                cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
                return;
            }
            cout << our_job->command << " : " << our_job->process_id << endl;
            if (our_job->is_stopped) {
                if (kill(our_job->process_id, SIGCONT) == -1) {
                    perror("smash error: kill failed");
                    return;
                }
                our_job->is_stopped = false;
            }
            int status;
            smash.pid_in_fg = our_job->process_id;
            smash.command_in_fg = our_job->command_obj;
            if (waitpid(our_job->process_id, &status, WUNTRACED) == -1) {
                perror("smash error: waitpid failed");
                return;
            }
            if (!our_job->is_stopped) {
                smash.jobs_list.removeJobById(our_job->job_id);
            }
            smash.pid_in_fg = 0;
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
BackgroundCommand::~BackgroundCommand()  {
    delete args ;
}

void BackgroundCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if(num_of_args>2){
        std::cerr << "smash error: bg: invalid arguments" << endl;
        return;
    }

    /// No args
    if (num_of_args==1){
        int job_id;
        if(smash.jobs_list.getLastStoppedJob(&job_id) == nullptr) {
            std::cerr<<"smash error: bg: there is no stopped jobs to resume" <<endl ;
            return;
        }
        JobEntry* our_job = smash.jobs_list.getLastStoppedJob(&job_id);
        cout << our_job->command << " : " << our_job->process_id << endl;
        if(kill(our_job->process_id,SIGCONT)==-1){
            perror("smash error: kill failed");
            return;
        }
        our_job->is_stopped = false;
        our_job->is_bg = true;
        return;
    }

    /// With args
    int num_of_minos=0;
    for (unsigned int i=0;i< strlen(args[1]);i++){
        if ((!isdigit(args[1][i]) && args[1][i] != '-')||num_of_minos>1){
            std::cerr << "smash error: fg: invalid arguments" << endl ;
            return;
        }
        if(args[1][i] == '-'){
            num_of_minos++;
        }

    }
        int job_id = atoi(args[1]);
        JobEntry* our_job = smash.jobs_list.getJobById(job_id);
        if(our_job== nullptr){
            cerr<<"smash error: bg: job-id " << job_id << " does not exist" << endl;
            return;
        }
        if (!our_job->is_stopped) {
            cerr<<"smash error: bg: job-id " << job_id << " is already running in the background" << endl;
            return;
        }
        cout << our_job->command << " : " << our_job->process_id << endl;
        if(kill(our_job->process_id,SIGCONT)==-1){
            perror("smash error: kill failed");
            return;
        }
        our_job->is_stopped = false;
        our_job->is_bg = true;
    }



/****************************
*
*     Quit COMMAND
*
*****************************/
QuitCommand::QuitCommand(const char *cmd_line) : BuiltInCommand(cmd_line){
    args = new char*[20];
    char* line = new char[strlen(cmd_line)+1];
    strcpy(line, cmd_line);
    _removeBackgroundSign(line);
    num_of_args = _parseCommandLine(line, args);
    delete[] line;
}
QuitCommand::~QuitCommand() {
    delete args;
}
void QuitCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if (num_of_args > 1) {
        if (strcmp(args[1], "kill") == 0) {
            SmallShell::jobs_list.removeFinishedJobs();
            cout << "smash: sending SIGKILL signal to " << smash.jobs_list.jobs.size() << " jobs:" << endl;
            for (unsigned int i = 0; i < smash.jobs_list.jobs.size(); i++) {
                cout << smash.jobs_list.jobs[i]->process_id << ": " << smash.jobs_list.jobs[i]->command << endl;

            }
            for (unsigned int i = 0; i < smash.jobs_list.jobs.size(); i++) {
                if (kill(smash.jobs_list.jobs[i]->process_id, SIGKILL) == -1) {
                    perror("smash error: kill failed");
                    return;
                }
            }

            exit(0);

        }
    }

    else{
        exit(0);
    }

}

/************************************
 *
 *       REDIRECTION COMMAND
 *
 ************************************/

RedirectionCommand::RedirectionCommand(const char *cmd_line) : Command(cmd_line), new_cmd(nullptr) {
    string cmd_s = _trim(string(cmd_line));
    int position = (int)cmd_s.find('>');
    if (cmd_s[position+1] == '>'){
        is_append = true;
        file_path = cmd_s.substr(position+2, cmd_s.size());
    }
    else{
        is_append = false;
        file_path = cmd_s.substr(position+1, cmd_s.size());
    }
    command = cmd_s.substr(0, position);
    command = _trim(command);
    file_path = _trim(file_path);
    _removeBackgroundSign((char *)command.c_str());
    _removeBackgroundSign((char *)file_path.c_str());


    stdout_location = dup(1);
    if (stdout_location == -1) {
        perror("smash error: dup failed");
        return;
    }
    if (close(1) == -1) {
        perror("smash error: close failed");
        return;
    }
    int fd;
    if (is_append){
        fd = open(file_path.c_str(), O_CREAT | O_RDWR | O_APPEND, 0777);
        if (fd == -1) {
            perror("smash error: open failed");
            return;
        }
    }
    else {
        fd = open(file_path.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0777);
        if (fd == -1) {
            perror("smash error: open failed");
            return;
        }
    }
    SmallShell &smash = SmallShell::getInstance();
    new_cmd = smash.CreateCommand(command.c_str());
}

void RedirectionCommand::execute() {
    if(new_cmd== nullptr){
        if (dup2(stdout_location, 1)==-1){
            perror("smash error: dup2 failed");
            return;
        }
        return;
    }
    new_cmd->execute();
    if (close(1) == -1) {
        perror("smash error: close failed");
        return;
    }
    if (dup2(stdout_location, 1)==-1){
        perror("smash error: dup2 failed");
        return;
    }
}

/************************************
 *
 *       PIPE COMMAND
 *
 ************************************/

PipeCommand::PipeCommand(const char* cmd_line): Command(cmd_line){
    string cmd_s = _trim(string(cmd_line));
    int position =cmd_s.find('|');
    if(cmd_s[position+1]=='&'){
        second_command_str=cmd_s.substr(position + 2, cmd_s.size());
        stderr_flag=true ;
    }
    else{
        second_command_str=cmd_s.substr(position + 1, cmd_s.size());
        stderr_flag=false ;
    }
    first_command_str=cmd_s.substr(0, position);
    first_command_str= _trim(first_command_str);
    second_command_str= _trim(second_command_str);
    _removeBackgroundSign((char*)first_command_str.c_str());
    _removeBackgroundSign((char*)second_command_str.c_str());

    SmallShell& smash = SmallShell::getInstance();
    first_command = smash.CreateCommand(first_command_str.c_str());
    second_command = smash.CreateCommand(second_command_str.c_str());
}

void PipeCommand::execute() {
    int swap = 0;
    if (stderr_flag){
        swap = 2;
    }
    else{
        swap = 1;
    }
    int fd[2];
    if(pipe(fd)==-1){
        perror("smash error: pipe failed");
        return;
    }
    pid_t pid1 = fork();
    if (pid1 == -1){
        perror("smash error: fork failed");
        return;
    }
    if (pid1 == 0) {
        // first child
        if(setpgrp()==-1) {
            perror("smash error: setgrp failed");
        }
        if (dup2(fd[1], swap)==-1){
            perror("smash error: dup2 failed");
            return;
        }
        if (close(fd[0])==-1){
            perror("smash error: close failed");
            return;
        }
        if (close(fd[1])==-1) {
            perror("smash error: close failed");
            return;
        }
        first_command->execute();
        exit(0);
    }
    pid_t pid2 = fork();
    if (pid2==-1) {
        perror("smash error: fork failed");
        return;
    }
    if (pid2 == 0) {
        // second child
        if(setpgrp()==-1) {
            perror("smash error: setgrp failed");
        }
        if (dup2(fd[0], 0)==-1){
            perror("smash error: dup2 failed");
            return;
        }
        if (close(fd[0]) == -1) {
            perror("smash error: close failed");
            return;
        }
        if (close(fd[1])==-1) {
            perror("smash error: close failed");
            return;
        }
        second_command->execute();
        exit(0);
    }
    if (close(fd[0])==-1){
        perror("smash error: close failed");
        return;
    }
    if (close(fd[1])==-1){
        perror("smash error: close failed");
        return;
    }

    if(waitpid(pid1, nullptr, WUNTRACED)==-1){
        perror("smash error: waitpid failed");
        return;
    }
    if(waitpid(pid2, nullptr, WUNTRACED)==-1){
        perror("smash error: waitpid failed");
        return;
    }



}

/************************************
*
*       cat COMMAND
*
************************************/
CatCommand::CatCommand(const char *cmd_line) : BuiltInCommand(cmd_line){
    args = new char*[20];
    char* line = new char[strlen(cmd_line)+1];
    strcpy(line, cmd_line);
    _removeBackgroundSign(line);
    num_of_args = _parseCommandLine(line, args);
    delete[] line;

}

void CatCommand::execute() {
    if(num_of_args<2){
        cerr <<"smash error: cat: not enough arguments"<<endl ;
    }
    for(int i=1;i<num_of_args;i++){
        int f = open(args[i], O_RDONLY,0777);
        if(f==-1){
            perror("smash error: open failed");
            return;
        }
        char* buffer= new char[200];
        int num_read=0;
        while((num_read= read(f,buffer,200)) !=0){
            if(num_read==-1){
                perror("smash error: read failed");
                return;
            }
            int num_write=0;
            while(num_write<num_read){
                num_write= (int)write(1,buffer,num_read);
                if(num_write==-1){
                    perror("smash error: write failed");
                    return;
                }
            }
        }
        if(close(f)==-1){
            perror("smash error: close failed");
            return;
        }
    }

}