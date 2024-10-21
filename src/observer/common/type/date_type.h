#pragma once
#include "common/rc.h"
#include "common/type/data_type.h"

#include <sstream>
#include <iomanip>
#include <limits>  // 用于 INT32_MAX

class DateType : public DataType {
public:
  DateType() : DataType(AttrType::DATE) {}
  virtual ~DateType() = default;

  /**
   * @return
   *  -1 表示 left < right
   *  0 表示 left = right
   *  1 表示 left > right
   *  INT32_MAX 表示未实现的比较
   */
  int compare(const Value &left, const Value &right) const override;
  /**
   * @brief 将 val 转换为 type 类型，并将结果保存到 result 中
   */
  RC cast_to(const Value &val, AttrType type, Value &result) const override;
  /**
   * @brief 从字符串设置日期类型的值，检查合法性（例如闰年等）
   */
  RC set_value_from_str(Value &val, const std::string &data) const override;
  /**
   * @brief 计算从 type 到 attr_type 的隐式转换的 cost，如果无法转换，返回 INT32_MAX
   */
  int cast_cost(AttrType type) override;
  /**
   * @brief 将 val 转换为字符串，并将结果保存到 result 中
   */
  RC to_string(const Value &val, std::string &result) const override;
private:
  /**
   * @brief 日期结构体，用于保存解析后的年、月、日
   */
  struct Date {
    int year;
    int month;
    int day;
  };

  /**
   * @brief 解析日期字符串，返回 Date 结构体
   * @param date_str 输入的日期字符串，格式为"YYYY-MM-DD"
   * @param date 输出的 Date 结构体
   * @return 成功返回 true，失败返回 false
   */
  bool parse_date(const std::string &date_str, Date &date) const {
    if(date_str.length() != 10){
      return false;
    }
    std::istringstream ss(date_str);
    char delimiter1, delimiter2;
    ss >> date.year >> delimiter1 >> date.month >> delimiter2 >> date.day;

    if (ss.fail() || delimiter1 != '-' || delimiter2 != '-' || !is_valid_date(date.year, date.month, date.day)) {
      return false; // 日期格式无效或日期非法
    }
    return true;
  }

  /**
   * @brief 检查日期是否合法
   * @param year 年份
   * @param month 月份
   * @param day 天数
   * @return true 表示合法，false 表示非法
   */
  bool is_valid_date(int year, int month, int day) const {
    if (month < 1 || month > 12) {
      return false;
    }
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int max_day = days_in_month[month - 1];
    if (month == 2 && is_leap_year(year)) {
      max_day = 29;
    }

    return day >= 1 && day <= max_day;
  }

  /**
   * @brief 判断是否为闰年
   * @param year 年份
   * @return true 表示是闰年，false 表示不是闰年
   */
  bool is_leap_year(int year) const {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
  }
};
