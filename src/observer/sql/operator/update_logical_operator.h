#pragma once

#include "sql/operator/logical_operator.h"

/**
 * @brief 逻辑算子，用于执行update语句
 * @ingroup LogicalOperator
 */
class UpdateLogicalOperator : public LogicalOperator
{
public:
  UpdateLogicalOperator(Table *table,const Value* values,std::string attr_name);
  virtual ~UpdateLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::UPDATE; }
  Table              *table() const { return table_; }
  const Value              *value() const {return values_;}
  std::string        attr_name() const {return attr_name_;}

private:
  Table *table_ = nullptr;
  const Value *values_ = nullptr;
  std::string attr_name_{};
};