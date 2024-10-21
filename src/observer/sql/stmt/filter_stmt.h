/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#pragma once

#include "sql/expr/expression.h"
#include "sql/parser/parse_defs.h"
#include "sql/stmt/stmt.h"
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
class Db;
class Table;
class FieldMeta;
class Value;
class Expression;

struct FilterObj
{
  bool  is_attr;
  bool is_like;
  bool is_expr;
  Field field;
  Value value;
  std::unique_ptr<Expression> expr;

  void init_attr(const Field &field)
  {
    is_attr     = true;
    is_like     = false;
    is_expr = false;
    this->field = field;
  }

  void init_value(const Value &value)
  {
    is_attr     = false;
    is_like     = false;
    is_expr = false;
    this->value = value;
  }

  void init_like_expr(const Value &value)
  {   
      //auto expr = std::to_string(value.data());
      auto expr = value.to_string();
      if (expr.empty())
      {
          throw std::invalid_argument("LIKE expression cannot be empty.");
      }
      expr.erase(std::remove_if(expr.begin(), expr.end(), ::isspace), expr.end());
      this->value = value;
      is_attr = false;
      is_like = true;
      is_expr = false;
  }

  void init_calc_expr(std::unique_ptr<Expression> expression)
  {
      expr = std::move(expression);
      is_attr = false;
      is_like = false;
      is_expr = true;
  }

};

class FilterUnit
{
public:
  FilterUnit() = default;
  ~FilterUnit() {}

  void set_comp(CompOp comp) { comp_ = comp; }

  CompOp comp() const { return comp_; }

  void set_left(FilterObj &obj) { 
    left_.expr = std::move(obj.expr);
    left_.is_attr = obj.is_attr;
    left_.is_like = obj.is_like;
    left_.is_expr = obj.is_expr;
    left_.field = obj.field;
    left_.value = obj.value;
  }
  void set_right(FilterObj &obj) { 
    right_.expr = std::move(obj.expr);
    right_.is_attr = obj.is_attr;
    right_.is_like = obj.is_like;
    right_.is_expr = obj.is_expr;
    right_.field = obj.field;
    right_.value = obj.value;
  }
  FilterObj &left()  { return left_; }
  FilterObj &right() { return right_; }

private:
  CompOp    comp_ = NO_OP;
  FilterObj left_;
  FilterObj right_;
};

/**
 * @brief Filter/谓词/过滤语句
 * @ingroup Statement
 */
class FilterStmt
{
public:
  FilterStmt() = default;
  virtual ~FilterStmt();

public:
  std::vector<FilterUnit *> &filter_units() { return filter_units_; }

public:
  static RC create(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
      const ConditionSqlNode *conditions, int condition_num, FilterStmt *&stmt);

  static RC create_filter_unit(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
      const ConditionSqlNode &condition, FilterUnit *&filter_unit);
  
  static RC create_expr(Expression* expr);

private:
  std::vector<FilterUnit *> filter_units_;  // 默认当前都是AND关系
};
