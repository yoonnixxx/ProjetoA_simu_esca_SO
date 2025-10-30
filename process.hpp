#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <vector>
#include <iostream>

struct Process {
    // Campos lidos do arquivo de configuração
    std::string id;           // ex.: "A", "B"...
    std::string color;        // ex.: "#1f77b4"
    int arrival_time = 0;     // ingresso
    int duration = 1;         // duração total
    int priority = 1;         // prioridade (1=baixa ... n=alta, ou como você definiu)
    std::string events = "-"; // lista de eventos (para Projeto A pode ficar "-")

    // Campos auxiliares de simulação
    int remaining_time = 0;   // para SRTF/execução
    int start_time = -1;      // primeiro tick de execução
    int finish_time = -1;     // término
    int waiting_time = 0;     // acumulado (se você calcular)
    bool completed = false;

    // Opcional: trilha para Gantt (pares [start, end] por “fatias” executadas)
    std::vector<std::pair<int,int>> slices;
};

// Helper opcional só para debug/estado
inline void show_processes(const std::vector<Process>& processes) {
    std::cout << "\nProcessos carregados:\n";
    for (const auto& p : processes) {
        std::cout << " - " << p.id
                  << " | Chegada: " << p.arrival_time
                  << " | Duracao: " << p.duration
                  << " | Prioridade: " << p.priority
                  << "\n";
    }
}

#endif // PROCESS_HPP
