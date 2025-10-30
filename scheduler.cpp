#include "scheduler.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>   // <- ostringstream
#include <limits>

// ---------- auxiliares ----------
static bool all_completed(const std::vector<Process>& processes) {
    for (const auto& p : processes) if (!p.completed) return false;
    return true;
}

// imprime o estado + já pede Enter (uma função só)
static void show_and_wait(int tick, const std::string& running_id, const std::vector<Process*>& ready) {
    std::ostringstream os;
    os << "Tick " << std::setw(3) << tick
       << " | Rodando: " << (running_id.empty() ? "-" : running_id)
       << " | Fila: ";
    for (const auto* p : ready) os << p->id << " ";
    os << "\nPressione Enter para avançar..." << std::flush;

    std::cout << os.str();  // escreve tudo de uma vez + flush

    std::string dummy;
    if (!std::getline(std::cin, dummy)) {
        std::cin.clear(); // se stdin vier fechado/EOF, não trava
    }
}

// ---------- FIFO ----------
void simulate_fifo(std::vector<Process>& processes, bool passo_a_passo) {
    int tick = 0;
    std::vector<Process*> ready;
    for (auto& p : processes) p.remaining_time = p.duration;

    while (!all_completed(processes)) {
        for (auto& p : processes)
            if (!p.completed && p.arrival_time == tick)
                ready.push_back(&p);

        if (!ready.empty()) {
            Process* current = ready.front();
            if (current->start_time == -1) current->start_time = tick;

            if (current->slices.empty() || current->slices.back().second != -1)
                current->slices.push_back({tick, -1});

            current->remaining_time--;
            if (current->remaining_time == 0) {
                current->completed = true;
                current->finish_time = tick + 1;
                current->slices.back().second = tick + 1;
                ready.erase(ready.begin());
            }

            if (passo_a_passo) {
                show_and_wait(tick, current->id, {});
            }
        } else if (passo_a_passo) {
            show_and_wait(tick, "-", {});
        }

        tick++;
    }
}

// ---------- SRTF ----------
void simulate_srtf(std::vector<Process>& processes, bool passo_a_passo) {
    for (auto& p : processes) p.remaining_time = p.duration;
    int tick = 0;

    while (!all_completed(processes)) {
        std::vector<Process*> ready;
        for (auto& p : processes)
            if (!p.completed && p.arrival_time <= tick)
                ready.push_back(&p);

        Process* current = nullptr;
        if (!ready.empty()) {
            auto cmp = [](Process* a, Process* b) {
                if (a->remaining_time != b->remaining_time)
                    return a->remaining_time < b->remaining_time;
                if (a->arrival_time != b->arrival_time)
                    return a->arrival_time < b->arrival_time;
                return a->id < b->id;
            };
            current = *std::min_element(ready.begin(), ready.end(), cmp);
        }

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
            std::sort(ready.begin(), ready.end(), [](Process* a, Process* b){
                if (a->remaining_time != b->remaining_time)
                    return a->remaining_time < b->remaining_time;
                if (a->arrival_time  != b->arrival_time)
                    return a->arrival_time  < b->arrival_time;
                return a->id < b->id;
            });
            show_and_wait(tick, current ? current->id : "-", ready);
        }

        tick++;
    }
}

// ---------- Prioridade Preemptivo ----------
void simulate_prio_preemptivo(std::vector<Process>& processes, bool passo_a_passo, int quantum) {
    for (auto& p : processes) p.remaining_time = p.duration;
    int tick = 0;
    int quantum_counter = 0;
    Process* current = nullptr;

    while (!all_completed(processes)) {
        std::vector<Process*> ready;
        for (auto& p : processes)
            if (!p.completed && p.arrival_time <= tick)
                ready.push_back(&p);

        if (!ready.empty()) {
            auto cmp = [](Process* a, Process* b) {
                if (a->priority != b->priority) return a->priority > b->priority;
                if (a->arrival_time != b->arrival_time) return a->arrival_time < b->arrival_time;
                return a->id < b->id;
            };
            Process* top = *std::max_element(ready.begin(), ready.end(), cmp);

            bool troca_por_quantum = (quantum > 0 && quantum_counter >= quantum);
            bool troca_por_preemp = (current != top);

            if (!current || troca_por_preemp || troca_por_quantum) {
                if (current && !current->completed && !current->slices.empty()
                    && current->slices.back().second == -1) {
                    current->slices.back().second = tick;
                }
                current = top;
                quantum_counter = 0;
                if (current->slices.empty() || current->slices.back().second != -1)
                    current->slices.push_back({tick, -1});
                if (current->start_time == -1) current->start_time = tick;
            }
        } else {
            current = nullptr;
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
            std::sort(ready.begin(), ready.end(), [](Process* a, Process* b) {
                if (a->priority != b->priority) return a->priority > b->priority;
                if (a->arrival_time != b->arrival_time) return a->arrival_time < b->arrival_time;
                return a->id < b->id;
            });
            show_and_wait(tick, current ? current->id : "-", ready);
        }

        tick++;
    }
}

// ---------- Gantt (SVG) ----------
void generate_gantt(const std::vector<Process>& processes, const std::string& filename) {
    std::ofstream svg(filename);
    if (!svg.is_open()) {
        std::cerr << "Erro: não foi possível criar o arquivo " << filename << "\n";
        return;
    }

    int width_per_tick = 20;
    int max_tick = 0;
    for (const auto& p : processes)
        for (const auto& s : p.slices)
            if (s.second > max_tick) max_tick = s.second;

    int width  = 50 + max_tick * width_per_tick + 50;
    int height = 40 * (int)processes.size() + 60;

    svg << "<svg xmlns='http://www.w3.org/2000/svg' width='" << width
        << "' height='" << height << "'>\n";
    svg << "<style> text{font-family:monospace;font-size:12px;} </style>\n";

    int y = 30;
    for (const auto& p : processes) {
        for (const auto& slice : p.slices) {
            if (slice.second <= slice.first) continue;
            int x = 50 + slice.first * width_per_tick;
            int w = (slice.second - slice.first) * width_per_tick;
            svg << "<rect x='" << x << "' y='" << y << "' width='" << w
                << "' height='20' fill='" << p.color << "' stroke='black'/>\n";
        }
        svg << "<text x='10' y='" << y + 15 << "'>" << p.id << "</text>\n";
        y += 40;
    }

    svg << "<line x1='50' y1='" << y << "' x2='" << width - 20 << "' y2='" << y
        << "' stroke='black'/>\n";
    for (int i = 0; i <= max_tick; i++) {
        int x = 50 + i * width_per_tick;
        svg << "<line x1='" << x << "' y1='" << y << "' x2='" << x
            << "' y2='" << y + 5 << "' stroke='black'/>\n";
        svg << "<text x='" << x - 3 << "' y='" << y + 20 << "'>" << i << "</text>\n";
    }

    svg << "</svg>\n";
    svg.close();
    std::cout << "Gráfico salvo em: " << filename << "\n";
}
