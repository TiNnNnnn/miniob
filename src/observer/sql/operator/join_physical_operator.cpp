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
// Created by WangYunlai on 2022/12/30.
//

#include "sql/operator/join_physical_operator.h"

NestedLoopJoinPhysicalOperator::NestedLoopJoinPhysicalOperator(std::unique_ptr<Expression> expr) {
  if(expr)
    expression_ = std::move(expr);
}

RC NestedLoopJoinPhysicalOperator::open(Trx *trx)
{
  if (children_.size() != 2) {
    LOG_WARN("nlj operator should have 2 children");
    return RC::INTERNAL;
  }

  RC rc         = RC::SUCCESS;
  left_         = children_[0].get();
  right_        = children_[1].get();
  right_closed_ = true;
  round_done_   = true;

  rc   = left_->open(trx);
  trx_ = trx;
  return rc;
}

RC NestedLoopJoinPhysicalOperator::next()
{

  // //开始遍历，需要找到下一个left_tuple(即第一个left tuple)
  // bool left_need_step = (left_tuple_ == nullptr);
  // RC   rc             = RC::SUCCESS;
  // //right已经完成一次遍历，left需要迭代到下一个tuple
  // if (round_done_) {
  //   left_need_step = true;
  // } else {
  //   rc = right_next();

  //   if (rc != RC::SUCCESS) {
  //     if (rc == RC::RECORD_EOF) {
  //       left_need_step = true;
  //     } else {
  //       return rc;
  //     }
  //   } else {
  //     return rc;  // got one tuple from right
  //   }
  // }

  // if (left_need_step) {
  //   rc = left_next();
  //   if (rc != RC::SUCCESS) {
  //     return rc;
  //   }
  // }
  // rc = right_next();
  // return rc;


  while(1){
    bool left_need_step = (left_tuple_ == nullptr);
    RC rc = RC::SUCCESS;
    if(round_done_){
      left_need_step = true;
    }else{
      //右边没有到达END,获取new right_tuple
      rc = right_next();
      if (rc != RC::SUCCESS) {
        if (rc == RC::RECORD_EOF) {
          left_need_step = true;
        } else {
          return rc;
        }
      }
      //判断left_tupe_+ new right_tuple_是否符合join_conditions
      //如果left_need_step = true，说明right已经到末尾,不是new right tuple
      if(!left_need_step){
        if(!expression_)return rc;

        Value value;
        std::vector<Tuple*>tuple_list;
        tuple_list.push_back(left_tuple_);
        tuple_list.push_back(right_tuple_);

        rc = expression_->get_value(tuple_list,value);
        if (rc != RC::SUCCESS) {
            return rc;
        }

        if(value.get_boolean()){
          return rc; 
        }else{
          continue;
        } 
      }
    }

    if(left_need_step){
      rc = left_next();
      if (rc != RC::SUCCESS) {
        //left 获取失败，说明已经结束迭代，返回RC_EOF
        return rc;
      }
    }


    rc = right_next();
    if(rc == RC::RECORD_EOF){
      continue;
    }
    
    if(!expression_){
      return rc;
    }

    Value value;
    std::vector<Tuple*>tuple_list;
    tuple_list.push_back(left_tuple_);
    tuple_list.push_back(right_tuple_);

    rc = expression_->get_value(tuple_list,value);
    if (rc != RC::SUCCESS) {
          return rc;
    }
    if(value.get_boolean()){
        return rc; 
    }
  }
}

RC NestedLoopJoinPhysicalOperator::close()
{
  RC rc = left_->close();
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to close left oper. rc=%s", strrc(rc));
  }

  if (!right_closed_) {
    rc = right_->close();
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to close right oper. rc=%s", strrc(rc));
    } else {
      right_closed_ = true;
    }
  }
  return rc;
}

Tuple *NestedLoopJoinPhysicalOperator::current_tuple() { return &joined_tuple_; }

RC NestedLoopJoinPhysicalOperator::left_next()
{
  RC rc = RC::SUCCESS;
  rc    = left_->next();
  if (rc != RC::SUCCESS) {
    return rc;
  }

  left_tuple_ = left_->current_tuple();
  joined_tuple_.set_left(left_tuple_);
  return rc;
}

RC NestedLoopJoinPhysicalOperator::right_next()
{
  RC rc = RC::SUCCESS;
  if (round_done_) {
    if (!right_closed_) {
      rc = right_->close();

      right_closed_ = true;
      if (rc != RC::SUCCESS) {
        return rc;
      }
    }

    rc = right_->open(trx_);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    right_closed_ = false;
    round_done_ = false;
  }

  rc = right_->next();
  if (rc != RC::SUCCESS) {
    if (rc == RC::RECORD_EOF) {
      round_done_ = true;
    }
    return rc;
  }

  right_tuple_ = right_->current_tuple();

  //TODO: 目前expression的 get_value只能传一个tuple,针对单表没有问题，但是多表情况下存在问题，

  joined_tuple_.set_right(right_tuple_);
  return rc;
}
