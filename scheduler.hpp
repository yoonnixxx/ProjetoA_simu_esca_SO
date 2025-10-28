#pragma once
#include "process.hpp"
#include <vector>
#include <iostream>

// Simulação do algoritmo FIFO
inline void simulate_fifo(const std::vector<Process>& processes) {
    std::cout << "\n=== Simulação FIFO ===\n";

    int current_time = 0;
    int total_wait = 0;
    int total_turnaround = 0;

    for (const auto& p : processes) {
        if (current_time < p.arrival_time)
            current_time = p.arrival_time; // espera até o processo chegar

        int start_time = current_time;
        int finish_time = start_time + p.duration;
        int wait_time = start_time - p.arrival_time;
        int turnaround = finish_time - p.arrival_time;

        total_wait += wait_time;
        total_turnaround += turnaround;
        current_time = finish_time;

        std::cout << "Processo " << p.name
                  << " | Início: " << start_time
                  << " | Fim: " << finish_time
                  << " | Espera: " << wait_time
                  << " | Turnaround: " << turnaround << "\n";
    }

    int n = processes.size();
    std::cout << "\nMédia de espera: " << (float)total_wait / n
              << "\nMédia de turnaround: " << (float)total_turnaround / n << "\n";
}

// Simulação do algoritmo Round Robin
inline void simulate_rr(const std::vector<Process>& processes, int quantum) {
    std::cout << "\n=== Simulação Round Robin (Quantum = " << quantum << ") ===\n";

    struct Job {
        Process p;
        int remaining;
    };

    std::vector<Job> queue;
    for (auto& p : processes)
        queue.push_back({p, p.duration});

    int current_time = 0;
    int total_wait = 0;
    int total_turnaround = 0;
    int completed = 0;
    int n = processes.size();

    std::vector<int> finish_time(n, 0);
    std::vector<int> wait_time(n, 0);
    std::vector<int> last_exec(n, 0);

    while (completed < n) {
        bool executed = false;

        for (int i = 0; i < n; ++i) {
            auto& job = queue[i];
            if (job.remaining <= 0) continue;
            if (job.p.arrival_time > current_time) continue;

            executed = true;

            // tempo de espera = tempo atual - (último tempo que ele rodou ou chegou)
            if (last_exec[i] == 0 && current_time > job.p.arrival_time)
                wait_time[i] += current_time - job.p.arrival_time;
            else if (last_exec[i] != 0)
                wait_time[i] += current_time - last_exec[i];

            int run_time = std::min(quantum, job.remaining);
            job.remaining -= run_time;
            current_time += run_time;
            last_exec[i] = current_time;

            std::cout << "Tempo " << (current_time - run_time)
                      << " - " << current_time
                      << " → Processo " << job.p.name
                      << " (restante: " << job.remaining << ")\n";

            if (job.remaining == 0) {
                completed++;
                finish_time[i] = current_time;
                total_turnaround += finish_time[i] - job.p.arrival_time;
            }
        }

        if (!executed) current_time++;
    }

    for (int i = 0; i < n; ++i)
        total_wait += wait_time[i];

    std::cout << "\nMédia de espera: " << (float)total_wait / n
              << "\nMédia de turnaround: " << (float)total_turnaround / n << "\n";
}
