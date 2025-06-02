#pragma once
#ifndef ANSWERS_HPP
#define ANSWERS_HPP

#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <system/aliases.hpp>
#include <vector>

using json = nlohmann::json;

struct Answer {
  i32 qst_idx;
  i32 rans_idx;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Answer, qst_idx, rans_idx)
};

struct ExamAnswers {
  i32 stage;
  std::vector<Answer> answers;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ExamAnswers, stage, answers)
};

class AnswersManager {
 public:
  static AnswersManager& instance();
  ~AnswersManager() = default;
  void load_from_json(const json& answers_json);
  std::string serialize_for_mpi(const std::vector<i32>& required_stages) const;
  void deserialize_from_mpi(const std::string& serialized_data);
  std::map<i32, i32> get_answers(i32 stage);
  std::string save_to_json() const;

 private:
  AnswersManager() = default;
  static std::unique_ptr<AnswersManager> _instance;
  std::map<i32, ExamAnswers> _answers;
  std::map<i32, std::map<i32, i32>> _cache_answers;
};

#endif  // ANSWERS_HPP