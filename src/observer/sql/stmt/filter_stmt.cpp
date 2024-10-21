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

#include "sql/stmt/filter_stmt.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "common/rc.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/parser/expression_binder.h"

FilterStmt::~FilterStmt()
{
  for (FilterUnit *unit : filter_units_) {
    delete unit;
  }
  filter_units_.clear();
}

RC FilterStmt::create(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
    const ConditionSqlNode *conditions, int condition_num, FilterStmt *&stmt)
{
  RC rc = RC::SUCCESS;
  stmt  = nullptr;

  FilterStmt *tmp_stmt = new FilterStmt();
  for (int i = 0; i < condition_num; i++) {
    FilterUnit *filter_unit = nullptr;

    rc = create_filter_unit(db, default_table, tables, conditions[i], filter_unit);
    if (rc != RC::SUCCESS) {
      delete tmp_stmt;
      LOG_WARN("failed to create filter unit. condition index=%d", i);
      return rc;
    }
    tmp_stmt->filter_units_.push_back(filter_unit);
  }

  stmt = tmp_stmt;
  return rc;
}

RC get_table_and_field(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
    const RelAttrSqlNode &attr, Table *&table, const FieldMeta *&field)
{
  if (common::is_blank(attr.relation_name.c_str())) {
    table = default_table;
  } else if (nullptr != tables) {
    auto iter = tables->find(attr.relation_name);
    if (iter != tables->end()) {
      table = iter->second;
    }
  } else {
    table = db->find_table(attr.relation_name.c_str());
  }
  if (nullptr == table) {
    LOG_WARN("No such table: attr.relation_name: %s", attr.relation_name.c_str());
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  field = table->table_meta().field(attr.attribute_name.c_str());
  if (nullptr == field) {
    LOG_WARN("no such field in table: table %s, field %s", table->name(), attr.attribute_name.c_str());
    table = nullptr;
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }
  return RC::SUCCESS;
}

RC FilterStmt::create_filter_unit(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
    const ConditionSqlNode& condition, FilterUnit *&filter_unit)
{
  RC rc = RC::SUCCESS;

  CompOp comp = condition.comp;
  if (comp < EQUAL_TO || comp >= NO_OP) {
    LOG_WARN("invalid compare operator : %d", comp);
    return RC::INVALID_ARGUMENT;
  }


  BinderContext binder_context;
  for(auto& e : *tables) binder_context.add_table(e.second);
  ExpressionBinder expression_binder(binder_context);

  filter_unit = new FilterUnit;
  
  if (condition.left_is_attr) { //左边是列属性
    Table           *table = nullptr;
    const FieldMeta *field = nullptr;
    rc                     = get_table_and_field(db, default_table, tables, condition.left_attr, table, field);
    if (rc != RC::SUCCESS) {
      LOG_WARN("cannot find attr");
      return rc;
    }
    FilterObj filter_obj;
    filter_obj.init_attr(Field(table, field));
    filter_unit->set_left(filter_obj);
  } else {
    if(condition.left_is_expr){ //左边是计算表达式属性
      std::unique_ptr<Expression>left(condition.left_expr);
      std::vector<std::unique_ptr<Expression>>bind_lefts;
      vector<unique_ptr<Expression>> filter_expressions;
      RC rc = expression_binder.bind_expression(left, bind_lefts);
      if(bind_lefts.size() != 1){
        LOG_WARN("cond bind expr must be one");
        return RC::INTERNAL;
      }
      if (OB_FAIL(rc)) {
          LOG_INFO("bind expression failed. rc=%s", strrc(rc));
          return rc;
      }
      left.swap(bind_lefts[0]);
      FilterObj filter_obj;
      filter_obj.init_calc_expr(std::move(left));
      filter_unit->set_left(filter_obj);
    }else{
        FilterObj filter_obj;
        filter_obj.init_value(condition.left_value);
        filter_unit->set_left(filter_obj);
    }
  }

  if (condition.right_is_attr) { //右边是列属性
    Table           *table = nullptr;
    const FieldMeta *field = nullptr;
    rc                     = get_table_and_field(db, default_table, tables, condition.right_attr, table, field);
    if (rc != RC::SUCCESS) {
      LOG_WARN("cannot find attr");
      return rc;
    }
    FilterObj filter_obj;
    filter_obj.init_attr(Field(table, field));
    filter_unit->set_right(filter_obj);
  } else {
    if(condition.right_is_expr){

      std::unique_ptr<Expression>right(condition.right_expr);
      std::vector<std::unique_ptr<Expression>>bind_rights;
      vector<unique_ptr<Expression>> filter_expressions;
      RC rc = expression_binder.bind_expression(right, bind_rights);
      if(bind_rights.size() != 1){
        LOG_WARN("cond bind expr must be one");
        return RC::INTERNAL;
      }
      if (OB_FAIL(rc)) {
          LOG_INFO("bind expression failed. rc=%s", strrc(rc));
          return rc;
      }
      right.swap(bind_rights[0]);
      FilterObj filter_obj;
      filter_obj.init_calc_expr(std::move(right));
      filter_unit->set_right(filter_obj);
    }else{
      FilterObj filter_obj;
      filter_obj.init_like_expr(condition.right_value);
      filter_unit->set_right(filter_obj);
    }
  }
  filter_unit->set_comp(comp);
  // 检查两个类型是否能够比较
  return rc;
}


// RC FilterStmt::create_expr(Expression* e)
// {
//   if(e->type() == ExprType::ARITHMETIC){
//       auto expr = static_cast<ArithmeticExpr*>(e);
//       auto left_expr = expr->left().get();
//       auto right_expr = expr->right().get();
//       create_expr(left_expr);
//       create_expr(right_expr);
//   }else if(e->type() == ExprType::UNBOUND_AGGREGATION){

//   }else if(e->type() == ExprType::UNBOUND_FIELD){
//       auto expr = static_cast<UnboundFieldExpr*>(e);
//       //e = static_cast<Expression*>(new FieldExpr(expr->table_name(),expr->f))


//   }
// }
