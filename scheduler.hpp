#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <vector>
#include <string>
#include "process.hpp"

// -------------------------------------------------------------------
// Protótipos exigidos pela main.cpp
// (Implemente-os em scheduler.cpp ou inline aqui, se preferir.)
// -------------------------------------------------------------------

// FIFO (First In First Out)
void simulate_fifo(std::vector<Process>& processes, bool passo_a_passo);

// SRTF (Shortest Remaining Time First) – preemptivo
void simulate_srtf(std::vector<Process>& processes, bool passo_a_passo);

// Prioridade Preemptivo (usa 'priority'; 'quantum' pode ser usado ou ignorado
// conforme sua regra; a main passa o valor lido da primeira linha do config)
void simulate_prio_preemptivo(std::vector<Process>& processes, bool passo_a_passo, int quantum);

// Gera um gráfico de Gantt no caminho indicado (ex.: "output/gantt.svg")
// Espera que cada Process tenha suas “slices” preenchidas durante a simulação.
void generate_gantt(const std::vector<Process>& processes, const std::string& filename);

#endif // SCHEDULER_HPP
