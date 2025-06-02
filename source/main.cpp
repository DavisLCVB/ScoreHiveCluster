#include <mpi.h>
#include <domain/coordinator.hpp>
#include <domain/evaluator.hpp>
#include <iostream>
#include <server/server.hpp>
#include <system/aliases.hpp>
#include <system/logger.hpp>

i32 main(i32 argc, char** argv) {
  MPI_Init(&argc, &argv);
  i32 rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  Logger::config(rank);
  if (rank == 0) {
    ServerConfig config;
    Server server(config);
    server.start();
    MPICoordinator::instance().free_types();
  } else {
    spdlog::info("Worker {} started", rank);
    bool shutdown = false;
    while (!shutdown) {
      auto& coordinator = MPICoordinator::instance();
      auto [exams, command] = coordinator.receive_from_master(0);
      if (command == MPICommand::SHUTDOWN) {
        shutdown = true;
        coordinator.free_types();
        spdlog::info("Worker {} received shutdown signal", rank);
        break;
      }
      spdlog::info("Worker {} received exams count: {}", rank, exams.size());
      auto results = Evaluator::instance().evaluate_exam_batch(exams);
      coordinator.send_to_master(results, 0);
    }
  }
  MPI_Finalize();
  return 0;
}