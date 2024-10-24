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
// Created by WangYunlai on 2021/6/10.
//

#pragma once

#include "sql/operator/physical_operator.h"
#include "sql/parser/parse.h"

/**
 * @brief 最简单的两表（称为左表、右表）join算子
 * @details 依次遍历左表的每一行，然后关联右表的每一行
 * @ingroup PhysicalOperator
 */
class NestedLoopJoinPhysicalOperator : public PhysicalOperator
{
public:
  NestedLoopJoinPhysicalOperator(std::unique_ptr<Expression> expr = nullptr);
  virtual ~NestedLoopJoinPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::NESTED_LOOP_JOIN; }

  RC     open(Trx *trx) override;
  RC     next() override;
  RC     close() override;
  Tuple *current_tuple() override;

  void set_predicates(std::unique_ptr<Expression> &&exprs);

private:
  RC left_next();   //! 左表遍历下一条数据
  RC right_next();  //! 右表遍历下一条数据，如果上一轮结束了就重新开始新的一轮

  RC filter(Tuple &tuple, bool &result);

private:
  Trx *trx_ = nullptr;

  //! 左表右表的真实对象是在PhysicalOperator::children_中，这里是为了写的时候更简单
  PhysicalOperator *left_        = nullptr;
  PhysicalOperator *right_       = nullptr;
  Tuple            *left_tuple_  = nullptr;
  Tuple            *right_tuple_ = nullptr;
  JoinedTuple       joined_tuple_;         //! 当前关联的左右两个tuple
  bool              round_done_   = true;  //! 右表遍历的一轮是否结束
  bool              right_closed_ = true;  //! 右表算子是否已经关闭

  std::unique_ptr<Expression> expression_; //join conditions (in conjunction oper,size = 1)
};

class HashJoinPhysicalOperator : public PhysicalOperator
{
public:
  HashJoinPhysicalOperator();
  virtual ~HashJoinPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::HASH_JOIN; }

  RC     open(Trx *trx) override;
  RC     next() override;
  RC     close() override;
  Tuple *current_tuple() override;

private:
  RC build_hash_table();   //! 建立哈希表，通常在处理左表时进行
  RC probe_hash_table();   //! 查找右表的当前元组是否在哈希表中有匹配的记录

private:
  Trx *trx_ = nullptr;
  
  //! 左表右表的真实对象是在PhysicalOperator::children_中，这里是为了写的时候更简单
  PhysicalOperator *left_        = nullptr;
  PhysicalOperator *right_       = nullptr;
  Tuple            *left_tuple_  = nullptr;
  Tuple            *right_tuple_ = nullptr;
  JoinedTuple       joined_tuple_;         //! 当前关联的左右两个tuple

  //! Hash table to store the left tuples
  //std::unordered_map<HashKeyType, std::vector<Tuple>> hash_table_;  //! 用于存储左表的哈希表，键为哈希值，值为对应的元组列表
  //HashKeyType       hash_key_;                                      //! 当前的哈希键，用于存储和查找

  //! Iterator for the current vector of matched tuples in the hash table
  std::vector<Tuple>::iterator hash_matched_iter_;   //! 当前匹配的元组列表迭代器，指向哈希表中左表的匹配记录

  bool              hash_built_ = false;  //! 用于标识哈希表是否已经构建完成
  bool              probing_    = false;  //! 用于标识当前是否处于探测阶段
};


