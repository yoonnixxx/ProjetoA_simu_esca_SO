#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <iostream>
#include <vector>
#include <string>

struct Process {
    std::string name;
    std::string color;
    int arrival_time;
    int duration;
    int priority;
    std::string dependency;

    // 🔹 Construtor padrão
    Process() = default;

    // 🔹 Construtor com parâmetros
    Process(const std::string& n, const std::string& c,
            int arr, int dur, int pri, const std::string& dep)
        : name(n), color(c), arrival_time(arr),
          duration(dur), priority(pri), dependency(dep) {}
};

// 🔹 Exibe os processos carregados
inline void show_processes(const std::vector<Process>& processes) {
    std::cout << "\nProcessos carregados:\n";
    for (const auto& p : processes) {
        std::cout << " - " << p.name
                  << " | chegada: " << p.arrival_time
                  << " | duracao: " << p.duration
                  << " | prioridade: " << p.priority
                  << "\n";
    }
}

#endif
