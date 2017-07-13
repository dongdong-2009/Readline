/*
 * Readline.cpp
 *
 *  Created on: 2017Äê6ÔÂ26ÈÕ
 *      Author: Howard
 */
#include "Readline.h"

/* readline */
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <iostream>  //cout

using namespace std;
using namespace std::placeholders;

CmdNode* CreateChild(CmdNode* parent, const char *name, CmdNode::CmdHandler handler)
{
    CmdNode *node = new CmdNode;
    node->name.assign(name);
    node->handler = handler;

    parent->childs.push_back(node);
    return node;
}

/**********************class Readline**********************/
Readline::Readline()
{
    rl_readline_name = "XXX";
    rl_attempted_completion_function = Readline::CompleteHandler;
    rl_attempted_completion_over = 1;

    auto handler = [](const char *arg)->void
    {
        cout << "invalid operation! arg = \"" << arg << "\"" << endl;
    };
    root = new CmdNode;
    root->name.assign("");
    root->handler = handler;

    CmdNode *node;
    node = CreateChild(root, "show", handler);
    CreateChild(node, "status", bind(&Readline::ShowStatus, this, _1));
    CreateChild(node, "account", handler);

    node = CreateChild(root, "start", handler);
    CreateChild(node, "robot", bind(&Readline::StartRobot, this, _1));
    CreateChild(node, "unit_test", bind(&Readline::StartUnitTest, this, _1));

    node = CreateChild(root, "stop", handler);
    CreateChild(node, "robot", bind(&Readline::StopRobot, this, _1));

    node = CreateChild(root, "set_debug_flag", bind(&Readline::SetDebugFlag, this, _1));
    node = CreateChild(root, "unset_debug_flag", bind(&Readline::UnsetDebugFlag, this, _1));

    CreateChild(root, "exit", bind(&Readline::Exit, this, _1));

    running = true;
}

Readline::~Readline()
{}

Readline& Readline::GetInstance()
{
    static Readline instance;
    return instance;
}

void Readline::Run()
{
    if (signal(SIGINT  /* Ctrl+C */, Readline::HandleDisabledSignal) == SIG_ERR ||
        signal(SIGTSTP /* Ctrl+Z */, Readline::HandleDisabledSignal) == SIG_ERR ||
        signal(SIGQUIT /* Ctrl+\ */, Readline::HandleDisabledSignal) == SIG_ERR)
    {
        cout << "failed to register interrupts with kernel." << endl;
        return;
    }

//    list<const char*> functions =
//    { };
//    DebugFlagHelper debugFlagHelper(functions);

    while (running)
    {
        char *line = readline("xxx: ");
        if (line == nullptr)
        {
            // Ctrl-D, Ctrl-D means EOF.
            continue;
        }
        /* Remove leading and trailing whitespace from the line.
        Then, if there is anything left, add it to the history list
        and execute it. */
        char *cmd = Strip(line);
        if (strlen(cmd) != 0)
        {
            ExecuteCommand(cmd);
            add_history (cmd);
        }
        free(line);
    }
}

char** Readline::CompleteHandler(const char *text, int start, int end)
{
    return Readline::GetInstance().CompleteHandlerImpl(text, start, end);
}

char* Readline::CommandGenerator(const char *text, int state)
{
    return Readline::GetInstance().CommandGeneratorImpl(text, state);
}

/* private functions */
void Readline::ExecuteCommand(const char *cmd)
{
    cout << "your entered: " << cmd << endl;

    CmdNode *node = root;
    const char *ptr = cmd;

    auto iter = node->childs.begin();
    while (iter != node->childs.end())
    {
        const string& name = (*iter)->name;
        if (name.compare(0, name.size(), ptr, name.size()) != 0)
        {
            ++iter;
            continue;
        }

        ptr = ptr + name.size();
        while (whitespace(*ptr) && ptr < cmd + strlen(cmd))
        {
            ++ptr;
        }

        node = (*iter);
        iter = node->childs.begin();

        if (ptr == cmd + strlen(cmd))
            break;
    }

    node->handler(ptr);
}

char** Readline::CompleteHandlerImpl(const char *text, int start, int end)
{
    char **matches = (char **)NULL;
    matches = rl_completion_matches (text, Readline::CommandGenerator);

    return matches;
}

