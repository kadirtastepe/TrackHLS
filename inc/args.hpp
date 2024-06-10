// Standard includes
#include <iostream>
#include <vector>
#include <string>

// CLI includes
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

class Args : public CLI::App {

  public:

    // Call base class constructor
    Args(int argc, char ** argv, std::string title) : CLI::App(title)
    {
      this->add_option
      (
        "--treename",
        this->treename,
        "Name of the tree."
      );
      this->add_option
      (
        "--xclbin",
        this->fXCLBinFile,
        "Path to the .xclbin file"
      );
      // Parse options
      this->parse(argc, argv);

      // Run some checks
      bool valid = this->check();

      // Print config
      this->print();
    };

    // Print variables
    void print ();

  private:

    bool check ();

  public:

    // Variables/arguments
    std::vector<std::string> fNamesIn;
    std::string fXCLBinFile = "";
    std::string treename  = "";
};


bool Args::check () {
  return true;
}

void Args::print () {
  std::cout << "[INFO] Commandline arguments:" << std::endl;
  std::cout << "[    ] XCL bibnary file" << this->fXCLBinFile << std::endl;
  std::cout << "[    ] Input files" << std::endl;
  for (auto & x : this->fNamesIn) {std::cout<< "       |- " << x << std::endl;}
}
