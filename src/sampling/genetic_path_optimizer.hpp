#pragma once

#include "../core/math_utils.hpp"
#include "../core/spectrum.hpp"
#include "../core/sampler.hpp"
#include <vector>
#include <algorithm>

namespace mcps {
namespace experimental {

/// @brief Individual path in the genetic algorithm population
template<typename T>
struct PathGene {
    std::vector<Vec3<T>> vertices;  ///< Path vertices
    std::vector<Vec3<T>> directions; ///< Directions at each vertex
    T fitness;                       ///< Path contribution (fitness)
    Spectrum<T> radiance;            ///< Accumulated radiance
    
    PathGene() : fitness(0) {}
    
    PathGene(int length) : fitness(0) {
        vertices.resize(length);
        directions.resize(length);
    }
};

/// @brief Genetic algorithm for evolving optimal light transport paths
/// Novel approach: Treats light paths as genes and evolves them toward
/// high-contribution paths, combining global search with local refinement
template<typename T>
class GeneticPathOptimizer {
public:
    GeneticPathOptimizer(int pop_size = 100, int max_path_length = 8)
        : population_size(pop_size), max_path_length(max_path_length),
          mutation_rate(T(0.2)), crossover_rate(T(0.7)),
          generation(0) {
        population.resize(population_size);
        for (auto& individual : population) {
            individual = PathGene<T>(max_path_length);
        }
    }
    
    /// @brief Initialize population with random paths
    template<typename Scene>
    void initialize_population(const Scene& scene, Sampler<T>& sampler) {
        for (auto& individual : population) {
            generate_random_path(individual, scene, sampler);
        }
    }
    
    /// @brief Evolve population for one generation
    template<typename Scene>
    void evolve_generation(const Scene& scene, Sampler<T>& sampler) {
        // Evaluate fitness
        evaluate_fitness(scene);
        
        // Sort by fitness (descending)
        std::sort(population.begin(), population.end(),
                  [](const PathGene<T>& a, const PathGene<T>& b) {
                      return a.fitness > b.fitness;
                  });
        
        // Create new generation
        std::vector<PathGene<T>> new_population;
        
        // Elitism: keep top performers
        int elite_count = population_size / 10;
        for (int i = 0; i < elite_count; ++i) {
            new_population.push_back(population[i]);
        }
        
        // Crossover and mutation
        while (new_population.size() < static_cast<size_t>(population_size)) {
            // Tournament selection
            PathGene<T> parent1 = tournament_select(sampler);
            PathGene<T> parent2 = tournament_select(sampler);
            
            PathGene<T> offspring;
            if (sampler.next_1d() < crossover_rate) {
                offspring = crossover(parent1, parent2, sampler);
            } else {
                offspring = parent1;
            }
            
            // Mutation
            if (sampler.next_1d() < mutation_rate) {
                mutate(offspring, scene, sampler);
            }
            
            new_population.push_back(offspring);
        }
        
        population = new_population;
        generation++;
    }
    
    /// @brief Get best path from current population
    const PathGene<T>& get_best_path() const {
        return population[0];
    }
    
    /// @brief Get average fitness of population
    T get_average_fitness() const {
        T sum = 0;
        for (const auto& individual : population) {
            sum += individual.fitness;
        }
        return sum / T(population_size);
    }
    
private:
    std::vector<PathGene<T>> population;
    int population_size;
    int max_path_length;
    T mutation_rate;
    T crossover_rate;
    int generation;
    
    /// @brief Generate random path
    template<typename Scene>
    void generate_random_path(PathGene<T>& path, const Scene& scene, Sampler<T>& sampler) {
        // Simplified: random walk in scene
        for (size_t i = 0; i < path.vertices.size(); ++i) {
            T u = sampler.next_1d();
            T v = sampler.next_1d();
            T w = sampler.next_1d();
            
            // Random position in scene bounds
            path.vertices[i] = Vec3<T>(u * T(10) - T(5), v * T(10) - T(5), w * T(10) - T(5));
            
            // Random direction
            T theta = sampler.next_1d() * T(2) * pi<T>();
            T phi = sampler.next_1d() * pi<T>();
            path.directions[i] = Vec3<T>(
                std::sin(phi) * std::cos(theta),
                std::sin(phi) * std::sin(theta),
                std::cos(phi)
            );
        }
    }
    