char* Readline::CommandGeneratorImpl(const char *text, int state)
{
    CmdNode *node = root;
    string word;
    for (char *ptr = rl_line_buffer; ptr < rl_line_buffer + strlen(rl_line_buffer); ++ptr)
    {
        if (!whitespace(*ptr))
        {
            word.append(ptr, ptr+1);
            continue;
        }

        if (word.size() != 0)
        {
            std::list<CmdNode*>::iterator iter;
            for (iter = node->childs.begin(); iter != node->childs.end(); ++iter)
            {
                if (word.compare((*iter)->name) == 0)
                {
                    break;
                }
            }
            if (iter != node->childs.end())
            {
                node = *iter;
                continue;
            }
            return ((char *)NULL);
        }
        word = "";
    }

    std::list<CmdNode*>::iterator iter = node->childs.begin();
    for (int i = 0; i < state; ++i)
    {
        while (iter != node->childs.end())
        {
            if (strncmp(text, (*iter)->name.c_str(), strlen(text)) == 0)
            {
                ++iter;
                break;
            }
            ++iter;
        }
    }

    for ( ; iter != node->childs.end(); ++iter)
    {
        if (strncmp(text, (*iter)->name.c_str(), strlen(text)) ==0)
            return Duplicate((*iter)->name.c_str());
    }

    /* If no names matched, then return NULL. */
    return ((char *)NULL);
}

char* Readline::Strip(char *str)
{
    register char *s, *t;
    /* handle prefix whitespace */
    for (s = str; whitespace(*s); s++)
        ;

    if (*s == '\0')
        return (s);

    t = s + strlen (s) - 1;
    while (t > s && whitespace (*t))
        t--;
    *++t = '\0';
    return s;
}

char* Readline::Duplicate(const char *str)
{
    char *result = (char*)malloc(strlen(str));
    strcpy(result, str);
    return result;
}

void Readline::HandleDisabledSignal(int status)
{
    cout << endl; // Move to a new line
    rl_on_new_line(); // Regenerate the prompt on a newline
    rl_replace_line("", 0); // Clear the previous text
    rl_redisplay();
}

void Readline::SetDebugFlag(const char *arg)
{
    cout << "hello" << endl;
//    DebugFlag& debugFlag =  DebugFlag::GetInstance();
//    debugFlag.SetState(arg, true);
}

void Readline::UnsetDebugFlag(const char *arg)
{
    cout << "hello" << endl;
//    dbgstrm << "Start, arg = " << arg << endl;
//
//    DebugFlag& debugFlag =  DebugFlag::GetInstance();
//    debugFlag.SetState(arg, false);
}

void Readline::ShowStatus(const char *arg)
{
    cout << "hello" << endl;
//    ControllerInterface& controller = GetControllerInterface();
//    controller.PrintRuntimeStatus(cout);
}

void Readline::StartUnitTest(const char *arg)
{
    cout << "hello" << endl;
//    shared_ptr<odb::database> db = Database::GetInstance().GetDatabase();
//    shared_ptr<odb::session> session = Database::GetInstance().GetSession();
//
//    StartTransaction(db, session, MaxTransactionRetryNumber);
//    MarketResult result(db->query<MarketEntity>());
//    EndTransaction();
}

void Readline::StartRobot(const char *arg)
{
    cout << "hello" << endl;
//    std::list<std::shared_ptr<RobotEntity>> robotEnts;
//    shared_ptr<odb::database> db = Database::GetInstance().GetDatabase();
//    shared_ptr<odb::session> session = Database::GetInstance().GetSession();
//
//    StartTransaction(db, session, MaxTransactionRetryNumber);
//    RobotResult robotResult(db->query<RobotEntity>());
//    for (auto i = robotResult.begin(); i != robotResult.end(); ++i)
//    {
//        auto robotEnt = i.load();
//        db->reload(robotEnt);
//
//        robotEnts.push_back(robotEnt);
//    }
//    EndTransaction();
//
//    ControllerInterface& controller = GetControllerInterface();
//    for (auto i = robotEnts.begin(); i != robotEnts.end(); ++i)
//    {
//        controller.Start(*i);
//    }
}

void Readline::StopRobot(const char *arg)
{
    cout << "hello" << endl;
//    ControllerInterface& controller = GetControllerInterface();
//    controller.Stop();
}

void Readline::Exit(const char *arg)
{
//    ControllerInterface& controller = GetControllerInterface();
//    controller.Stop();
    running = false;
}

