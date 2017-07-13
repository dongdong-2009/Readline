/*
 * Readline.h
 *
 *  Created on: 2017Äê6ÔÂ26ÈÕ
 *      Author: Howard
 */

#ifndef _Readline_h_
#define _Readline_h_

#include <functional>
#include <list>
#include <memory>
#include <string>

class RobotEntity;

/**********************struct CmdNode**********************/
struct CmdNode
{
    typedef std::function<void(const char*)> CmdHandler;

    std::string name;
    std::list<CmdNode*> childs;
    CmdHandler handler;
};
CmdNode* CreateChild(CmdNode* parent, const char *name, CmdNode::CmdHandler handler);

/**********************class Readline**********************/
class Readline
{
public:
    Readline();
    ~Readline();
    static Readline& GetInstance();

    void Run();

    static char** CompleteHandler(const char *text, int start, int end);

    /* Generator function for command completion. STATE lets us
    know whether to start from scratch; without any state
    (i.e. STATE == 0), then we start at the top of the list. */
    static char* CommandGenerator(const char *text, int state);

private:
    void ExecuteCommand(const char *cmd);
    char** CompleteHandlerImpl(const char *text, int start, int end);
    char* CommandGeneratorImpl(const char *text, int state);
    char* Strip(char *str);
    char* Duplicate(const char *str);

    /* command handler */
    static void HandleDisabledSignal(int status);
    void SetDebugFlag(const char *arg);
    void UnsetDebugFlag(const char *arg);
    void ShowStatus(const char *arg);
    void StartUnitTest(const char *arg);
    void StartRobot(const char *arg);
    void StopRobot(const char *arg);
    void Exit(const char *arg);

private:
    CmdNode *root;
    bool running;
};

#endif /* _Readline_h_ */
