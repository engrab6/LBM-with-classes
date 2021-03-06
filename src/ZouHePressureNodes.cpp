#include "ZouHePressureNodes.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

ZouHePressureNodes::ZouHePressureNodes(LatticeModel &lm
  , CollisionModel &cm)
  : BoundaryNodes(false, false, lm),
    nodes {},
    cm_ (cm),
    beta1_ {},
    beta2_ {},
    beta3_ {}
{
  const auto c = lm_.GetLatticeSpeed();
  const auto cs_sqr = c * c / 3.0;
  beta1_ = c / cs_sqr / 9.0;
  beta2_ = 0.5 / c;
  beta3_ = beta2_ - beta1_;
}

void ZouHePressureNodes::AddNode(std::size_t x
  , std::size_t y
  , double rho_node)
{
  const auto nx = lm_.GetNumberOfColumns();
  const auto ny = lm_.GetNumberOfRows();
  const auto left = x == 0;
  const auto right = x == nx - 1;
  const auto bottom = y == 0;
  const auto top = y == ny - 1;
  auto side = -1;
  if (right) side = 0;
  if (top) side = 1;
  if (left) side = 2;
  if (bottom) side = 3;
  // adds a corner node
  if ((top || bottom) && (left || right)) {
    side = right * 1 + top * 2;
    nodes.push_back(ValueNode(x, y, nx, rho_node, true, side));
  }
  // adds a side node
  else {
    nodes.push_back(ValueNode(x, y, nx, rho_node, false, side));
  }
}

void ZouHePressureNodes::UpdateNodes(std::vector<std::vector<double>> &df
  , bool is_modify_stream)
{
  if (!is_modify_stream) {
    for (auto node : nodes) {
      if (node.b1) {
        ZouHePressureNodes::UpdateCorner(df, node);
      }
      else {
        ZouHePressureNodes::UpdateSide(df, node);
      }
    }  // n
  }
}

