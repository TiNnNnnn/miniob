#include "common/type/vector_type.h"
#include "common/value.h"

int VectorType::compare(const Value &left, const Value &right) const {
    //检查数据类型
    if (left.attr_type() != AttrType::VECTOR || right.attr_type() != AttrType::VECTOR) {
        return INT32_MAX;
    }
    //尝试解析vector格式的数据
    VectorType left_vec, right_vec;
    if (!parse_vector(left.get_string(), left_vec.x_,left_vec.dim_) || !parse_vector(right.get_string(), right_vec.x_,right_vec.dim_)){
        return INT32_MAX; 
    }

    // 确保两个向量维度相同
    if (left_vec.dim_ != right_vec.dim_) {
        return INT32_MAX;
    }

    // 比较每个维度的值
    for (int i = 0; i < left_vec.dim_; ++i) {
        if (left_vec.x_[i] < right_vec.x_[i]) {
            return -1;
        } else if (left_vec.x_[i] > right_vec.x_[i]) {
            return 1;
        }
    }

    return 0;  // 完全相等
    
}

RC VectorType::add(const Value &left, const Value &right, Value &result) const {
    if (left.attr_type() != AttrType::VECTOR || right.attr_type() != AttrType::VECTOR) {
        return RC::INVALID_ARGUMENT;
    }
    VectorType left_vec, right_vec;
    if (!parse_vector(left.get_string(), left_vec.x_, left_vec.dim_) || !parse_vector(right.get_string(), right_vec.x_, right_vec.dim_)) {
        return RC::INVALID_ARGUMENT;
    }
    if (left_vec.dim_ != right_vec.dim_) {
        return RC::INVALID_ARGUMENT;
    }
    float * result_vec = new float[left_vec.dim_];
    for (int i = 0; i < left_vec.dim_; ++i) {
        result_vec[i] = left_vec.x_[i] + right_vec.x_[i];
    }
    string vec_str = vector_to_str_format(result_vec,left_vec.dim_);
    result.set_data(vec_str.c_str(),vec_str.length());
    return RC::SUCCESS;
}

RC VectorType::subtract(const Value &left, const Value &right, Value &result) const {
        if (left.attr_type() != AttrType::VECTOR || right.attr_type() != AttrType::VECTOR) {
        return RC::INVALID_ARGUMENT;
    }
    VectorType left_vec, right_vec;
    if (!parse_vector(left.get_string(), left_vec.x_, left_vec.dim_) || !parse_vector(right.get_string(), right_vec.x_, right_vec.dim_)) {
        return RC::INVALID_ARGUMENT;
    }
    if (left_vec.dim_ != right_vec.dim_) {
        return RC::INVALID_ARGUMENT;
    }
    float * result_vec = new float[left_vec.dim_];
    for (int i = 0; i < left_vec.dim_; ++i) {
        result_vec[i] = left_vec.x_[i] - right_vec.x_[i];
    }
    string vec_str = vector_to_str_format(result_vec,left_vec.dim_);
    result.set_data(vec_str.c_str(),vec_str.length());
    return RC::SUCCESS;
}

RC VectorType::multiply(const Value &left, const Value &right, Value &result) const {
        if (left.attr_type() != AttrType::VECTOR || right.attr_type() != AttrType::VECTOR) {
        return RC::INVALID_ARGUMENT;
    }
    VectorType left_vec, right_vec;
    if (!parse_vector(left.get_string(), left_vec.x_, left_vec.dim_) || !parse_vector(right.get_string(), right_vec.x_, right_vec.dim_)) {
        return RC::INVALID_ARGUMENT;
    }
    if (left_vec.dim_ != right_vec.dim_) {
        return RC::INVALID_ARGUMENT;
    }
    float * result_vec = new float[left_vec.dim_];
    for (int i = 0; i < left_vec.dim_; ++i) {
        result_vec[i] = left_vec.x_[i] * right_vec.x_[i];
    }
    string vec_str = vector_to_str_format(result_vec,left_vec.dim_);
    result.set_data(vec_str.c_str(),vec_str.length());
    return RC::SUCCESS;
}

