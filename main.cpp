#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "process.hpp"
#include "scheduler.hpp"

int main() {
    std::ifstream file("config_exemplo.txt");
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de configuração!\n";
        return 1;
    }

    std::string line;
    std::getline(file, line); // Primeira linha: algoritmo e quantum
    std::string algorithm;
    int quantum = 0;

    {
        std::stringstream header(line);
        std::getline(header, algorithm, ';');
        std::string q;
        if (std::getline(header, q, ';') && !q.empty())
            quantum = std::stoi(q);
    }

    std::vector<Process> processes;

    // Ler cada linha e criar processos
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string name, color, dep;
        int arrival, duration, priority;

        std::getline(ss, name, ';');
        std::getline(ss, color, ';');
        std::string s_arr, s_dur, s_pri;
        std::getline(ss, s_arr, ';');
        std::getline(ss, s_dur, ';');
        std::getline(ss, s_pri, ';');
        std::getline(ss, dep, ';');

        arrival = std::stoi(s_arr);
        duration = std::stoi(s_dur);
        priority = std::stoi(s_pri);

        processes.emplace_back(name, color, arrival, duration, priority, dep);
    }

    file.close();

    std::cout << "Algoritmo: " << algorithm << "\nQuantum: " << quantum << "\n";
    show_processes(processes);

    if (algorithm == "FIFO") {
        simulate_fifo(processes);
    } 
    else if (algorithm == "RR" || algorithm == "RoundRobin" || algorithm == "ROUNDROBIN") {
        simulate_rr(processes, quantum);
    } 
    else {
        std::cout << "\nAlgoritmo não reconhecido.\n";
    }

    return 0;
}