void ZouHePressureNodes::UpdateSide(std::vector<std::vector<double>> &df
  , ValueNode &node)
{
  const auto n = node.n;
  const auto rho_node = node.d1;
  const auto c = lm_.GetLatticeSpeed();
  std::vector<double> vel = {0.0, 0.0};
  switch (node.i1) {
    case 0: {  // right
      vel[0] = -1.0 + (df[n][0] + df[n][N] + df[n][S] + 2.0 * (df[n][E] +
          df[n][NE] + df[n][SE])) / rho_node;
      vel[0] *= c;
      const auto df_diff = 0.5 * (df[n][S] - df[n][N]);
      for (auto &u : vel) u *= rho_node;
      df[n][W] = df[n][E] - 2.0 * beta1_ * vel[0];
      df[n][NW] = df[n][SE] + df_diff - beta3_ * vel[0];
      df[n][SW] = df[n][NE] - df_diff - beta3_ * vel[0];
//      df[n][W] = df[n][E] - 2.0 / 3.0 * vel[0];
//      df[n][NW] = df[n][SE] + df_diff - vel[0] / 6.0;
//      df[n][SW] = df[n][NE] - df_diff - vel[0] / 6.0;
      break;
    }
    case 1: {  // top
      vel[1] = -1.0 + (df[n][0] + df[n][E] + df[n][W] + 2.0 * (df[n][N] +
          df[n][NE] + df[n][NW])) / rho_node;
      vel[1] *= c;
      const auto df_diff = 0.5 * (df[n][E] - df[n][W]);
      for (auto &u : vel) u *= rho_node;
      df[n][S] = df[n][N] - 2.0 *beta1_ * vel[1];
      df[n][SW] = df[n][NE] + df_diff - beta3_ * vel[1];
      df[n][SE] = df[n][NW] - df_diff - beta3_ * vel[1];
//      df[n][S] = df[n][N] - 2.0 / 3.0 * vel[1];
//      df[n][SW] = df[n][NE] + df_diff - vel[1] / 6.0;
//      df[n][SE] = df[n][NW] - df_diff - vel[1] / 6.0;
      break;
    }
    case 2: {  // left
      vel[0] = 1.0 - (df[n][0] + df[n][N] + df[n][S] + 2.0 * (df[n][W] +
          df[n][NW] + df[n][SW])) / rho_node;
      vel[0] *= c;
      const auto df_diff = 0.5 * (df[n][S] - df[n][N]);
      for (auto &u : vel) u *= rho_node;
      df[n][E] = df[n][W] + 2.0 *beta1_ * vel[0];
      df[n][NE] = df[n][SW] + df_diff + beta3_ * vel[0];
      df[n][SE] = df[n][NW] - df_diff + beta3_ * vel[0];
//      df[n][E] = df[n][W] + 2.0 / 3.0 * vel[0];
//      df[n][NE] = df[n][SW] + df_diff + vel[0] / 6.0;
//      df[n][SE] = df[n][NW] - df_diff + vel[0] / 6.0;
      break;
    }
    case 3: {  // bottom
      vel[1] = 1.0 - (df[n][0] + df[n][E] + df[n][W] + 2.0 * (df[n][S] +
          df[n][SW] + df[n][SE])) / rho_node;
      vel[1] *= c;
      const auto df_diff = 0.5 * (df[n][W] - df[n][E]);
      for (auto &u : vel) u *= rho_node;
      df[n][N] = df[n][S] + 2.0 * beta1_ * vel[1];
      df[n][NE] = df[n][SW] + df_diff + beta3_ * vel[1];
      df[n][NW] = df[n][SE] - df_diff + beta3_ * vel[1];
//      df[n][N] = df[n][S] + 2.0 / 3.0 * vel[1];
//      df[n][NE] = df[n][SW] + df_diff + vel[1] / 6.0;
//      df[n][NW] = df[n][SE] - df_diff + vel[1] / 6.0;
      break;
    }
    default: {
      throw std::runtime_error("Not a side");
    }
  }
}

void ZouHePressureNodes::UpdateCorner(std::vector<std::vector<double>> &df
    , ValueNode &node)
{
  const auto n = node.n;
  const auto rho_node = node.d1;
  switch (node.i1) {
    case 0: {  // bottom-left
      df[n][E] = df[n][W];
      df[n][N] = df[n][S];
      df[n][NE] = df[n][SW];
      df[n][NW] = 0.5 * (rho_node - df[n][0] - df[n][E] - df[n][N] - df[n][W] -
          df[n][S] - df[n][NE] - df[n][SW]);
      df[n][SE] = df[n][NW];
      break;
    }
    case 1: {  // bottom-right
      df[n][W] = df[n][E];
      df[n][N] = df[n][S];
      df[n][NW] = df[n][SE];
      df[n][NE] = 0.5 * (rho_node - df[n][0] - df[n][E] - df[n][N] - df[n][W] -
          df[n][S] - df[n][NW] - df[n][SE]);
      df[n][SW] = df[n][NE];
      break;
    }
    case 2: {  // top-left
      df[n][E] = df[n][W];
      df[n][S] = df[n][N];
      df[n][SE] = df[n][NW];
      df[n][NE] = 0.5 * (rho_node - df[n][0] - df[n][E] - df[n][N] - df[n][W] -
          df[n][S] - df[n][NW] - df[n][SE]);
      df[n][SW] = df[n][NE];
      break;
    }
    case 3: {  // top-right
      df[n][W] = df[n][E];
      df[n][S] = df[n][N];
      df[n][SW] = df[n][NE];
      df[n][NW] = 0.5 * (rho_node - df[n][0] - df[n][E] - df[n][N] - df[n][W] -
          df[n][S] - df[n][NE] - df[n][SW]);
      df[n][SE] = df[n][NW];
      break;
    }
    default: {
      throw std::runtime_error("Not a corner");
    }
  }
}
