#pragma once
#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include <domain/coordinator.hpp>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <system/aliases.hpp>

struct AnswersScores {
  double correct_answer = +1.0;
  double wrong_answer = 0.0;
  double unscored_answer = 0.0;
};

class Evaluator {
 public:
  static Evaluator& instance();
  ~Evaluator() = default;
  std::vector<MPIResult> evaluate_exam_batch(const std::vector<MPIExam>& exams);

 private:
  Evaluator();
  static std::unique_ptr<Evaluator> _instance;
  AnswersScores _scores;

  MPIResult _evaluate_exam(const MPIExam& exam);
};

#endif  // EVALUATOR_HPP