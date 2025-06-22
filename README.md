# 🚀 AeroSwarm - Parallel Drone Swarm Simulator

![Demo GIF](frames/output.gif)  <!-- Your generated GIF here -->

A high-performance C++ simulator for autonomous drone swarms with parallel pathfinding.

## ✨ Features
- **Parallelized terrain navigation** using `std::shared_mutex`
- **Obstacle avoidance** with probabilistic maps
- **Real-time visualization** (GIF generation)
- **Scalable architecture** for 1000+ drones

## 🛠️ Build
```bash
mkdir build && cd build
cmake ..
make
./AeroSwarm 
``` 

## 📊 Example Output
Iteration 142: Drone [3] reached (12,24)
Iteration 143: Drone [1] pathfinding...
Target found in 144 steps!

## 📜 License

MIT License - Free for research and commercial use.