#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include <limits>   // <- p/ numeric_limits
#include "process.hpp"
#include "scheduler.hpp"

// utilidades simples
static std::string to_upper(std::string s) { for (auto &c : s) c = std::toupper(c); return s; }
static std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, b - a + 1);
}

// carrega config: 1a linha "ALGO;quantum", demais: "id;cor;ingresso;duracao;prioridade;eventos"
static bool load_config(const std::string &filename, std::string &alg, int &quantum,
                        std::vector<Process> &processes) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro: não foi possível abrir " << filename << "\n";
        return false;
    }

    std::string line; int line_num = 0;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string token; std::vector<std::string> tks;
        while (std::getline(ss, token, ';')) tks.push_back(trim(token));

        if (line_num == 0) {
            alg = tks.size() >= 1 ? to_upper(tks[0]) : "FIFO";
            if (tks.size() >= 2) { try { quantum = std::stoi(tks[1]); } catch (...) { quantum = 1; } }
            else quantum = 1;
        } else {
            Process p;
            p.id     = tks.size() > 0 && !tks[0].empty() ? tks[0] : ("T" + std::to_string(line_num));
            p.color  = tks.size() > 1 && !tks[1].empty() ? tks[1] : "#cccccc";
            try { p.arrival_time = tks.size() > 2 ? std::stoi(tks[2]) : 0; } catch (...) { p.arrival_time = 0; }
            try { p.duration     = tks.size() > 3 ? std::stoi(tks[3]) : 1; } catch (...) { p.duration     = 1; }
            try { p.priority     = tks.size() > 4 ? std::stoi(tks[4]) : 1; } catch (...) { p.priority     = 1; }
            p.events = tks.size() > 5 ? tks[5] : "-";
            processes.push_back(p);
        }
        ++line_num;
    }
    return true;
}

int main(int argc, char* argv[]) {
    std::string configPath = "config_exemplo.txt";
    std::string mode; // "step" | "run"
    bool modo_passos = false;

    // CLI
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "--config" || a == "-c") && i + 1 < argc) configPath = argv[++i];
        else if ((a == "--mode"   || a == "-m") && i + 1 < argc) mode = argv[++i];
        else if (a == "--help" || a == "-h") {
            std::cout << "Uso: simulador --config <arquivo> --mode step|run\n"
                      << "Algoritmos: FIFO, SRTF, PRIORIDADE\n"
                      << "Obs.: quantum e' ignorado por FIFO/SRTF.\n";
            return 0;
        }
    }

    std::string algoritmo; int quantum = 1; std::vector<Process> processes;
    if (!load_config(configPath, algoritmo, quantum, processes)) return 1;

    // modo (menu como fallback)
    if (mode.empty()) {
        std::cout << "Escolha o modo de execução:\n1. Passo a Passo\n2. Completo\n> ";
        int escolha; std::cin >> escolha;

        // drena o '\n' do >> para o primeiro getline do step não pular
if (std::cin.peek() == '\n')
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        modo_passos = (escolha == 1);
    } else {
        modo_passos = (to_upper(mode) == "STEP");
    }

    // resumo
    show_processes(processes);
    std::cout << "\nAlgoritmo: " << algoritmo
              << " | Quantum: " << quantum
              << " | Modo: " << (modo_passos ? "Passo-a-Passo" : "Completo") << "\n";

    // despacha algoritmo
    if (algoritmo == "FIFO") {
        simulate_fifo(processes, modo_passos);
    } else if (algoritmo == "SRTF") {
        simulate_srtf(processes, modo_passos);
    } else if (algoritmo == "PRIORIDADE" || algoritmo == "PRIO" || algoritmo == "PRIOP") {
        simulate_prio_preemptivo(processes, modo_passos, quantum);
    } else {
        std::cerr << "Algoritmo '" << algoritmo << "' não reconhecido. Use FIFO, SRTF ou PRIORIDADE.\n";
        return 1;
    }

    // Gantt
    std::cout << "\nGerando gráfico de Gantt em ./output/gantt.svg ...\n";
    generate_gantt(processes, "output/gantt.svg");
    std::cout << "Concluído.\n";
    return 0;
}