RC VectorType::negative(const Value &val, Value &result) const {
  if (val.attr_type() != AttrType::VECTOR) {
    return RC::INVALID_ARGUMENT;
  }
  VectorType vec;
  if (!parse_vector(val.get_string(), vec.x_, vec.dim_)) {
    return RC::INVALID_ARGUMENT;
  }
  float * result_vec = new float[vec.dim_];
  for (int i = 0; i < vec.dim_; ++i) {
    result_vec[i] = -vec.x_[i];
  }
  string vec_str = vector_to_str_format(result_vec,vec.dim_);
  result.set_data(vec_str.c_str(),vec_str.length());
  return RC::SUCCESS;
}

RC VectorType::l2_distance(const Value &left, const Value&right,Value &result) const{

  return RC::SUCCESS;
}

RC VectorType::cosine_distance(const Value &left,const Value& right,Value &result)const{

  return RC::SUCCESS;
}

RC VectorType::inner_distance(const Value &left,const Value&right,Value &result) const{
  
  return RC::SUCCESS;
}

RC VectorType::set_value_from_str(Value &val, const string &data) const {
    VectorType vec;
    float *x;
    int dim;
    if(!vec.parse_vector(data,x,dim)){
        return RC::INVALID_VEC_TYPE;
    }
    val.set_data(data.c_str(),data.length()); //实际存储时使用char*
    return RC::SUCCESS;
}

RC VectorType::to_string(const Value &val, string &result) const {
    if (val.attr_type() != AttrType::VECTOR) {
      return RC::UNSUPPORTED;
    }
    stringstream ss;
    ss << val.data();
    result = ss.str();
    return RC::SUCCESS;
}

RC VectorType::cast_to(const Value &val, AttrType type, Value &result) const {
  if (type == AttrType::VECTOR) {
      result.set_value(val);
      return RC::SUCCESS;
  }
  return RC::UNIMPLEMENTED; // 不支持其他类型的转换
}

int VectorType::cast_cost(AttrType type) {
  return type == AttrType::VECTOR ? 0 : INT32_MAX;
}

bool VectorType::parse_vector(const std::string &vec_str, float * &x, int& dim) const {
    
    // 检查字符串是否符合基本格式 '[1,1,1]'
    if (vec_str.size() < 2 || vec_str.front() != '[' || vec_str.back() != ']') {
      return false;  // 格式不对
    }
    // 去掉首尾的括号部分
    std::string inner_str = vec_str.substr(1, vec_str.size() - 2);

    // 用字符串流来逐一解析每个数字
    std::stringstream ss(inner_str);
    std::string item;
    std::vector<float> values;

    // 按照逗号分割并解析浮点数
    while (std::getline(ss, item, ',')) {
      // 去除空白字符
      item.erase(0, item.find_first_not_of(' '));
      item.erase(item.find_last_not_of(' ') + 1);
      
      // 检查是否是合法的数字
      if (item.empty() || (!isdigit(item[0]) && item[0] != '-' && item[0] != '.')) {
        return false;  // 非法字符
      }

      // 转换为 float 并存储
      try {
        values.push_back(std::stof(item));
      } catch (const std::invalid_argument &e) {
        return false;  // 转换失败，可能输入了非法字符
      }
    }

    // 确定维度
    dim = values.size();
    
    // 分配数组内存
    x = new float[dim];

    // 复制数据到输出数组
    for (int i = 0; i < dim; ++i) {
      x[i] = values[i];
    }

    return true;  // 成功解析
  }

std::string VectorType::vector_to_str_format(const float* x, int dim) const {
    // 检查输入
    if (x == nullptr || dim <= 0) {
        return "[]";  // 空向量或非法输入时返回空的方括号
    }

    // 使用字符串流来构建字符串
    std::stringstream ss;
    ss << "[";

    // 遍历数组并将浮点数转换为字符串
    for (int i = 0; i < dim; ++i) {
        ss << x[i];  // 将每个浮点数加入字符串流
        if (i != dim - 1) {
            ss << ",";  // 非最后一个元素时添加逗号分隔符
        }
    }

    ss << "]";  // 结束的方括号

    // 返回生成的字符串
    return ss.str();
}
