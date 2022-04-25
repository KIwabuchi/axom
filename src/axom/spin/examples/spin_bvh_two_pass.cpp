// Copyright (c) 2017-2022, Lawrence Livermore National Security, LLC and
// other Axom Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

// Axom includes
#include "axom/core.hpp"
#include "axom/mint.hpp"
#include "axom/primal.hpp"
#include "axom/spin.hpp"
#include "axom/slic.hpp"
#include "axom/quest.hpp"

#include "axom/CLI11.hpp"
#include "axom/fmt.hpp"

namespace mint = axom::mint;
namespace primal = axom::primal;
namespace spin = axom::spin;
namespace slic = axom::slic;
namespace quest = axom::quest;

using IndexType = axom::IndexType;
using UMesh = mint::UnstructuredMesh<mint::SINGLE_SHAPE>;

enum class ExecPolicy
{
  CPU,
  OpenMP,
  CUDA
};

const std::map<std::string, ExecPolicy> validExecPolicies {
  {"seq", ExecPolicy::CPU},
#ifdef AXOM_USE_OPENMP
  {"omp", ExecPolicy::OpenMP},
#endif
#ifdef AXOM_USE_CUDA
  {"cuda", ExecPolicy::CUDA}
#endif
};

//------------------------------------------------------------------------------
void initialize_logger()
{
  // initialize logger
  slic::initialize();
  slic::setLoggingMsgLevel(slic::message::Info);

  // setup the logstreams
  std::string fmt = "";
  slic::LogStream* logStream = nullptr;

  fmt = "[<LEVEL>]: <MESSAGE>\n";
  logStream = new slic::GenericOutputStream(&std::cout, fmt);

  // register stream objects with the logger
  slic::addStreamToAllMsgLevels(logStream);
}

//------------------------------------------------------------------------------
void finalize_logger()
{
  slic::flushStreams();
  slic::finalize();
}

template <typename ExecSpace>
void find_collisions_broadphase(const mint::Mesh* mesh,
                                axom::Array<IndexType>& firstPair,
                                axom::Array<IndexType>& secondPair)
{
  using PointType = axom::primal::Point<double, 3>;
  using BoxType = axom::primal::BoundingBox<double, 3>;
  using exec_pol = typename axom::execution_space<ExecSpace>::loop_policy;
  using reduce_pol = typename axom::execution_space<ExecSpace>::reduce_policy;

  int allocatorId;

  const int ncells = mesh->getNumberOfCells();

  axom::Array<BoxType> aabbs(ncells, ncells, allocatorId);
  const auto v_aabbs = aabbs.view();

  // Initialize the bounding box for each cell
  mint::for_all_cells<ExecSpace, mint::xargs::coords>(
    mesh,
    AXOM_LAMBDA(IndexType cellIdx,
                axom::numerics::Matrix<double> & coords,
                const IndexType* nodeIds) {
      AXOM_UNUSED_VAR(nodeIds);
      int numNodes = coords.getNumColumns();
      BoxType aabb;

      for(IndexType inode = 0; inode < numNodes; ++inode)
      {
        const double* node = coords.getColumn(inode);
        PointType vtx {node[mint::X_COORDINATE],
                       node[mint::Y_COORDINATE],
                       node[mint::Z_COORDINATE]};
        aabb.addPoint(vtx);
      }  // END for all cells nodes

      v_aabbs[cellIdx] = aabb;
    });

  spin::BVH<3, ExecSpace, double> bvh;
  bvh.setAllocatorID(allocatorId);
  bvh.initialize(v_aabbs, v_aabbs.size());

  const auto bvh_device = bvh.getTraverser();

  auto bbIsect = [] AXOM_HOST_DEVICE(const BoxType& bb1,
                                     const BoxType& bb2) -> bool {
    return bb1.intersectsWith(bb2);
  };

  axom::Array<IndexType> offsets(ncells, ncells, allocatorId);
  axom::Array<IndexType> counts(ncells, ncells, allocatorId);

  const auto v_counts = counts.view();

  RAJA::ReduceSum<reduce_pol, IndexType> total_count_reduce(0);

  // First pass: get number of bounding box collisions for each cell
  axom::for_all<ExecSpace>(
    ncells,
    AXOM_LAMBDA(IndexType icell) {
      IndexType count = 0;

      auto countCollisions = [&](axom::int32 currentNode,
                                 const axom::int32* leafNodes) {
        AXOM_UNUSED_VAR(currentNode);
        AXOM_UNUSED_VAR(leafNodes);
        if(currentNode > icell)
        {
          count++;
        }
      };

      bvh_device.traverse_tree(v_aabbs[icell], countCollisions, bbIsect);
      v_counts[icell] = count;
      total_count_reduce += count;
    });

  // Generate offsets
  RAJA::exclusive_scan<exec_pol>(RAJA::make_span(counts.data(), ncells),
                                 RAJA::make_span(offsets.data(), ncells),
                                 RAJA::operators::plus<IndexType> {});

  // Create array for all bounding box collisions
  IndexType ncollisions = total_count_reduce.get();

  SLIC_INFO(axom::fmt::format("Found {} candidate collisions.", ncollisions));

  firstPair = axom::Array<IndexType>(ncollisions, ncollisions, allocatorId);
  secondPair = axom::Array<IndexType>(ncollisions, ncollisions, allocatorId);

  const auto v_offsets = offsets.view();
  const auto v_first_pair = firstPair.view();
  const auto v_second_pair = secondPair.view();

  // Second pass: fill broad-phase collisions array
  axom::for_all<ExecSpace>(
    ncells,
    AXOM_LAMBDA(IndexType icell) {
      IndexType offset = v_offsets[icell];

      auto fillCollisions = [&](axom::int32 currentNode,
                                const axom::int32* leafs) {
        if(currentNode > icell)
        {
          v_first_pair[offset] = currentNode;
          v_second_pair[offset] = leafs[currentNode];
          offset++;
        }
      };

      bvh_device.traverse_tree(v_aabbs[icell], fillCollisions, bbIsect);
    });
}

