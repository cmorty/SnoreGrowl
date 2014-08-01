#include <memory>
#include "growl.hpp"

int main(int argc, char **argv) {
  static_cast<void>(argc); static_cast<void>(argv); // prevent unused warnings
  std::vector<std::string> n;
  n.push_back("alice");
  n.push_back( "bob" );
  std::auto_ptr<Growl> growl(
      new Growl(GROWL_TCP,"","gntp_send++"));
  growl->Register(n);

  GrowlNotificationData data("bob", 1, "title", "message");
  growl->Notify(data);
}
