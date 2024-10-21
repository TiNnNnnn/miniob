#include "sql/operator/update_logical_operator.h"


UpdateLogicalOperator::UpdateLogicalOperator(Table *table,const Value* values,std::string attr_name)
                                    :table_(table),values_(values),attr_name_(attr_name){}