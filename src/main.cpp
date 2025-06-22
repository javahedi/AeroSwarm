#include <iostream> // For input/output operations
#include <stdexcept> // For exception handling, like std::runtime_error

#include "run.hpp" // Include the run function header



int main() {
    std::cout << "Program started!\n";  // DEBUG
    try {
        run_drone_simulation(); // Run the drone simulation
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl; // Handle any exceptions
    }
    return 0; // Exit the program
}
// End of main.cpp

