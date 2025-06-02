#pragma once
#ifndef COORDINATOR_HPP
#define COORDINATOR_HPP

#include <mpi.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <system/aliases.hpp>
#include <vector>

using json = nlohmann::json;

struct CoordinatorConfig {
  i32 mpi_tag_answers = 100;
  i32 mpi_tag_exams = 101;
  i32 mpi_tag_results = 102;
  i32 mpi_tag_command = 103;
};

struct MPIQuestion {
  i32 qst_idx;
  i32 ans_idx;
};

struct MPIExam {
  i32 stage;
  i32 id_exam;
  std::vector<MPIQuestion> answers;
};

struct MPIExamHeader {
  i32 stage;
  i32 id_exam;
  i32 answers_size;
};

enum class MPICommand : u8 {
  SHUTDOWN = 0,
  REVIEW = 1,
};

struct MPIResult {
  i32 stage;
  i32 id_exam;
  i32 correct_answers;
  i32 wrong_answers;
  i32 unscored_answers;
  double score;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(MPIResult, stage, id_exam, correct_answers,
                                 wrong_answers, unscored_answers, score)
};

class MPICoordinator {
 public:
  static MPICoordinator& instance();
  void set_config(const CoordinatorConfig& config);
  ~MPICoordinator();
  void create_types();
  void free_types();
  void send_exam_batch(const std::vector<MPIExam>& exams, int dest_rank,
                       int tag);
  std::vector<MPIExam> receive_exam_batch(int source_rank, int tag);
  void send_answers(const std::string& answers, int dest_rank, int tag);
  std::string receive_answers(int source_rank, int tag);
  void send_results(const std::vector<MPIResult>& results, int dest_rank,
                    int tag);
  std::vector<MPIResult> receive_results(int source_rank, int tag);
  void send_to_workers(const json& exams_to_review, i32 mpi_size);
  json receive_results_from_workers(i32 mpi_size);
  std::pair<std::vector<MPIExam>, MPICommand> receive_from_master(
      i32 master_rank);
  void send_to_master(const std::vector<MPIResult>& results, i32 master_rank);
  void send_command(MPICommand command, i32 dest_rank, i32 tag);
  MPICommand receive_command(int source_rank, int tag);
  void send_shutdown_signal(i32 mpi_size);

 private:
  MPICoordinator();
  static std::unique_ptr<MPICoordinator> _instance;
  MPI_Datatype _mpi_question_type = MPI_DATATYPE_NULL;
  MPI_Datatype _mpi_result_type = MPI_DATATYPE_NULL;
  MPI_Datatype _mpi_exam_header_type = MPI_DATATYPE_NULL;
  CoordinatorConfig _config;
  bool _types_created = false;

  std::vector<std::vector<MPIExam>> _slice_exams(const json& exams,
                                                 i32 mpi_size);
};

#endif  // COORDINATOR_HPP