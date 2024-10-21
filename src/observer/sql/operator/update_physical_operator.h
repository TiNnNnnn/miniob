#pragma once

#include "sql/operator/physical_operator.h"

class Trx;
class UpdateStmt;

/**
 * @brief 物理算子，更新
 * @ingroup PhysicalOperator
 */
class UpdatePhysicalOperator : public PhysicalOperator
{
public:
  UpdatePhysicalOperator(Table *table,const Value* values,std::string attr_name) 
              : table_(table), values_(values),attr_name_(attr_name) {}

  virtual ~UpdatePhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::UPDATE; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override { return nullptr; }

private:
  Table              *table_ = nullptr;
  Trx                *trx_   = nullptr;
  const Value* values_ =nullptr;
  std::string attr_name_;
  std::vector<Record> records_;
};