    /// @brief Evaluate fitness of all individuals
    template<typename Scene>
    void evaluate_fitness(const Scene& scene) {
        for (auto& individual : population) {
            // Fitness = path contribution (simplified)
            // In full implementation, would trace path through scene
            individual.fitness = estimate_path_contribution(individual, scene);
        }
    }
    
    /// @brief Estimate path contribution (simplified)
    template<typename Scene>
    T estimate_path_contribution(const PathGene<T>& path, const Scene& scene) const {
        // Simplified fitness: shorter paths with good angles are better
        T fitness = T(1);
        
        for (size_t i = 0; i < path.vertices.size() - 1; ++i) {
            Vec3<T> segment = path.vertices[i + 1] - path.vertices[i];
            T length = segment.length();
            
            // Penalize very short or very long segments
            if (length < T(0.1) || length > T(5)) {
                fitness *= T(0.5);
            }
            
            // Reward alignment between direction and segment
            T alignment = dot(path.directions[i], segment.normalized());
            fitness *= (T(1) + alignment) * T(0.5);
        }
        
        return fitness;
    }
    
    /// @brief Tournament selection
    PathGene<T> tournament_select(Sampler<T>& sampler, int tournament_size = 5) {
        PathGene<T> best = population[0];
        T best_fitness = best.fitness;
        
        for (int i = 0; i < tournament_size; ++i) {
            int idx = static_cast<int>(sampler.next_1d() * population_size);
            if (idx >= population_size) idx = population_size - 1;
            
            if (population[idx].fitness > best_fitness) {
                best = population[idx];
                best_fitness = best.fitness;
            }
        }
        
        return best;
    }
    
    /// @brief Crossover two paths
    PathGene<T> crossover(const PathGene<T>& p1, const PathGene<T>& p2, Sampler<T>& sampler) {
        PathGene<T> offspring(max_path_length);
        
        // Single-point crossover
        int crossover_point = static_cast<int>(sampler.next_1d() * max_path_length);
        
        for (int i = 0; i < max_path_length; ++i) {
            if (i < crossover_point) {
                offspring.vertices[i] = p1.vertices[i];
                offspring.directions[i] = p1.directions[i];
            } else {
                offspring.vertices[i] = p2.vertices[i];
                offspring.directions[i] = p2.directions[i];
            }
        }
        
        return offspring;
    }
    
    /// @brief Mutate a path
    template<typename Scene>
    void mutate(PathGene<T>& path, const Scene& scene, Sampler<T>& sampler) {
        // Random mutation type
        T r = sampler.next_1d();
        
        if (r < T(0.33)) {
            // Perturb random vertex
            int idx = static_cast<int>(sampler.next_1d() * max_path_length);
            if (idx >= max_path_length) idx = max_path_length - 1;
            
            T dx = (sampler.next_1d() - T(0.5)) * T(2);
            T dy = (sampler.next_1d() - T(0.5)) * T(2);
            T dz = (sampler.next_1d() - T(0.5)) * T(2);
            
            path.vertices[idx] += Vec3<T>(dx, dy, dz);
        } else if (r < T(0.66)) {
            // Perturb random direction
            int idx = static_cast<int>(sampler.next_1d() * max_path_length);
            if (idx >= max_path_length) idx = max_path_length - 1;
            
            T theta = sampler.next_1d() * T(2) * pi<T>();
            T phi = sampler.next_1d() * pi<T>();
            path.directions[idx] = Vec3<T>(
                std::sin(phi) * std::cos(theta),
                std::sin(phi) * std::sin(theta),
                std::cos(phi)
            );
        } else {
            // Swap two random segments
            int i1 = static_cast<int>(sampler.next_1d() * (max_path_length - 1));
            int i2 = static_cast<int>(sampler.next_1d() * (max_path_length - 1));
            if (i1 >= max_path_length - 1) i1 = max_path_length - 2;
            if (i2 >= max_path_length - 1) i2 = max_path_length - 2;
            
            std::swap(path.vertices[i1], path.vertices[i2]);
            std::swap(path.directions[i1], path.directions[i2]);
        }
    }
};

} // namespace experimental
} // namespace mcps
