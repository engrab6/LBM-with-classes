#include "Particle.hpp"
#include <cmath>
#include "ParticleNode.hpp"

Particle::Particle(double stiffness
  , double center_x
  , double center_y)
  : center {ParticleNode(center_x, center_y, center_x, center_y)},
    nodes {}
{}

void Particle::AddNode(double x
  , double y
  , double x_ref
  , double y_ref
  , double u_x
  , double u_y
  , double force_x
  , double force_y)
{
  nodes.push_back(ParticleNode(x, y, x_ref, y_ref, u_x, u_y, force_x, force_y));
}

void Particle::CreateCylinder(std::size_t num_nodes
  , double radius)
{
  for (auto i = 0u; i < num_nodes; ++i) {
    auto x = center.coord[0] + radius * sin(2.0 * pi_ * static_cast<double>(i) /
        num_nodes);
    auto y = center.coord[1] + radius * cos(2.0 * pi_ * static_cast<double>(i) /
        num_nodes);
    Particle::AddNode(x, y, x, y, 0.0, 0.0, 0.0, 0.0);
  }
}
