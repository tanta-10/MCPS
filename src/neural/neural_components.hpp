#pragma once

#include "../core/math_utils.hpp"
#include "../core/spectrum.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

namespace mcps {
namespace experimental {

/// @brief Tiny MLP for radiance prediction with novel activation functions
template<typename T>
class TinyMLP {
public:
    TinyMLP(int input_dim, int hidden_dim, int output_dim)
        : input_dim(input_dim), hidden_dim(hidden_dim), output_dim(output_dim) {
        // Initialize weights with Xavier initialization
        weights_ih.resize(input_dim * hidden_dim);
        weights_ho.resize(hidden_dim * output_dim);
        bias_h.resize(hidden_dim);
        bias_o.resize(output_dim);
        
        initialize_weights();
    }
    
    /// @brief Forward pass
    std::vector<T> forward(const std::vector<T>& input) {
        // Hidden layer
        std::vector<T> hidden(hidden_dim, T(0));
        for (int h = 0; h < hidden_dim; ++h) {
            T sum = bias_h[h];
            for (int i = 0; i < input_dim; ++i) {
                sum += input[i] * weights_ih[i * hidden_dim + h];
            }
            hidden[h] = novel_activation(sum); // Novel activation
        }
        
        // Output layer
        std::vector<T> output(output_dim, T(0));
        for (int o = 0; o < output_dim; ++o) {
            T sum = bias_o[o];
            for (int h = 0; h < hidden_dim; ++h) {
                sum += hidden[h] * weights_ho[h * output_dim + o];
            }
            output[o] = std::max(T(0), sum); // ReLU for output (radiance is positive)
        }
        
        return output;
    }
    
    /// @brief Novel activation: Swish + sinusoidal modulation
    T novel_activation(T x) const {
        // Swish: x * sigmoid(x)
        T swish = x / (T(1) + std::exp(-x));
        
        // Add sinusoidal component for periodic patterns
        T sin_mod = std::sin(x * T(0.5)) * T(0.1);
        
        return swish + sin_mod;
    }
    
    /// @brief Online learning update (simplified gradient descent)
    void update(const std::vector<T>& input, const std::vector<T>& target, T learning_rate) {
        // Forward pass
        auto output = forward(input);
        
        // Simplified backprop (just output layer for speed)
        for (int o = 0; o < output_dim; ++o) {
            T error = output[o] - target[o];
            bias_o[o] -= learning_rate * error;
            
            // Update output weights (simplified)
            for (int h = 0; h < hidden_dim; ++h) {
                weights_ho[h * output_dim + o] -= learning_rate * error * T(0.01);
            }
        }
    }
    
private:
    int input_dim, hidden_dim, output_dim;
    std::vector<T> weights_ih;  // Input to hidden
    std::vector<T> weights_ho;  // Hidden to output
    std::vector<T> bias_h;
    std::vector<T> bias_o;
    
    void initialize_weights() {
        T scale_ih = std::sqrt(T(2.0) / T(input_dim));
        T scale_ho = std::sqrt(T(2.0) / T(hidden_dim));
        
        for (auto& w : weights_ih) w = (T(std::rand()) / T(RAND_MAX) - T(0.5)) * scale_ih;
        for (auto& w : weights_ho) w = (T(std::rand()) / T(RAND_MAX) - T(0.5)) * scale_ho;
        for (auto& b : bias_h) b = T(0);
        for (auto& b : bias_o) b = T(0);
    }
};

/// @brief Neural radiance cache with online learning
/// Novel: Learns incident radiance distribution during rendering
template<typename T>
class NeuralRadianceCache {
public:
    struct CacheEntry {
        Vec3<T> position;
        Vec3<T> normal;
        Spectrum<T> radiance;
        T confidence;
        int query_count;
        
        CacheEntry() : position(), normal(), radiance(), confidence(T(0)), query_count(0) {}
    };
    
    NeuralRadianceCache(int capacity = 10000)
        : max_capacity(capacity),
          mlp(6, 32, 3), // Input: pos(3) + normal(3), Hidden: 32, Output: RGB(3)
          learning_rate(T(0.01)) {}
    
