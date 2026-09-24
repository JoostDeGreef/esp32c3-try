#include <Uptime.h> 
#include <UptimeString.h> 

#include "Action.h"
#include "CRC.h"
#include "Led.h"
#include "Output.h"
#include "Timer.h"
#include "USBSerial.h"
#include "Wireless.h"

void setup()
{
  // setup all components
  if(   !Output.Setup()
     || !USBSerial.Setup()
     || !Timer.Setup()
     || !OnboardLed.Setup()
     || !Wireless.Setup())
  {
    Output("General setup failed. This is a catostrophy! Rebooting in 1 second." CRLF);
    delay(1000);
    ESP.restart();
  }
  // Install the uptime timer
  Timer.AddEvent(Uptime::calculateUptime, 1000);
  // Show we are alive by switching the led on and sending a message
  OnboardLed.On();
  Output("Setup done. Ready for your command. Control me Miraculous one." CRLF);
}

// return the help string
std::string help()
{
  return
    "Syntax: action (data) with:" CRLF
    "  action : one of the following actions:" CRLF
    "    help,?   : show this help" CRLF
    "    on,+     : turn led on" CRLF
    "    off,-    : turn led off" CRLF
    "    node <id>: Select node <id> to send rest of input to" CRLF
    "    toggle,^ : toggle led status" CRLF
    "    status   : show some status" CRLF
    "    reboot   : reboot the node" CRLF
    " Note: unique partial matches are accepted, for instance 'tog -> toggle'" CRLF;
}

// report a status string
std::string status()
{
  return
      Format("----" CRLF)
    + Format(" Up: %s" CRLF,UptimeString::getUptime4())
    + Format("Led: %s" CRLF,OnboardLed.State()?"On":"Off")
    + Format("Peers:" CRLF)
    + Wireless.ListPeerStatus()
    + Wireless.GetESPInfo()
    + Format("----" CRLF);
}

std::string ForwardInput(std::string & input)
{
  auto nodeStr = GetFirstPart(input);
  if(nodeStr.empty())
  {
    return Format("No node id specified" CRLF);
  }
  MAC node(nodeStr);
  if(!node.IsValid())
  {
    return Format("invalid node id \"%s\" specified" CRLF, nodeStr);
  }
  Wireless.SendMessage(node, input);
  return std::string();
}

void ProcessInput(const MAC & source, std::string & input)
{
  auto actionStr = GetFirstPart(input);
  auto action = ParseAction(actionStr);
  auto ShowResult = [&](const std::string & res)
  {
    if(!res.empty())
    {
      if(source.IsValid())
      {
        Wireless.SendActionData(source, Action::Text, res);
      }
      else
      {
        Output(res);
      }
    }
  };
  switch(action)
  {
    case Action::None:
      delay(50);
      break;
    case Action::Help:
      ShowResult(help());
      break;
    case Action::Status:
      ShowResult(status());
      break;
    case Action::Text:
      Output(input);
      input.clear();
      break;
    case Action::Error:
      Output(input);
      input.clear();
      break;
    case Action::Reboot:
      ShowResult("Rebooting..." CRLF);
      delay(150);
      ESP.restart();
      break;
    case Action::On:
      ShowResult("Switching led on" CRLF);
      OnboardLed.On();
      break;
    case Action::Off:
      ShowResult("Switching led off" CRLF);
      OnboardLed.Off();
      break;
    case Action::Toggle:
      ShowResult("Toggling led state" CRLF);
      OnboardLed.Toggle();
      break;
    case Action::Node:
      ShowResult(ForwardInput(input));
      input.clear();
      break;
    case Action::Ping:
      //Debug("Handling ping" CRLF);
      if( source.IsValid() )
      {
        Wireless.SendAction(source, Action::Pong);
      }
      break;
    case Action::Pong:
      //Debug("Handling pong" CRLF);
      break;
    default:
      Debug("Unhandled action");
      input.clear();
      break;
  }
}

void loop() 
{
  static MAC local;
  // pulse events
  Timer.Tick();
  // command received over usb-serial?
  auto input = USBSerial.ReadInput();
  while(!input.empty())
  {
    ProcessInput(local, input);
  }
  // command received from remote node?
  auto [remote, remoteInput] = Wireless.ReadInput();
  while(!remoteInput.empty())
  {
    ProcessInput(remote, remoteInput);
  }
}
