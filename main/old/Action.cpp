#include <map>

#include <Arduino.h>

#include "Action.h"
#include "Support.h"

using namespace std;

namespace
{
  // create a list of string to action mappings,
  // with partial matches generated
  // for instance, it fill have valid entries for h,he,hel,help, 
  // but since on and off both have a partial map on o, o will not be valid
  // internal actions have no partian mapping
  const std::map<string, Action> & GetActions()
  {
    static std::map<string, Action> res;
    if(res.empty())
    {
      // add all possible partial strings. 
      // if a partial is added which alreday exists, set the action to 'error'
      auto AddMatches = [&](const string & command, Action action, bool createPartials)
      {
        for(size_t i=createPartials?1:command.size(); i<=command.size(); ++i)
        {
          auto status = res.emplace(command.substr(0,i), action);
          if(!status.second)
          {
            status.first->second = Action::Error;
          }
        }
      };
      // default
      AddMatches("none",   Action::None,    false);
      // user actions
      AddMatches("?",      Action::Help,    false);
      AddMatches("help",   Action::Help,    true);
      AddMatches("node",   Action::Node,    true);
      AddMatches("+",      Action::On,      false);
      AddMatches("on",     Action::On,      true);
      AddMatches("-",      Action::Off,     false);
      AddMatches("off",    Action::Off,     true);
      AddMatches("^",      Action::Toggle,  false);
      AddMatches("toggle", Action::Toggle,  true);
      AddMatches("status", Action::Status,  true);
      AddMatches("reboot", Action::Reboot,  true);
      // internal actions
      AddMatches("ping",   Action::Ping,    false);
      AddMatches("pong",   Action::Pong,    false);
      // remove all duplicate partial matches, and add 'error'
      auto iterNext = res.begin();
      while(iterNext != res.end())
      {
        auto iterRemove = iterNext++; 
        if (iterRemove->second == Action::Error)
        {
          res.erase(iterRemove);
        }
      }
      AddMatches("error",  Action::Error,   false);
    }
    return res;
  }
}

Action ParseAction(const string & text)
{
  auto actions = GetActions();
  auto iter = actions.find(ToLower(text));
  return (iter==actions.end()) ? Action::Error : iter->second;
}

std::string ToString(const Action action)
{
  static std::map<Action, string> representation;
  if(representation.empty())
  {
    auto actions = GetActions();
    for(const auto & kv: actions)
    {
      auto status = representation.emplace(kv.second,kv.first);
      if(!status.second && kv.first.size()<status.first->second.size())
      {
        status.first->second = kv.first;
      }
    }
  }
  auto resIter = representation.find(action);
  return resIter == representation.end() ? "missing" : resIter->second;
}