    /// @brief Query radiance at position with normal
    Spectrum<T> query(const Vec3<T>& pos, const Vec3<T>& normal, T& confidence) {
        // Encode input
        std::vector<T> input = {pos.x, pos.y, pos.z, normal.x, normal.y, normal.z};
        
        // Query MLP
        auto output = mlp.forward(input);
        
        // Find nearest cache entry for confidence estimation
        T min_dist = std::numeric_limits<T>::max();
        const CacheEntry* nearest = nullptr;
        
        for (const auto& entry : cache) {
            T dist = (entry.position - pos).length();
            if (dist < min_dist) {
                min_dist = dist;
                nearest = &entry;
            }
        }
        
        if (nearest && min_dist < T(1.0)) {
            confidence = nearest->confidence / (T(1) + min_dist);
        } else {
            confidence = T(0);
        }
        
        return Spectrum<T>(output[0], output[1], output[2]);
    }
    
    /// @brief Update cache with new sample
    void update(const Vec3<T>& pos, const Vec3<T>& normal, const Spectrum<T>& radiance) {
        // Add to cache
        if (cache.size() < static_cast<size_t>(max_capacity)) {
            CacheEntry entry;
            entry.position = pos;
            entry.normal = normal;
            entry.radiance = radiance;
            entry.confidence = T(1);
            entry.query_count = 1;
            cache.push_back(entry);
        } else {
            // Replace least confident entry
            auto min_it = std::min_element(cache.begin(), cache.end(),
                [](const CacheEntry& a, const CacheEntry& b) {
                    return a.confidence < b.confidence;
                });
            
            if (min_it != cache.end()) {
                min_it->position = pos;
                min_it->normal = normal;
                min_it->radiance = radiance;
                min_it->confidence = T(1);
                min_it->query_count = 1;
            }
        }
        
        // Update MLP
        std::vector<T> input = {pos.x, pos.y, pos.z, normal.x, normal.y, normal.z};
        std::vector<T> target = {radiance.r, radiance.g, radiance.b};
        mlp.update(input, target, learning_rate);
    }
    
    /// @brief Get cache statistics
    int get_size() const { return static_cast<int>(cache.size()); }
    
private:
    std::vector<CacheEntry> cache;
    int max_capacity;
    TinyMLP<T> mlp;
    T learning_rate;
};

/// @brief Multi-resolution hash encoding for position encoding
/// Novel: Combines multiple resolution levels with learnable hash functions
template<typename T>
class HashEncoding {
public:
    HashEncoding(int num_levels = 16, int features_per_level = 2)
        : num_levels(num_levels), features_per_level(features_per_level) {
        hash_tables.resize(num_levels);
        
        // Initialize hash tables with different resolutions
        for (int level = 0; level < num_levels; ++level) {
            int resolution = 16 * (1 << level); // Exponential growth
            int table_size = resolution * resolution * resolution;
            
            hash_tables[level].resize(table_size * features_per_level);
            
            // Random initialization
            for (auto& val : hash_tables[level]) {
                val = (T(std::rand()) / T(RAND_MAX) - T(0.5)) * T(0.1);
            }
        }
    }
    
    /// @brief Encode position into multi-resolution features
    std::vector<T> encode(const Vec3<T>& pos) const {
        std::vector<T> features;
        features.reserve(num_levels * features_per_level);
        
        for (int level = 0; level < num_levels; ++level) {
            int resolution = 16 * (1 << level);
            
            // Compute grid cell
            int x = static_cast<int>(pos.x * resolution) % resolution;
            int y = static_cast<int>(pos.y * resolution) % resolution;
            int z = static_cast<int>(pos.z * resolution) % resolution;
            
            // Hash function (simple for now)
            int hash = (x * 73856093) ^ (y * 19349663) ^ (z * 83492791);
            hash = hash % static_cast<int>(hash_tables[level].size() / features_per_level);
            if (hash < 0) hash += static_cast<int>(hash_tables[level].size() / features_per_level);
            
            // Look up features
            for (int f = 0; f < features_per_level; ++f) {
                features.push_back(hash_tables[level][hash * features_per_level + f]);
            }
        }
        
        return features;
    }
    
    int get_output_dim() const {
        return num_levels * features_per_level;
    }
    
private:
    int num_levels;
    int features_per_level;
    std::vector<std::vector<T>> hash_tables;
};

} // namespace experimental
} // namespace mcps
