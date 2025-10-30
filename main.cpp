#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include "process.hpp"
#include "scheduler.hpp"

// =====================================================
// Função auxiliar: converte string para upper
// =====================================================
std::string to_upper(std::string s) {
    for (auto &c : s) c = std::toupper(c);
    return s;
}

// =====================================================
// Função auxiliar: trim
// =====================================================
std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

// =====================================================
// Leitura do arquivo de configuração
// =====================================================
bool load_config(const std::string &filename, std::string &algoritmo, int &quantum,
                 std::vector<Process> &processes) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro: não foi possível abrir o arquivo " << filename << std::endl;
        return false;
    }

    std::string line;
    int line_num = 0;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue; // ignora comentários e linhas vazias

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        while (std::getline(ss, token, ';')) tokens.push_back(trim(token));

        if (line_num == 0) {
            // Primeira linha: algoritmo;quantum
            if (tokens.size() >= 1) algoritmo = to_upper(tokens[0]);
            if (tokens.size() >= 2) {
                try { quantum = std::stoi(tokens[1]); }
                catch (...) { quantum = 1; }
            } else quantum = 1;
        } else {
            // Demais linhas: id;cor;ingresso;duracao;prioridade;eventos
            Process p;
            try {
                p.id = tokens.size() > 0 ? tokens[0] : "T" + std::to_string(line_num);
                p.color = tokens.size() > 1 && !tokens[1].empty() ? tokens[1] : "#CCCCCC";
                p.arrival_time = tokens.size() > 2 ? std::stoi(tokens[2]) : 0;
                p.duration = tokens.size() > 3 ? std::stoi(tokens[3]) : 1;
                p.priority = tokens.size() > 4 ? std::stoi(tokens[4]) : 1;
                p.events = tokens.size() > 5 ? tokens[5] : "-";
            } catch (...) {
                std::cerr << "Aviso: erro ao ler linha " << line_num + 1
                          << ". Usando valores padrão." << std::endl;
            }
            processes.push_back(p);
        }
        ++line_num;
    }
    return true;
}

// =====================================================
// Função principal
// =====================================================
int main(int argc, char* argv[]) {
    std::string configPath = "config_exemplo.txt";
    std::string mode = ""; // "step" ou "run"
    bool modo_passos = false;

    // -------------------------------------------------
    // Leitura dos argumentos de linha de comando
    // -------------------------------------------------
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "--config" || a == "-c") && i + 1 < argc) configPath = argv[++i];
        else if ((a == "--mode" || a == "-m") && i + 1 < argc) mode = argv[++i];
        else if (a == "--help" || a == "-h") {
            std::cout << "Uso: simulador --config <arquivo> --mode step|run\n"
                      << "Algoritmos disponíveis: FIFO, SRTF, PRIORIDADE\n"
                      << "Observação: quantum é ignorado por FIFO/SRTF.\n";
            return 0;
        }
    }

    std::string algoritmo;
    int quantum = 1;
    std::vector<Process> processes;

    if (!load_config(configPath, algoritmo, quantum, processes)) {
        std::cerr << "Falha ao carregar a configuração.\n";
        return 1;
    }

    // -------------------------------------------------
    // Definição do modo de execução
    // -------------------------------------------------
    if (mode.empty()) {
        std::cout << "Escolha o modo de execução:\n"
                  << "1. Passo a Passo\n2. Completo\n> ";
        int escolha;
        std::cin >> escolha;
        modo_passos = (escolha == 1);
    } else {
        std::string up = to_upper(mode);
        modo_passos = (up == "STEP");
    }

    std::cout << "\n=== Simulador de Escalonamento ===\n"
              << "Arquivo: " << configPath
              << "\nAlgoritmo: " << algoritmo
              << "\nQuantum: " << quantum
              << "\nModo: " << (modo_passos ? "Passo-a-Passo" : "Completo") << "\n\n";

    // -------------------------------------------------
    // Seleção do algoritmo
    // -------------------------------------------------
    if (algoritmo == "FIFO") {
        simulate_fifo(processes, modo_passos);
    } else if (algoritmo == "SRTF") {
        simulate_srtf(processes, modo_passos);
    } else if (algoritmo == "PRIORIDADE" || algoritmo == "PRIOP" || algoritmo == "PRIO") {
        simulate_prio_preemptivo(processes, modo_passos, quantum);
    } else {
        std::cerr << "Erro: algoritmo '" << algoritmo
                  << "' não reconhecido.\nUse FIFO, SRTF ou PRIORIDADE.\n";
        return 1;
    }

    // -------------------------------------------------
    // Exportação do gráfico de Gantt ao final
    // -------------------------------------------------
    std::cout << "\nGerando gráfico de Gantt em ./output/gantt.svg ...\n";
    generate_gantt(processes, "output/gantt.svg");
    std::cout << "Simulação concluída.\n";

    return 0;
}
