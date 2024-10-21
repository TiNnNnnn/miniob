#include "common/type/date_type.h"
#include "common/value.h"


int DateType::compare(const Value &left, const Value &right) const  {
    //检查数据类型
    if (left.attr_type() != AttrType::DATE || right.attr_type() != AttrType::DATE) {
      return INT32_MAX;
    }
    //尝试解析date格式的数据
    Date left_date, right_date;
    if (!parse_date(left.get_string(), left_date) || !parse_date(right.get_string(), right_date)) {
      return INT32_MAX; 
    }
    //进行比较
    if (left_date.year < right_date.year) {
      return -1;
    } else if (left_date.year > right_date.year) {
      return 1;
    } else if (left_date.month < right_date.month) {
      return -1;
    } else if (left_date.month > right_date.month) {
      return 1;
    } else if (left_date.day < right_date.day) {
      return -1;
    } else if (left_date.day > right_date.day) {
      return 1;
    } else {
      return 0;
    }
}

RC DateType::cast_to(const Value &val, AttrType type, Value &result) const {
    if (type == AttrType::DATE) {
      result.set_value(val);
      return RC::SUCCESS;
    }
    return RC::UNIMPLEMENTED; // 不支持其他类型的转换
}

RC DateType::set_value_from_str(Value &val, const std::string &data) const{
    Date date_value;
    if (!parse_date(data, date_value)) {
      return RC::INVALID_DATE_TYPE; // 日期格式无效
    }
    val.set_data(data.c_str(),data.length());
    return RC::SUCCESS;
}

int DateType::cast_cost(AttrType type) {
  return type == AttrType::DATE ? 0 : INT32_MAX;
}

RC DateType::to_string(const Value &val, std::string &result) const {
    if (val.attr_type() != AttrType::DATE) {
      return RC::UNSUPPORTED;
    }
    stringstream ss;
    ss << val.value_.pointer_value_;
    result = ss.str();
    return RC::SUCCESS;
}