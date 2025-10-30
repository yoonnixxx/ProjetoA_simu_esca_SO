#include "scheduler.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>

// =====================================================
// Funções auxiliares
// =====================================================

// Retorna verdadeiro se todos os processos foram concluídos
bool all_completed(const std::vector<Process>& processes) {
    for (const auto& p : processes) if (!p.completed) return false;
    return true;
}

// Imprime o estado atual da simulação
void print_status(int tick, const std::string& running_id, const std::vector<Process*>& ready) {
    std::cout << "Tick " << std::setw(3) << tick << " | Rodando: "
              << (running_id.empty() ? "-" : running_id) << " | Fila: ";
    for (const auto* p : ready) std::cout << p->id << " ";
    std::cout << "\n";
}

// =====================================================
// FIFO (First In First Out)
// =====================================================
void simulate_fifo(std::vector<Process>& processes, bool passo_a_passo) {
    std::cout << "Executando algoritmo FIFO...\n";

    int tick = 0;
    std::vector<Process*> ready;

    for (auto& p : processes) p.remaining_time = p.duration;

    while (!all_completed(processes)) {
        // Adiciona novos processos prontos
        for (auto& p : processes)
            if (!p.completed && p.arrival_time == tick)
                ready.push_back(&p);

        if (!ready.empty()) {
            Process* current = ready.front();
            if (current->start_time == -1) current->start_time = tick;

            current->remaining_time--;
            if (current->slices.empty() || current->slices.back().second != -1)
                current->slices.push_back({tick, -1});

            if (current->remaining_time == 0) {
                current->completed = true;
                current->finish_time = tick + 1;
                current->slices.back().second = tick + 1;
                ready.erase(ready.begin());
            }

            if (passo_a_passo) {
                print_status(tick, current->id, {});
                std::cin.get();
            }
        } else if (passo_a_passo) {
            print_status(tick, "-", {});
            std::cin.get();
        }
        tick++;
    }
}

// =====================================================
// SRTF (Shortest Remaining Time First)
// =====================================================
void simulate_srtf(std::vector<Process>& processes, bool passo_a_passo) {
    std::cout << "Executando algoritmo SRTF...\n";

    for (auto& p : processes) p.remaining_time = p.duration;
    int tick = 0;
    Process* current = nullptr;

    while (!all_completed(processes)) {
        // Atualiza prontos
        std::vector<Process*> ready;
        for (auto& p : processes)
            if (!p.completed && p.arrival_time <= tick)
                ready.push_back(&p);

        // Escolhe o menor tempo restante
        if (!ready.empty()) {
            auto cmp = [](Process* a, Process* b) {
                return a->remaining_time < b->remaining_time ||
                       (a->remaining_time == b->remaining_time && a->arrival_time < b->arrival_time);
            };
            current = *std::min_element(ready.begin(), ready.end(), cmp);
        } else current = nullptr;

        if (current) {
            if (current->start_time == -1) current->start_time = tick;
            if (current->slices.empty() || current->slices.back().second != -1)
                current->slices.push_back({tick, -1});

            current->remaining_time--;
            if (current->remaining_time == 0) {
                current->completed = true;
                current->finish_time = tick + 1;
                current->slices.back().second = tick + 1;
            }
        }

        if (passo_a_passo) {
            std::vector<Process*> fila = ready;
            std::sort(fila.begin(), fila.end(), [](Process* a, Process* b) {
                return a->remaining_time < b->remaining_time;
            });
            print_status(tick, current ? current->id : "-", fila);
            std::cin.get();
        }
        tick++;
    }
}

// =====================================================
// Prioridade Preemptivo
// =====================================================
void simulate_prio_preemptivo(std::vector<Process>& processes, bool passo_a_passo, int quantum) {
    std::cout << "Executando algoritmo de Prioridade Preemptivo...\n";

    for (auto& p : processes) p.remaining_time = p.duration;
    int tick = 0;
    int quantum_counter = 0;
    Process* current = nullptr;

    while (!all_completed(processes)) {
        std::vector<Process*> ready;
        for (auto& p : processes)
            if (!p.completed && p.arrival_time <= tick)
                ready.push_back(&p);

        // Seleciona a maior prioridade (ou menor número, se preferir inverter)
        if (!ready.empty()) {
            auto cmp = [](Process* a, Process* b) {
                return a->priority > b->priority ||
                       (a->priority == b->priority && a->arrival_time < b->arrival_time);
            };
            Process* top = *std::max_element(ready.begin(), ready.end(), cmp);

            // Preempção se mudou o processo ou quantum esgotou
            if (current != top || (quantum > 0 && quantum_counter >= quantum)) {
                if (current && !current->completed && !current->slices.empty() &&
                    current->slices.back().second == -1)
                    current->slices.back().second = tick;

                current = top;
                quantum_counter = 0;
                if (current->slices.empty() || current->slices.back().second != -1)
                    current->slices.push_back({tick, -1});
                if (current->start_time == -1) current->start_time = tick;
            }
        }

        if (current) {
            current->remaining_time--;
            quantum_counter++;
            if (current->remaining_time == 0) {
                current->completed = true;
                current->finish_time = tick + 1;
                current->slices.back().second = tick + 1;
                quantum_counter = 0;
            }
        }

        if (passo_a_passo) {
            print_status(tick, current ? current->id : "-", ready);
            std::cin.get();
        }
        tick++;
    }
}

// =====================================================
// Geração do gráfico de Gantt (SVG simples)
// =====================================================
void generate_gantt(const std::vector<Process>& processes, const std::string& filename) {
    std::ofstream svg(filename);
    if (!svg.is_open()) {
        std::cerr << "Erro: não foi possível criar o arquivo " << filename << "\n";
        return;
    }

    int width_per_tick = 20;
    int height = 40 * processes.size() + 50;
    int width = 0;
    for (const auto& p : processes)
        for (const auto& s : p.slices)
            if (s.second > width / width_per_tick)
                width = s.second * width_per_tick + 50;

    svg << "<svg xmlns='http://www.w3.org/2000/svg' width='" << width
        << "' height='" << height << "'>\n";
    svg << "<style> text{font-family:monospace;font-size:12px;} </style>\n";

    int y = 30;
    for (const auto& p : processes) {
        for (const auto& slice : p.slices) {
            int x = 50 + slice.first * width_per_tick;
            int w = (slice.second - slice.first) * width_per_tick;
            svg << "<rect x='" << x << "' y='" << y << "' width='" << w
                << "' height='20' fill='" << p.color << "' stroke='black'/>\n";
        }
        svg << "<text x='10' y='" << y + 15 << "'>" << p.id << "</text>\n";
        y += 40;
    }

    // Eixo do tempo
    svg << "<line x1='50' y1='" << y << "' x2='" << width - 20 << "' y2='" << y
        << "' stroke='black'/>\n";
    for (int i = 0; i <= (width - 70) / width_per_tick; i++) {
        int x = 50 + i * width_per_tick;
        svg << "<line x1='" << x << "' y1='" << y << "' x2='" << x
            << "' y2='" << y + 5 << "' stroke='black'/>\n";
        svg << "<text x='" << x - 3 << "' y='" << y + 20 << "'>" << i << "</text>\n";
    }

    svg << "</svg>\n";
    svg.close();
    std::cout << "Gráfico salvo em: " << filename << "\n";
}
