#include "evaluator.hpp"

#include <domain/answers.hpp>

std::unique_ptr<Evaluator> Evaluator::_instance = nullptr;

Evaluator& Evaluator::instance() {
  if (!_instance) {
    _instance.reset(new Evaluator());
  }
  return *_instance;
}

Evaluator::Evaluator() {
  _scores = AnswersScores();
}

std::vector<MPIResult> Evaluator::evaluate_exam_batch(
    const std::vector<MPIExam>& exams) {
  std::vector<MPIResult> results;
  results.resize(exams.size());
  for (size_t i = 0; i < exams.size(); i++) {
    results[i] = _evaluate_exam(exams[i]);
  }
  return results;
}

MPIResult Evaluator::_evaluate_exam(const MPIExam& exam) {
  auto student_answers = exam.answers;
  auto correct_answers = AnswersManager::instance().get_answers(exam.stage);
  if (correct_answers.empty()) {
    return MPIResult{exam.stage,
                     exam.id_exam,
                     0,
                     0,
                     static_cast<i32>(student_answers.size()),
                     0.0};
  }
  i32 correct_answers_count = 0;
  i32 wrong_answers_count = 0;
  i32 unscored_answers_count = 0;
  for (auto& answer : student_answers) {
    auto correct_answer_it = correct_answers.find(answer.qst_idx);
    if (correct_answer_it == correct_answers.end()) {
      unscored_answers_count++;
      continue;
    }
    if (correct_answer_it->second == answer.ans_idx) {
      correct_answers_count++;
    } else {
      wrong_answers_count++;
    }
  }
  double score = correct_answers_count * _scores.correct_answer +
                 wrong_answers_count * _scores.wrong_answer +
                 unscored_answers_count * _scores.unscored_answer;
  return MPIResult{
      exam.stage,          exam.id_exam,           correct_answers_count,
      wrong_answers_count, unscored_answers_count, score};
}