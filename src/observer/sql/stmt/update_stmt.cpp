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
#include "sql/stmt/update_stmt.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/parser/expression_binder.h"


UpdateStmt::UpdateStmt(Table *table, const Value *values, FilterStmt * filter_stmt,std::string attr_name,int value_amount)
    : table_(table), values_(values), filter_stmt_(filter_stmt),attr_name_(attr_name),value_amount_(value_amount)
{}

UpdateStmt::~UpdateStmt(){
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}

RC UpdateStmt::create(Db *db, const UpdateSqlNode &update, Stmt *&stmt)
{
  if (nullptr == db) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }
  stmt = nullptr;
  const char *table_name = update.relation_name.c_str();
  if (nullptr == db || nullptr == table_name || update.attribute_name.size() == 0 || update.value.length() == 0) {
    LOG_WARN("invalid argument. db=%p, table_name=%p, attribute_name =%p, value len = %d",
        db, table_name, update.attribute_name, update.value.length());
    return RC::INVALID_ARGUMENT;
  }

  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  //检查value type 和 update.attribute_name匹配
  //TODO：实际上目前不需要进行检测
  bool flag = false;
  const TableMeta &table_meta = table->table_meta();
  for(auto e :*table_meta.field_metas()){
      if(e.name() == update.attribute_name){
        flag = true;
        //TODO：特殊处理
        if(e.type() == AttrType::FLOATS && update.value.attr_type() == AttrType::INTS){
            continue;
        }
        if(e.type() != update.value.attr_type()){
          return RC::SCHEMA_FIELD_TYPE_MISMATCH;
        }
      }
  }
  if(!flag)return RC::SCHEMA_FIELD_NOT_EXIST;

  //TODO: 检查condition合法性（实际上目前不需要进行检测）

  std::unordered_map<std::string, Table *> table_map;
  table_map.insert(std::pair<std::string, Table *>(std::string(table_name), table));

  FilterStmt *filter_stmt = nullptr;
  RC          rc          = FilterStmt::create(
      db, table, &table_map, update.conditions.data(), static_cast<int>(update.conditions.size()), filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
    return rc;
  }
  stmt = new UpdateStmt(table,&update.value,filter_stmt,update.attribute_name,1);
  return RC::SUCCESS;
}
