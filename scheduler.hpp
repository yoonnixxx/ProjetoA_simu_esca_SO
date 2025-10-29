#pragma once
#include "process.hpp"
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>

// ======================= Função para desenhar linha do tempo =======================
inline void draw_timeline(
    const std::vector<std::string>& exec_names,
    const std::vector<int>& start_times,
    const std::vector<int>& end_times,
    const std::vector<Process>& processes = {}
) {
    std::cout << "\n=== Linha do Tempo ===\n";

    auto hex_to_ansi = [](const std::string& hex) -> std::string {
        if (hex.size() != 7 || hex[0] != '#') return "\033[0m";
        int r = std::stoi(hex.substr(1, 2), nullptr, 16);
        int g = std::stoi(hex.substr(3, 2), nullptr, 16);
        int b = std::stoi(hex.substr(5, 2), nullptr, 16);
        return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
    };

    std::unordered_map<std::string, std::string> color_map;
    for (const auto& p : processes)
        color_map[p.name] = hex_to_ansi(p.color);


    // === Linha detalhada com tempos ===
    for (size_t i = 0; i < exec_names.size(); ++i) {
        std::string name = exec_names[i];
        int start = start_times[i];
        int end = end_times[i];
        std::string color = color_map.count(name) ? color_map[name] : "\033[0m";
        std::cout << color << "Tempo " << start << " - " << end << " -> " << name << "\033[0m\n";
    }

    // === Linha compacta (CPU) ===
    std::cout << "\nCPU: ";
    for (const auto& name : exec_names) {
        std::string color = color_map.count(name) ? color_map[name] : "\033[0m";
        std::cout << color << "[" << name << "]" << "\033[0m";
    }
    std::cout << "\n";
}

// ======================= FIFO (FCFS) =======================
inline void simulate_fifo(std::vector<Process> processes) {
    std::cout << "\n=== Simulacao FIFO ===\n";

    struct Item {
        Process p;
        int original_idx; // para desempate estável quando arrival_time empata
    };

    std::vector<Item> items;
    items.reserve(processes.size());
    for (int i = 0; i < (int)processes.size(); ++i)
        items.push_back({processes[i], i});

    // Ordena por chegada e mantém a ordem original quando empata (estável)
    std::stable_sort(items.begin(), items.end(),
        [](const Item& a, const Item& b) {
            return a.p.arrival_time < b.p.arrival_time;
        });

    int current_time = 0;
    double total_wait = 0.0, total_turn = 0.0;

    for (const auto& it : items) {
        const auto& p = it.p;

        int start  = std::max(current_time, p.arrival_time);
        int finish = start + p.duration;
        int wait   = start - p.arrival_time;
        int turn   = finish - p.arrival_time;

        std::cout << "Processo " << p.name
                  << " | Inicio: " << start
                  << " | Fim: "    << finish
                  << " | Espera: " << wait
                  << " | Tempo de vida: " << turn << "\n";

        total_wait += wait;
        total_turn += turn;
        current_time = finish; // CPU segue direto para o próximo
    }

    int n = (int)items.size();
    std::cout << "\nMedia de espera: " << (total_wait / n)
              << "\nMedia de tempo de vida: " << (total_turn / n) << "\n";
}

// ======================= SRTF (Shortest Remaining Time First) =======================
inline void simulate_srtf(std::vector<Process> processes) {
    std::cout << "\n=== Simulacao SRTF ===\n";

    int n = processes.size();
    std::vector<int> remaining_time(n);
    for (int i = 0; i < n; ++i)
        remaining_time[i] = processes[i].duration;

    int complete = 0, current_time = 0, shortest = 0, finish_time;
    bool found = false;
    int total_wait = 0, total_lifetime = 0;

    std::vector<std::string> exec_names;
    std::vector<int> start_times;
    std::vector<int> end_times;

    while (complete != n) {
        int minm = INT_MAX;
        found = false;

        for (int j = 0; j < n; ++j) {
            if ((processes[j].arrival_time <= current_time) &&
                (remaining_time[j] > 0) &&
                (remaining_time[j] < minm)) {
                minm = remaining_time[j];
                shortest = j;
                found = true;
            }
        }

        if (!found) {
            current_time++;
            continue;
        }

        // Executa processo por 1 unidade de tempo
        exec_names.push_back(processes[shortest].name);
        start_times.push_back(current_time);

        remaining_time[shortest]--;
        current_time++;

        end_times.push_back(current_time);

        if (remaining_time[shortest] == 0) {
            complete++;
            finish_time = current_time;
            int wait = finish_time - processes[shortest].duration - processes[shortest].arrival_time;
            if (wait < 0) wait = 0;
            total_wait += wait;
            total_lifetime += finish_time - processes[shortest].arrival_time;
        }
    }

    std::cout << "Media de espera: " << (float)total_wait / n << "\n";
    std::cout << "Media de tempo de vida: " << (float)total_lifetime / n << "\n";

    draw_timeline(exec_names, start_times, end_times);
}

// ======================= Prioridade Preemptiva (sem quantum) =======================
// Regra: números MAIORES = maior prioridade.
// Preempção somente quando um processo com prioridade MAIOR fica disponível.
// (Se quiser um quantum no futuro, dá pra estender facilmente.)
inline void simulate_priority_preemptive(std::vector<Process> processes, int /*quantum_ignored*/) {
    std::cout << "\n=== Simulacao Prioridade Preemptiva (sem quantum) ===\n";

    const int n = (int)processes.size();
    std::vector<int> remaining(n);
    for (int i = 0; i < n; ++i) remaining[i] = processes[i].duration;

    int finished = 0;
    int t = 0;
    int current = -1; // índice do processo rodando
    double total_wait = 0.0, total_turn = 0.0;

    std::vector<std::string> exec_names;
    std::vector<int> start_times, end_times;

    auto best_ready_index = [&](int now) -> int {
        int idx = -1;
        int best_pr = INT_MIN; // maior número = maior prioridade
        for (int i = 0; i < n; ++i) {
            if (processes[i].arrival_time <= now && remaining[i] > 0) {
                if (processes[i].priority > best_pr) {
                    best_pr = processes[i].priority;
                    idx = i;
                } else if (processes[i].priority == best_pr) {
                    // desempate: menor tempo de chegada; persistir ordem caso necessário
                    if (idx == -1 || processes[i].arrival_time < processes[idx].arrival_time) {
                        idx = i;
                    }
                }
            }
        }
        return idx;
    };

    while (finished < n) {
        // Quem está disponível agora com maior prioridade?
        int best = best_ready_index(t);

        // Se não há nada pronto, CPU ociosa
        if (best == -1) {
            t++;
            continue;
        }

        // Política de preempção: troca somente se o "best" tem prioridade > atual
        if (current == -1 || remaining[current] == 0 ||
            processes[best].priority > processes[current].priority) {
            current = best;
        }

        // Executa 1 unidade de tempo do processo atual
        exec_names.push_back(processes[current].name);
        start_times.push_back(t);

        remaining[current]--;
        t++;

        end_times.push_back(t);

        if (remaining[current] == 0) {
            finished++;
            int finish = t;
            int wait = finish - processes[current].arrival_time - processes[current].duration;
            if (wait < 0) wait = 0;
            total_wait += wait;
            total_turn += finish - processes[current].arrival_time;

            current = -1; // libera CPU; no próximo ciclo escolhemos de novo
        }
    }

    std::cout << "Media de espera: " << (total_wait / n) << "\n";
    std::cout << "Media de tempo de vida: " << (total_turn / n) << "\n";

    draw_timeline(exec_names, start_times, end_times, processes);
}


