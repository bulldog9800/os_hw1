#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

using std::string;
using std::vector;

const std::string WHITESPACE = " \n\r\t\f\v";

class Command {
// TODO: Add your data members
    char* cmd_line;
    pid_t pid;
 public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  char* getCommand();
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:

  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
    char** args;
    int num_of_args;
    bool is_bg ;
    char* command ;
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand();
  void execute() override;
};







class PipeCommand : public Command {
  string first_command_str ;
  string second_command_str;
  bool stderr_flag ;
  Command* first_command;
  Command* second_command;
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
    string command;
    string file_path;
    bool is_append;
    Command* new_cmd;
    int stdout_location;
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
    char** args;
    int num_of_args;
public:
  ChangeDirCommand(const char* cmd_line);
  virtual ~ChangeDirCommand();
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand(){}
  void execute() override;
};

class ChangePromptCommand : public BuiltInCommand {
    string new_prompt;
    char** args;
    int num_of_args;
public:
    ChangePromptCommand(const char *cmd_line);
    virtual ~ChangePromptCommand();
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
    char** args;
    int num_of_args;
public:
  QuitCommand(const char* cmd_line);
  virtual ~QuitCommand() ;
  void execute() override;
};


class JobEntry {
public:
    int job_id;
    int process_id;
    bool is_bg;
    bool is_stopped;
    string command;
    time_t start_time;
    Command* command_obj;
    bool operator==(const JobEntry& j) const;
    JobEntry(int job_id, int pid, bool is_bg, bool is_stopped, string& command, time_t start, Command* command_obj);
};

class JobsList {

 public:
    vector<JobEntry*> jobs;
    int max_job_id;
  JobsList();
  ~JobsList() = default;
  void addJob(Command* cmd, pid_t pid, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  JobEntry * getJobByPid(int pid);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
    char** args;
    int num_of_args;
 public:
  KillCommand(const char* cmd_line);
  virtual ~KillCommand() ;
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    char** args;
    int num_of_args;
 public:
  ForegroundCommand(const char* cmd_line);
  virtual ~ForegroundCommand() ;
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    char** args;
    int num_of_args;
 public:
  BackgroundCommand(const char* cmd_line);
  virtual ~BackgroundCommand() ;
  void execute() override;
};


class CatCommand : public BuiltInCommand {
    char** args;
    int num_of_args;
 public:
  CatCommand(const char* cmd_line);
  virtual ~CatCommand() {}
  void execute() override;
};




class SmallShell {
 private:
  string prompt;
  string last_working_dir;

  SmallShell();

 public:
    pid_t pid_in_fg;
    pid_t smash_pid;
    Command* command_in_fg ;
  static JobsList jobs_list;
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  const string& getPrompt();
  void changePrompt(const string& new_prompt);
  const string& getLWD();
  void changeLWD(const string& new_lwd);

  // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
