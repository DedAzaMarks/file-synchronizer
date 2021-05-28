#include <iostream>
#include <chrono>
#include <string>
#include <string_view>

class TimerGuard {
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
    std::string message_;
    std::ostream& out_;
public:

    TimerGuard(std::string_view message = "", std::ostream& out = std::cout)
    : start_(std::chrono::high_resolution_clock::now())
    , message_(message)
    , out_(out)
    {
    }

    ~TimerGuard() {
        auto current = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = current - start_;
        out_ << message_ << " " << diff.count() << "\n";
    }
};