struct Arguments
{
  std::string file_name;
  ExecPolicy exec_space {ExecPolicy::CPU};

  void parse(int argc, char** argv, axom::CLI::App& app)
  {
    app
      .add_option("-f,--file", this->file_name, "specifies the input mesh file")
      ->check(axom::CLI::ExistingFile)
      ->required();

    std::string pol_info = "Sets execution space of the .\n";
    pol_info += "Set to \'seq\' to use sequential execution policy.";
#ifdef AXOM_USE_OPENMP
    pol_info += "\nSet to \'omp\' to use an OpenMP execution policy.";
#endif
#ifdef AXOM_USE_CUDA
    pol_info += "\nSet to \'gpu\' to use a GPU execution policy.";
#endif
    app.add_option("-e, --exec_space", this->exec_space, pol_info)
      ->capture_default_str()
      ->transform(axom::CLI::CheckedTransformer(validExecPolicies));

    app.get_formatter()->column_width(40);

    // could throw an exception
    app.parse(argc, argv);

    slic::flushStreams();
  }
};

int main(int argc, char** argv)
{
  initialize_logger();
  Arguments args;
  axom::CLI::App app {"Demo of the BVH 2-pass algorithm"};

  try
  {
    args.parse(argc, argv, app);
  }
  catch(const axom::CLI::ParseError& e)
  {
    int retval = -1;
    retval = app.exit(e);
    finalize_logger();
    return retval;
  }
#ifdef AXOM_USE_CUDA
  if(args.exec_space == ExecPolicy::GPU)
  {
    using GPUExec = axom::CUDA_EXEC<256>;
    axom::setDefaultAllocator(axom::execution_space<GPUExec>::allocatorID());
  }
#endif

  std::unique_ptr<UMesh> surface_mesh;

  // Read file
  SLIC_INFO("Reading file: '" << args.file_name << "'...\n");
  {
    quest::STLReader reader;
    reader.setFileName(args.file_name);
    reader.read();

    // Get surface mesh
    surface_mesh.reset(new UMesh(3, mint::TRIANGLE));
    reader.getMesh(surface_mesh.get());
  }

  SLIC_INFO("Mesh has " << surface_mesh->getNumberOfNodes() << " vertices and "
                        << surface_mesh->getNumberOfCells() << " triangles.");

  axom::Array<IndexType> firstPair, secondPair;

  switch(args.exec_space)
  {
  case ExecPolicy::CPU:
    benchmark_point_in_cell<axom::SEQ_EXEC>(surface_mesh.get(),
                                            firstPair,
                                            secondPair);
    break;
#ifdef AXOM_USE_RAJA
  #ifdef AXOM_USE_OPENMP
  case ExecPolicy::OpenMP:
    benchmark_point_in_cell<axom::OMP_EXEC>(surface_mesh.get(),
                                            firstPair,
                                            secondPair);
    break;
  #endif
  #ifdef AXOM_USE_CUDA
  case ExecPolicy::GPU:
    benchmark_point_in_cell<axom::CUDA_EXEC<256>>(surface_mesh.get(),
                                                  firstPair,
                                                  secondPair);
    break;
  #endif
#endif
  default:
    SLIC_ERROR("Unsupported execution space.");
    return 1;
  }

  finalize_logger();
}
