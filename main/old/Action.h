#pragma once

#include <string>

// Action is an 8 bit enum, starting with values 'A'.
enum class Action : uint8_t
{
  // default
  None,         // do nothing 
  // user input actions
  Help,         // output help message
  Node,         // select node for command
  Off,          // turn board led off
  On,           // turn board led on
  Reboot,       // reboot the board
  Status,       // outboard board status
  Text,         // output text message 
  Toggle,       // toggle board led
  // internal actions
  Error,        // invalid action (could not be parsed, fi due to duplicate partial match)
  Ping,         // request Pong for live nodes
  Pong,         // return action for Ping
};

Action ParseAction(const std::string &text);
std::string ToString(const Action action);
