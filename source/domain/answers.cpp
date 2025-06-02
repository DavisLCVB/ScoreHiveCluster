#include "answers.hpp"
#include <spdlog/spdlog.h>
#include <mutex>

std::unique_ptr<AnswersManager> AnswersManager::_instance = nullptr;

AnswersManager& AnswersManager::instance() {
  static std::once_flag flag;
  std::call_once(flag, []() { _instance.reset(new AnswersManager()); });
  return *_instance;
}

void AnswersManager::load_from_json(const json& answers_json) {
  for (const auto& exam_answers : answers_json) {
    auto ans = exam_answers.get<ExamAnswers>();
    _answers[ans.stage] = ans;
  }
}

std::string AnswersManager::serialize_for_mpi(
    const std::vector<i32>& required_stages) const {
  json serialized = json::array();
  for (const auto& stage : required_stages) {
    auto it = _answers.find(stage);
    if (it != _answers.end()) {
      serialized.push_back(it->second);
    }
  }
  return serialized.dump();
}

void AnswersManager::deserialize_from_mpi(const std::string& serialized_data) {
  json answers_json = json::parse(serialized_data);
  load_from_json(answers_json);
}

std::map<i32, i32> AnswersManager::get_answers(i32 stage) {
  auto it = _cache_answers.find(stage);
  if (it != _cache_answers.end()) {
    return it->second;
  }
  // cache miss
  auto it_answers = _answers.find(stage);
  if (it_answers == _answers.end()) {
    return std::map<i32, i32>();
  }
  auto vec_answers = it_answers->second.answers;
  std::map<i32, i32> correct_answers;
  for (const auto& answer : vec_answers) {
    correct_answers[answer.qst_idx] = answer.rans_idx;
  }
  _cache_answers[stage] = correct_answers;
  return correct_answers;
}

std::string AnswersManager::save_to_json() const {
  json answers_json = json::array();
  for (const auto& [stage, answers] : _answers) {
    answers_json.push_back(answers);
  }
  return answers_json.dump();
}